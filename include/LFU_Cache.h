#ifndef LFU_CACHE_H
#define LFU_CACHE_H

#include <memory>
#include <mutex>
#include <unordered_map>

template<typename Key, typename value>
class LfuCache;

template<typename Key, typename Value>
class FreqList
{
private:
    struct Node
    {
        Key key;
        Value value;
        int freq;
        std::shared_ptr<Node> prev;
        std::shared_ptr<Node> next;

        Node()
            : freq(1)       // 这个频度是用来记录Node节点自身的频度
            , prev(nullptr) // 指向前一个直接点
            , next(nullptr) // 指向后一个直接点
        {}

        Node(Key key, Value value)
            : key(key)
            , value(value)
            , freq(1)
            , prev(nullptr)
            , next(nullptr)
        {}
    };

public:
    using NodePtr = std::shared_ptr<Node>;
    friend class LfuCache<Key, Value>;

    explicit FreqList(int freq)
        : freq_(freq)
    {
        dummyHead_ = std::make_shared<Node>();
        dummyTail_ = std::make_shared<Node>();
        dummyHead_->next = dummyTail_;
        dummyTail_->prev = dummyHead_;
    }

    bool isEmpty() const
    {
        return dummyHead_->next == dummyTail_;
    }

    void addNewNode(NodePtr node)
    {
        if (!node || !dummyHead_ || !dummyTail_)
        {
            return;
        }
        
        node->next = dummyTail_;
        node->prev = dummyTail_->prev;
        dummyTail_->prev->next = node;
        dummyTail_->prev = node;
    }

    void removeNode(NodePtr node)
    {
        if (!node || !dummyHead_ || !dummyTail_)
        {
            return;
        }

        if (!node->prev || !node->next)
        {
            return;
        }

        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = nullptr;
        node->next = nullptr;
    }

    NodePtr getFirstNode()
    {
        return dummyHead_->next;
    }

    void print() const
    {
        NodePtr p = dummyHead_->next;
        while (p != dummyTail_)
        {
            std::cout << "[Key: " << p->key << ", Value: " << p->value << ", Freq: " << p->freq << "] ";
            p = p->next;
        }
        std::cout << std::endl;
    }

private:
    int freq_;              // 这个频度是用来记录这个频度链表的频度
    NodePtr dummyHead_;     // 哨兵头节点
    NodePtr dummyTail_;     // 哨兵尾节点
};

template<typename Key, typename Value>
class LfuCache
{
public:
    using Node = typename FreqList<Key,Value>::Node;
    using NodePtr = std::shared_ptr<Node>;
    using NodeMap = std::unordered_map<Key,NodePtr>;
    using Freq = int;
    using Freqlist = FreqList<Key,Value>;
    using FqtFqListMap = std::unordered_map<Freq, std::shared_ptr<Freqlist>>;


    explicit LfuCache(int capcity, int MaxAverNum = 5)
        : capcity_(capcity)
        , minFreq_(INT16_MAX)
        , curTotalNum_(0)
        , MaxAverNum_(MaxAverNum)
        , curAverNum_(0)
    {}

    void put(Key key, Value value)
    {
        if (capcity_ <= 0)
        {
            std::cout << "capcity <= 0" << std::endl;
            return;
        }
        std::lock_guard<std::mutex> lock(mtx_);
        if (auto it = nodemap_.find(key) ; it != nodemap_.end())
        {
            // 这说明该key-value已经存放在了一个频度链表中，但是不知道该节点的频度是多少？
            // 更新该key对应value值，这个value是最新的
            it->second->value = value;
            // 直接调整这个节点使其位于频度加1的频度链表中
            getInternal(it->second,value);
            return;
        }
        // 这已经说明这是个新节点，之前从未访问过
        putInternal(key, value);
    }

    Value get(Key key)
    {
        Value value{};
        get(key,value);
        return value;
    }

    bool get(Key key, Value &value)
    {
       std::lock_guard<std::mutex> lock(mtx_);
       if (auto it = nodemap_.find(key); it != nodemap_.end())
       {
            getInternal(nodemap_[key],value);
            return true;
       }
       return false;
    }

    // 清空缓存,回收资源
    void purge()
    {
      nodemap_.clear();
      fqlistmap_.clear();
    }

    void print() const
    {
        for (auto& it : fqlistmap_)
        {
            std::cout << "Freq: " << it.first << " -> ";
            it.second->print();  // 打印该频度对应的链表
        }
    }

    void getTotalNum() const
    {
        std::cout << "curTotalNum : " << curTotalNum_ << std::endl;
    }

    void getAverNum() const
    {
        std::cout << "curAverNum : " << curAverNum_ << std::endl;
    }

private:
    void getInternal(NodePtr node, Value& value);
    void removeFromFreqlist(NodePtr node);
    void addToFreqlsit(NodePtr node);
    void putInternal(Key key, Value& value);
    void KickOut();
    void addFreqNum();
    void handleOverMaxAverageNum();
    void updateMinFq();
    void decreaseFreqNum(int num);

private:
    int capcity_;            // 表示缓存队列里的容量
    int minFreq_;            // 表示频度最小的频度链表队列
    std::mutex mtx_;         // 互斥锁，用于多线程访问
    NodeMap nodemap_;        // map 中存放的是 Key-->Node
    FqtFqListMap fqlistmap_; // map 中存放的是 freq-->freqList

