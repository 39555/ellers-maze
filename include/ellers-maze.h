#pragma once
#include <__algorithm/ranges_all_of.h>
#include <__algorithm/ranges_lower_bound.h>
#include <__concepts/constructible.h>
#include <__concepts/convertible_to.h>
#include <__ranges/counted.h>
#include <__ranges/iota_view.h>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <coroutine>
#include <exception>
#include <iostream>
#include <iterator>
#include <ranges>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>

enum class line_t : std::int8_t {
      vertical =   0 // false bit
    , horizontal = 1 // true  bit
};
inline line_t& operator++(line_t& l){
    l = static_cast<line_t>(1 - static_cast<int>(l));
    return l;
};


using wall_t =    bool;
using walls_container = std::vector<bool>;
static inline constexpr wall_t wall     = static_cast<wall_t>(true) ;
static inline constexpr wall_t not_wall = static_cast<wall_t>(false) ;

struct line : private walls_container{
    static_assert(std::is_same_v<walls_container::value_type , wall_t>, "the type of wall's container must be the same as the wall_t type");
    using walls_container::value_type;
    using walls_container::reference;

    template<std::convertible_to<wall_t>...Args>
    explicit line(line_t type, Args... walls_init_list)
    : walls_container{static_cast<wall_t>(type), static_cast<wall_t>(walls_init_list)...} {
    }
    explicit line(size_t width, line_t type): walls_container(width+1, not_wall) {
        (*this)[0] = static_cast<wall_t>(type );  
    }
    explicit line(): walls_container{static_cast<wall_t>(line_t::vertical)}{}
    explicit line(walls_container&& container, line_t type): walls_container() {
        walls_container::reserve(container.size() + 1);
        walls_container::push_back(static_cast<wall_t>(type));
        walls_container::insert(begin(), 
                    std::begin(container), std::end(container)); 
    }
    [[nodiscard]] reference operator[](size_t n){
        return walls_container::operator[](n+1);
    }
    [[nodiscard]] size_t size() const {return walls_container::size() - 1 ; }
    [[nodiscard]] walls_container::iterator begin() noexcept { return std::next(walls_container::begin()) ; }
    [[nodiscard]] walls_container::iterator end  () noexcept { return walls_container::end() ;}
    [[nodiscard]] walls_container::const_iterator cbegin() const noexcept { return std::next(walls_container::cbegin()) ; }
    [[nodiscard]] walls_container::const_iterator cend  () const noexcept { return walls_container::cend() ;}
    [[nodiscard]] line_t type() { wall_t first = *walls_container::begin(); return static_cast<line_t>( first ); }
};


struct line_generator{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type{
        line line_;
        std::exception_ptr exception_;
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { exception_ = std::current_exception(); }                                                                    
        void return_void() { }
        line_generator get_return_object() 
        { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
        template <std::convertible_to<line> From>
        std::suspend_always yield_value(From&& from) 
        { line_ = std::forward<From>(from); return {}; }
    };

private:
    handle_type h_;
    bool full_ = false;
public:
    line_generator(std::coroutine_handle<promise_type> h) : h_(h) {}
    ~line_generator() { h_.destroy(); };

   // explicit operator bool(){
   //     fill();       
  //      return !h_.done();
   // }

    line operator()(){
        fill();
        full_ = false;
        return std::move(h_.promise().line_);
    }
 
    void fill(){
        if (!full_){
            h_();
            if (h_.promise().exception_)
                std::rethrow_exception(h_.promise().exception_);
            full_ = true;
        }
    }

    [[nodiscard]] const line& get() const{
        return h_.promise().line_;
    }

    operator const line&() const{
        return get();
    }
};

static inline auto random_bool = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());

using cell_i = std::int32_t; 

class maze{
    
