#include <chrono>
#include <cstring>
#include <iostream>

#include "kernels.hpp"

// Runs a single kernel in a tight loop for `seconds` wall-clock seconds.
// Sized for n=512 so working set fits in L2 but not L1.
int main() {
    constexpr int n = 512;
    constexpr double seconds = 8.0;

    auto* A = alloc_aligned<double>(n * n);
    auto* B = alloc_aligned<double>(n * n);
    auto* C = alloc_aligned<double>(n * n);

    // non-zero inputs so the compiler can't constant-fold the work away
    for (int i = 0; i < n * n; ++i) {
        A[i] = static_cast<double>(i) * 0.001;
        B[i] = static_cast<double>(n * n - i) * 0.001;
    }

    using clock = std::chrono::steady_clock;
    auto deadline = clock::now() + std::chrono::duration<double>(seconds);

    std::cout << "profiling multiply_mm_naive and multiply_mm_tiled for "
              << seconds << "s (n=" << n << ")...\n";
    std::cout.flush();

    long iters = 0;
    while (clock::now() < deadline) {
        multiply_mm_naive(A, n, n, B, n, n, C);
        multiply_mm_tiled(A, n, n, B, n, n, C);
        ++iters;
    }

    std::cout << "done (" << iters << " iterations)\n";

    free_aligned(A);
    free_aligned(B);
    free_aligned(C);
}
