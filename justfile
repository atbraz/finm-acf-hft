cxx := "/opt/homebrew/opt/llvm/bin/clang++"

# configure and build all targets into `dir` with optional compiler flags
_build dir flags="":
    cmake -B {{dir}} -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_CXX_FLAGS="{{flags}}" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build {{dir}}

build:        (_build "build")
build-O1:     (_build "build-O1" "-O1")
build-O2:     (_build "build-O2" "-O2")
build-O3:     (_build "build-O3" "-O3")

run: build
    ./build/main

run-O1: build-O1
    ./build-O1/main

run-O2: build-O2
    ./build-O2/main

run-O3: build-O3
    ./build-O3/main

test: build
    ./build/tests

bench: build
    ./build/bench

bench-O1: build-O1
    ./build-O1/bench

bench-O2: build-O2
    ./build-O2/bench

bench-O3: build-O3
    ./build-O3/bench

report:
    typst compile discussions/report.typ discussions/report.pdf

clean:
    rm -rf build build-O1 build-O2 build-O3
