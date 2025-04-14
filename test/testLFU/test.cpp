#include "LRU_Cache.h"
#include "LRU_K_Cache.h"
#include "LFU_Cache.h"

int main()
{
    LfuCache<int, std::string> cache(4,3);
    cache.put(1,"gch");
    cache.put(2,"th");
    cache.put(3,"xhj");
    cache.put(4,"wyh");
    cache.print();
    cache.getTotalNum();
    cache.getAverNum();
    std::cout<<std::endl;
    
    cache.put(1,"gch");
    cache.put(2,"th");
    cache.print();
    cache.getTotalNum();
    cache.getAverNum();
    std::cout<<std::endl;

    cache.put(5,"lyw");
    cache.print();
    cache.getTotalNum();
    cache.getAverNum();
    std::cout<<std::endl;

    cache.get(1);
    cache.get(2);
    cache.print();
    cache.getTotalNum();
    cache.getAverNum();
    std::cout<<std::endl;

    cache.get(4);
    cache.print();
    cache.getTotalNum();
    cache.getAverNum();
    
    return 0;
}