#ifndef MATRIX
#define MATRIX
#include "data_types.h"
#include <math.h>

// TODO: this should be more generic, i.e. not matrix sizes,
// but multidimentional array sizes
#define MatrixRowSize(matrix) (sizeof(matrix) / sizeof(matrix[0]))
#define MatrixColSize(matrix) (sizeof(matrix[0]) / sizeof(float))

struct MatrixDimension
{
    int row;
    int col;
};

void
matrix_multiply(float *matrix1, int m1_row, int m1_col,
                float *matrix2, int m2_row, int m2_col,
                float *result);

static void
inspect_matrix(float *matrix, int num_row, int num_col);

static float
matrix_determinant(float *matrix, int len);

static void
matrix_of_minors(float *matrix, int len,
                 float *result);

static void
matrix_transpose(float *matrix, int num_row, int num_col,
                 float *result);

static void
matrix_scalar_multiply(float *matrix, int num_row, int num_col, float scalar);
#endif /* MATRIX */
