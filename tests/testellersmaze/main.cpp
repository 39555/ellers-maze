#include <__ranges/join_view.h>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "ellers-maze.h"
#include "unistd.h"
#include "ranges"

void test_line_type_switching(){

    auto gen = ellersmaze(10);

    while( true) {
       line l =  gen();
        std::cout << ( static_cast<int>(l.type()) == 1 ? "  horizontal  " : "  vertical "  ) << std::endl;
        sleep(1) ;
    }
}

void test_print_lines()
{
    // | 1 | 2 | 3 | 4 |
    //  ___ ___ ___ ___
    // ^~ 
    int width = 30;
    auto m = maze(width);
    //auto gen = ellersmaze(width);
    while(true) {
       auto l  = m.gen_v_line();
       auto setv = m.cells_and_its_set; 
       auto l2 = m.gen_h_line();
       auto seth = m.cells_and_its_set;
        //std::cout << ' ';
        std::cout << "|";
        for(size_t i{0}; i < l.size(); i++){
            auto ch = l[i];
            std::cout   << "   " << (ch == 1 ? '|' : ' ' ) ;
        }
        std::cout << "|";
        std::cout << std::endl;
        std::cout << "|";
        //std::cout << ' ' <<  std::to_string(std::get<cell_i>(setv[0])) << ' ';
        for(size_t i{0}; i < l.size(); i++)
        {
            auto ch = l[i];
            auto ch2 = l2[i];
            auto n = (ch2 == 1 ? '_' : ' ');
            auto n2 = n;
            if(false) {

                    n = std::to_string(std::get<cell_i>(setv[i+1]))[0];
                    n2 = std::to_string(std::get<cell_i>(seth[i+1]))[0];
            }
            std::string str = {(ch2 == 1 ? "___" : "   ")};
            str[0] = n;
            str[2] = n2;
            std::cout  << str.c_str() << (ch == 1 ? '|' : ' ' ) ;
        }
        
         std::cout << "|";
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
    int width = 50;
    auto maze_= maze(width );

    while( true ) {
        auto res1 = maze_.gen_v_line();
        auto res2 = maze_.gen_h_line();

        for (size_t i = 0; i < width ; i++)
        {
            auto c1 = res1[i];
            auto c2 = res2[i];
            std::cout << (c1 == 1 ? '|' : ' ' ) << (c2 == 1 ? '_' : ' ' );
        }
         std::cout << std::endl;
        
        sleep(1);
    }
}

//free_list<int> make_free_list()
//{
    //free_list<int> lst;
    //lst.reserve(10);
   // for (size_t i = 0; i < 10; i++){
     //   lst.insert(i);
  //  }
    //return lst;
//}
/*
void test_erase()
{
    auto lst = make_free_list();
    lst.erase(lst.begin());
    lst.erase(lst.begin());
    lst.erase(lst.begin() + 5);
    lst.erase(lst.begin() + 2);
    auto it = std::ranges::find(lst, 5);
    lst.erase(it);
}
*/
void test_free_list_iterator()
{
   // auto lst = make_free_list();
    

    
    //std::cout <<  *(++lst.begin());
    //std::cout << *(lst.begin() +1);
    
    //for(auto i : lst)
    {
        //std::cout << i ;
    }
    std::vector<int> v{1,2,3,4,6,5,6,6};
    //std::cout << v[5] ;
    
    //v.erase(v.begin()+4);
    //auto ptr = &(*v.begin());

    //std::cout << *ptr;
}

int main()
{
    test_print_lines();
    //test_free_list_iterator();
    return 0;
}
