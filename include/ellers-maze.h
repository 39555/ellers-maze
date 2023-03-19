#pragma once

#include <vector>
#include "stdint.h"
#include <coroutine>
#include <ranges>
#include <random>


namespace ellrs {

// which line is. vertical or horizontal.
enum class line_t : std::int8_t {
      vertical =   0 // false bit
    , horizontal = 1 // true  bit
};

namespace details {

    inline line_t& operator++(line_t& l){
    l = static_cast<line_t>(1 - static_cast<int>(l));
    return l;
    };

    // a simple rand based on uniform int distribution between 0-1 and std::default_random_engine
    inline bool default_rand_bool() {
    static auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());
    return gen();
    }
}

using wall_t =    bool;
static inline constexpr wall_t wall     = static_cast<wall_t>(true) ;
static inline constexpr wall_t not_wall = static_cast<wall_t>(false) ;

using walls_container = std::vector<bool>;  //  compressed bitset
static_assert(std::is_same_v<walls_container::value_type , wall_t>
, "the type of wall's container must be the same as the wall_t type");

/* A line is a simple wrapper of vector<bool> or other std container from walls_container typedef.
   It is a sequence of bits 0,1 that represents a wall == 1(true) and a way == 0(false).
   A line needs to store a current line type(line_t) in a first bit, so
   '.begin()' starts from the second element.
   And the method '.type()' returns which type this line is: vertical or horizontal.
*/
struct line : private walls_container{
    using walls_container::value_type;
    using walls_container::reference;
    ///
    // construct from an init list of bits
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
    ///
    [[nodiscard]] reference operator[](size_t n){
        return walls_container::operator[](n+1);
    }
    [[nodiscard]] size_t size() const {return walls_container::size() - 1 ; }
    [[nodiscard]] walls_container::iterator begin() noexcept { return std::next(walls_container::begin()) ; }
    [[nodiscard]] walls_container::iterator end  () noexcept { return walls_container::end() ;}
    [[nodiscard]] walls_container::const_iterator begin() const noexcept { return std::next(walls_container::cbegin()) ; }
    [[nodiscard]] walls_container::const_iterator end  () const noexcept { return walls_container::cend() ;}
    // return a type of a maze line
    [[nodiscard]] line_t type() { wall_t first = *walls_container::begin(); return static_cast<line_t>( first ); }
};

// a type that store a index of cell
using cell_i = std::int32_t; 

/*  the main of Eller's algorithm
    The result  of generation just is an array of bits, 
    so you can use it in many cases.
    A generation of a maze must be starts with a generation 
    of a vertical line by 'gen_v_line'
    and then a horizontal buy 'gen_h_line'. But while using a result horizontal line must be first
                           |   |   |   |
                        _1_|_2_|_3_|_4_|
     use a horisontal   ~^ ^~  and then use a vertical  
*/  
template<typename RandomBoolProvider = decltype(&details::default_rand_bool)>
requires requires(RandomBoolProvider& rand){ {rand()} -> std::convertible_to<bool>;}
class mazer{
public:
    using random_type = RandomBoolProvider;
private:
    random_type random_bool_;
    cell_i width_;

    using set_i =  cell_i; // max(sets) = len(cells)
    enum class next_cell_without_set : std::conditional_t<std::is_unsigned_v<cell_i>, std::make_signed<cell_i>, cell_i>
    { end_of_list = -1 }; // the index of next cell without set

    // store index to set in sets container
    std::vector< std::variant<set_i
                            , next_cell_without_set>>   
                              cells_and_its_set;
    // put cells by sets
    using set_type = std::vector< cell_i >;
    std::vector<set_type> sets;  
public:
    mazer(cell_i width, RandomBoolProvider random_bool = &details::default_rand_bool) : random_bool_(random_bool), width_(width)
    , cells_and_its_set(width)
    , sets(width) {
        if(! (width > 0)) throw std::runtime_error("The width of maze must be greater then 0");
         // assign an original set to the each cell
        for(auto cell : std::ranges::iota_view(cell_i{0}, width_)) {
            sets[cell].reserve(static_cast<size_t>(width/2));
            push_cell_to_set(cell, cell); // 1 cell -> 1 set, 2 cell -> 2 set
        }
     }
     cell_i get_maze_width() {return width_;} 
 #pragma region algorithm_implementation
    walls_container gen_v_line(){
        auto result = walls_container(width_, not_wall);
        for(cell_i cell{0}; cell < width_-1; cell++){
            cell_i next_cell = cell + 1;
            set_i  set =      get_cell_set(cell);
            set_i next_set = get_cell_set(next_cell);
            if(set == next_set || random_bool_() /* build wall or not*/){
                result[cell] = wall;
            }
            else{
                merge_sets(set, next_set);
            }
        }
        result.back() = wall;
        return result;
    }

