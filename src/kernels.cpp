#include <cstring>

#include "kernels.hpp"

void multiply_mv_row_major(const double* matrix, int rows, int cols,
                           const double* vector, double* result) {
    for (int i = 0; i < rows; ++i) {
        double sum = 0.0;
        for (int j = 0; j < cols; ++j)
            sum += matrix[i * cols + j] * vector[j];
        result[i] = sum;
    }
}

void multiply_mv_col_major(const double* matrix, int rows, int cols,
                           const double* vector, double* result) {
    for (int i = 0; i < rows; ++i)
        result[i] = 0.0;
    for (int j = 0; j < cols; ++j) {
        for (int i = 0; i < rows; ++i)
            result[i] += matrix[j * rows + i] * vector[j];
    }
}

void multiply_mm_naive(const double* matrixA, int rowsA, int colsA,
                       const double* matrixB, int rowsB, int colsB,
                       double* result) {
    (void)rowsB;
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k)
                sum += matrixA[i * colsA + k] * matrixB[k * colsB + j];
            result[i * colsB + j] = sum;
        }
    }
}

void multiply_mm_transposed_b(const double* matrixA, int rowsA, int colsA,
                               const double* matrixB_transposed, int rowsB, int colsB,
                               double* result) {
    (void)colsB;
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < rowsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k)
                sum += matrixA[i * colsA + k] * matrixB_transposed[j * colsA + k];
            result[i * rowsB + j] = sum;
        }
    }
}

void multiply_mm_tiled(const double* matrixA, int rowsA, int colsA,
                       const double* matrixB, int rowsB, int colsB,
                       double* result, int tile) {
    (void)rowsB;
    std::memset(result, 0, rowsA * colsB * sizeof(double));
    for (int i0 = 0; i0 < rowsA; i0 += tile) {
        int i_end = i0 + tile < rowsA ? i0 + tile : rowsA;
        for (int k0 = 0; k0 < colsA; k0 += tile) {
            int k_end = k0 + tile < colsA ? k0 + tile : colsA;
            for (int j0 = 0; j0 < colsB; j0 += tile) {
                int j_end = j0 + tile < colsB ? j0 + tile : colsB;
                for (int i = i0; i < i_end; ++i)
                    for (int k = k0; k < k_end; ++k) {
                        double a = matrixA[i * colsA + k];
                        for (int j = j0; j < j_end; ++j)
                            result[i * colsB + j] += a * matrixB[k * colsB + j];
                    }
            }
        }
    }
}
