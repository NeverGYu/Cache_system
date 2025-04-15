#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <list>

template<typename Key, typename Value>
class ArcLruPart;

template<typename Key, typename Value>
class ArcLfuPart;

template<typename Key, typename Value>
class ArcCache;

/* 链表节点 ---  与之前相同 */
template<typename Key, typename Value>
class ArcNode
{
public:
    friend class ArcLruPart<Key,Value>;
    friend class ArcLfuPart<Key,Value>;

    ArcNode()
        : accessCount_(1)
        , prev(nullptr)
        , next(nullptr)
    {}

    ArcNode(Key key, Value value)
        : key_(key)
        , value_(value)
        , accessCount_(1)
        , prev(nullptr)
        , next(nullptr)
    {}

    Key getKey() const { return key_; }
    Value getValue() const {return value_;}
    int getAccessCount() const { return accessCount_; }
    void setValue(const Value& value) { value_ = value; }
    void incrementAccessCount() { ++accessCount_; }
private:
    Key key_;                       // 键
    Value value_;                   // 值
    int accessCount_;               // 其实就是频率
    std::shared_ptr<ArcNode> prev;  // 指向上一个结点
    std::shared_ptr<ArcNode> next;  // 指向下一个结点
};

/* Lru 部分 */
template<typename Key, typename Value>
class ArcLruPart
{
    friend class ArcCache<Key,Value>;
public:
    using Node = ArcNode<Key,Value>;
    using NodePtr = std::shared_ptr<Node>;
    using NodeMap = std::unordered_map<Key,NodePtr>;

    
    ArcLruPart(int capacity, int ghostCapacity, int transformThreshold)
        : Capacity_(capacity)
        , ghostCapacity_(ghostCapacity)
        , transformThreshold_(transformThreshold)
    {
       initialLists();
    }

    bool put(Key key, Value value)
    {
        if (Capacity_ < 0)
        {
            std::cout << "capacity < 0" << std::endl;
            return false;
        }
        std::lock_guard<std::mutex> lock(mtx_);
        if (auto it = mainCache_.find(key); it != mainCache_.end())
        {
            // 这表明key对应的结点存在mainCache中，更新lru结点链表
            return updateExistsNode(it->second, value);
        }       
        return addNewNode(key,value);
    }

