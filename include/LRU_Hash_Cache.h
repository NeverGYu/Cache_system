#include "LRU_Cache.h"
#include <vector>

template<typename Key, typename Value>
class HashLru
{
    HashLru(int capcity, int sliceNum)
        : capcity_(capcity)
        , sliceNum_(sliceNum)
    {
        int sliceSize = std::ceil(capcity/sliceNum);    // 切片大小，向上取整，每个分片的大小，也代表分片LRU的容量
        for (int i = 0; i < sliceSize; i++)
        {
            LruSliceCache_.emplace_back(new LruCache<Key, Value>(sliceSize));
        }
    }

    void put(Key key, Value value)
    {
        // 获取key的hash值，并计算出对应的分片的索引
        int sliceIndex = Hash(key) % sliceNum_;
        return LruSliceCache_[sliceIndex]->put(key,value);
    }

    bool get(Key key, Value& value)
    {
        // 获取key的hash值，并计算出对应的分片索引
        size_t sliceIndex = Hash(key) % sliceNum_;
        return lruSliceCaches_[sliceIndex]->get(key, value);
    }

    Value get(Key key)
    {
        Value value;
        memset(&value, 0, sizeof(value));
        get(key, value);
        return value;
    }
    
private:
    // 将 key 转换为对应hash值
    size_t Hash(Key key)
    {
        std::hash<Key> hashFunc;
        return hashFunc(key);
    }

    int capcity_;    // 容量大小,也就是元素的个数
    int sliceNum_;   // 切片的数量
    std::vector<std::unique_ptr<LruCache<Key,Value>>> LruSliceCache_;
};