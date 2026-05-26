cmake_minimum_required(VERSION 3.28.3)

project(CentralitiesUncertainGraphs)

set(CMAKE_CXX_STANDARD 20)

add_executable(main
    src/main.cpp
    src/uncertain_graph.cpp
)

# compile options: optimized with -03 or for debugging with -fsanitize
target_compile_options(main PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native>
    $<$<CONFIG:Debug>:-g -fsanitize=address>
)

# link options: we just need to specify whether -fsanitize is activated or not
target_link_options(main PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address>
)

# include the include directory with header files
target_include_directories(main PRIVATE include)