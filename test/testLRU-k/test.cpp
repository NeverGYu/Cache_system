#include "LRU_Cache.h"
#include "LRU_K_Cache.h"

int main()
{
    Lru_k_Cache<int,std::string> cache(3,3,3);

    cache.put(1,"gch");
    cache.put(2,"th");
    cache.put(3,"xhj");
    cache.print();
    cache.print_history();

    cache.get(1);
    cache.get(2);
    cache.get(3);
    cache.print();
    cache.print_history();

    cache.put(1,"gch");
    cache.put(2,"th");
    cache.put(3,"xhj");
    cache.print();
    cache.print_history();

    return 0;
}