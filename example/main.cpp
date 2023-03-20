
#include <cassert>
#include <iostream>
#include <vector>
#include <thread> 

#include "ellers-maze.h"



int main()
{
    constexpr int width = 30;
    auto lgen = ellrs::coro::maze(width); // or just ellrs::mazer(width);


    std::cout << std::string((width)*3 + width +1, '_') << '\n'; // first floor
    while(true) {

        ellrs::line  vertical  =  lgen();
        ellrs::line  horizontal = lgen();
        assert(vertical.type() == ellrs::line_t::vertical);
        assert(horizontal.type() == ellrs::line_t::horizontal);
        assert(vertical.size() == horizontal.size() && vertical.size() == width );

        std::cout << '|'; // start line border
        // print first row of vertical line to make double size ~ >   |
        //                                                      ~ >   |
        for(const auto bit : vertical){
            std::cout  << "   " << (bit == ellrs::wall ? '|' : ' ' ) ;
            }
        std::cout  << '\n' << '|';

        for(size_t i{0}; i < width; i++){
            auto v_bit =   vertical[i];
            auto h_bit = horizontal[i];
            std::cout  << (h_bit == ellrs::wall ? "___" : "   ") << (v_bit == ellrs::wall ? '|' : ' ' ) ;
        }
        std::cout  << std::endl;
        
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    return 0;
}


