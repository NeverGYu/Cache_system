#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>
#include <iostream>

template<typename Key, typename Value>
class LruCache;

template<typename Key, typename Value>
class LruNode
{
public:
    friend class LruCache<Key,Value>;
    LruNode(Key key, Value value)
        : key_(key)
        , value_(value)
        , accessCount_(1)
        , prev_(nullptr)
        , next_(nullptr)
    {}
    
    // 提供访问的接口
    Key getKey(){ return key_; }
    Value getValue(){ return value_; }
    std::size_t getAccessCount() { return accessCount_; }
    void setValue(const Value& value) { value_ = value;}
    void incrementAccessCount() { ++accessCount_; }

private:
    Key key_;
    Value value_;
    int accessCount_; 
    std::shared_ptr<LruNode> prev_;
    std::shared_ptr<LruNode> next_;
};  

template<typename Key, typename Value>
class LruCache
{
public:
    using NodeType = LruNode<Key,Value>;
    using NodePtr = std::shared_ptr<NodeType>;
    using NodeMap = std::unordered_map<Key,NodePtr>;

    LruCache(int capcity)
        : capcity_(capcity)
    {
        initlizeList();
    }

    // 放入元素
    void put(Key key, Value value)
    {
        // 判断容量是否为空
        if (capcity_ < 0)
        {
            return;
            std::cout << "capcity initial = 0" << std::endl;
        }
        std::lock_guard<std::mutex> lock(mtx);
        // 判断在hashmap中是否存在该key
        auto it = hashmap_.find(key);
        if ( it != hashmap_.end())
        {
            // 如果存在
            updateExistingNode(it->second,value);
            return;
        }
        // 如果不存在，就将该节点添加到链表中
        addNewNode(key,value);
    }

    // 取出元素
    Value get(Key key)
    {
        Value value{};
        get(key, value);
        return value;
    }

    bool get(Key key, Value& value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = hashmap_.find(key); 
        if(it != hashmap_.end())
        {
            moveToMostRecent(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }

    void remove(Key key)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = hashmap_.find(key); 
        if(it != hashmap_.end())
        {
            removeNode(it->second);
            hashmap_.erase(it);
        }
    }

    void print()
    {
        NodePtr p = dummyHead->next_;
        while (p != dummyTail)
        { 
            std::cout << p->getKey() << ":" <<p->getValue() << "    ";
            p = p->next_;
        }
        std::cout << std::endl;
    }

private:
    void initlizeList()
    {
        dummyHead = std::make_shared<NodeType>(Key(),Value());
        dummyTail = std::make_shared<NodeType>(Key(),Value());
        dummyHead->next_ = dummyTail;
        dummyTail->prev_ = dummyHead; 
    }

    void updateExistingNode(NodePtr node, Value& value)
    {
        // 可能该key对应的值可以改变
        node->setValue(value);
        // 将该节点插入到最优先的位置
        moveToMostRecent(node);
    }

    void moveToMostRecent(NodePtr node)
    {
        // 先修改该节点的前后节点关系
        removeNode(node);
        // 在将该节点插入到队列尾部
        insertNode(node);
    }

    void removeNode(NodePtr node)
    {
        node->prev_->next_ = node->next_;
        node->next_->prev_ = node->prev_;
    }

    void insertNode(NodePtr node)
    {
        node->next_ = dummyTail;
        node->prev_ = dummyTail->prev_;
        dummyTail->prev_->next_ = node;
        dummyTail->prev_ = node;
    }

    void addNewNode(Key key, Value value)
    {
        // 先判断容量是否大于给定阈值
        if (hashmap_.size() >= capcity_)
        {
            // 将访问次数最少的节点删除
            evictLeastNode();
        }

        // 创建一个新节点
        NodePtr newNode = std::make_shared<NodeType>(key,value);
        // 再将这个新节点插入到链表中
        insertNode(newNode);
        // 向NodeMap注册该节点的key
        hashmap_[key] = newNode;
    }

    void evictLeastNode()
    {
        NodePtr leastNode = dummyHead->next_;
        removeNode(leastNode);
        hashmap_.erase(leastNode->getKey());
    }

private:
    int capcity_;          // 容量
    NodeMap hashmap_;      // hash表
    NodePtr dummyHead;     // 头指针
    NodePtr dummyTail;     // 尾指针
    std::mutex mtx;        // 互斥锁
};