    bool get(Key key, Value& value, bool& shouldTransform)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (auto it = mainCache_.find(key); it != mainCache_.end())
        {
            shouldTransform = updateAccess(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }

    bool HitGhost(Key key)
    {
        if (auto it = ghostCache_.find(key); it != ghostCache_.end())
        {
            // 这表示命中
            removeNode(it->second);
            return true;
        }
        return false;
    }

    void increaseCapacity() { ++Capacity_; }

    bool decreaseCapacity()
    {
        if (Capacity_ < 0 )
        {
            return false;
        }
        if (mainCache_.size() == Capacity_)
        {
            evictLeastNode();
        }
        --Capacity_;
        return true;
    }

    void print()
    {
        auto main = dummyHead_->next;
        auto ghost = GhostHead_->next;
        std::cout << "[mainCache|Lru]: ";
        while (main != dummyTail_ )
        {
            std::cout <<"[ Key : "<< main->getKey() << ", "<<"Value : "<< main->getValue() <<" ] " ;
            main = main->next;
        }
        std::cout << std::endl;
        std::cout << "[ghostCache|Lru]: ";
        while (ghost != GhostTail_)
        {
            std::cout<<"[ Key : "<< ghost->getKey() << ", "<<"Value : "<< ghost->getValue() <<" ] ";
            ghost = ghost->next;
        }
    }

private:
    void initialLists()
    {
        dummyHead_ = std::make_shared<Node>();
        dummyTail_ = std::make_shared<Node>();
        dummyHead_->next = dummyTail_;
        dummyTail_->prev = dummyHead_;

        GhostHead_ = std::make_shared<Node>();
        GhostTail_ = std::make_shared<Node>();
        GhostHead_->next = GhostTail_;
        GhostTail_->prev = GhostHead_;
    }

    bool updateExistsNode(NodePtr& node, Value& value)
    {
        // 更新结点的value
        node->setValue(value);
        // 更新前后结点的关系
        removeNode(node);
        // 将结点添加到尾结点的前面
        insertNodeToLruTail(node);
        return true;
    }

    void removeNode(NodePtr node)
    {
        if (!node || !node->prev || !node->next) 
        {
            return;
        }
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = nullptr;
        node->next = nullptr;
    }

    void insertNodeToLruTail(NodePtr& node)
    {
        // 安全起见，插入前先移除旧连接
        removeNode(node);

        node->prev = dummyTail_->prev;
        node->next = dummyTail_;
        dummyTail_->prev->next = node;
        dummyTail_->prev = node;
    }

    bool addNewNode(Key key, Value& value)
    {
        // 判断缓存容量是否已满
        if (mainCache_.size() >= Capacity_  )
        {
            // 已满就踢出一个最久的结点，被踢出的结点就到幽灵链表中
            evictLeastNode();
        }
        // 将新节点插入到链表的尾部,并注册在mainCache中
        NodePtr node = std::make_shared<Node>(key,value);
        insertNodeToLruTail(node);
        mainCache_[key] = node;
        return true;
    }

    void evictLeastNode()
    {
        // 踢出头节点之后的结点
        NodePtr node = dummyHead_->next;
        if (!node || node == dummyTail_)
        {
            std::cout << "Lru is null" << std::endl;
            return;
        }
        removeNode(node);
        // 被踢出的结点就到幽灵链表中
        if (ghostCache_.size() >= ghostCapacity_  )
        {
            // 如果幽灵链表已满，就按照先入先出的规则踢出结点，再将新节点放入到Ghost链表中
            removeNodeFromGhost();
        }
        addToGhost(node);
        // 更新结点信息
        mainCache_.erase(node->getKey());
    }

    void removeNodeFromGhost()
    {
        // 从Ghost链表中删除一个结点
        NodePtr node = GhostHead_->next;
        if (!node || node == GhostTail_)
        {
            std::cout << "GhostHead next is GhostTail" << std::endl;
            return ;
        }
        if (!node->prev || !node->next) {
            std::cout << "Invalid ghost node links: node->prev or node->next is null!" << std::endl;
            return;
        }

        removeNode(node);
        ghostCache_.erase(node->getKey());
    }

    void insertNodeToGhostTail(NodePtr node)
    {
        node->prev = GhostTail_->prev;
        node->next = GhostTail_;
        GhostTail_->prev->next = node;
        GhostTail_->prev = node;
    }

    void addToGhost(NodePtr node)
    {
        NodePtr ghostNode = std::make_shared<Node>(node->getKey(), node->getValue());
        // 重置节点的访问计数
        ghostNode->accessCount_ = 1;
        // 添加到幽灵链表中
        insertNodeToGhostTail(ghostNode);
        // 注册到ghostCache
        ghostCache_[node->getKey()] = ghostNode;
    }

    bool updateAccess(NodePtr node)
    {
        node->incrementAccessCount();
        insertNodeToLruTail(node);
        return node->accessCount_ >= transformThreshold_;
    }
    
    int Capacity_;              // lru链表的容量
    int ghostCapacity_;         // 幽灵链表的容量
    int transformThreshold_; // 转换阈值
    std::mutex mtx_;            // 互斥锁
    NodeMap mainCache_;         // 根据key找到lru链表的NodePtr
    NodeMap ghostCache_;        // 根据key找到ghostlru链表的NodePtr
    NodePtr dummyHead_;         // lru的头节点
    NodePtr dummyTail_;         // lru的尾节点
    NodePtr GhostHead_;         // ghostlru的头节点
    NodePtr GhostTail_;         // ghostlru的尾节点
};

template<typename Key, typename Value>
class ArcLfuPart
{
    friend class ArcCache<Key,Value>;
public:
    using Node = ArcNode<Key,Value>;
    using NodePtr = std::shared_ptr<ArcNode<Key,Value>>;
    using NodeMap = std::unordered_map<Key,NodePtr>;
    using Freq = std::size_t;
    using FreqList = std::list<NodePtr>;
    using FreqMap = std::unordered_map<Freq,std::shared_ptr<FreqList>>;

    friend class ArcCache<Key,Value>;

    ArcLfuPart(int capacity, int ghostCapacity, int transformThreshold)
        : capacity_(capacity)
        , ghostCapacity_(ghostCapacity)
        , transformThreshold_(transformThreshold)
        , minFreq_(0)
    {
        initialList();
    }
    
