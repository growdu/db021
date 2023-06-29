# 持久化到磁盘

我们的数据库允许您插入记录并读回记录，但前提是您保持程序运行。如果您终止程序并重新启动它，则所有记录都将消失。

与 sqlite 一样，我们将通过将整个数据库保存到文件中来持久化记录。

我们已经通过将行序列化为页面大小的内存块来做到这一点。为了增加持久性，我们可以简单地将这些内存块写入文件，并在下次程序启动时将它们读回内存中。

为了简化此操作，我们将创建一个称为寻呼器的抽象。我们向pager询问 页码 x ，pager会给我们一个内存块。它首先在其缓存中查找。在缓存未命中时，它会将数据从磁盘复制到内存中（通过读取数据库文件）。

如下图所示，我们的数据库架构和sqlite的映射关系如下：

![](./img/arch-part5.gif)

pager访问页面缓存和文件。Table 对象通过pager发出页面请求：

```c
+typedef struct {
+  int file_descriptor;
+  uint32_t file_length;
+  void* pages[TABLE_MAX_PAGES];
+} Pager;
+
 typedef struct {
-  void* pages[TABLE_MAX_PAGES];
+  Pager* pager;
   uint32_t num_rows;
 } Table;
```

我重命名为 new_table() db_open() ，因为它现在具有打开数据库连接的效果。打开一个连接意味着：

- 打开数据库文件

- 初始化pager数据结构

- 初始化表数据结构

```c
-Table* new_table() {
+Table* db_open(const char* filename) {
+  Pager* pager = pager_open(filename);
+  uint32_t num_rows = pager->file_length / ROW_SIZE;
+
   Table* table = malloc(sizeof(Table));
-  table->num_rows = 0;
+  table->pager = pager;
+  table->num_rows = num_rows;

   return table;
 }
```

db_open() 调用 pager_open() 打开数据库文件并跟踪其大小。它还将页面缓存初始化为所有 NULL。

```c
+Pager* pager_open(const char* filename) {
+  int fd = open(filename,
+                O_RDWR |      // Read/Write mode
+                    O_CREAT,  // Create file if it does not exist
+                S_IWUSR |     // User write permission
+                    S_IRUSR   // User read permission
+                );
+
+  if (fd == -1) {
+    printf("Unable to open file\n");
+    exit(EXIT_FAILURE);
+  }
+
+  off_t file_length = lseek(fd, 0, SEEK_END);
+
+  Pager* pager = malloc(sizeof(Pager));
+  pager->file_descriptor = fd;
+  pager->file_length = file_length;
+
+  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
+    pager->pages[i] = NULL;
+  }
+
+  return pager;
+}
```

按照我们的新抽象，我们将获取页面的逻辑移动到它自己的方法中：

```c
void* row_slot(Table* table, uint32_t row_num) {
   uint32_t page_num = row_num / ROWS_PER_PAGE;
-  void* page = table->pages[page_num];
-  if (page == NULL) {
-    // Allocate memory only when we try to access page
-    page = table->pages[page_num] = malloc(PAGE_SIZE);
-  }
+  void* page = get_page(table->pager, page_num);
   uint32_t row_offset = row_num % ROWS_PER_PAGE;
   uint32_t byte_offset = row_offset * ROW_SIZE;
   return page + byte_offset;
 }
```

该方法 get_page() 具有处理缓存未命中的逻辑。我们假设页面一个接一个地保存在数据库文件中：第 0 页位于偏移量 0，第 1 页位于偏移量 4096，第 2 页位于偏移量 8192，依此类推。如果请求的页面位于文件边界之外，我们知道它应该是空白的，所以我们只是分配一些内存并返回它。稍后将缓存刷新到磁盘时，该页面将添加到文件中。