cmake_minimum_required(VERSION 3.20)
project(hft_phase5 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(EXISTS "/opt/homebrew/opt/llvm/bin/llvm-ar")
    set(CMAKE_AR "/opt/homebrew/opt/llvm/bin/llvm-ar")
endif()
if(EXISTS "/opt/homebrew/opt/llvm/bin/llvm-ranlib")
    set(CMAKE_RANLIB "/opt/homebrew/opt/llvm/bin/llvm-ranlib")
endif()

add_library(hft_core STATIC src/OrderBook.cpp)
target_include_directories(hft_core PUBLIC include)
target_compile_options(hft_core PUBLIC -Wall -Wextra -Wpedantic)
target_compile_options(hft_core PUBLIC
    $<$<CONFIG:Release>:-O3 -march=native -flto>)
target_link_options(hft_core PUBLIC
    $<$<CONFIG:Release>:-flto>)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(hft_bench bench/bench_main.cpp)
target_link_libraries(hft_bench PRIVATE hft_core)

add_executable(hft_stream bench/stream_main.cpp)
target_link_libraries(hft_stream PRIVATE hft_core)

add_executable(hft_test test/test_orderbook.cpp)
target_link_libraries(hft_test PRIVATE hft_core Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(hft_test)
