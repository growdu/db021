# 数据库是如何工作的

注： 本文翻译自[db_tutorial](https://cstack.github.io/db_tutorial/).

数据库计算机世界的一个基础软件，要想深入了解数据库，就不得不思考如下几个问题：

- 数据以什么格式保存？（在内存和磁盘上）
- 它何时从内存移动到磁盘？
- 为什么每个表只能有一个主键？
- 回滚事务如何工作？
- 索引的格式是什么样的？
- 什么进行全表扫描？如何进行全表扫描？
- 预处理语句以什么格式保存？

总而言之，数据库究竟是如何工作的呢？

为了理解数据库是如何，本文将仿照sqlite用C从头开始构建一个数据库。

本文将分为如下13个部分：

1. [简介和设置REPL](./part1.md)
2. [世界上最简单的SQL编译器和虚拟机](./part2.md)
3. [仅支持追加的单表内存数据库](./part3.md)
4. [特殊场景完善](./part4.md)
5. [持久化到磁盘](./part5.md)
6. [游标抽象](./part6.md)
7. [B-树简介](./part7.md)
8. [B-树叶节点格式](./part8.md)
9. [二进制搜索和重复键](./part9.md)
10. [拆分叶节点](./part10.md)
11. [递归搜索B-树](./part11.md)
12. [扫描级B-树](./part12.md)
13. [拆分后更新父节点](./part13.md)
14. [拆分内部节点](./part14.md)

实际的数据库架构大致如下：

![](./img/arch2.png)