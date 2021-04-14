#include "engine.h"

void Translate(float x, float y, float z, float *transform_matrix)
{
    float translation_matrix[4][4] = {{1, 0, 0, x},
                                      {0, 1, 0, y},
                                      {0, 0, 1, z},
                                      {0, 0, 0, 1}};
    MatrixMultiply((float *)translation_matrix, 4, 4,
                   (float *)transform_matrix, 4, 4,
                   (float *)transform_matrix);
}

void RotateZAxis(float theta, float *transform_matrix)
{
    float rotation_matrix[4][4] = {{cosf(theta), -sinf(theta), 0, 0},
                                   {sinf(theta), cosf(theta), 0, 0},
                                   {0, 0, 1, 0},
                                   {0, 0, 0, 1}};
    MatrixMultiply((float *)rotation_matrix, 4, 4,
                   (float *)transform_matrix, 4, 4,
                   (float *)transform_matrix);
}

void RotateXAxis(float theta, float *transform_matrix)
{
    float rotation_matrix[4][4] = {{1, 0, 0, 0},
                                   {0, cosf(theta), -sinf(theta), 0},
                                   {0, sinf(theta), cosf(theta), 0},
                                   {0, 0, 0, 1}};

    MatrixMultiply((float *)rotation_matrix, MatrixRowSize(rotation_matrix), MatrixColSize(rotation_matrix),
                   (float *)transform_matrix, 4, 4,
                   (float *)transform_matrix);
}

void RotateYAxis(float theta, float *transform_matrix)
{
    float rotation_matrix[4][4] = {{cosf(theta), 0, sinf(theta), 0},
                                   {0, 1, 0, 0},
                                   {-sinf(theta), 0, cosf(theta), 0},
                                   {0, 0, 0, 1}};

    MatrixMultiply((float *)rotation_matrix, 4, 4,
                   (float *)transform_matrix, 4, 4,
                   (float *)transform_matrix);
}

void ProjectionMatrix(float *transform_matrix)
{
    float width = 1 / 50.0f;
    float height = 1 / 50.0f;
    float z_far = -1 / 50.0f;
    float z_near = 0;
    float projection_matrix[4][4] = {{1 / width, 0, 0, 0},
                                     {0, 1 / height, 0, 0},
                                     {0, 0, -2 / (z_far - z_near), -(z_far + z_near) / (z_far - z_near)},
                                     {0, 0, 0, 1}};
    MatrixMultiply((float *)transform_matrix, 4, 4,
                   (float *)projection_matrix, MatrixRowSize(projection_matrix), MatrixColSize(projection_matrix),
                   (float *)transform_matrix);
}

Point Project(float *transform_matrix, int m_row, int m_col,
              float *vertex, int v_row, int v_col)
{
    float point[4];
    MatrixMultiply((float *)transform_matrix, 4, 4,
                   (float *)vertex, 4, 1,
                   (float *)point);
    Point p = {};
    p.x = *(point + 0);
    p.y = *(point + 1);
    return p;
}

void IdentityMatrix4x4(float *matrix)
{
    float identity[4][4] = {{1, 0, 0, 0},
                            {0, 1, 0, 0},
                            {0, 0, 1, 0},
                            {0, 0, 0, 1}};
    for (int i = 0; i < 16; i++)
    {
        *(matrix + i) = identity[(int)(i / 4.0f)][i % 4];
    }
}


