#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>

// Part 1 — baseline kernel declarations

void multiply_mv_row_major(const double* __restrict__ matrix, int rows, int cols,
                           const double* __restrict__ vector, double* __restrict__ result);

void multiply_mv_col_major(const double* __restrict__ matrix, int rows, int cols,
                           const double* __restrict__ vector, double* __restrict__ result);

void multiply_mm_naive(const double* __restrict__ matrixA, int rowsA, int colsA,
                       const double* __restrict__ matrixB, int rowsB, int colsB,
                       double* __restrict__ result);

void multiply_mm_transposed_b(const double* __restrict__ matrixA, int rowsA, int colsA,
                               const double* __restrict__ matrixB_transposed, int rowsB,
                               int colsB, double* __restrict__ result);

// Part 2 — optimized kernel variants
void multiply_mm_tiled(const double* __restrict__ matrixA, int rowsA, int colsA,
                       const double* __restrict__ matrixB, int rowsB, int colsB,
                       double* __restrict__ result, int tile = 64);

// Part 2 — aligned memory helpers
// alignment must be a power of 2 and count*sizeof(T) must be a multiple of alignment.
// Caller is responsible for calling free_aligned() on the returned pointer.
template <typename T>
T* alloc_aligned(std::size_t count, std::size_t alignment = 64) {
    // std::aligned_alloc requires size to be a multiple of alignment
    std::size_t byte_count = ((count * sizeof(T) + alignment - 1) / alignment) * alignment;
    void* ptr = std::aligned_alloc(alignment, byte_count);
    if (!ptr)
        throw std::bad_alloc{};
    return static_cast<T*>(ptr);
}

inline void free_aligned(void* ptr) {
    std::free(ptr);
}
