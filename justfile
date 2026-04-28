cxx := "/opt/homebrew/opt/llvm/bin/clang++"
build_dir := "build"

build:
    cmake -B {{build_dir}} -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build {{build_dir}}

debug:
    cmake -B {{build_dir}} -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build {{build_dir}}

run: build
    ./{{build_dir}}/hft_app

test: debug
    ./{{build_dir}}/hft_test

bench: build
    ./{{build_dir}}/hft_bench --ticks 10000 --seed 42 --label reference --out results/reference.csv

bench-all: build
    bash bench/run_all.sh

asan:
    cmake -B {{build_dir}} -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build {{build_dir}}
    ./{{build_dir}}/hft_test

clean:
    rm -rf {{build_dir}} build-* results/*.csv
