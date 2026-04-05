CXX := /opt/homebrew/opt/llvm/bin/clang++

.DEFAULT_GOAL := build
LLVM := /opt/homebrew/opt/llvm/bin

.PHONY: build build-O1 build-O2 build-O3 build-all \
        run run-O1 run-O2 run-O3 \
        test bench bench-O1 bench-O2 bench-O3 bench-all \
        profile-sample profile-pgo profile-remarks profile-time profile-dtrace profile-all \
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

build-all: build build-O1 build-O2 build-O3

bench-all: build-all
	python3 scripts/bench_all.py

define cmake_build_pgo
	cmake -B build-pgo -G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_CXX_FLAGS="-O2 -fprofile-instr-generate -fcoverage-mapping" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=OFF
	cmake --build build-pgo
endef

profile-sample: build-O2
	./build-O2/profile &
	sleep 1 && sample profile 6 1 -file discussions/profile_sample.txt
	@echo "saved → discussions/profile_sample.txt"

profile-pgo:
	$(cmake_build_pgo)
	LLVM_PROFILE_FILE="build-pgo/kernels.profraw" ./build-pgo/profile
	$(LLVM)/llvm-profdata merge -o build-pgo/kernels.profdata build-pgo/kernels.profraw
	$(LLVM)/llvm-profdata show --function="multiply_m" --all-functions build-pgo/kernels.profdata \
		| grep -A6 "multiply_mm\|multiply_mv\|Counters\|Entries\|Function" > discussions/profile_pgo.txt
	@echo "saved → discussions/profile_pgo.txt"

profile-remarks:
	$(CXX) -O3 -march=native -c src/kernels.cpp -I include \
		-Rpass=loop-vectorize -Rpass-missed=loop-vectorize \
		-Rpass-analysis=loop-vectorize -Rpass=loop-unroll \
		-Rpass=slp-vectorizer -Rpass-missed=slp-vectorizer \
		-o /dev/null 2> discussions/profile_remarks.txt
	@echo "saved → discussions/profile_remarks.txt"

profile-time: build-all
	@for dir in build build-O1 build-O2 build-O3; do \
		printf "\n=== %s ===\n" "$$dir"; \
		/usr/bin/time -l "./$$dir/bench" 2>&1 \
			| grep -E "real|user|sys|maximum resident|page reclaims|page faults"; \
	done

profile-dtrace: build-O2
	@echo "running with sudo (hardware counters require root on macOS)..."
	sudo dtrace -n 'profile-997 /execname=="profile"/ { @fn[ustack(4)] = count(); } tick-8s { exit(0); }' -c './build-O2/profile' -o discussions/profile_dtrace.txt 2>/dev/null || true
	@echo "saved → discussions/profile_dtrace.txt"

profile-all: profile-sample profile-pgo profile-remarks profile-time
	@echo "all profiles saved to discussions/profile_*.txt"

report:
	typst compile discussions/report.typ discussions/report.pdf

clean:
	rm -rf build build-O1 build-O2 build-O3 build-pgo
