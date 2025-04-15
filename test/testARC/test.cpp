#include "LRU_Cache.h"
#include "LRU_K_Cache.h"
#include "LFU_Cache.h"
#include "LFU_Hash_Cache.h"
#include "ARC_Cache.h"

int main()
{
    // ArcLruPart<int,std::string> ArcLru(3,3,2);
    // ArcLru.put(1,"gch");
    // ArcLru.put(2,"th");
    // ArcLru.put(3,"wyh");
    // ArcLru.put(4,"xhj");
    // ArcLru.print();
    // std::cout<<std::endl;
    // ArcLru.put(5,"lyw");
    // ArcLru.put(6,"ztn");
    // ArcLru.print();
    // std::cout<<std::endl;
    // ArcLru.put(7,"wsh");
    // ArcLru.print();
    // std::cout<<std::endl;

    // ArcLfuPart<int ,std::string> ArcLfu(3,3,2);
    // ArcLfu.put(1,"gch"); 
    // ArcLfu.put(2,"th");
    // ArcLfu.put(3,"xhj");
    // ArcLfu.print();
    // ArcLfu.put(4,"wyh");
    // ArcLfu.print();
    // std::cout << std::endl;
    // ArcLfu.put(5,"lyw");
    // ArcLfu.put(6,"xy");
    // ArcLfu.print();
    // std::cout << std::endl;
    // ArcLfu.put(4,"wyh");
    // ArcLfu.put(5,"lyw");
    // ArcLfu.print();
    // std::cout << std::endl;
    // ArcLfu.put(7,"ztn");
    // ArcLfu.print();

    ArcCache<int ,std::string> Arc(3,3,2);
    Arc.put(1,"gch");
    Arc.put(2,"th");
    Arc.put(3,"xhj");
    Arc.print();

    Arc.put(4,"wyh");
    Arc.put(5,"lyw");
    Arc.put(6,"xy");
    Arc.print();

    Arc.put(1,"gch");
    Arc.print();

    Arc.put(1,"gch");
    Arc.print();

    std::cout << Arc.get(1) << std::endl;
    Arc.print();
    
    return 0;
}