    int curTotalNum_;        // 总的访问次数
    int curAverNum_;         // 平均访问次数
    int MaxAverNum_;         // 最大访问次数
};

template<typename Key, typename Value>
void LfuCache<Key,Value>::getInternal(NodePtr node, Value& value)
{
    /*
    * 目的
    * 1. 先将node从它原先的频度链表中删除
    * 2. 再将node添加到更高频度的链表中
    */ 
    value = node->value;
    // 将节点从这个频度链表中删除
    removeFromFreqlist(node);
    // 更新节点的频度
    ++node->freq;
    // 再将该节点添加到更高频度的链表
    addToFreqlsit(node);
    // 判断原先的频度链表是否为空,并且是频度最小的链表，如果是就删除频度链表
    if (node->freq - 1 == minFreq_ && fqlistmap_[node->freq - 1]->isEmpty())
    {
        minFreq_++;
    }
    // 总访问频次和当前平均访问频次都随之增加
    addFreqNum();
}

template<typename Key, typename Value>
void LfuCache<Key,Value>::removeFromFreqlist(NodePtr node)
{
    if (!node)
    {
        std::cout << "node is nullptr" << std::endl;
        return;
    }
    // 获得节点的频度，然后找出对应的频度链表,再将节点从这个频度链表中删除
    fqlistmap_[node->freq]->removeNode(node);
}

template<typename Key, typename Value>
void LfuCache<Key,Value>::addToFreqlsit(NodePtr node)
{
    if (!node)
    {
        std::cout << "node is nullptr" << std::endl;
        return;
    }
    // 获得节点的频度，然后找出对应的频度链表,再将该节点添加到更高频度的链表
    auto freq = node->freq;
    // 判断频度链表是否存在
    if (auto it = fqlistmap_.find(freq); it ==  fqlistmap_.end())
    {
        // 如果不存在
        fqlistmap_[freq] = std::make_shared<FreqList<Key,Value>>(freq);
    }
    // 如果存在
    fqlistmap_[freq]->addNewNode(node);
}

template<typename Key, typename Value>
void LfuCache<Key,Value>::putInternal(Key key, Value& value)
{
    // 判断缓存容量是否已满
    if (capcity_ == nodemap_.size())
    {
        // 剔除该节点所属频率队列中的最不经常访问的节点
        KickOut();
    }
    // 创建一个结点对象
    NodePtr node =  std::make_shared<Node>(key,value);
    // 将其添加到对应的频度队列中
    addToFreqlsit(node);
    // 将该key注册到nodemap中
    nodemap_[key] = node;
    /// 更新最小频度队列
    minFreq_ = std::min(minFreq_ , 1);
    // 总访问频次和当前平均访问频次都随之增加
    addFreqNum();
}

template<typename Key, typename Value>
void LfuCache<Key,Value>::KickOut()
{
    // 获得最小频率的节点
    auto node = fqlistmap_[minFreq_]->getFirstNode();
    // 将节点从频率链表中删除
    removeFromFreqlist(node);
    // 将结点从nodemap中删除
    nodemap_.erase(node->key);
    // 
    decreaseFreqNum(node->freq);
}

template<typename Key, typename Value>
void LfuCache<Key,Value>::addFreqNum()
{
    // 总的访问次数加一
    ++curTotalNum_;
    // 判断节点个数是否为0
    if (nodemap_.size() == 0)
    {
        curAverNum_ = 0;
    }
    else
    {
        curAverNum_ = curTotalNum_/nodemap_.size();
    }
    // 如果平均访问次数大于给定的最大访问次数，就需要调整
    if (curAverNum_ > MaxAverNum_)
    {
        handleOverMaxAverageNum();
    }
    
}

template<typename Key, typename Value>
void LfuCache<Key,Value>::handleOverMaxAverageNum()
{
    if (nodemap_.size() == 0)
    {
        return;
    }

    for (auto it = nodemap_.begin(); it != nodemap_.end(); ++it)
    {
        // 判断节点是否为空
        if (it->second)
        {
            return;
        }
        // 取出nodemap里所有的节点
        NodePtr node  = it->second;
        // 将节点从原先的频率链表中删除
        removeFromFreqlist(node);
        // 修改节点的频率、
        node->freq -= MaxAverNum_/2;
        if (node->freq < 1)
        {
            node->freq = 1;
        }
        // 添加到新的频率链表
        addToFreqlsit(node);
    }
    
    // 更新最小频率
    updateMinFq();
}

template<typename Key, typename Value>
void LfuCache<Key,Value>::updateMinFq()
{
    minFreq_ = INT16_MAX;
    for (const auto& pair : fqlistmap_) 
    {
        // 判断频率队列是否存在
        if (pair.second && !pair.second->isEmpty()) 
        {
            minFreq_ = std::min(minFreq_, pair.first);
        }
    }
    // 如果频率队列都不存在，最小频率置为一
    if (minFreq_ == INT16_MAX)
    {
        minFreq_ = 1;
    } 
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::decreaseFreqNum(int num)
{
    // 减少平均访问频次和总访问频次
    curTotalNum_ -= num;
    if (nodemap_.empty())
        curAverNum_ = 0;
    else
        curAverNum_ = curTotalNum_ / nodemap_.size();
}

#endif