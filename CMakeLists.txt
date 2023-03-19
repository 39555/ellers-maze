cmake_minimum_required(VERSION 3.25.2)

PROJECT(ellers-maze)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(include)
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${INC_DIRS})
target_compile_options(${PROJECT_NAME} INTERFACE -fcoroutines-ts)

add_executable(testellersmaze ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp)
target_include_directories(testellersmaze PUBLIC ${INC_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_hardlink "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}/compile_commands.json"
     COMMAND_ECHO STDOUT
    )