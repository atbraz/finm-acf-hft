cmake_minimum_required(VERSION 3.20)
project(hft_phase4 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# LTO produces bitcode .o files; pair them with matching llvm-ar/ranlib so the
# static archive index stays valid. Apple's /usr/bin/ar can't handle bitcode.
if(EXISTS "/opt/homebrew/opt/llvm/bin/llvm-ar")
    set(CMAKE_AR "/opt/homebrew/opt/llvm/bin/llvm-ar")
endif()
if(EXISTS "/opt/homebrew/opt/llvm/bin/llvm-ranlib")
    set(CMAKE_RANLIB "/opt/homebrew/opt/llvm/bin/llvm-ranlib")
endif()

# --- compile-time variant toggles -------------------------------------------
option(HFT_USE_RAW_PTR    "Store orders as raw pointers instead of unique_ptr/shared_ptr" OFF)
option(HFT_ALIGN_CACHE    "Use alignas(64) on hot-path structs"                            ON)
option(HFT_USE_POOL       "Use the hand-rolled ObjectPool for orders"                      ON)
option(HFT_BOOK_IMPL_FLAT "Use flat sorted-vector OrderBook (OFF -> std::multimap)"        ON)

set(HFT_DEFS
    HFT_USE_RAW_PTR=$<BOOL:${HFT_USE_RAW_PTR}>
    HFT_ALIGN_CACHE=$<BOOL:${HFT_ALIGN_CACHE}>
    HFT_USE_POOL=$<BOOL:${HFT_USE_POOL}>
    HFT_BOOK_IMPL_FLAT=$<BOOL:${HFT_BOOK_IMPL_FLAT}>
)

# --- shared library of core modules -----------------------------------------
add_library(hft_core STATIC
    src/MarketData.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderManager.cpp
    src/TradeLogger.cpp
)
target_include_directories(hft_core PUBLIC include)
target_compile_options(hft_core PUBLIC -Wall -Wextra -Wpedantic)
target_compile_definitions(hft_core PUBLIC ${HFT_DEFS})

target_compile_options(hft_core PUBLIC
    $<$<CONFIG:Release>:-O3 -march=native -flto>)
target_link_options(hft_core PUBLIC
    $<$<CONFIG:Release>:-flto>)

# --- executables ------------------------------------------------------------
add_executable(hft_app   src/main.cpp)
target_link_libraries(hft_app PRIVATE hft_core)

add_executable(hft_bench bench/bench_main.cpp)
target_link_libraries(hft_bench PRIVATE hft_core)

# --- tests (Catch2 v3 via FetchContent) ------------------------------------
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(hft_test test/test_latency.cpp)
target_link_libraries(hft_test PRIVATE hft_core Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(hft_test)
