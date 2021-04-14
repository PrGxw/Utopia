#ifndef ENGINE
#define ENGINE

#include "matrix.h"

struct Point
{
    float x;
    float y;
    float z;
    float padding;
};
void RotateZAxis(float theta, float *transform_matrix);
void RotateXAxis(float theta, float *transform_matrix);
void RotateYAxis(float theta, float *transform_matrix);
void Translate(float x, float y, float z, float *transform_matrix);
void ProjectionMatrix(float *transform_matrix);
Point Project(float *transform_matrix, int m_row, int m_col,
              float *vertex, int v_row, int v_col);
void IdentityMatrix4x4(float * matrix);
#endif /* ENGINE */
