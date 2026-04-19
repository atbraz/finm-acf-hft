cxx := "/opt/homebrew/opt/llvm/bin/clang++"

build:
    cmake -B build -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build

run: build
    ./build/hft_phase3

asan:
    cmake -B build -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build

clean:
    rm -rf build
