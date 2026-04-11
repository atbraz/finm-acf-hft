cxx := "/opt/homebrew/opt/llvm/bin/clang++"

build:
    cmake -B build -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build

server: build
    ./build/hft_server

client: build
    ./build/hft_client

viz:
    uv run scripts/viz.py

clean:
    rm -rf build
