# 仅支持追加的单表内存数据库

我们将从小处着手，对数据库施加很多限制。目前，它有如下限制：

- 支持两种操作：插入一行和打印所有行

- 仅驻留在内存中（不需要持久化到磁盘）

- 支持单个硬编码表

我们的硬编码用户表结构如下所示：

|列名|类型|
|----|----|
|id	     | integer      |
|username| 	varchar(32) |
|email   |	varchar(255)|

这是一个简单的架构，但它要求我们能够支持多种数据类型和多种大小的文本数据类型。

insert 语句现在需按照如下格式编写：

```sql
insert 1 cstack foo@bar.com
```

这意味着我们需要升级我们的 prepare_statement 函数来解析参数.

```c
if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
     statement->type = STATEMENT_INSERT;
+    int args_assigned = sscanf(
+        input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id),
+        statement->row_to_insert.username, statement->row_to_insert.email);
+    if (args_assigned < 3) {
+      return PREPARE_SYNTAX_ERROR;
+    }
     return PREPARE_SUCCESS;
   }
```

我们将这些解析的参数存储到语句对象内的新 Row 数据结构中：

```c
+#define COLUMN_USERNAME_SIZE 32
+#define COLUMN_EMAIL_SIZE 255
+typedef struct {
+  uint32_t id;
+  char username[COLUMN_USERNAME_SIZE];
+  char email[COLUMN_EMAIL_SIZE];
+} Row;
+
 typedef struct {
   StatementType type;
+  Row row_to_insert;  // only used by insert statement
 } Statement;
```

现在我们需要将该数据复制到表示表的某个数据结构中。SQLite使用B树进行快速查找，插入和删除。我们将从更简单的东西开始。像B树一样，它会将行分组到页面中，但不是将这些页面排列为树，而是将它们排列为一个数组。

以下是实现细节：

- 将行存储在称为页的内存块中

- 每个页面存储尽可能多的行

- 行被序列化为每页的紧凑表示形式

- 按需分配页面

- 保留指向页面的固定大小的指针数组

我们先定义行的序列化表示（我们将行序列化到内存的某个地址里）：

```c
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

#define ID_SIZE  size_of_attribute(Row, id)
#define USERNAME_SIZE  size_of_attribute(Row, username)
#define EMAIL_SIZE  size_of_attribute(Row, email)
#define ID_OFFSET (uint32_t)0
#define USERNAME_OFFSET  (ID_OFFSET + ID_SIZE)
#define EMAIL_OFFSET  (USERNAME_OFFSET + USERNAME_SIZE)
#define ROW_SIZE  (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)

#define PAGE_SIZE  4096
#define TABLE_MAX_PAGES 100
#define ROWS_PER_PAGE  (PAGE_SIZE / ROW_SIZE)
#define TABLE_MAX_ROWS  (ROWS_PER_PAGE * TABLE_MAX_PAGES)
```

序列化后的行结构将如下所示：

|列名|类型|offset|
|----|----|----- |
|id	     | integer      |0|
|username| 	varchar(32) |4|
|email   |	varchar(255)|36|
|total   |  291         |  |

我们还需要代码来进行序列化和反序列化转换。

```c
+void serialize_row(Row* source, void* destination) {
+  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
+  memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
+  memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
+}
+
+void deserialize_row(void* source, Row* destination) {
+  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
+  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
+  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
+}
```

接下来，一个 Table 指向行页并跟踪行数的结构：

```c
+const uint32_t PAGE_SIZE = 4096;
+#define TABLE_MAX_PAGES 100
+const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
+const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
+
+typedef struct {
+  uint32_t num_rows;
+  void* pages[TABLE_MAX_PAGES];
+} Table;
```

我将页面大小设为 4 KB，因为它与大多数计算机体系结构的虚拟内存系统中使用的页面大小相同。这意味着我们数据库中的一页对应于操作系统使用的一个页面。操作系统会将页面作为整个单元移入和移出内存，而不是分解它们。

我添加了一个随意的限制，即我们最多分配 100 页。当我们切换到树结构时，数据库的最大大小将仅受文件最大大小的限制。（尽管我们仍然会限制一次在内存中保留的页面数）。

