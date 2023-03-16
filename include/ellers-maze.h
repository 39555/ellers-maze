#pragma once
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


enum class line_t : std::int8_t {
      vertical =   0 // false bit
    , horizontal = 1 // true  bit
};
inline line_t& operator++(line_t& l){
    l = static_cast<line_t>(1 - static_cast<int>(l));
    return l;
};

using wall_t =    bool;
static inline constexpr bool wall = true ;
using walls_container = std::vector<bool>;

struct line : private walls_container{

    template<std::convertible_to<wall_t>...Args>
    explicit line(line_t type, Args... walls_init_list)
    : walls_container{static_cast<wall_t>(type), static_cast<wall_t>(walls_init_list)...} {
    }
    explicit line(size_t width, line_t type): walls_container(width+1, false) {
        (*this)[0] = static_cast<wall_t>(type );  
    }
    explicit line(): walls_container{static_cast<wall_t>(line_t::vertical)}{}
    explicit line(walls_container container, line_t type): walls_container() {
        walls_container::reserve(container.size() + 1);
        (*this)[0] = static_cast<wall_t>(type);
        walls_container::insert(std::next(walls_container::begin()), 
                    container.begin(), container.end()); 
    }
    [[nodiscard]] size_t length() {return walls_container::size();}
    [[nodiscard]] walls_container::iterator begin() noexcept { return std::next(walls_container::begin()); }
    [[nodiscard]] walls_container::iterator end  () noexcept { return walls_container::end();}
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


using cell_i = std::uint32_t;
using set_i = std::int32_t;
static constexpr set_i invalid_set = -1;
static auto random_bool = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());

inline walls_container v_line(cell_i width, std::vector<set_i> sets){
    auto result = walls_container(width, false);
    for(cell_i cell : std::ranges::iota_view(cell_i(0), width)){
        set_i set = sets[cell];
        set_i next_set = sets[cell+1];
        if(set != invalid_set && next_set != invalid_set){
            bool need_wall = random_bool();
			if(set== next_set || need_wall){
                result[cell] = true;
				continue;
			}
			else{
                sets[cell+1] = set;
			}
        }
    }
    return result;
}

inline walls_container h_line(cell_i width, std::vector<set_i> sets_indexes){   
    using cell_set_t = std::vector<cell_i>;
    auto cells = std::ranges::iota_view(cell_i{0}, width);
    constexpr int invalid_cell = -1;

    auto result = walls_container(width, false);

    std::vector<cell_set_t> sets;
    for(cell_i cell : cells ){
        auto set = sets_indexes[cell];
        // warning if sets is empty
        if(set != invalid_set) sets[set].push_back(cell);
    }
    for(auto set : sets)
    {
        bool bIsBuildWay{true};
		bool bStopBuildWay{false};
        static constexpr set_i marked = -2;  
        if(set.size() <= 1) continue; // guaranteed way
        for(auto cell : set)
        {
            if(cell == *std::prev(set.end())) if(!bStopBuildWay) continue;
        
            if(random_bool()){
                sets_indexes[cell] = marked;
                if(!bIsBuildWay) bStopBuildWay = true;
                result[cell] = true;
                continue;
            }
            else bIsBuildWay = false;
        }

        for(cell_i cell : cells | std::ranges::views::filter([&sets_indexes](cell_i c){ 
                                    return sets_indexes[c] == marked;}))
        {
            for(cell_i j : cells ){
                if(sets_indexes[j] == invalid_set)  sets_indexes[cell] = j;
            }
        }
	}		
}

inline line_generator ellersmaze(cell_i width) noexcept{
    line_t l = line_t::vertical;
    auto sets = std::vector<int>(width) ;
    while (true){
        walls_container  res;
        switch (l) {
        case line_t::vertical: {
            res = v_line(width, sets);
            break;
        }
        case line_t::horizontal: {
            res = h_line(width, sets);
             break;
        }             
        }
        co_yield line{res, l};
        ++l;
    }
    
}
