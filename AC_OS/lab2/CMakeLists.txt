cmake_minimum_required(VERSION 3.10)
project(ParallelImageProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PNG REQUIRED)

set(SOURCES
    main.cpp
    filters.cpp
    parallel_processor.cpp
)

add_executable(parallel_image_processor ${SOURCES})

target_link_libraries(parallel_image_processor PNG::PNG pthread)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(parallel_image_processor PRIVATE -Wall -Wextra -O2)
endif()



#_benchmark_
add_executable(benchmark benchmark.cpp filters.cpp parallel_processor.cpp)

target_link_libraries(benchmark PNG::PNG pthread)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(benchmark PRIVATE -Wall -Wextra -O2)
endif()
