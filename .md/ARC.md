# ARC(自适应替换缓存)
- ARC 算法是缓存淘汰算法里面的一种，是结合了 LRU 和 LFU 淘汰算法两种算法优点的一种性能优越算法。
- ARC 算法通过两个表的内存空间动态调整来实现内存更有效的使用。
- 假设我们有一个8个页面大小的缓存。为了使 ARC 可以工作，在缓存中，它需要一个2倍大小的管理表
# 算法思想
- 管理表（由四个链表组成）。
  - 最近最多使用的页面链表 （LRU list）
  - 最近最频繁使用的页面链表（LFU list）
  - 存储那些最近从最近最多使用链表中淘汰的页面信息 （Ghost list for LRU）
  - 存储那些最近从最近最频繁使用链表中淘汰的页面信息（Ghost list for LFU）
![Capture_20250414_170343](https://github.com/user-attachments/assets/9ec1df01-6325-497e-91fd-93918ccc2692)
# 算法过程
- 假设我们从磁盘上读取一个页面，并把它放入cache中。这个页面会放入LRU 链表中。
![Capture_20250414_171958](https://github.com/user-attachments/assets/8523b6ae-7b50-4e11-81c6-31efb259cfa8)
- 接下来我们读取另外一个不同的页面。它也会被放入缓存。显然，它也会被放入LRU 链表的最近最多使用的位置（位置1）：
![Capture_20250414_172029](https://github.com/user-attachments/assets/19a13d8d-5f43-4250-881f-68ae64fe6617)
- 如果我们再读一次第一个页面。我们可以看到，这个页面在缓存中将会被移到 LFU 链表中。所有进入 LFU 链表中的页面都必须至少被访问两次。
![Capture_20250414_172111](https://github.com/user-attachments/assets/e911d8f5-d29d-474a-9f29-d8c6fb627369)
- 随着时间的推移，这两个链表不断的被填充，缓存也相应的被填充。
![Capture_20250414_172422](https://github.com/user-attachments/assets/16ffa3e8-5056-40ed-850d-e0f91109c1ae)
- 这时，缓存已经满了，而你读进了一个没有被缓存的页面。所以，我们必须从缓存中淘汰一个页面，为这个新的数据页提供位置。这时在LRU链表中，最近最少使用的页面将会被淘汰出去。这个页面的信息会被放进LRU ghost链表中。
![Capture_20250414_172821](https://github.com/user-attachments/assets/e6ac5c14-8568-4de5-8680-dddddf345860)
- 现在这个被淘汰的页面不再被缓存引用，所以我们可以把这个数据页的数据释放掉。新的数据页将会被缓存表引用。
![Capture_20250414_173035](https://github.com/user-attachments/assets/98193ffb-4399-4272-a10a-39d47664247e)
- 随着更多的页面被淘汰，这个在LRU ghost中的页面信息也会向ghost链表尾部移动。在随后的一个时间点，这个被淘汰页面的信息也会到达链表尾部，LRU链表的下一次的淘汰过程发生之后，这个页面信息也会从LRU ghost链表中移除，那是就再也没有任何对它的引用了。
- 如果这个页面在被从LRU ghost链表中移除之前，会发生幽灵命中！由于这个页面的数据已经从缓存中移除了，所以系统还是必须从后端存储媒介中再读一次，但是由于这个幽灵命中，系统知道，这是一个刚刚淘汰的页面，而不是第一次读取或者说很久之前读取的一个页面。ARC用这个信息来调整它自己，以适应当前的I/O模式
![Capture_20250414_173239](https://github.com/user-attachments/assets/6c18f572-d1e6-44e2-aa2f-cf64a305b734)
- 很显然，这说明我们的LRU缓存太小。在这种情况下，LRU链表的长度将会被增加一。显然，LFU链表的长度将会被减一。
![Capture_20250414_173643](https://github.com/user-attachments/assets/49c3e10c-4823-4ffe-9506-708fee31085b)
- 但是同样的机制存在于LFU这边。如果一次命中发生在LFU ghost 链表中，它会减少LRU链表的长度（减一），以此在LFU 链表中加一个可用空间。

  
