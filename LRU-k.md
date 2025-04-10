# 前提
- LRU-K 算法是对 LRU 算法的改进，将原先进入缓存队列的评判标准从访问一次改为访问K次，可以说朴素的 LRU 算法为 LRU-1。
# 算法思想
- LRU-K 算法有两个队列：一个是缓存队列，一个是数据访问历史队列。
- 当访问一个数据时，首先先在访问历史队列中累加访问次数，当历史访问记录超过 K 次后，才将数据缓存至缓存队列，从而避免缓存队列被污染。同时访问历史队列中的数据可以按照 LRU 的规则进行淘汰。
# 算法流程
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


