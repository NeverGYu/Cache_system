#include "LFU_Cache.h"
#include <iostream>

int main()
{
    LfuCache<int, std::string> cache(4);
    cache.put(1,"gch");
    cache.put(2,"th");
    cache.put(3,"xhj");
    cache.put(4,"wyh");
    cache.print();
    std::cout << std::endl;
    
    cache.put(1,"gch");
    cache.put(2,"th");
    cache.print();
    std::cout << std::endl;

    cache.put(5,"lyw");
    cache.print();
    std::cout << std::endl;

    cache.get(1);
    cache.get(2);
    cache.print();
    std::cout << std::endl;

    cache.get(4);
    cache.print();
    
    return 0;
}