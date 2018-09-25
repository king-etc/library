Lock Free Queues
==================

Public domain implementation of four different lock free queues:
  * SPSC lock free dynamic queue which requires a memory allocation with each insert.
  * MPSC lock free dynamic queue which requires a memory allocation with each insert.
  * SPSC wait free bound queue/ring buffer which which uses a fixed size pre-allocated buffer.
  * MPMC lock free bound queue/ring buffer which which uses a fixed size pre-allocated buffer.
  
需要存储的数据如果是对象，使用指针指向对应的对象。注意出队时需要析构并删除该对象的内存。
