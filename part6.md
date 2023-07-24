# 游标

本文内容较短，我们只是为了更容易的实现b树，简单地重构一下。

我们将添加一个Cursor 表示表中对象的位置。Cursor应提供如下几个方面的能力：

- 在表的开头创建游标
- 在表的末尾创建游标
- 访问=游标指向的行
- 将游标前进到下一行

这是本文我们要实现的能力，后面我们还希望游标提供如下能力：

- 删除游标指向的行
- 修改游标指向的行
- 在表中搜索给定的 ID，并创建一个指向具有该 ID 的行的游标

Cursor 类型定义如下：

```c
+typedef struct {
+  Table* table;
+  uint32_t row_num;
+  bool end_of_table;  // Indicates a position one past the last element
+} Cursor;
```

鉴于我们当前的表数据结构，只需在表中标识位置即可。

游标还具有对它所属的表的引用（因此我们的游标函数可以只将游标作为参数）。

最后，它有一个名为 end_of_table 的布尔值。这样我们就可以表示表格末尾的位置（这是我们可能想要插入一行的地方）。

table_start() 和 table_end() 创建新游标：

```c
+Cursor* table_start(Table* table) {
+  Cursor* cursor = malloc(sizeof(Cursor));
+  cursor->table = table;
+  cursor->row_num = 0;
+  cursor->end_of_table = (table->num_rows == 0);
+
+  return cursor;
+}
+
+Cursor* table_end(Table* table) {
+  Cursor* cursor = malloc(sizeof(Cursor));
+  cursor->table = table;
+  cursor->row_num = table->num_rows;
+  cursor->end_of_table = true;
+
+  return cursor;
+}
```

我们的 row_slot() 函数将变为 cursor_value() ，它返回指向光标描述的位置的指针：

```c
-void* row_slot(Table* table, uint32_t row_num) {
+void* cursor_value(Cursor* cursor) {
+  uint32_t row_num = cursor->row_num;
   uint32_t page_num = row_num / ROWS_PER_PAGE;
-  void* page = get_page(table->pager, page_num);
+  void* page = get_page(cursor->table->pager, page_num);
   uint32_t row_offset = row_num % ROWS_PER_PAGE;
   uint32_t byte_offset = row_offset * ROW_SIZE;
   return page + byte_offset;
 }
```

在我们当前的表结构中前进光标就像增加行号一样简单。这在B树中会稍微复杂一些。

```c
+void cursor_advance(Cursor* cursor) {
+  cursor->row_num += 1;
+  if (cursor->row_num >= cursor->table->num_rows) {
+    cursor->end_of_table = true;
+  }
+}
```

插入行时，我们在表的末尾打开一个游标，写入该游标位置，然后关闭游标。

```c
 Row* row_to_insert = &(statement->row_to_insert);
+  Cursor* cursor = table_end(table);

-  serialize_row(row_to_insert, row_slot(table, table->num_rows));
+  serialize_row(row_to_insert, cursor_value(cursor));
   table->num_rows += 1;

+  free(cursor);
+
   return EXECUTE_SUCCESS;
 }
```

选择表格中的所有行时，我们在表格的开头打开一个光标，打印该行，然后将光标前进到下一行。重复直到我们到达表格的末尾。

```c
ExecuteResult execute_select(Statement* statement, Table* table) {
+  Cursor* cursor = table_start(table);
+
   Row row;
-  for (uint32_t i = 0; i < table->num_rows; i++) {
-    deserialize_row(row_slot(table, i), &row);
+  while (!(cursor->end_of_table)) {
+    deserialize_row(cursor_value(cursor), &row);
     print_row(&row);
+    cursor_advance(cursor);
   }
+
+  free(cursor);
+
   return EXECUTE_SUCCESS;
 }
```

好了，就是这样！就像我说的，这是一个较短的重构，应该可以帮助我们将表数据结构重写为 B 树。 execute_select() 并且可以 execute_insert() 完全通过游标与表交互，而无需假设表的存储方式。