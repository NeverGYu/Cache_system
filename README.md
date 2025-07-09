# 项目介绍
- 来源于：代码随想录《缓存系统》
- 本项目使用多个页面替换策略实现一个线程安全的缓存系统
# LRU （Least Recently Used）简介
- 设计原则借鉴了时间局部性原理，该算法认为如果数据最近被访问过，那么将来被访问的几率也更高
- 其原理是将数据按照其被访问的时间形成一个有序序列，最久未被使用的数据应该最早被淘汰掉，即当缓存空间被占满时，缓存内最长时间未被使用的数据将被淘汰掉。
## 算法思想
- 假设缓存内部数据如下：
  
  <img src="https://github.com/user-attachments/assets/da320207-9b86-45d1-ab2a-734f0c76eaae" width="100" />
- 当调用缓存获取 key = 1 的数据，LRU 算法需要将 1 这个节点移到头节点，其余节点不变
  <img src="https://github.com/user-attachments/assets/f81ba4bf-e67c-4afc-840a-766c6226bc29" width="400" />
- 我们可以直接将数据添加到头节点
    ![Capture_20250407_182302](https://github.com/user-attachments/assets/0e8da94f-880b-4834-91b5-a229ca2b592e)
## 算法步骤
- 新数据直接插入到列表头部。
- 缓存数据被命中，将数据移到列表头部。
- 缓存已满时，移除列表尾部数据。
## 数据结构
- 使用散列表存储节点 key，可以将获得节点 (key,value) 的时间复杂度为 O(1)。
- 使用双向链表用于维护节点的顺序，支持高效的删除和插入操作，时间复杂度也为 O(1)。
  ![Capture_20250407_184837](https://github.com/user-attachments/assets/1eb6e4d7-c1be-48d1-b22d-a8d790fc8dd3)
## 入口函数
### put 函数
  ![2d695a7c-7952-4d0a-b5aa-f2c3bb7f9225](https://github.com/user-attachments/assets/b0bd058f-be8e-4648-bb36-11502aae5a06)
### get 函数
  ![e7068e48-fcd6-44a7-b9e3-adeb349b96e8](https://github.com/user-attachments/assets/08b102f6-3716-461e-a8fd-0556a680b822)

# LRU-K 简介
- LRU-K 算法是对 LRU 算法的改进，将原先进入缓存队列的评判标准从访问一次改为访问K次，可以说朴素的 LRU 算法为 LRU-1。
## 算法思想
- LRU-K 算法有两个队列：一个是缓存队列，一个是数据访问历史队列。
- 当访问一个数据时，首先先在访问历史队列中累加访问次数，当历史访问记录超过 K 次后，才将数据缓存至缓存队列，从而避免缓存队列被污染。同时访问历史队列中的数据可以按照 LRU 的规则进行淘汰。
## 算法流程
- 页面第一次被访问，添加到历史队列中
  ![Capture_20250410_182544](https://github.com/user-attachments/assets/b892d770-18f4-4511-9eb8-12903468148f)
- 当历史队列中的页面满了，根据一定的缓存策略(FIFO、LRU、LFU)进行淘汰老的页面。
  ![Capture_20250410_182619](https://github.com/user-attachments/assets/785e7ffe-2bda-4eb8-8931-6d1ec9f05133)
- 当历史队列中的某个页面第k次访问时，该页面从历史队列中出栈，并存放至缓存队列。
  ![Capture_20250410_183756](https://github.com/user-attachments/assets/96467e8d-2dbb-4c8a-9f70-74771c4b867d)
- 缓存队列中的页面再次被访问k次时，历史队列中该页面出栈，并且更新缓存队列中该页面的位置。
  ![Capture_20250410_184353](https://github.com/user-attachments/assets/20ee8ee2-c37d-40ce-94f6-b03b512d69a7)
- 当缓存队列需要淘汰页面时，淘汰最后一个页面，也就是第k次访问距离现在最久的那个页面。
  ![Capture_20250410_185059](https://github.com/user-attachments/assets/8e54aa5c-69bf-440f-bb09-d02ef471a867)
## 小结
- LRU-k 的命中率要比 LRU 要高，但是因为需要维护一个历史队列，因此内存消耗会比 LRU 多。
- 实际应用中 LRU-2 是综合各种因素后最优的选择，LRU-3 或者更大的 K 值命中率会高，但适应性差，需要大量的数据访问才能将历史访问记录清除掉。
  
# LFU（最不经常使用）
- 它是基于“如果一个数据在最近一段时间内使用次数很少，那么在将来一段时间内被使用的可能性也很小”的思路实现的。
- 这个缓存算法一般实现内部都会使用一个计数器来记录条目被访问的频率，然后依据计数器访问数值比较，把最小的条目首先被移除。
- 这个方法并不经常使用，因为它无法对一个拥有最初高访问率之后长时间没有被访问的条目缓存负责。
## 算法思想
  ![Capture_20250411_132000](https://github.com/user-attachments/assets/a7b64f01-1cde-4f0e-b83d-262db7d28b9f)

# ARC(自适应替换缓存)
- ARC 算法是缓存淘汰算法里面的一种，是结合了 LRU 和 LFU 淘汰算法两种算法优点的一种性能优越算法。
- ARC 算法通过两个表的内存空间动态调整来实现内存更有效的使用。
- 假设我们有一个8个页面大小的缓存。为了使 ARC 可以工作，在缓存中，它需要一个2倍大小的管理表
## 算法思想
- 管理表（由四个链表组成）。
  - 最近最多使用的页面链表 （LRU list）
  - 最近最频繁使用的页面链表（LFU list）
  - 存储那些最近从最近最多使用链表中淘汰的页面信息 （Ghost list for LRU）
  - 存储那些最近从最近最频繁使用链表中淘汰的页面信息（Ghost list for LFU）
![Capture_20250414_170343](https://github.com/user-attachments/assets/9ec1df01-6325-497e-91fd-93918ccc2692)
## 算法过程
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
