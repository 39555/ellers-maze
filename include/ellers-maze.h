#pragma once
#include <__algorithm/ranges_all_of.h>
#include <__algorithm/ranges_lower_bound.h>
#include <__concepts/constructible.h>
#include <__concepts/convertible_to.h>
#include <__ranges/counted.h>
#include <__ranges/iota_view.h>
#include <cstdarg>
#include <cstdint>
#include <coroutine>
#include <exception>
#include <iterator>
#include <ranges>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_set>

enum class line_t : std::int8_t {
      vertical =   0 // false bit
    , horizontal = 1 // true  bit
};
inline line_t& operator++(line_t& l){
    l = static_cast<line_t>(1 - static_cast<int>(l));
    return l;
};

template<template<typename...> typename Container, typename T, typename ... Ts>
inline bool insert_sorted(Container<T, Ts...>& v, T n){
    auto insert_itr = std::lower_bound(std::begin(v), std::end(v), n);
    if(insert_itr == std::end(v) || *insert_itr != n){
        v.insert(insert_itr, n);
        return true;
    }
    return false;
}

using wall_t =    bool;
using walls_container = std::vector<bool>;

struct line : private walls_container{

    using walls_container::value_type;
    template<std::convertible_to<wall_t>...Args>
    explicit line(line_t type, Args... walls_init_list)
    : walls_container{static_cast<wall_t>(type), static_cast<wall_t>(walls_init_list)...} {
    }
    explicit line(size_t width, line_t type): walls_container(width+1, false) {
        (*this)[0] = static_cast<wall_t>(type );  
    }
    explicit line(): walls_container{static_cast<wall_t>(line_t::vertical)}{}
    explicit line(walls_container&& container, line_t type): walls_container() {
        walls_container::reserve(container.size() + 1);
        walls_container::push_back(static_cast<wall_t>(type));
        walls_container::insert(begin(), 
                    std::begin(container), std::end(container)); 
    }
    [[nodiscard]] size_t size() {return walls_container::size() - 1 ; }
    [[nodiscard]] walls_container::iterator begin() noexcept { return std::next(walls_container::begin()) ; }
    [[nodiscard]] walls_container::iterator end  () noexcept { return walls_container::end() ;}
    [[nodiscard]] walls_container::const_iterator cbegin() const noexcept { return std::next(walls_container::cbegin()) ; }
    [[nodiscard]] walls_container::const_iterator cend  () const noexcept { return walls_container::cend() ;}
    [[nodiscard]] line_t type() { bool first = *walls_container::begin(); return static_cast<line_t>( first ); }
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

    explicit operator bool(){
        fill();       
        return !h_.done();
    }

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
static inline constexpr wall_t wall     = static_cast<wall_t>(true) ;
static inline constexpr wall_t not_wall = static_cast<wall_t>(false) ;
using cell_i = std::int32_t; 
constexpr cell_i invalid_cell = -1;

class maze{
    
    using set_i =  std::int32_t;
    static constexpr set_i invalid_set = -1;
    cell_i width_;
    std::ranges::iota_view<cell_i, cell_i>
    enumerate_cells = std::ranges::iota_view(cell_i{0}, width_);
    std::vector<set_i> cells_and_its_set;
    std::vector<std::vector<cell_i>> sets; 
public:
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
            set_i  set =      cells_and_its_set[cell];
            set_i& next_set = cells_and_its_set[cell+1];
            //if(set != invalid_set && next_set != invalid_set){
                if(set == next_set || random_bool() /* build wall or not*/){
                    result[cell] = wall;
                }
                else{
                    pop_cell_from_its_set(cell+1);
                    push_cell_to_set(set, cell+1);
                }
            //}
        }
        return result;
    }

    walls_container gen_h_line(){   
        
        auto result = walls_container(width_, not_wall);

        for(auto set : sets)
        {
            bool bIsBuildWay{true};
            bool bStopBuildWay{false};
            if(set.size() <= 1) continue; // guaranteed way
            for(size_t i{0}; i < set.size(); i++)
            {
                auto cell = set[i];
                if((cell == invalid_cell) 
                || ( (i == set.size() - 1) && !bStopBuildWay ) ) continue;
            
                if(random_bool() /* build a wall or not */){
                    pop_cell_from_its_set(cell);
                    if(!bIsBuildWay) bStopBuildWay = true;
                    result[cell] = wall;
                    continue;
                }
                else bIsBuildWay = false;
            }

            for(cell_i cell : enumerate_cells | std::ranges::views::filter([this](cell_i c){ 
                                        return has_no_set(c);}))
            {
                auto empty_set = std::ranges::find_if(enumerate_cells, [this](cell_i set){
                    return std::ranges::all_of(cells_and_its_set, [set](set_i s){
                        return s != set;
                    });
                });
                push_cell_to_set(*empty_set, cell);
            }
        }		
        return result;
    }
    void pop_cell_from_its_set(cell_i cell){
        auto& set = cells_and_its_set[cell];
        *std::ranges::find(sets[set], cell) = invalid_cell;
        set = invalid_set;
    }
    void push_cell_to_set(set_i set, cell_i cell) {
        sets[set].push_back(cell);
        cells_and_its_set[cell] = set;
    }
    bool has_no_set(cell_i cell){
        return cells_and_its_set[cell] == invalid_set;
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
            break;
        }
        case line_t::horizontal: {
            res = maze_.gen_h_line();
             break;
        }             
        }
        line out = line{std::move(res), l};
        co_yield out;
        ++l;
    }
    
}