    using set_i =  cell_i; // max(sets) = len(cells)
    static constexpr set_i invalid_set = -1;
    cell_i width_;
    std::ranges::iota_view<cell_i, cell_i>
    enumerate_cells = std::ranges::iota_view(cell_i{0}, width_);
public:
    enum class next_cell_without_set : cell_i { end_of_list =-1 };
    std::vector< std::variant<set_i
                            , next_cell_without_set>>   // the index of next cell without set
                              cells_and_its_set;

    using set_type = std::vector< cell_i >;
    std::vector<set_type> sets; 

    set_i& get_cell_set(cell_i cell){ return std::get<set_i>(cells_and_its_set[cell]);}

    maze(cell_i width) : width_(width)
    , cells_and_its_set(width)
    , sets(width) {
        if(! (width > 0)) throw std::runtime_error("The width of maze must be greater then 0");
         // assign an original set to the each cell
        for(auto cell : enumerate_cells) {
            sets[cell].reserve(static_cast<size_t>(width/2));
            push_cell_to_set(cell, cell); // 1 cell -> 1 set, 2 cell -> 2 set
        }
     }
    walls_container gen_v_line(){
        auto result = walls_container(width_, not_wall);
        for(cell_i cell{0}; cell < width_-1; cell++){
            cell_i next_cell = cell + 1;

            set_i  set =      get_cell_set(cell);
            set_i next_set = get_cell_set(next_cell);
            if(set == next_set || random_bool() /* build wall or not*/){
                result[cell] = wall;
            }
            else{
                merge_sets(set, next_set);
            }
        }
        return result;
    }

    walls_container gen_h_line(){   
        
        auto result = walls_container(width_, not_wall);

        // using for finding changed cells
        next_cell_without_set next = next_cell_without_set::end_of_list;

        for(auto set : sets)
        {
            bool need_build_way       {true };
            bool way_is_already_built {false};

            for(auto it = set.begin(); it != set.end(); ++it)
            for(auto cell : set){
                if(( (cell ==  *std::prev(set.end())) && ! way_is_already_built ) ) continue; // keep guaranteed way
            
                if(! need_build_way  || random_bool() /* build a wall or not */){
                    
                     // pop cell from it`s set
                    *it= std::move(set.back());   // unordered erace 
                    set.pop_back();

                    cells_and_its_set[cell] = next;
                    next = static_cast<next_cell_without_set>(cell);

                    if(!need_build_way) way_is_already_built = true;

                    result[cell] = wall;
                }
                else need_build_way = false; // way exists now
            }
        }
        // push cells without sets to unique set
        while(next != next_cell_without_set::end_of_list){
            cell_i cell = static_cast<cell_i>(next);
            auto empty_set = std::ranges::find_if(sets, [cell, this](auto& set){
                    return set.size() == 0;
                    });
            next = std::get<next_cell_without_set>(cells_and_its_set[cell]) ;
            push_cell_to_set(std::distance(sets.begin(), empty_set ), cell);
        }
        return result;
    }
    void merge_sets(set_i a, set_i to_a) {
        for(cell_i cell : sets[to_a]){
            cells_and_its_set[cell] = a;
        }
        sets[a].insert(sets[a].end(), sets[to_a].begin(), sets[to_a].end());
        sets[to_a].clear();

    }
private:
    void push_cell_to_set(set_i set, cell_i cell) {
        sets[set].push_back(cell);
        cells_and_its_set[cell] = set;
    }
    };

inline line_generator ellersmaze(cell_i width) noexcept{
    line_t l = line_t::vertical;
    auto maze_ = maze(width);
    while ( true ){
        
        walls_container  res;
        switch (l) {
        case line_t::vertical: {
            res = maze_.gen_v_line();
            //maze_.debug_print_sets_numbers();
            break;
        }
        case line_t::horizontal: {
            res = maze_.gen_h_line();
            //maze_.debug_print_sets_numbers();
             break;
        }             
        }
        co_yield line{std::move(res), l};
        ++l;
    }
    
}
