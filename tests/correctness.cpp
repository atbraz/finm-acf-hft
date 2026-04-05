#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

#include "kernels.hpp"

static bool approx_eq(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

// Matrix: [[1, 2], [3, 4]] stored row-major = {1, 2, 3, 4}
// Vector: {5, 6}
// Expected: {1*5+2*6, 3*5+4*6} = {17, 39}
void test_multiply_mv_row_major() {
    const double mat[4] = {1, 2, 3, 4};
    const double vec[2] = {5, 6};
    double result[2] = {0, 0};

    multiply_mv_row_major(mat, 2, 2, vec, result);

    assert(approx_eq(result[0], 17.0) && "result[0] should be 17");
    assert(approx_eq(result[1], 39.0) && "result[1] should be 39");
    std::cout << "PASS test_multiply_mv_row_major\n";
}

// Same logical matrix [[1, 2], [3, 4]] stored col-major = {1, 3, 2, 4}
// Vector: {5, 6}
// Expected: same result {17, 39}
void test_multiply_mv_col_major() {
    const double mat[4] = {1, 3, 2, 4};  // col-major
    const double vec[2] = {5, 6};
    double result[2] = {0, 0};

    multiply_mv_col_major(mat, 2, 2, vec, result);

    assert(approx_eq(result[0], 17.0) && "result[0] should be 17");
    assert(approx_eq(result[1], 39.0) && "result[1] should be 39");
    std::cout << "PASS test_multiply_mv_col_major\n";
}

// A: [[1,2],[3,4]], B: [[5,6],[7,8]]
// C = A*B = [[19,22],[43,50]]
void test_multiply_mm_naive() {
    const double A[4] = {1, 2, 3, 4};
    const double B[4] = {5, 6, 7, 8};
    double C[4] = {0, 0, 0, 0};

    multiply_mm_naive(A, 2, 2, B, 2, 2, C);

    assert(approx_eq(C[0], 19.0) && "C[0] should be 19");
    assert(approx_eq(C[1], 22.0) && "C[1] should be 22");
    assert(approx_eq(C[2], 43.0) && "C[2] should be 43");
    assert(approx_eq(C[3], 50.0) && "C[3] should be 50");
    std::cout << "PASS test_multiply_mm_naive\n";
}

// Same A: [[1,2],[3,4]]
// B_transposed = transpose([[5,6],[7,8]]) = [[5,7],[6,8]] stored row-major = {5,7,6,8}
// Expected C = A*B = [[19,22],[43,50]] (same result)
void test_multiply_mm_transposed_b() {
    const double A[4] = {1, 2, 3, 4};
    const double Bt[4] = {5, 7, 6, 8};  // transpose of B
    double C[4] = {0, 0, 0, 0};

    multiply_mm_transposed_b(A, 2, 2, Bt, 2, 2, C);

    assert(approx_eq(C[0], 19.0) && "C[0] should be 19");
    assert(approx_eq(C[1], 22.0) && "C[1] should be 22");
    assert(approx_eq(C[2], 43.0) && "C[2] should be 43");
    assert(approx_eq(C[3], 50.0) && "C[3] should be 50");
    std::cout << "PASS test_multiply_mm_transposed_b\n";
}

int main() {
    test_multiply_mv_row_major();
    test_multiply_mv_col_major();
    test_multiply_mm_naive();
    test_multiply_mm_transposed_b();
    std::cout << "All tests passed.\n";
    return 0;
}
