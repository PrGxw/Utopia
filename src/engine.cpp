#include "engine.h"

void RotateZAxis(float theta, float *transform_matrix)
{
    float rotation_matrix[4][4] = {{cosf(theta), sinf(theta), 0, 0},
                                   {-sinf(theta), cosf(theta), 0, 0},
                                   {0, 0, 1, 0},
                                   {0, 0, 0, 1}};
    MatrixMultiply((float *)transform_matrix, 4, 4,
                   (float *)rotation_matrix, MatrixRowSize(rotation_matrix), MatrixColSize(rotation_matrix),
                   (float *)transform_matrix);
}

void RotateXAxis(float theta, float *transform_matrix)
{
    float rotation_matrix[4][4] = {{1, 0, 0, 0},
                                   {0, cosf(theta), sinf(theta), 0},
                                   {0, -sinf(theta), cosf(theta), 0},
                                   {0, 0, 0, 1}};

    MatrixMultiply((float *)transform_matrix, 4, 4,
                   (float *)rotation_matrix, MatrixRowSize(rotation_matrix), MatrixColSize(rotation_matrix),
                   (float *)transform_matrix);
}


void RotateYAxis(float theta, float *transform_matrix)
{
    float rotation_matrix[4][4] = {{cosf(theta), 0, -sinf(theta), 0},
                                   {0, 1, 0, 0},
                                   {sinf(theta), 0, cosf(theta), 0},
                                   {0, 0, 0, 1}};

    MatrixMultiply((float *)transform_matrix, 4, 4,
                   (float *)rotation_matrix, MatrixRowSize(rotation_matrix), MatrixColSize(rotation_matrix),
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
              float *vertex, int v_row, int v_col,
              int screen_width, int screen_height)
{
    float point[4];
    MatrixMultiply((float *)transform_matrix, 4, 4,
                   (float *)vertex, 4, 1,
                   (float *)point);
    Point p = {};
    p.x = *(point + 0) + screen_width / 2;
    p.y = *(point + 1) + screen_height / 2;
    return p;
}

