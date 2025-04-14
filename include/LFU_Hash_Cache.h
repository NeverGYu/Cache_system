#pragma once
#include "LFU_Cache.h"
#include <vector>
#include <memory>
#include <cmath>

template<typename Key, typename Value>
class HashLfu : public LfuCache<Key,Value>
{
public:
    HashLfu(int capacity, int sliceNum, int maxAverNum = 5)
        : capacity_(capacity)
        , sliceNum_(sliceNum)
        , LfuCache<Key, Value>(capacity, maxAverNum)
    {
        // 切片大小
        int slice = std::ceil(static_cast<double>(capacity_) / sliceNum_);
        for (int i = 0; i < sliceNum_; ++i)
        {
            hashLfu_.emplace_back(std::make_unique<LfuCache<Key, Value>>(slice, maxAverNum));
        }
    }

    void put(Key key, Value value)
    {
        // 根据hash值计算出索引
        int sliceIndex = Hash(key) % sliceNum_;
        return hashLfu_[sliceIndex]->put(key,value);
    }

    Value get(Key key)
    {
        Value value{};
        this->get(key,value);
        return value;
    }

    bool get(Key key, Value &value)
    {
        // 根据hash值计算出索引
        int sliceIndex = Hash(key)%sliceNum_;
        return hashLfu_[sliceIndex]->get(key,value);
    }

    void purge()
    {
        
        for (auto& hashSliceLfu : hashLfu_)
        {
            hashSliceLfu->LfuCache<Key,Value>::purge();
        }
    }

    void print()
    {
        for (auto &it : hashLfu_)
        {
            it->print();
        }   
    }
private:
    // 将Key计算成hash值
    size_t Hash(Key key)
    {
        std::hash<Key> hahsFunc;
        return hahsFunc(key);
    }
private:
    int capacity_;   // 缓存容量
    int sliceNum_;   // 分片个数
    std::vector<std::unique_ptr<LfuCache<Key,Value>>> hashLfu_;  // 保存各个Lfu缓存的vecotr
};