    bool put(Key key, Value value)
    {
        if (capacity_ <= 0)
        {
            std::cout << "capacity < 0" << std::endl;
            return false;
        }
        std::lock_guard<std::mutex> lock(mtx_);
        if (auto it = lfuCache_.find(key) ; it != lfuCache_.end())
        {
            
            return updateExistsNode(it->second,value);
        }
        return addNewNode(key,value);
    }

    bool get(Key key, Value& value) 
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = lfuCache_.find(key);
        if (it != lfuCache_.end()) 
        {
            updateNodeFrequency(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }

    bool HitGhost(Key key) 
    {
        auto it = GhostCache_.find(key);
        if (it != GhostCache_.end()) 
        {
            removeNode(it->second);
            GhostCache_.erase(it);
            return true;
        }
        return false;
    }

    void increaseCapacity() { ++capacity_; }
    
    bool decreaseCapacity() 
    {
        if (capacity_ <= 0)
        {
            return false;
        }
        if (lfuCache_.size() == capacity_) 
        {
            evictLeastFrequent();
        }
        --capacity_;
        return true;
    }

    void print()
    {
        std::cout << "[mainCache|Lfu]: ";
        for (auto it : FreqMap_)
        {
            std::cout <<"[ Freq: " << it.first <<"]: ";
            for (auto p : *it.second)
            {
                std::cout << "[ Key: "<< p->getKey() << ", " << "Value: " << p->getValue() << "], ";
            }
            std::cout << std::endl;
        }
        auto ghost = GhostHead_->next;
        std::cout  << "[ghostCache|Lfu]: ";
        while (ghost != GhostTail_)
        {   
            std::cout <<"[ Key : "<< ghost->getKey() << ", "<<"Value : "<< ghost->getValue() <<" ] ";
            ghost = ghost->next;
        } 
        std::cout << std::endl;
    }
private:
    void initialList()
    {
        GhostHead_ = std::make_shared<Node>();
        GhostTail_ = std::make_shared<Node>();
        GhostHead_->next = GhostTail_;
        GhostTail_->prev = GhostHead_;
    }

    bool addNewNode(Key key,Value value)
    {
        // 判断lfu缓存是否已满
        if (lfuCache_.size() >= capacity_)
        {
            evictLeastFrequent();
        }
        // 构造一个结点
        NodePtr node = std::make_shared<Node>(key,value);
        // 注册到lfu缓存
        lfuCache_[key] = node;
        // 添加到频率为1的频率链表
        addtoFreqList(node);
        return true;
    }

    void addtoFreqList(NodePtr node)
    {
        // 首先判断频率链表是否存在，不存在就创建一个频率链表
        if(auto it = FreqMap_.find(1); it == FreqMap_.end())
        {
            // 这说明频率链表不存在，需要创建一个频率链表
            FreqMap_[1] = std::make_shared<FreqList>();
        }
        // 将该节点添加到list中
        FreqMap_[1]->push_back(node);
        // 更新最小频率
        minFreq_ = 1;
    }

    void evictLeastFrequent()
    {
        // 判断整个频率队列是否为空
        if (FreqMap_.empty())
        {
            return;
        } 
        // 首先找到频率最小的链表
        auto& minFreqlist = FreqMap_[minFreq_];
        if (minFreqlist->empty())
        {
            return;
        }
        // 删除该链表中时间最远的结点
        auto Node = minFreqlist->front();
        minFreqlist->pop_front();
        // 判断该链表是否为空
        if (minFreqlist->empty())
        {
            // 这表示为空，删除该链表
            FreqMap_.erase(minFreq_);
            // 更新最小频率
            if (!FreqMap_.empty())
            {
                minFreq_ = FreqMap_.begin()->first;
            } 
        }
        // 判断幽灵缓存是否已满
        if (GhostCache_.size() >= ghostCapacity_)
        {
            removeNodeFromGhost();
        }
        // 将删除的结点添加到幽灵链表中
        addToGhost(Node);
        // 从主缓存中删除时间最远的结点
        lfuCache_.erase(Node->getKey());
    }
    void removeNode(NodePtr node)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void removeNodeFromGhost()
    {
        auto OldNode = GhostHead_->next;
        if (OldNode == GhostTail_)
        {
            return;
        }  
        removeNode(OldNode);
        GhostCache_.erase(OldNode->getKey());
    }

    void addToGhost(NodePtr node)
    {
        if (!node)
        {
            std::cout << "node is null" << std::endl;
            return;
        }
        node->prev = GhostTail_->prev;
        node->next = GhostTail_;
        GhostTail_->prev->next = node;
        GhostTail_->prev = node;
        GhostCache_[node->getKey()] = node;
    }

    bool updateExistsNode(NodePtr node, Value& value)
    {
        // 更新结点的value
        node->setValue(value);
        // 更新结点频率
        updateNodeFrequency(node);
        return true;
    }

    void updateNodeFrequency(NodePtr node)
    {
        size_t oldFreq = node->getAccessCount();
        node -> incrementAccessCount();
        size_t newFreq = node->getAccessCount();
        // 将结点从旧队列删除
        auto& oldList = FreqMap_[oldFreq];
        oldList->remove(node);
        // 判断旧频率队列是否为空
        if (oldList->empty())
        {
            FreqMap_.erase(oldFreq);
            if (oldFreq == minFreq_) 
            {
                minFreq_ = newFreq;
            }
        }
        if (auto it = FreqMap_.find(newFreq); it == FreqMap_.end())
        {
            FreqMap_[newFreq] = std::make_shared<FreqList>();
        }
        // 将结点添加到新队列
        FreqMap_[newFreq]->push_back(node);
    }

private:
    std::size_t capacity_;              // lfu缓存容量
    std::size_t ghostCapacity_;         // 幽灵列表的容量
    std::size_t transformThreshold_;    // 转换的阈值
    std::size_t minFreq_;               // 频度队列的最小频率值
    std::mutex mtx_;                    // 互斥锁
    NodeMap lfuCache_;                  // key <--> NodePtr
    NodeMap GhostCache_;                // key <--> NodePtr
    FreqMap FreqMap_;                   // freq <--> list<NodePtr>
    NodePtr GhostHead_;                 // 幽灵队列的头节点
    NodePtr GhostTail_;                 // 幽灵队列的尾节点
};

template<typename Key, typename Value>
class ArcCache
{
public:
    ArcCache(int capacity, int ghostCapacity = 3,int transformThreshold = 2)
        : capacity_(capacity)
        , transformThreshold_(transformThreshold)
        , lruPart(std::make_unique<ArcLruPart<Key,Value>>(capacity, ghostCapacity, transformThreshold))
        , lfuPart(std::make_unique<ArcLfuPart<Key,Value>>(capacity, ghostCapacity, transformThreshold))
    {}

