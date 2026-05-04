CXX := /opt/homebrew/opt/llvm/bin/clang++
BUILD_DIR := build

.PHONY: build debug run test bench asan clean

build:
	cmake -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR)

debug:
	cmake -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR)

run: build
	./$(BUILD_DIR)/hft_app

test: debug
	./$(BUILD_DIR)/hft_test

bench: build
	./$(BUILD_DIR)/hft_bench

asan:
	cmake -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR)
	./$(BUILD_DIR)/hft_test

clean:
	rm -rf $(BUILD_DIR) build-* results/*.csv
