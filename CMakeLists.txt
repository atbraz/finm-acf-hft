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

add_library(hft_core INTERFACE)
target_include_directories(hft_core INTERFACE include)
target_compile_options(hft_core INTERFACE -Wall -Wextra -Wpedantic)
target_compile_options(hft_core INTERFACE
    $<$<CONFIG:Release>:-O3 -march=native -flto>)
target_link_options(hft_core INTERFACE
    $<$<CONFIG:Release>:-flto>)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

include(CTest)
include(Catch)
