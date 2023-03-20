# ellersmaze
## Table of Contents

- [About](#about)
- [Getting Started](#getting_started)
- [Implementation](#implementation)
- [Links](#links)

## About <a name = "about"></a>
This is my realization of Eller's Algorithm to generate an infinite maze as described by [Jamis Buck](https://weblog.jamisbuck.org/2010/12/29/maze-generation-eller-s-algorithm).
This library just generates an array of bools, each index of this array indicate a cell with a wall(true) or a not wall(false) witch you can use in many contexts from a console to the Unreal Engine. The [example](https://github.com/autogalkin/ellersmaze/blob/master/example/main.cpp) shows how I use it in a console:


https://user-images.githubusercontent.com/97976281/226358399-9aaf4e0f-7195-4ffa-bbd7-84ae50b0ced5.mov


## Getting Started <a name = "getting_started"></a>

The library provides a c++20 coroutine's co_yield wrapper:
```cpp
auto lgen = ellrs::coro::maze(width);
while(true) {
    ellrs::line  vertical  =  lgen();
    ellrs::line  horizontal = lgen();
}
```
Or just a simple class
```cpp
ellrs::mazer maze{width};
while(true) {
ellrs::line  vertical  =  maze.gen_v_line();
ellrs::line  horizontal = maze.gen_h_line();
}
```

You must start generation with a vertical line, but while using a result horizontal line must be first:
```
                           |   |   |   |
                        _1_|_2_|_3_|_4_|
    print a horisontal ~^  ^~  and then print a vertical wall.
```

To check a type of line you can use 'line::type()':
```cpp
ellrs::line line{ellrs::line_t::vertical};
assert(line.type() == ellrs::line_t::vertical);
```

Also you can use your own random bool generator to generate a more interested result:
```cpp
bool randomBool() {
   return rand() > (RAND_MAX / 2);
}
ellrs::mazer maze{width, &randomBool};

```
## Implementation <a name = "implementation"></a>
The vertical line generator:
```cpp
bool wall = true;
bool not_wall = false;
std::vector<bool> gen_v_line(){
    auto result = std::vector<bool>(width_, not_wall);
    for(cell_i cell{0}; cell < width_-1; cell++){ // without the last cell
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
    result.back() = wall; // a last right border
    return result;
}
```
And the horizontal walls generator:
```cpp
enum class next_cell_without_set : int32_t {end_of_list = -1}; // using to find changed cells
std::vector<bool> gen_h_line(){   
    auto result = std::vector<bool>(width_, not_wall);
    next_cell_without_set next 
            = next_cell_without_set::end_of_list;
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
```
## Links <a name = "links"></a>
- Thank you [Jamis Buck](https://weblog.jamisbuck.org/2010/12/29/maze-generation-eller-s-algorithm) for describe the algorithm
