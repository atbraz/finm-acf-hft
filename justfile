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

llvm := "/opt/homebrew/opt/llvm/bin"

# --- profiling ---

# ensure profile output directory exists
_profile-dir:
    @mkdir -p profile

# sysinfo: capture system details for reproducibility
profile-sysinfo: _profile-dir
    bash scripts/sysinfo.sh

# sample: wall-clock call-tree (no sudo needed)
profile-sample: build-O2 _profile-dir
    ./build-O2/profile &
    sleep 1 && sample profile 6 1 -file profile/sample.txt
    @echo "saved → profile/sample.txt"

# pgo: LLVM instrumented profiling — per-function call counts and block frequencies
_build-pgo:
    cmake -B build-pgo -G Ninja \
        -DCMAKE_CXX_COMPILER={{cxx}} \
        -DCMAKE_CXX_FLAGS="-O2 -fprofile-instr-generate -fcoverage-mapping" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF
    cmake --build build-pgo

profile-pgo: _build-pgo _profile-dir
    LLVM_PROFILE_FILE="build-pgo/kernels.profraw" ./build-pgo/profile
    {{llvm}}/llvm-profdata merge -o build-pgo/kernels.profdata build-pgo/kernels.profraw
    {{llvm}}/llvm-profdata show --function="multiply_m" --all-functions build-pgo/kernels.profdata \
        | grep -A6 "multiply_mm\|multiply_mv\|Counters\|Entries\|Function" > profile/pgo.txt
    @echo "saved → profile/pgo.txt"
    @cat profile/pgo.txt

# remarks: compiler vectorization and optimization diagnostics
profile-remarks: _profile-dir
    {{cxx}} -O3 -march=native -c src/kernels.cpp -I include \
        -Rpass=loop-vectorize \
        -Rpass-missed=loop-vectorize \
        -Rpass-analysis=loop-vectorize \
        -Rpass=loop-unroll \
        -Rpass=slp-vectorizer \
        -Rpass-missed=slp-vectorizer \
        -o /dev/null 2> profile/remarks.txt
    @echo "saved → profile/remarks.txt"
    @cat profile/remarks.txt

# time: BSD time -l across all opt levels — max RSS, page faults, wall time
profile-time: build-all _profile-dir
    @for dir in build build-O1 build-O2 build-O3; do \
        printf "\n=== %s ===\n" "$dir"; \
        /usr/bin/time -l "./$dir/bench" 2>&1 \
            | grep -E "real|user|sys|maximum resident|page reclaims|page faults"; \
    done

# dtrace: hardware event sampling — requires sudo
profile-dtrace: build-O2 _profile-dir
    @echo "running with sudo (hardware counters require root on macOS)..."
    sudo dtrace -n 'profile-997 /execname=="profile"/ { @fn[ustack(4)] = count(); } tick-8s { exit(0); }' -c './build-O2/profile' -o profile/dtrace.txt 2>/dev/null || true
    @echo "saved → profile/dtrace.txt"

# xctrace: Instruments CPU Counters — L1/L2 cache misses, branch mispredictions
profile-xctrace-counters: build-O2 _profile-dir
    rm -rf profile/xctrace_counters.trace
    xcrun xctrace record \
        --template 'CPU Counters' \
        --output profile/xctrace_counters.trace \
        --launch -- ./build-O2/profile
    @echo "saved → profile/xctrace_counters.trace"
    @echo "open with: open profile/xctrace_counters.trace"

# xctrace: Time Profiler — detailed call tree with timestamps
profile-xctrace-time: build-O2 _profile-dir
    rm -rf profile/xctrace_time.trace
    xcrun xctrace record \
        --template 'Time Profiler' \
        --output profile/xctrace_time.trace \
        --launch -- ./build-O2/profile
    @echo "saved → profile/xctrace_time.trace"
    @echo "open with: open profile/xctrace_time.trace"

# xctrace: run both Instruments profiles
profile-xctrace: profile-xctrace-counters profile-xctrace-time
    @echo "traces saved to profile/xctrace_*.trace"
    @echo "open in Instruments: open profile/xctrace_counters.trace"

# profile-all: run all CLI profilers (text output) + sysinfo
profile-all: profile-sysinfo profile-sample profile-pgo profile-remarks profile-time
    @echo "\nall profiles saved to profile/"

build-all: build build-O1 build-O2 build-O3

bench-all: build-all
    python3 scripts/bench_all.py

report:
    typst compile discussions/report.typ discussions/report.pdf

clean:
    rm -rf build build-O1 build-O2 build-O3 build-pgo
