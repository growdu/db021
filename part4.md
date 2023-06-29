# 完善特殊场景

当前我们使用sscanf来读取输入，sscanf有一些缺点。如果它正在读取的字符串大于它正在读取的缓冲区，则会导致缓冲区溢出并开始写入意外位置。
我们需要在每个字符串复制到 Row 结构之前检查每个字符串的长度，防止输入过长的字符串。因此，我们需要将输入中的空格过滤掉。
下面将使用 strtok（） 来做到这一点。

```c
+PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement) {
+  statement->type = STATEMENT_INSERT;
+
+  char* keyword = strtok(input_buffer->buffer, " ");
+  char* id_string = strtok(NULL, " ");
+  char* username = strtok(NULL, " ");
+  char* email = strtok(NULL, " ");
+
+  if (id_string == NULL || username == NULL || email == NULL) {
+    return PREPARE_SYNTAX_ERROR;
+  }
+
+  int id = atoi(id_string);
+  if (strlen(username) > COLUMN_USERNAME_SIZE) {
+    return PREPARE_STRING_TOO_LONG;
+  }
+  if (strlen(email) > COLUMN_EMAIL_SIZE) {
+    return PREPARE_STRING_TOO_LONG;
+  }
+
+  statement->row_to_insert.id = id;
+  strcpy(statement->row_to_insert.username, username);
+  strcpy(statement->row_to_insert.email, email);
+
+  return PREPARE_SUCCESS;
+}
+
 PrepareResult prepare_statement(InputBuffer* input_buffer,
                                 Statement* statement) {
   if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
+    return prepare_insert(input_buffer, statement);
-    statement->type = STATEMENT_INSERT;
-    int args_assigned = sscanf(
-        input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id),
-        statement->row_to_insert.username, statement->row_to_insert.email);
-    if (args_assigned < 3) {
-      return PREPARE_SYNTAX_ERROR;
-    }
-    return PREPARE_SUCCESS;
   }
```

在输入缓冲区上连续调用，通过在到达分隔符（在我们的例子中为空格）时插入一个空字符， strtok 将其分解为子字符串。它返回指向子字符串开头的指针。
我们可以对每个文本值调用 strlen（） 以查看它是否太长。

我们可以像处理任何其他错误代码一样处理错误：

```c
 enum PrepareResult_t {
   PREPARE_SUCCESS,
+  PREPARE_STRING_TOO_LONG,
   PREPARE_SYNTAX_ERROR,
   PREPARE_UNRECOGNIZED_STATEMENT
 };
```

```c
switch (prepare_statement(input_buffer, &statement)) {
   case (PREPARE_SUCCESS):
     break;
+  case (PREPARE_STRING_TOO_LONG):
+    printf("String is too long.\n");
+    continue;
   case (PREPARE_SYNTAX_ERROR):
     printf("Syntax error. Could not parse statement.\n");
     continue;
```

我们也没有对id的合法性进行校验，需要添加：

```c
enum PrepareResult_t {
   PREPARE_SUCCESS,
+  PREPARE_NEGATIVE_ID,
   PREPARE_STRING_TOO_LONG,
   PREPARE_SYNTAX_ERROR,
   PREPARE_UNRECOGNIZED_STATEMENT
};
```
```c
int id = atoi(id_string);
+  if (id < 0) {
+    return PREPARE_NEGATIVE_ID;
+  }
   if (strlen(username) > COLUMN_USERNAME_SIZE) {
     return PREPARE_STRING_TOO_LONG;
   }
```

```c
 switch (prepare_statement(input_buffer, &statement)) {
       case (PREPARE_SUCCESS):
         break;
+      case (PREPARE_NEGATIVE_ID):
+        printf("ID must be positive.\n");
+        continue;
       case (PREPARE_STRING_TOO_LONG):
         printf("String is too long.\n");
         continue;
```

特殊场景测试结果如下：

```shell
PS D:\code\db021\code> .\db.exe
db > insert -1 test test@qq.com
ID must be positive.
db > insert -1 test_test_test_test_test test@qq.com 
ID must be positive.
db > insert 1 test_test_test_test_test test@qq.com  
Executed.
db > select
(1, test_test_test_test_test, test@qq.com)
Executed.
db > insert 2 test_test_test_test_test_tttttttttttttttttttttttttttttttttttttttttttttt test@qq.com 
String is too long.
db >
```
