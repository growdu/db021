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