由于页面在内存中可能不会彼此相邻存在，为了使读取/写入行变得更加容易，我们假设行不应跨越页面边界。

以下是我们如何确定特定行在内存中读取/写入的位置：

```c
+void* row_slot(Table* table, uint32_t row_num) {
+  uint32_t page_num = row_num / ROWS_PER_PAGE;
+  void* page = table->pages[page_num];
+  if (page == NULL) {
+    // Allocate memory only when we try to access page
+    page = table->pages[page_num] = malloc(PAGE_SIZE);
+  }
+  uint32_t row_offset = row_num % ROWS_PER_PAGE;
+  uint32_t byte_offset = row_offset * ROW_SIZE;
+  return page + byte_offset;
+}
```

现在我们可以根据表结构使用 execute_statement进行读/写操作：

```c
-void execute_statement(Statement* statement) {
+ExecuteResult execute_insert(Statement* statement, Table* table) {
+  if (table->num_rows >= TABLE_MAX_ROWS) {
+    return EXECUTE_TABLE_FULL;
+  }
+
+  Row* row_to_insert = &(statement->row_to_insert);
+
+  serialize_row(row_to_insert, row_slot(table, table->num_rows));
+  table->num_rows += 1;
+
+  return EXECUTE_SUCCESS;
+}
+
+ExecuteResult execute_select(Statement* statement, Table* table) {
+  Row row;
+  for (uint32_t i = 0; i < table->num_rows; i++) {
+    deserialize_row(row_slot(table, i), &row);
+    print_row(&row);
+  }
+  return EXECUTE_SUCCESS;
+}
+
+ExecuteResult execute_statement(Statement* statement, Table* table) {
   switch (statement->type) {
     case (STATEMENT_INSERT):
-      printf("This is where we would do an insert.\n");
-      break;
+      return execute_insert(statement, table);
     case (STATEMENT_SELECT):
-      printf("This is where we would do a select.\n");
-      break;
+      return execute_select(statement, table);
   }
 }
```

最后，我们需要初始化表，创建相应的内存释放函数并处理更多错误情况：

```c
+ Table* new_table() {
+  Table* table = (Table*)malloc(sizeof(Table));
+  table->num_rows = 0;
+  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
+     table->pages[i] = NULL;
+  }
+  return table;
+}
+
+void free_table(Table* table) {
+    for (int i = 0; table->pages[i]; i++) {
+	free(table->pages[i]);
+    }
+    free(table);
+}
```
```c
 int main(int argc, char* argv[]) {
+  Table* table = new_table();
   InputBuffer* input_buffer = new_input_buffer();
   while (true) {
     print_prompt();
@@ -105,13 +203,22 @@ int main(int argc, char* argv[]) {
     switch (prepare_statement(input_buffer, &statement)) {
       case (PREPARE_SUCCESS):
         break;
+      case (PREPARE_SYNTAX_ERROR):
+        printf("Syntax error. Could not parse statement.\n");
+        continue;
       case (PREPARE_UNRECOGNIZED_STATEMENT):
         printf("Unrecognized keyword at start of '%s'.\n",
                input_buffer->buffer);
         continue;
     }

-    execute_statement(&statement);
-    printf("Executed.\n");
+    switch (execute_statement(&statement, table)) {
+      case (EXECUTE_SUCCESS):
+        printf("Executed.\n");
+        break;
+      case (EXECUTE_TABLE_FULL):
+        printf("Error: Table full.\n");
+        break;
+    }
   }
 }
```

通过这些更改，我们实际上可以将数据保存在数据库中！

```shell
PS D:\code\db021\code> make  
gcc -g -O0 main.c -o db
PS D:\code\db021\code> .\db.exe
db > insert 1 cstack foo@bar.com
Executed.
db > insert 2 bob bob@example.com
Executed.
db > select
(1, cstack, foo@bar.com)
(2, bob, bob@example.com)
Executed.
db > insert foo bar 1
Syntax error. Could not parse statement.
db > .exit
PS D:\code\db021\code>
```

现在我们可以基于当前代码编写一些测试用例，原因如下：

- 我们计划大幅改变存储表的数据结构，测试将捕获回归。

- 有几个边缘情况我们没有手动测试（例如填满表格）

我们将在下一部分中解决这些问题。