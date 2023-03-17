#include <__ranges/join_view.h>
#include <iostream>
#include <iterator>
#include "ellers-maze.h"
#include "unistd.h"
#include "ranges"

void test_line_type_switching(){

    auto gen = ellersmaze(10);

    while( gen ) {
       line l =  gen();
        std::cout << ( static_cast<int>(l.type()) == 1 ? "  horizontal  " : "  vertical "  ) << std::endl;
        sleep(1) ;
    }
}

void test_print_lines()
{
    auto gen = ellersmaze(10);
    while( gen ) {
       line l =  gen();
       line l2 = gen();
    
        for(size_t i{0}; i < 10; i++)
        {
            auto ch = *(l.begin() + i);
            auto ch2 = *(l2.begin() + i);

            std::cout << (ch == 1 ? '|' : ' ' )<< (ch == 1 ? '_' : ' ' );
        }
         std::cout << std::endl;
        sleep(1) ;
    }
}
template<typename T>  
void print_container(std::ostream& os, T& container, const std::string& delimiter)  
{  
    std::copy(std::begin(container),   
              std::end  (container),   
              std::ostream_iterator<typename T::value_type>(os, delimiter.c_str())); 
    os << std::endl;
}  

void test_line_container()
{
    std::vector<bool> data;
    data.resize(10, false);
    auto data1 = data;
    auto data3 = data;
    line l{std::move(data1), line_t::vertical};
    line l2{std::move(data3), line_t::vertical};
    line l3{std::move(data), line_t::vertical};
    print_container(std::cout, l, " ");
}
void test_maze()
{
    auto maze_= maze(50);

    while( true ) {
        auto res1 = maze_.gen_v_line();
        auto res2 = maze_.gen_h_line();

        for (size_t i = 0; i < 50; i++)
        {
            auto c1 = res1[i];
            auto c2 = res2[i];
            std::cout << (c1 == 1 ? '|' : ' ' ) << (c2 == 1 ? '_' : ' ' );
        }
         std::cout << std::endl;
        //print_container(std::cout, res1, " ");
        //print_container(std::cout, res2, " ");
        /*
        auto del = "#";
        for(auto& c : {res1, res2})
        {
            for(size_t i{0}; i < 10; i++)
            {
                auto ch =  c[i];
                std::cout << (ch == 1 ? del : " " ) << " ";
            }
            std::cout << std::endl;
            //del = "#";
        }
        
        */
        
        sleep(1);
    }
}
int main()
{
    test_maze();

    return 0;
}
