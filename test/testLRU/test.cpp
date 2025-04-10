#include "LRU_Cache.h"

int main()
{
    LruCache<int,std::string> cache(3);

    cache.put(1,"gch");
    cache.put(2,"th");
    cache.put(3,"xhj");
    cache.put(4,"wyh");
    cache.print();

    cache.put(2,"th");
    cache.print();

    cache.put(2,"lyw");
    cache.print();
    
    std::string ret = cache.get(2);
    std::cout << ret << std::endl;

    cache.remove(4);
    cache.print();

    return 0;
}