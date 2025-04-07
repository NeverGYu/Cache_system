# LRU （Least Recently Used）简介
- 设计原则借鉴了时间局部性原理，该算法认为如果数据最近被访问过，那么将来被访问的几率也更高
- 其原理是将数据按照其被访问的时间形成一个有序序列，最久未被使用的数据应该最早被淘汰掉，即当缓存空间被占满时，缓存内最长时间未被使用的数据将被淘汰掉。
# 算法思想
- 假设缓存内部数据如下：
  
  <img src="https://github.com/user-attachments/assets/da320207-9b86-45d1-ab2a-734f0c76eaae" width="100" />
- 当调用缓存获取 key = 1 的数据，LRU 算法需要将 1 这个节点移到头节点，其余节点不变
  <img src="https://github.com/user-attachments/assets/f81ba4bf-e67c-4afc-840a-766c6226bc29" width="400" />
- 我们可以直接将数据添加到头节点
    ![Capture_20250407_182302](https://github.com/user-attachments/assets/0e8da94f-880b-4834-91b5-a229ca2b592e)
# 算法步骤
- 新数据直接插入到列表头部。
- 缓存数据被命中，将数据移到列表头部。
- 缓存已满时，移除列表尾部数据。
# 数据结构
- 使用散列表存储节点 key，可以将获得节点 (key,value) 的时间复杂度为 O(1)。
- 使用双向链表用于维护节点的顺序，支持高效的删除和插入操作，时间复杂度也为 O(1)。
  ![Capture_20250407_184837](https://github.com/user-attachments/assets/1eb6e4d7-c1be-48d1-b22d-a8d790fc8dd3)
