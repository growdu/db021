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