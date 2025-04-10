#pragma once
#include "LRU_Cache.h"

template<typename Key, typename Value>
class Lru_k_Cache : public LruCache<Key,Value>
{
public:
    Lru_k_Cache(int capcity,int historyCapcity,int threshold)
        : LruCache<Key, Value>(capcity)
        , threshold_(threshold)
        , history_(std::make_unique<LruCache<Key,int>>(historyCapcity))
    {}

    void put(Key key, Value value)
    {
        // 首先判断key是否在缓存队列中
        if (LruCache<Key,Value>::get(key,value))
        {
            // 可能 key 对应的 value 会发生改变，所以直接覆盖
            LruCache<Key,Value>::put(key,value);
        }

        // 如果不在缓存队列中，那么将其放在历史队列中，并且判断次数是否满足加入到缓存队列中
        int historyCount = history_->get(key);
        history_->put(key,++historyCount);

        if (historyCount >= threshold_)
        {
            // 再将其从历史队列中删除掉
            history_->remove(key);
            // 先将其添加到缓存队列中
            LruCache<Key,Value>::put(key,value);
        }
        
    }

    Value get(Key key)
    {
        // 获取该数据的访问次数
        int historyCount = history_->get(key);
        // 如果访问到数据，则更新历史访问节点值
        history_->put(key,++historyCount);
        // 从缓存中获取数据，不一定能获取到，因为可能不在缓存中
        return LruCache<Key, Value>::get(key);
    }

    void print_history()
    {
        history_->print();
    }

private:
    int threshold_;   // 阈值，用来标明Key被查询几次后，才可从历史队列添加到缓存队列中
    std::unique_ptr<LruCache<Key,int>> history_;  // 历史队列 
};