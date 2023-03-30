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

The library provides a class with a one function:
```cpp
ellrs::maze maze{width};
auto rand = ellrs::default_rand_bool{}; // you can pass your own random bool generator
while(true) {
    auto  [ vtype, vertical ]   = maze.getline(rand);
    auto  [ htype, horizontal ] = maze.getline(rand);

    assert(vtype == ellrs::line_t::vertical);
    assert(htype == ellrs::line_t::horizontal);
}
```

You must start generation with a vertical line, but while using a result horizontal line must be first:
```
                           |   |   |   |
                        _1_|_2_|_3_|_4_|
    print a horisontal ~^  ^~  and then print a vertical wall.
```

Also you can use your own random bool generator to generate a more interested result:
```cpp
bool randomBool() {
   return rand() > (RAND_MAX / 2);
}
maze.getline(&randomBool);

```
## Implementation <a name = "implementation"></a>
The vertical line generator:
```cpp
bool wall = true;
bool not_wall = false;
std::vector<bool> gen_v_line(){
    auto result = std::vector<bool>(width_, not_wall);
    for(cell_i cell{0}; cell < width_-1; cell++){ // without a last cell
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
```
## Links <a name = "links"></a>
- Thank you [Jamis Buck](https://weblog.jamisbuck.org/2010/12/29/maze-generation-eller-s-algorithm) for describe the algorithm
