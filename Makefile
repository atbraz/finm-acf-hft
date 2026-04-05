CXX := /opt/homebrew/opt/llvm/bin/clang++

.DEFAULT_GOAL := build
.PHONY: build build-O1 build-O2 build-O3 \
        run run-O1 run-O2 run-O3 \
        test bench bench-O1 bench-O2 bench-O3 \
        report clean

define cmake_build
	cmake -B $(1) -G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_CXX_FLAGS="$(2)" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(1)
endef

build:
	$(call cmake_build,build,)

build-O1:
	$(call cmake_build,build-O1,-O1)

build-O2:
	$(call cmake_build,build-O2,-O2)

build-O3:
	$(call cmake_build,build-O3,-O3)

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
