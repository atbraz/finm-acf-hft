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
    std::cout << std::left << std::setw(36) << name
              << std::right << std::setw(6) << n
              << std::setw(12) << std::fixed << std::setprecision(4) << mean
              << std::setw(12) << std::fixed << std::setprecision(4) << stddev
              << "\n";
}

static void bench_mv(int n) {
    // aligned
    auto* mat_a = alloc_aligned<double>(n * n);
    auto* vec_a = alloc_aligned<double>(n);
    auto* res_a = alloc_aligned<double>(n);
    std::memset(mat_a, 0, n * n * sizeof(double));
    std::memset(vec_a, 0, n * sizeof(double));
    std::memset(res_a, 0, n * sizeof(double));

    // unaligned
    auto* mat_u = new double[n * n]();
    auto* vec_u = new double[n]();
    auto* res_u = new double[n]();

    auto [mean_row_a, sd_row_a] = time_stats([&] {
        multiply_mv_row_major(mat_a, n, n, vec_a, res_a);
    });
    auto [mean_row_u, sd_row_u] = time_stats([&] {
        multiply_mv_row_major(mat_u, n, n, vec_u, res_u);
    });
    auto [mean_col_a, sd_col_a] = time_stats([&] {
        multiply_mv_col_major(mat_a, n, n, vec_a, res_a);
    });
    auto [mean_col_u, sd_col_u] = time_stats([&] {
        multiply_mv_col_major(mat_u, n, n, vec_u, res_u);
    });

    print_row("mv_row_major (aligned)", n, mean_row_a, sd_row_a);
    print_row("mv_row_major (unaligned)", n, mean_row_u, sd_row_u);
    print_row("mv_col_major (aligned)", n, mean_col_a, sd_col_a);
    print_row("mv_col_major (unaligned)", n, mean_col_u, sd_col_u);

    free_aligned(mat_a);
    free_aligned(vec_a);
    free_aligned(res_a);
    delete[] mat_u;
    delete[] vec_u;
    delete[] res_u;
}

static void bench_mm(int n) {
    auto* A = alloc_aligned<double>(n * n);
    auto* B = alloc_aligned<double>(n * n);
    auto* C = alloc_aligned<double>(n * n);
    std::memset(A, 0, n * n * sizeof(double));
    std::memset(B, 0, n * n * sizeof(double));
    std::memset(C, 0, n * n * sizeof(double));

    // skip naive and tiled for large n to avoid very long runtimes
    int runs = (n >= 512) ? 10 : 100;

    auto [mean_naive, sd_naive] = time_stats([&] {
        multiply_mm_naive(A, n, n, B, n, n, C);
    }, runs);
    auto [mean_tb, sd_tb] = time_stats([&] {
        multiply_mm_transposed_b(A, n, n, B, n, n, C);
    }, runs);
    auto [mean_tiled, sd_tiled] = time_stats([&] {
        multiply_mm_tiled(A, n, n, B, n, n, C);
    }, runs);

    print_row("mm_naive", n, mean_naive, sd_naive);
    print_row("mm_transposed_b", n, mean_tb, sd_tb);
    print_row("mm_tiled", n, mean_tiled, sd_tiled);

    free_aligned(A);
    free_aligned(B);
    free_aligned(C);
}

int main() {
    std::cout << std::left << std::setw(36) << "function"
              << std::right << std::setw(6) << "n"
              << std::setw(12) << "mean(ms)"
              << std::setw(12) << "stddev(ms)"
              << "\n"
              << std::string(66, '-') << "\n";

    for (int n : {64, 256, 512, 1024}) {
        bench_mv(n);
        bench_mm(n);
        std::cout << std::string(66, '-') << "\n";
    }

    return 0;
}
