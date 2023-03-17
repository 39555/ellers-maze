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
    [[nodiscard]] size_t size() {return walls_container::size() - 1 ; }
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

struct free_list_sentinel{
};

using index_t = std::int32_t ;
enum class next_free_i: index_t { end_of_list = -1};



template<typename T>
struct free_list_iterator {
    template <typename> friend class free_list;
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = value_type*;
    using reference         = value_type&;  

    using free_list_value_type = std::variant<T, next_free_i>;
    using free_list_pointer = free_list_value_type*;
    free_list_iterator() = default;
    free_list_iterator(free_list_pointer ptr) : ptr_(ptr) {
        //ptr_ = 
        satisfy<forward>(ptr_);
    }
private:
    enum direction {forward = 1, backward = -1};
    free_list_pointer ptr_ = nullptr;
public:
    reference operator*() const { return std::get<T>(*ptr_);   }
    pointer operator-> ()       { return &std::get<T>(*ptr_);  }

    free_list_iterator& operator++() {
         ptr_++; 
         //ptr_ = 
         satisfy<forward>(ptr_);
         return *this; }  

    free_list_iterator& operator--() {
         ptr_--; 
         ptr_ = satisfy<backward>(ptr_);
         return *this; } 
    free_list_iterator& operator--(int) {
        free_list_iterator tmp = *this;
          --(*this); 
          return tmp; }

    free_list_iterator operator++(int) {
         free_list_iterator tmp = *this;
          ++(*this); 
          return tmp; }

     bool operator== (const free_list_sentinel ) const { return ptr_ == nullptr; };
     bool operator== (free_list_iterator const & other) const { return ptr_ == other.ptr_; };
     bool operator!=( free_list_iterator const & other ) const{   
        return !(*this == other);
    } 
    free_list_iterator& operator+=(difference_type n){
        free_list_pointer cur_ptr = ptr_;
        while(ptr_ - cur_ptr < n){
            ++(*this);
        }
        return *this;
    }
    free_list_iterator operator+(difference_type n){
        free_list_iterator new_{*this} ; new_+=n; return new_;
    }
    difference_type operator-(const free_list_iterator& right) { 
        free_list_pointer temp = std::max(ptr_, right.ptr_);
        free_list_pointer dest = std::min(ptr_, right.ptr_);
        difference_type i = 0;
        while(temp > dest){
            --temp;
            satisfy<backward>(temp);
            ++i;
        }
        return i;
        }
    free_list_iterator operator-(difference_type n){
        return *this + (-n);
    }
    free_list_iterator operator-=(difference_type n)
    {*this += -n; return *this;}

private:
    template<direction Direction>
    void satisfy(free_list_pointer& ptr) {
        while(ptr && ptr->index() == 1){ // skip free index
            if constexpr (Direction == direction::forward) ++ptr;
            else                                           --ptr;
         }
    }
    
};


template <typename T>
class free_list : private std::vector<std::variant<T
                                                 , next_free_i>>
{
        using underlying_type = std::variant<T,   next_free_i>;
public:
        using iter_type = free_list_iterator<T>;
        //using sentinel_type = free_list_sentinel;
private:
    using super         = std::vector<underlying_type>;  // or boost::small_vector
    using iterator = typename super::iterator;
    next_free_i first_free_ = next_free_i::end_of_list;
public:
    using super::reserve;
    using difference_type = typename super::difference_type;
    index_t insert(T elt){
        if (first_free_ != next_free_i::end_of_list){
            const index_t ind = static_cast<index_t>(first_free_);
            first_free_ = std::get<next_free_i>(super::operator[](static_cast<index_t>(first_free_)));
            super::operator[](ind) = elt;
            return ind;
        }
        else{
            super::push_back(std::variant<T, next_free_i>{elt});
            return static_cast<index_t>(super::size() - 1);
        }
    }
    void erase(iter_type where){
        *where.ptr_     = first_free_;
        first_free_ = static_cast<next_free_i>( super::size() - (&super::operator[](super::size()) - where.ptr_ ) );
    }
    
    void clear(){
        super::clear();
        first_free_ = next_free_i::end_of_list;
    }
public:
    size_t size() {
        return end() - begin();
    }
    iter_type begin(){ return  iter_type( super::data());}
    iter_type end()  { return  iter_type{super::data() + super::size()};}//{return sentinel_type{};}
};

static inline auto random_bool = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());

using cell_i = std::int32_t; 
constexpr cell_i invalid_cell = -1;

class maze{
    
    using set_i =  cell_i; // max(sets) = len(cells)
    static constexpr set_i invalid_set = -1;
    cell_i width_;
    std::ranges::iota_view<cell_i, cell_i>
    enumerate_cells = std::ranges::iota_view(cell_i{0}, width_);

    enum class next_cell_without_set : size_t {};
    std::vector< std::variant<set_i
                            , next_cell_without_set>>   // the index of next cell without set
                              cells_and_its_set;

    using set_type = free_list< cell_i >;
    std::vector<set_type> sets; 

    set_i& get_cell_set(cell_i cell){ return std::get<set_i>(cells_and_its_set[cell]);}
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
            set_i  set =      get_cell_set(cell);
            set_i& next_set = get_cell_set(cell+1);
            if(set == next_set || random_bool() /* build wall or not*/){
                result[cell] = wall;
            }
            else{
                pop_cell_from_its_set(cell+1);
                push_cell_to_set(set, cell+1);
            }
        }
        return result;
    }

    walls_container gen_h_line(){   
        
        auto result = walls_container(width_, not_wall);

        next_cell_without_set invalid_next = static_cast<next_cell_without_set>(invalid_cell);
        next_cell_without_set next = invalid_next ;
        for(auto set : sets)
        {
            bool bIsBuildWay{true};
            bool bStopBuildWay{false};
            if(set.size() <= 1) continue; // guaranteed way
            
            for(size_t i{0}; auto cell : set)
            {
                if((cell == invalid_cell) 
                || ( (i == set.size() - 1) && !bStopBuildWay ) ) continue;
            
                if(random_bool() /* build a wall or not */){
                    pop_cell_from_its_set(cell);
                    cells_and_its_set[cell] = next;
                    next = static_cast<next_cell_without_set>(cell);
                    if(!bIsBuildWay) bStopBuildWay = true;
                    result[cell] = wall;
                    continue;
                }
                else bIsBuildWay = false;
                ++i;
            }
        }
        // push cells without sets to unique set
        while(next != invalid_next ){
            cell_i cell = static_cast<cell_i>(next);
            auto empty_set = std::ranges::find_if(sets, [cell, this](auto& set){
                    // ! max O(n2) 
                    return set.size() == 0;
                    });
            auto dist = std::distance(sets.begin(), empty_set );
            next = std::get<next_cell_without_set>(cells_and_its_set[cell]) ;
            push_cell_to_set(dist, cell);
        }

        return result;
    }
private:
    void pop_cell_from_its_set(cell_i cell){
        auto& set = get_cell_set(cell);
        free_list_iterator<cell_i> finded = std::ranges::find(sets[set], cell);
        sets[set].erase(finded);
        set = invalid_set;
    }
    void push_cell_to_set(set_i set, cell_i cell) {
        sets[set].insert(cell);
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
            break;
        }
        case line_t::horizontal: {
            res = maze_.gen_h_line();
             break;
        }             
        }
        co_yield line{std::move(res), l};
        ++l;
    }
    
}
