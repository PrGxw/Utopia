#include "matrix.h"

// TODO: decide whether to operate on the original matrix or not
static void
MatrixTranspose(float *matrix, int num_row, int num_col,
                float *result)
{
    float *dup = (float *)malloc(num_row * num_col * sizeof(float));
    float *tmp = dup;
    for (int i = 0; i < num_row * num_col; i++)
    {
        *(tmp++) = *(matrix++);
    }
    for (int col = 0; col < num_col; col++)
    {
        for (int row = 0; row < num_row; row++)
        {
            *(result++) = *(dup + row * num_col + col);
        }
    }
}

static void
MatrixScalarMultiply(float *matrix, int num_row, int num_col, float scalar)
{
    for (int i = 0; i < num_row * num_col; i++)
    {
        *(matrix++) *= scalar;
    }
}

static MatrixDimension
matrix_dimension_after_multiply(int m1_row, int m1_col,
                                int m2_row, int m2_col)
{
    MatrixDimension new_dimension;
    new_dimension.row = m1_row;
    new_dimension.col = m2_col;
    return new_dimension;
}
static void
MatrixOfMinors(float *matrix, int len,
               float *result)
{
    for (int i = 0; i < len * len; i++)
    {
        float *minor = (float *)malloc((len - 1) * (len - 1) * sizeof(float));
        int index = 0;
        int cur_row = ((int)(i / len));
        int cur_col = (i % len);
        int sign = (cur_row + cur_col) % 2 == 0 ? 1 : -1;
        for (int row = 0; row < len; row++)
        {
            for (int col = 0; col < len; col++)
            {
                if (row != cur_row && col != cur_col)
                {
                    *(minor + index) = *(matrix + col + row * len);
                    index++;
                }
            }
        }
        *(result + i) = sign * MatrixDeterminant(minor, len - 1);
    }
}

static float
MatrixDeterminant(float *matrix, int len)
{
    if (len == 2)
    {
        float a = *matrix++;
        float b = *matrix++;
        float c = *matrix++;
        float d = *matrix++;
        return a * d - b * c;
    }
    int res = 0;
    int sign = 1;
    for (int i = 0; i < len; i++)
    {
        float *minor = (float *)malloc((len - 1) * (len - 1) * sizeof(float));
        int index = 0;
        for (int row = 0; row < len; row++)
        {
            for (int col = 0; col < len; col++)
            {
                if (row != 0 && col != i)
                {
                    *(minor + index) = *(matrix + col + row * len);
                    index++;
                }
            }
        }
        res += (float)(sign * (*(matrix + i)) * MatrixDeterminant(minor, len - 1));
        free(minor);
        sign = sign * -1;
    }
    return res;
}

// https://www.mathsisfun.com/algebra/matrix-inverse-minors-cofactors-adjugate.html
 void
MatrixInverse(float *matrix, int len,
               float *result)
{
    MatrixOfMinors(matrix, len, result);
    // printf("matrix of minors\n");
    InspectMatrix(result, len, len);
    MatrixTranspose(result, len, len, result);

    // printf("transposed\n");
    InspectMatrix(result, len, len);
    // printf("original\n");
    InspectMatrix(matrix, len, len);
    float determinant = 0;
    for (int i = 0; i < len; i++)
    {
        determinant += matrix[i] * result[i * len];
    }
    MatrixScalarMultiply(result, len, len, 1 / determinant);
    // printf("inverse\n");
    InspectMatrix(result, len, len);
}

float* DupMatrix(float *matrix, int m_row, int m_col)
{
    float* matrix_dup = (float *)malloc(m_row * m_col * sizeof(float));
    for (int i = 0; i < m_row * m_col; i++)
    {
        *(matrix_dup + i) = *(matrix + i);
    }
    return matrix_dup;
}

void FreeMatrix(float *matrix)
{
    free(matrix);
}

void MatrixMultiply(float *matrix1, int m1_row, int m1_col,
                    float *matrix2, int m2_row, int m2_col,
                    float *result)
{
    float *matrix1_dup = DupMatrix(matrix1, m1_row, m2_row);
    float *matrix2_dup = DupMatrix(matrix2, m2_row, m2_row);

    for (int row = 0; row < m1_row; row++)
    {
        for (int col = 0; col < m2_col; col++)
        {
            float res = 0;
            for (int i = 0; i < m1_col; i++)
            {
                float value1 = *(matrix1_dup + (m1_col * row) + i);
                float value2 = *(matrix2_dup + col + (m2_col * i));
                res += value1 * value2;
            }
            *result++ = res;
        }
    }

    FreeMatrix(matrix1_dup);
    FreeMatrix(matrix2_dup);
}

