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
    // aligned
    auto* A_a = alloc_aligned<double>(n * n);
    auto* B_a = alloc_aligned<double>(n * n);
    auto* C_a = alloc_aligned<double>(n * n);
    std::memset(A_a, 0, n * n * sizeof(double));
    std::memset(B_a, 0, n * n * sizeof(double));
    std::memset(C_a, 0, n * n * sizeof(double));

    // unaligned
    auto* A_u = new double[n * n]();
    auto* B_u = new double[n * n]();
    auto* C_u = new double[n * n]();

    int runs = (n >= 512) ? 10 : 100;

    auto [mean_naive_a, sd_naive_a] = time_stats([&] {
        multiply_mm_naive(A_a, n, n, B_a, n, n, C_a);
    }, runs);
    auto [mean_naive_u, sd_naive_u] = time_stats([&] {
        multiply_mm_naive(A_u, n, n, B_u, n, n, C_u);
    }, runs);
    auto [mean_tb_a, sd_tb_a] = time_stats([&] {
        multiply_mm_transposed_b(A_a, n, n, B_a, n, n, C_a);
    }, runs);
    auto [mean_tb_u, sd_tb_u] = time_stats([&] {
        multiply_mm_transposed_b(A_u, n, n, B_u, n, n, C_u);
    }, runs);
    auto [mean_tiled_a, sd_tiled_a] = time_stats([&] {
        multiply_mm_tiled(A_a, n, n, B_a, n, n, C_a);
    }, runs);
    auto [mean_tiled_u, sd_tiled_u] = time_stats([&] {
        multiply_mm_tiled(A_u, n, n, B_u, n, n, C_u);
    }, runs);

    print_row("mm_naive (aligned)", n, mean_naive_a, sd_naive_a);
    print_row("mm_naive (unaligned)", n, mean_naive_u, sd_naive_u);
    print_row("mm_transposed_b (aligned)", n, mean_tb_a, sd_tb_a);
    print_row("mm_transposed_b (unaligned)", n, mean_tb_u, sd_tb_u);
    print_row("mm_tiled (aligned)", n, mean_tiled_a, sd_tiled_a);
    print_row("mm_tiled (unaligned)", n, mean_tiled_u, sd_tiled_u);

    free_aligned(A_a);
    free_aligned(B_a);
    free_aligned(C_a);
    delete[] A_u;
    delete[] B_u;
    delete[] C_u;
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
