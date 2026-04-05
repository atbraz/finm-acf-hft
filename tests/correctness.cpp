#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

#include "kernels.hpp"

static bool approx_eq(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

// ── helpers ──────────────────────────────────────────────────────────────────

static void fill_random(double* data, int count, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(-100.0, 100.0);
    for (int i = 0; i < count; ++i)
        data[i] = dist(rng);
}

// Row-major src[rows][cols] -> col-major dst (same logical matrix)
// Also serves as matrix transpose: row-major A(rows,cols) -> row-major A^T(cols,rows)
static void transpose_storage(const double* src, double* dst, int rows, int cols) {
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            dst[j * rows + i] = src[i * cols + j];
}

static double max_abs_diff(const double* a, const double* b, int count) {
    double d = 0.0;
    for (int i = 0; i < count; ++i)
        d = std::max(d, std::abs(a[i] - b[i]));
    return d;
}

static double tolerance(int inner_dim) { return inner_dim * 1e-9; }

// ── hardcoded tests ──────────────────────────────────────────────────────────

void test_multiply_mv_row_major() {
    const double mat[4] = {1, 2, 3, 4};
    const double vec[2] = {5, 6};
    double result[2] = {0, 0};

    multiply_mv_row_major(mat, 2, 2, vec, result);

    assert(approx_eq(result[0], 17.0) && "result[0] should be 17");
    assert(approx_eq(result[1], 39.0) && "result[1] should be 39");
    std::cout << "PASS test_multiply_mv_row_major\n";
}

void test_multiply_mv_col_major() {
    const double mat[4] = {1, 3, 2, 4};
    const double vec[2] = {5, 6};
    double result[2] = {0, 0};

    multiply_mv_col_major(mat, 2, 2, vec, result);

    assert(approx_eq(result[0], 17.0) && "result[0] should be 17");
    assert(approx_eq(result[1], 39.0) && "result[1] should be 39");
    std::cout << "PASS test_multiply_mv_col_major\n";
}

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

void test_multiply_mm_transposed_b() {
    const double A[4] = {1, 2, 3, 4};
    const double Bt[4] = {5, 7, 6, 8};
    double C[4] = {0, 0, 0, 0};

    multiply_mm_transposed_b(A, 2, 2, Bt, 2, 2, C);

    assert(approx_eq(C[0], 19.0) && "C[0] should be 19");
    assert(approx_eq(C[1], 22.0) && "C[1] should be 22");
    assert(approx_eq(C[2], 43.0) && "C[2] should be 43");
    assert(approx_eq(C[3], 50.0) && "C[3] should be 50");
    std::cout << "PASS test_multiply_mm_transposed_b\n";
}

void test_multiply_mm_tiled() {
    const double A[4] = {1, 2, 3, 4};
    const double B[4] = {5, 6, 7, 8};
    double C[4] = {};

    multiply_mm_tiled(A, 2, 2, B, 2, 2, C);

    assert(approx_eq(C[0], 19.0));
    assert(approx_eq(C[1], 22.0));
    assert(approx_eq(C[2], 43.0));
    assert(approx_eq(C[3], 50.0));
    std::cout << "PASS test_multiply_mm_tiled\n";
}

// ── hardcoded edge cases ─────────────────────────────────────────────────────

void test_mv_1x1() {
    double mat = 3.0, vec = 7.0, result = 0.0;
    multiply_mv_row_major(&mat, 1, 1, &vec, &result);
    assert(approx_eq(result, 21.0));
    result = 0.0;
    multiply_mv_col_major(&mat, 1, 1, &vec, &result);
    assert(approx_eq(result, 21.0));
    std::cout << "PASS test_mv_1x1\n";
}

void test_mm_1x1() {
    double a = 3.0, b = 7.0, c = 0.0;
    multiply_mm_naive(&a, 1, 1, &b, 1, 1, &c);
    assert(approx_eq(c, 21.0));
    c = 0.0;
    multiply_mm_transposed_b(&a, 1, 1, &b, 1, 1, &c);
    assert(approx_eq(c, 21.0));
    c = 0.0;
    multiply_mm_tiled(&a, 1, 1, &b, 1, 1, &c);
    assert(approx_eq(c, 21.0));
    std::cout << "PASS test_mm_1x1\n";
}

void test_mm_identity() {
    const double I[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    const double A[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    double C[9] = {};

    multiply_mm_naive(A, 3, 3, I, 3, 3, C);
    for (int i = 0; i < 9; ++i)
        assert(approx_eq(C[i], A[i]));
    std::cout << "PASS test_mm_identity\n";
}

void test_mv_non_square() {
    // [[1,2,3],[4,5,6]] * [1,2,3] = [14, 32]
    const double mat_row[6] = {1, 2, 3, 4, 5, 6};
    const double vec[3] = {1, 2, 3};
    double result[2] = {};

    multiply_mv_row_major(mat_row, 2, 3, vec, result);
    assert(approx_eq(result[0], 14.0));
    assert(approx_eq(result[1], 32.0));

    // Same logical matrix in col-major: {1,4, 2,5, 3,6}
    const double mat_col[6] = {1, 4, 2, 5, 3, 6};
    double result2[2] = {};
    multiply_mv_col_major(mat_col, 2, 3, vec, result2);
    assert(approx_eq(result2[0], 14.0));
    assert(approx_eq(result2[1], 32.0));
    std::cout << "PASS test_mv_non_square\n";
}

void test_mm_non_square() {
    // A(2,3) * B(3,2) = C(2,2)
    // A = [[1,2,3],[4,5,6]], B = [[1,2],[3,4],[5,6]]
    // C = [[22,28],[49,64]]
    const double A[6] = {1, 2, 3, 4, 5, 6};
    const double B[6] = {1, 2, 3, 4, 5, 6};
    double C[4] = {};

    multiply_mm_naive(A, 2, 3, B, 3, 2, C);
    assert(approx_eq(C[0], 22.0));
    assert(approx_eq(C[1], 28.0));
    assert(approx_eq(C[2], 49.0));
    assert(approx_eq(C[3], 64.0));

    // B^T(2,3) row-major = [[1,3,5],[2,4,6]]
    const double Bt[6] = {1, 3, 5, 2, 4, 6};
    double C2[4] = {};
    multiply_mm_transposed_b(A, 2, 3, Bt, 2, 3, C2);
    for (int i = 0; i < 4; ++i)
        assert(approx_eq(C2[i], C[i]));

    double C3[4] = {};
    multiply_mm_tiled(A, 2, 3, B, 3, 2, C3);
    for (int i = 0; i < 4; ++i)
        assert(approx_eq(C3[i], C[i]));

    std::cout << "PASS test_mm_non_square\n";
}

// ── randomized cross-validation ──────────────────────────────────────────────

struct dim2 {
    int rows, cols;
};
struct dim3 {
    int m, k, n;
};

// Row-major MV vs col-major MV on the same logical matrix
static int fuzz_mv(std::mt19937& rng, int trials) {
    dim2 sizes[] = {
        {1, 1},     {2, 2},     {3, 3},     {7, 7},     {16, 16},   {31, 31},
        {32, 32},   {33, 33},   {50, 50},   {63, 63},   {64, 64},   {65, 65},
        {100, 100}, {128, 128}, {200, 200},
        // non-square
        {3, 5},     {5, 3},     {1, 100},   {100, 1},   {17, 31},   {64, 33},
        {100, 200},
    };
    int total = 0;
    for (auto [rows, cols] : sizes) {
        for (int t = 0; t < trials; ++t) {
            std::vector<double> mat_row(rows * cols);
            std::vector<double> mat_col(rows * cols);
            std::vector<double> vec(cols);
            std::vector<double> res_row(rows);
            std::vector<double> res_col(rows);

            fill_random(mat_row.data(), rows * cols, rng);
            fill_random(vec.data(), cols, rng);
            transpose_storage(mat_row.data(), mat_col.data(), rows, cols);

            multiply_mv_row_major(mat_row.data(), rows, cols, vec.data(), res_row.data());
            multiply_mv_col_major(mat_col.data(), rows, cols, vec.data(), res_col.data());

            double diff = max_abs_diff(res_row.data(), res_col.data(), rows);
            double tol = tolerance(cols);
            if (diff > tol) {
                std::cerr << "FAIL fuzz_mv: rows=" << rows << " cols=" << cols << " trial=" << t
                          << " max_diff=" << diff << " tol=" << tol << "\n";
                return -1;
            }
            ++total;
        }
    }
    return total;
}

// Naive MM vs transposed-B vs tiled (default tile)
static int fuzz_mm(std::mt19937& rng, int trials) {
    dim3 sizes[] = {
        // square
        {1, 1, 1},       {2, 2, 2},       {3, 3, 3},       {7, 7, 7},
        {16, 16, 16},    {31, 31, 31},    {32, 32, 32},    {33, 33, 33},
        {50, 50, 50},    {63, 63, 63},    {64, 64, 64},    {65, 65, 65},
        {100, 100, 100}, {128, 128, 128},
        // non-square
        {3, 4, 2},       {2, 3, 4},       {1, 50, 1},      {50, 1, 50},
        {17, 31, 23},    {33, 64, 33},    {100, 50, 75},
        // tile-boundary stress
        {31, 32, 33},    {33, 32, 31},    {63, 64, 65},    {65, 64, 63},
        {95, 96, 97},
    };
    int total = 0;
    for (auto [m, k, n] : sizes) {
        for (int t = 0; t < trials; ++t) {
            std::vector<double> A(m * k);
            std::vector<double> B(k * n);
            std::vector<double> Bt(n * k);
            std::vector<double> C_naive(m * n);
            std::vector<double> C_trans(m * n);
            std::vector<double> C_tiled(m * n);

            fill_random(A.data(), m * k, rng);
            fill_random(B.data(), k * n, rng);
            transpose_storage(B.data(), Bt.data(), k, n);

            multiply_mm_naive(A.data(), m, k, B.data(), k, n, C_naive.data());
            multiply_mm_transposed_b(A.data(), m, k, Bt.data(), n, k, C_trans.data());
            multiply_mm_tiled(A.data(), m, k, B.data(), k, n, C_tiled.data());

            double tol = tolerance(k);
            double diff_trans = max_abs_diff(C_naive.data(), C_trans.data(), m * n);
            double diff_tiled = max_abs_diff(C_naive.data(), C_tiled.data(), m * n);

            if (diff_trans > tol) {
                std::cerr << "FAIL fuzz_mm transposed_b: m=" << m << " k=" << k << " n=" << n
                          << " trial=" << t << " max_diff=" << diff_trans << "\n";
                return -1;
            }
            if (diff_tiled > tol) {
                std::cerr << "FAIL fuzz_mm tiled: m=" << m << " k=" << k << " n=" << n
                          << " trial=" << t << " max_diff=" << diff_tiled << "\n";
                return -1;
            }
            ++total;
        }
    }
    return total;
}

// Tiled MM with varying tile sizes against naive as reference
static int fuzz_mm_tiles(std::mt19937& rng) {
    int tiles[] = {1, 2, 7, 15, 16, 17, 31, 32, 33, 64};
    int sizes[] = {1, 7, 15, 16, 17, 31, 32, 33, 50, 63, 64, 65};
    constexpr int TRIALS = 5;
    int total = 0;
    for (int n : sizes) {
        for (int tile : tiles) {
            for (int t = 0; t < TRIALS; ++t) {
                std::vector<double> A(n * n);
                std::vector<double> B(n * n);
                std::vector<double> C_naive(n * n);
                std::vector<double> C_tiled(n * n);

                fill_random(A.data(), n * n, rng);
                fill_random(B.data(), n * n, rng);

                multiply_mm_naive(A.data(), n, n, B.data(), n, n, C_naive.data());
                multiply_mm_tiled(A.data(), n, n, B.data(), n, n, C_tiled.data(), tile);

                double diff = max_abs_diff(C_naive.data(), C_tiled.data(), n * n);
                double tol = tolerance(n);
                if (diff > tol) {
                    std::cerr << "FAIL fuzz_mm_tiles: n=" << n << " tile=" << tile << " trial=" << t
                              << " max_diff=" << diff << "\n";
                    return -1;
                }
                ++total;
            }
        }
    }
    return total;
}

// ── main ─────────────────────────────────────────────────────────────────────

int main() {
    test_multiply_mv_row_major();
    test_multiply_mv_col_major();
    test_multiply_mm_naive();
    test_multiply_mm_transposed_b();
    test_multiply_mm_tiled();
    test_mv_1x1();
    test_mm_1x1();
    test_mm_identity();
    test_mv_non_square();
    test_mm_non_square();

    constexpr unsigned SEED = 42;
    constexpr int TRIALS = 50;
    std::mt19937 rng(SEED);

    int mv = fuzz_mv(rng, TRIALS);
    if (mv < 0)
        return 1;
    std::cout << "PASS fuzz_mv (" << mv << " trials)\n";

    int mm = fuzz_mm(rng, TRIALS);
    if (mm < 0)
        return 1;
    std::cout << "PASS fuzz_mm (" << mm << " trials)\n";

    int tiles = fuzz_mm_tiles(rng);
    if (tiles < 0)
        return 1;
    std::cout << "PASS fuzz_mm_tiles (" << tiles << " trials)\n";

    std::cout << "All tests passed.\n";
    return 0;
}
