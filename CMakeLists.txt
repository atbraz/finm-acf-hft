cmake_minimum_required(VERSION 3.20)
project(hft_kernels CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Shared object library — compiled once, linked into all three executables
add_library(kernels OBJECT src/kernels.cpp)
target_include_directories(kernels PUBLIC include)
target_compile_options(kernels PRIVATE -Wall -Wextra)

add_executable(main src/main.cpp)
add_executable(tests tests/correctness.cpp)
add_executable(bench benchmarks/bench.cpp)
add_executable(profile src/profile_driver.cpp)

foreach(target main tests bench profile)
    target_link_libraries(${target} PRIVATE kernels)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
endforeach()
