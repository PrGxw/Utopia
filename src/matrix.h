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
MatrixMultiply(float *matrix1, int m1_row, int m1_col,
                float *matrix2, int m2_row, int m2_col,
                float *result);

void
InspectMatrix(float *matrix, int num_row, int num_col);

static float
MatrixDeterminant(float *matrix, int len);

static void
MatrixOfMinors(float *matrix, int len,
                 float *result);

static void
MatrixTranspose(float *matrix, int num_row, int num_col,
                 float *result);

static void
MatrixScalarMultiply(float *matrix, int num_row, int num_col, float scalar);
#endif /* MATRIX */
