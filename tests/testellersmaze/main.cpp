#include <iostream>
#include "ellers-maze.h"
#include "unistd.h"

void test_line_type_switching(){

    auto gen = ellersmaze(10);

    while( gen ) {
       line l =  gen();
        std::cout << ( static_cast<int>(l.type()) == 1 ? "  horizontal  " : "  vertical "  ) << std::endl;
        sleep(1) ;
    }
}

int main()
{
    line l{line_t::horizontal, 0, 0, 0, 1, 0, 1};
    line l2(5, line_t::horizontal);
    for(auto wall : l2)
    {
        std::cout << wall;
    }
    
    std::cout << std::endl << (bool)l.type();
    return 0;
}