    void put(Key key , Value value)
    {
        bool inGhost = checkGhostCache(key);
        if (!inGhost) 
        {
            if (lruPart->put(key, value)) 
            {
                lfuPart->put(key, value);
            }
        } 
        else 
        {
            lruPart->put(key, value);
        }
    }

    bool get(Key key, Value& value)
    {
        // 这会将key踢出ghost
        checkGhostCache(key);
        bool shouldTransform = false;
        if (lruPart->get(key, value, shouldTransform)) 
        {
            if (shouldTransform) 
            {
                lfuPart->put(key, value);
            }
            return true;
        }
        return lfuPart->get(key, value);
    }

    Value get(Key key)
    {
        Value value{};
        get(key, value);
        return value;
    }

    void print()
    {
        lruPart->print();
        std::cout << std::endl;
        lfuPart->print();
        std::cout << std::endl;
    }

private:
    bool checkGhostCache(Key key)
    {
        bool inGhost  = false;
        if (lruPart->HitGhost(key))
        {
            if (lfuPart->decreaseCapacity())
            {
                lruPart->increaseCapacity();
            }
            inGhost = true;
        }
        else if (lfuPart->HitGhost(key))
        {
            if (lruPart->decreaseCapacity())
            {
                lfuPart->increaseCapacity();
            }
            inGhost = true;
        }
        return inGhost;
    }

    int capacity_;
    int transformThreshold_;
    std::unique_ptr<ArcLruPart<Key,Value>> lruPart;
    std::unique_ptr<ArcLfuPart<Key,Value>> lfuPart;
};