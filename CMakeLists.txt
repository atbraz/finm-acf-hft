cmake_minimum_required(VERSION 3.20)
project(main CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main src/main.cpp)
target_compile_options(main PRIVATE -Wall -Wextra)
