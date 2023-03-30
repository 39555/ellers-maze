#pragma once

#include <vector>
#include "stdint.h"
#include <coroutine>
#include <ranges>
#include <random>
#include <concepts>
#include <variant>

namespace ellrs {

// which line is. vertical or horizontal.
enum class line_t : std::int8_t {
      vertical =   0 
    , horizontal = 1 
};

inline line_t& operator++(line_t& l){
l = static_cast<line_t>(1 - static_cast<int>(l));
return l;
};

// a simple rand based on uniform int distribution between 0-1 and std::default_random_engine
struct default_rand_bool{
        std::uniform_int_distribution<> distr_{0,1};
        std::default_random_engine      engine_{};
        bool operator()(){ return distr_(engine_);}

};

using wall_t =    bool;
static inline constexpr wall_t wall     = static_cast<wall_t>(true) ;
static inline constexpr wall_t not_wall = static_cast<wall_t>(false) ;

// a container of generated walls
using line = std::vector<bool>;  //  compressed bitset
static_assert(std::is_same_v<line::value_type , wall_t>
, "the type of wall's container must be the same as the wall_t type");

// a type that store a index of cell
using cell_i = size_t; 



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
class maze{
    using set_i =  cell_i; // max(sets) = len(cells)
    enum class next_cell_without_set : std::conditional_t<std::is_unsigned_v<cell_i>, std::make_signed_t<cell_i>, cell_i>
    { end_of_list = -1 };
    // an iter over cells
    static constexpr auto cells = [](auto from, auto to) {return std::views::iota(static_cast<cell_i>(from), static_cast<cell_i>(to));};

    // store index to set in sets container
    std::vector< std::variant<set_i
                            , next_cell_without_set>>   
                              cells_and_its_set;
    // put cells by sets
    using set_type = std::vector< cell_i >;
    std::vector<set_type> sets_;  
    std::vector<cell_i> cached_empty_sets_{};
    cell_i width_;
    line_t current_state_ = line_t::vertical;
    
public:
    maze(cell_i width) noexcept(false) : width_(width)
    , cells_and_its_set(width)
    , sets_(width) {
        if(not (width > 0)) throw std::runtime_error("The width of maze must be greater then 0");
         // assign an original set to the each cell
        for(auto cell : cells(cell_i{0}, width_)) {
            sets_[cell].reserve(static_cast<size_t>(width/5));
            push_cell_to_set(cell, cell); // 1 cell -> 1 set, 2 cell -> 2 set
        }
     }
     
    template<typename T>
    requires requires(T rand_bool){ {rand_bool()} -> std::convertible_to<bool>;}
    std::pair<line_t, line> getline(T&& rand_bool) noexcept {
        std::pair<line_t, line> r{current_state_, line(width_)};
        switch (current_state_) {
            case line_t::vertical: {
                r.second = gen_v_line(rand_bool);
                break;
            }
            case line_t::horizontal: {
                r.second = gen_h_line(rand_bool);
                break;
            }             
        }
        ++current_state_;
        return r;
    }
    cell_i get_maze_width() const noexcept {return width_;} 


#pragma region algorithm_implementation
private:

    template<typename T>
    line gen_v_line(T&& rand_bool) noexcept {
        auto result = line(width_, not_wall);
        const auto gen = cells(0, width_-1 /*without a last cell*/) 
                        | std::views::transform([this, &rand_bool](auto cell){
            cell_i next_cell = cell + 1;
            set_i  set     = get_cell_set(cell);
            set_i next_set = get_cell_set(next_cell);
            if(set == next_set || rand_bool() /* build wall or not*/){
                return wall;
            }
            else{
                merge_sets(set, next_set);
                return not_wall;
            }
        });
        result.assign(gen.begin(), gen.end());
        result.push_back(wall) ; // a last right border
        return result;
    }
    template<typename T>
    line gen_h_line(T&& rand_bool) noexcept {   
        auto result = line(width_, not_wall);
        next_cell_without_set next 
                = next_cell_without_set::end_of_list; // using to find changed cells

        for(auto& set : sets_){
            bool way_exists           {false}; // way is already started or not
            bool way_finish           {false}; // this flag  indicates that we have built ..
            // ..a wall after a certain path because we will no longer need another path

            for(size_t i{0}; i < set.size(); i++){
                auto cell = set[i];
                if(( (i == set.size()-1 ) && ! way_exists ) ) continue; // keep guaranteed way
            
                if( way_finish  || rand_bool() /* build a wall or not */){
                     // pop cell from it`s set
                    *(set.begin() + i) = pop_back(set);  //  unordered erace 
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
            if(cached_empty_sets_.empty()){
                cache_empty_sets();
            }
            cell_i cell = static_cast<cell_i>(next);
            next = std::get<next_cell_without_set>(cells_and_its_set[cell]) ;
            push_cell_to_set(pop_back(cached_empty_sets_), cell);
        }
        return result;
    }
#pragma endregion algorithm_implementation

private:
    set_i& get_cell_set(cell_i cell) noexcept { return std::get<set_i>(cells_and_its_set[cell]);}
    void push_cell_to_set(set_i set, cell_i cell) noexcept {
        sets_[set].push_back(cell);
        cells_and_its_set[cell] = set;
    }
    void merge_sets(set_i a, set_i to_a) noexcept {
        for(cell_i cell : sets_[to_a]){
            cells_and_its_set[cell] = a;
        }
        sets_[a].insert(sets_[a].end(), sets_[to_a].begin(), sets_[to_a].end());
        sets_[to_a].clear();
    };
    void cache_empty_sets() noexcept {
        for(size_t i{0}; i < sets_.size(); i++){
            if(sets_[i].size() == 0){
                cached_empty_sets_.push_back(i);
            }
        }
    }
    template<typename T>
    auto pop_back(T& container) const noexcept -> typename T::value_type {
        set_i elt = std::move(container.back());
        container.pop_back();
        return elt;
    }
};


} // namespace ellrs