    walls_container gen_h_line(){   
        auto result = walls_container(width_, not_wall);
        next_cell_without_set next 
                = next_cell_without_set::end_of_list; // using to find changed cells

        for(auto& set : sets){
            bool way_exists           {false}; // way is already started or not
            bool way_finish           {false}; // this flag  indicates that we have built ..
            // ..a wall after a certain path because we will no longer need another path

            for(size_t i{0}; i < set.size(); i++){
                auto cell = set[i];
                if(( (i == set.size()-1 ) && ! way_exists ) ) continue; // keep guaranteed way
            
                if( way_finish  || random_bool_() /* build a wall or not */){

                     // pop cell from it`s set
                    *(set.begin() + i) = std::move(set.back());   // an unordered erace 
                    set.pop_back();
                    cells_and_its_set[cell] = next;
                    next = static_cast<next_cell_without_set>(cell);
                    if(way_exists) way_finish = true;

                    result[cell] = wall;
                }
                else way_exists = true;
            }
        }
        // push cells without sets to unique set
        while(next != next_cell_without_set::end_of_list){
            auto empty_set = std::ranges::find_if(sets, [](const auto& set){
                    return set.size() == 0;
                    });
            cell_i cell = static_cast<cell_i>(next);
            next = std::get<next_cell_without_set>(cells_and_its_set[cell]) ;
            push_cell_to_set(std::distance(sets.begin(), empty_set ), cell);
        }
        return result;
    }
#pragma endregion algorithm_implementation

private:
    void merge_sets(set_i a, set_i to_a) {
        for(cell_i cell : sets[to_a]){
            cells_and_its_set[cell] = a;
        }
        sets[a].insert(sets[a].end(), sets[to_a].begin(), sets[to_a].end());
        sets[to_a].clear();
    }
    set_i& get_cell_set(cell_i cell){ return std::get<set_i>(cells_and_its_set[cell]);}
    void push_cell_to_set(set_i set, cell_i cell) {
        sets[set].push_back(cell);
        cells_and_its_set[cell] = set;
    }
};


// using a co_yield to generate a maze
namespace coro {

//a co_yield line generator, using as return type from function 
struct linegen{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    class promise_type{
        friend linegen;
        line line_;
        std::exception_ptr exception_;
        public:
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { exception_ = std::current_exception(); }                                                                    
        void return_void() { }
        linegen get_return_object() 
        { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
        template <std::convertible_to<line> ...From>
        std::suspend_always yield_value(From&& ...from) 
        { line_ = { std::forward<From>(from)...}; return {}; }
    };
private:
    handle_type h_;
public:
    linegen(std::coroutine_handle<promise_type> h) : h_(h) {}
    ~linegen() { h_.destroy(); };
    line operator()(){
        h_();
        if (h_.promise().exception_)
            std::rethrow_exception(h_.promise().exception_);
        return std::move(h_.promise().line_);
    }
};

// a coroutine function that returns a maze`s lines generator
inline linegen maze(cell_i width) noexcept{
    using namespace details;
    line_t line_type = line_t::vertical;
    auto maze_ = mazer(width);
    for(;;){
        walls_container  res;
        switch (line_type) {
            case line_t::vertical: {
                res = maze_.gen_v_line();
                break;
            }
            case line_t::horizontal: {
                res = maze_.gen_h_line();
                break;
            }             
        }
        co_yield line{std::move(res), line_type};
        ++line_type;
    }
}
}

} // namespace ellrs