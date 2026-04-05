#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

#include "kernels.hpp"

// Returns {mean_ms, stddev_ms} over `runs` executions of f().
template <typename Func>
std::pair<double, double> time_stats(Func&& f, int runs = 100) {
    using clock = std::chrono::high_resolution_clock;
    std::vector<double> times;
    times.reserve(runs);
    for (int i = 0; i < runs; ++i) {
        auto t0 = clock::now();
        f();
        auto t1 = clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    double mean = std::accumulate(times.begin(), times.end(), 0.0) / runs;
    double sq = 0.0;
    for (double t : times)
        sq += (t - mean) * (t - mean);
    return {mean, std::sqrt(sq / runs)};
}

static void print_row(const char* name, int n, double mean, double stddev) {
    std::cout << std::left << std::setw(30) << name
              << std::right << std::setw(6) << n
              << std::setw(12) << std::fixed << std::setprecision(4) << mean
              << std::setw(12) << std::fixed << std::setprecision(4) << stddev
              << "\n";
}

static void bench_mv(int n) {
    auto* mat = alloc_aligned<double>(n * n);
    auto* vec = alloc_aligned<double>(n);
    auto* result = alloc_aligned<double>(n);

    std::memset(mat, 0, n * n * sizeof(double));
    std::memset(vec, 0, n * sizeof(double));
    std::memset(result, 0, n * sizeof(double));

    auto [mean_row, sd_row] = time_stats([&] {
        multiply_mv_row_major(mat, n, n, vec, result);
    });
    auto [mean_col, sd_col] = time_stats([&] {
        multiply_mv_col_major(mat, n, n, vec, result);
    });

    print_row("mv_row_major", n, mean_row, sd_row);
    print_row("mv_col_major", n, mean_col, sd_col);

    free_aligned(mat);
    free_aligned(vec);
    free_aligned(result);
}

static void bench_mm(int n) {
    auto* A = alloc_aligned<double>(n * n);
    auto* B = alloc_aligned<double>(n * n);
    auto* C = alloc_aligned<double>(n * n);

    std::memset(A, 0, n * n * sizeof(double));
    std::memset(B, 0, n * n * sizeof(double));
    std::memset(C, 0, n * n * sizeof(double));

    auto [mean_naive, sd_naive] = time_stats([&] {
        multiply_mm_naive(A, n, n, B, n, n, C);
    });
    auto [mean_tb, sd_tb] = time_stats([&] {
        multiply_mm_transposed_b(A, n, n, B, n, n, C);
    });

    print_row("mm_naive", n, mean_naive, sd_naive);
    print_row("mm_transposed_b", n, mean_tb, sd_tb);

    free_aligned(A);
    free_aligned(B);
    free_aligned(C);
}

int main() {
    std::cout << std::left << std::setw(30) << "function"
              << std::right << std::setw(6) << "n"
              << std::setw(12) << "mean(ms)"
              << std::setw(12) << "stddev(ms)"
              << "\n"
              << std::string(60, '-') << "\n";

    for (int n : {64, 256, 512, 1024}) {
        bench_mv(n);
        bench_mm(n);
        std::cout << std::string(60, '-') << "\n";
    }

    return 0;
}
