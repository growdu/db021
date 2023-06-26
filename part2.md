# 世界上最简单的编译器和虚拟机

我们将仿照sqlite来实现自己的sqlite。sqlite 的“前端”是一个 SQL 编译器，它解析字符串并输出称为字节码的内部表示。

此字节码将传递到虚拟机，由虚拟机执行它。

![](./img/arch2.png)

像这样将事情分成两个步骤有几个优点：

- 降低每个部分的复杂性（例如，虚拟机不必担心语法错误）

- 允许编译一次常见查询并缓存字节码以提高性能

考虑到这一点，让我们重构我们的 main 函数，并在此过程中支持两个新关键字：

```c
 int main(int argc, char* argv[]) {
   InputBuffer* input_buffer = new_input_buffer();
   while (true) {
     print_prompt();
     read_input(input_buffer);

-    if (strcmp(input_buffer->buffer, ".exit") == 0) {
-      exit(EXIT_SUCCESS);
-    } else {
-      printf("Unrecognized command '%s'.\n", input_buffer->buffer);
+    if (input_buffer->buffer[0] == '.') {
+      switch (do_meta_command(input_buffer)) {
+        case (META_COMMAND_SUCCESS):
+          continue;
+        case (META_COMMAND_UNRECOGNIZED_COMMAND):
+          printf("Unrecognized command '%s'\n", input_buffer->buffer);
+          continue;
+      }
     }
+
+    Statement statement;
+    switch (prepare_statement(input_buffer, &statement)) {
+      case (PREPARE_SUCCESS):
+        break;
+      case (PREPARE_UNRECOGNIZED_STATEMENT):
+        printf("Unrecognized keyword at start of '%s'.\n",
+               input_buffer->buffer);
+        continue;
+    }
+
+    execute_statement(&statement);
+    printf("Executed.\n");
   }
 }
 ```

 像 .exit 这样的非 SQL 语句称为“元命令”。它们都以点开头，因此我们检查它们并在单独的函数中处理它们。

 接下来，我们添加一个步骤，将输入行转换为语句的内部表示形式。这是我们的 sqlite 前端的hacky版本。

 最后，我们将准备好的语句传递给 execute_statement 。这个功能最终将成为我们的虚拟机。

 请注意，我们的两个新函数返回指示成功或失败的枚举：

 ```c
 typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum { 
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT 
} PrepareResult;
```
do_meta_command 只是现有功能的包装器，为更多命令留出了空间：

```c
MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}
```

我们现在的“准备好的语句”只包含一个具有两个可能值的枚举。它将包含更多数据，因为我们允许在语句中使用参数：

```c
typedef enum { 
    STATEMENT_INSERT, 
    STATEMENT_SELECT 
} StatementType;

typedef struct {
  StatementType type;
} Statement;
```
prepare_statement （我们的“SQL 编译器”）现在不理解 SQL。其实它只懂两个字：

```c
PrepareResult prepare_statement(InputBuffer* input_buffer,
                                Statement* statement) {
  if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
    statement->type = STATEMENT_INSERT;
    return PREPARE_SUCCESS;
  }
  if (strcmp(input_buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}
```
请注意，我们使用 strncmp 比较“insert”，因为 “insert” 关键字后跟数据。（例如 insert 1 cstack foo@bar.com ）

最后， execute_statement 包含几个存根：

```c
void execute_statement(Statement* statement) {
  switch (statement->type) {
    case (STATEMENT_INSERT):
      printf("This is where we would do an insert.\n");
      break;
    case (STATEMENT_SELECT):
      printf("This is where we would do a select.\n");
      break;
  }
}
```

请注意，它不会返回任何错误代码，因为目前还没有可能出错的问题。

通过这些重构，我们添加了两个新关键字，其执行结果如下所示：

```shell
PS D:\code\db021\code> make
gcc -g -O0 main.c -o db
PS D:\code\db021\code> .\db.exe
db > insert foo bar
This is where we would do an insert.
db > exit
Unrecognized keyword at start of 'exit'.
db > .exit
db > delete foo
Unrecognized keyword at start of 'delete foo'.
db > select
This is where we would do a select.
Executed.
db > .tables
Unrecognized command '.tables'
db > .exit
PS D:\code\db021\code>
```

我们数据库的骨架正在形成...如果它存储数据不是很好吗？在下一部分中，我们将实现 insert 和 select ，创建世界上最差的数据存储。同时，如下部分是我们的所有代码：

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define MAX_INPUT_BUFFER 4096
#endif

 typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum { 
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT 
} PrepareResult;

typedef enum { 
    STATEMENT_INSERT, 
    STATEMENT_SELECT 
} StatementType;

typedef struct {
  StatementType type;
} Statement;

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
  #ifdef _WIN32
  input_buffer->buffer = malloc(MAX_INPUT_BUFFER);
  #else
  input_buffer->buffer = NULL;
  #endif
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

PrepareResult prepare_statement(InputBuffer* input_buffer,
                                Statement* statement) {
  if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
    statement->type = STATEMENT_INSERT;
    return PREPARE_SUCCESS;
  }
  if (strcmp(input_buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement) {
  switch (statement->type) {
    case (STATEMENT_INSERT):
      printf("This is where we would do an insert.\n");
      break;
    case (STATEMENT_SELECT):
      printf("This is where we would do a select.\n");
      break;
  }
}

void print_prompt() { printf("db > "); }

void read_input(InputBuffer* input_buffer) {
#ifdef _WIN32
  size_t len;
  fgets(input_buffer->buffer, MAX_INPUT_BUFFER, stdin);
  len = strlen(input_buffer->buffer);
  if (len <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }
  input_buffer->input_length = len - 1;
  input_buffer->buffer[len - 1] = '\0';
#else
  
  ssize_t bytes_read =
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
#endif
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

int main(int argc, char* argv[]) {
  InputBuffer* input_buffer = new_input_buffer();
  while (true) {
    print_prompt();
    read_input(input_buffer);

    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_command(input_buffer)) {
        case (META_COMMAND_SUCCESS):
          continue;
        case (META_COMMAND_UNRECOGNIZED_COMMAND):
          printf("Unrecognized command '%s'\n", input_buffer->buffer);
          continue;
      }
     }

    Statement statement;
    switch (prepare_statement(input_buffer, &statement)) {
      case (PREPARE_SUCCESS):
        break;
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        printf("Unrecognized keyword at start of '%s'.\n",
               input_buffer->buffer);
        continue;
    }
    execute_statement(&statement);
    printf("Executed.\n");
  }
}
```
makefile与之前的一致，没有什么改变。

```makefile
all:
	gcc -g -O0 main.c -o db
clean:
	rm -rf db
```