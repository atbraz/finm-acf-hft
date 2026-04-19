cmake_minimum_required(VERSION 3.20)
project(hft_phase3 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hft_phase3
    src/main.cpp
    src/market_snapshot.cpp
    src/order_manager.cpp
)

target_compile_options(hft_phase3 PRIVATE -Wall -Wextra)
