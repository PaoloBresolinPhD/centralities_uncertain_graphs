cmake_minimum_required(VERSION 3.28.3)
project(CentralitiesUncertainGraphs)
set(CMAKE_CXX_STANDARD 20)

# find OpenMP
find_package(OpenMP REQUIRED)

# create a library with the code shared among all executables
add_library(centralities_lib STATIC
    src/utils.cpp
)

# executable that loads a dataset, preprocesses it and saves it back
add_executable(preprocess_data
    src/preprocess_data.cpp
)

# executable that runs the mc + exact algorithm on an input dataset
add_executable(run_lin_experiments
    src/run_lin_experiments.cpp
    src/lin_algorithms.cpp
    src/uncertain_graph.cpp
    src/possible_world.cpp
)

# link the library to the executables
target_link_libraries(preprocess_data PRIVATE centralities_lib)
target_link_libraries(run_lin_experiments PRIVATE centralities_lib)

# iterate over all executables
foreach(target centralities_lib preprocess_data run_lin_experiments)

    # compile options: optimized with -03 or for debugging with -fsanitize
    target_compile_options(${target} PRIVATE
        $<$<CONFIG:Release>:-O3 -march=native>
        $<$<CONFIG:Debug>:-g -fsanitize=address>
    )

    # link options: we just need to specify whether -fsanitize is activated or not
    target_link_options(${target} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
    )

    # include the include directory with header files
    target_include_directories(${target} PUBLIC include)

    # link OpenMP
    target_link_libraries(${target} PRIVATE OpenMP::OpenMP_CXX)

endforeach()