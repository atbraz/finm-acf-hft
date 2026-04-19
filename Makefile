CXX := /opt/homebrew/opt/llvm/bin/clang++

.PHONY: build run asan clean

build:
	cmake -B build -G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build build

run: build
	./build/hft_phase3

asan:
	cmake -B build -G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build build

clean:
	rm -rf build