void InspectMatrix(float *matrix, int num_row, int num_col)
{
    // printf("\n");
    for (int row = 0; row < num_row; row++)
    {
        for (int col = 0; col < num_col; col++)
        {
            // printf("%f ", *matrix++);
        }
        // printf("\n");
    }
    // printf("\n");
}

// int main()
// {
//     float a[2][3] = {{1, 2, 3}, {4, 5, 6}};
//     float b[3][2] = {{7, 8}, {9, 10}, {11, 12}};
//     float result[2][2];
//     printf("matrix multiply:\n");
//     MatrixMultiply((float *)a, MatrixRowSize(a), MatrixColSize(a),
//                     (float *)b, MatrixRowSize(b), MatrixColSize(b),
//                     (float *)result);
//     InspectMatrix((float *)result, MatrixRowSize(result), MatrixColSize(result));

//     printf("\nmatrix determinant:\n");
//     float c[3][3] = {{1,2,3}, {4,5,6}, {7,8,9}};
//     float det = MatrixDeterminant((float*)c, MatrixRowSize(c));
//     InspectMatrix((float *)c, MatrixRowSize(c), MatrixColSize(c));
//     printf("determinant: %f\n", det);

//     float d[4][4] = {{1, 3, 5, 9}, {1, 3, 1, 7}, {4, 3, 9, 7}, {5, 2, 0, 9}};
//     float det = MatrixDeterminant((float *)d, MatrixRowSize(d));
//     InspectMatrix((float *)d, MatrixRowSize(d), MatrixColSize(d));
//     printf("determinant: %f\n", det);

//     printf("\nmatrix of minors\n");
//     float minors[4][4];
//     float e[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {1, 7, 9, 5}, {0, 6, 9, 7}};
//     MatrixOfMinors((float *)e, MatrixColSize(e), (float *)minors);
//     InspectMatrix((float *)minors, MatrixRowSize(minors), MatrixColSize(minors));

//     printf("\nmatrix transpose\n");
//     float f[3][3] = {{3,0,2}, {2,0,-2}, {0,1,1}};
//     float transposed[3][3];
//     MatrixTranspose((float *)f, MatrixRowSize(f), (float *)transposed);
//     InspectMatrix((float *)f, MatrixRowSize(f), MatrixColSize(f));
//     InspectMatrix((float *)transposed, MatrixRowSize(transposed), MatrixColSize(transposed));

//     printf("\nmatrix inverse\n");
//     float g[3][3] = {{3,0,2}, {2,0,-2}, {0,1,1}};
//     float inversed[3][3];
//     MatrixInverse((float *)g, MatrixRowSize(g), (float *)inversed);
//     InspectMatrix((float *)g, MatrixRowSize(g), MatrixColSize(g));
//     InspectMatrix((float *)inversed, MatrixRowSize(inversed), MatrixColSize(inversed));

//     float g[4][4] = {{1, 3, 5, 9}, {1, 3, 1, 7}, {4, 3, 9, 7}, {5, 2, 0, 9}};
//     float inversed[4][4];
//     MatrixInverse((float *)g, MatrixRowSize(g), (float *)inversed);
//     InspectMatrix((float *)g, MatrixRowSize(g), MatrixColSize(g));
//     InspectMatrix((float *)inversed, MatrixRowSize(inversed), MatrixColSize(inversed));
//     getchar();
//     float theta = 3.1415926*0.25;
//     float point[4] = {1,0,0,1};
//     float rotation_matrix[4][4] = {{cosf(theta), sinf(theta), 0, 0},
//                                    {-sinf(theta), cosf(theta), 0, 0},
//                                    {0, 0, 1, 0},
//                                    {0, 0, 0, 1}};
//     float result[4] = {};
//     MatrixMultiply((float *)rotation_matrix, MatrixRowSize(rotation_matrix), MatrixColSize(rotation_matrix),
//                     (float *)point, MatrixRowSize(point), MatrixColSize(point),
//                     (float *)result);
//     InspectMatrix(result, 4, 1);
//     getchar();
// }
