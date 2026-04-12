cmake_minimum_required(VERSION 3.20)
project(hft_phase2 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(hft_server src/hft_server.cpp src/StochasticProcesses.cpp)
add_executable(hft_client src/hft_client.cpp)

foreach(target hft_server hft_client)
    target_link_libraries(${target} PRIVATE Threads::Threads)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
endforeach()
