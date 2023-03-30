
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread> 

#include "ellrs/maze.hpp"



int main()
{
    constexpr int width = 30;
    std::cout << std::string((width)*3 + width +1, '_') << '\n'; // first floor
    auto maze = ellrs::maze(width);
    auto rand = ellrs::default_rand_bool{};
    while(true) {
        auto  [vtype, vertical  ] = maze.getline(rand);
        auto  [htype, horizontal] = maze.getline(rand);
        assert(vtype == ellrs::line_t::vertical  );
        assert(htype == ellrs::line_t::horizontal);
        assert(vertical.size() == horizontal.size() && vertical.size() == width);

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


