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

struct Shape
{
    void* geometry;
    int num_elements;
    int32 color_hex;
};

void RotateZAxis(float theta, float *transform_matrix);
void RotateXAxis(float theta, float *transform_matrix);
void RotateYAxis(float theta, float *transform_matrix);
void Translate(float x, float y, float z, float *transform_matrix);
void ProjectionMatrix(float *transform_matrix);
Point Project(float *transform_matrix, int m_row, int m_col,
              float *vertex, int v_row, int v_col);
void IdentityMatrix4x4(float * matrix);
float EdgeFunction(Point *p1, Point *p2, Point *targetp);
bool ContainedInTriangle(Shape* triangle, Point* targetp);
float TriangleArea(Point* p1, Point* p2, Point* p3);
void FindTriangleBoudningBox(Shape* vertices, Point* bounding_top_left, Point* bounding_bot_right);
void PointToImage(Point *image_point, Point *vertex, float focal_distance);
void ImageToNDC(Point *image_point, Point *ndc_point, float frame_height, float frame_width);
void NDCToRaster(Point *ndc_point, Point *raster_point, int height, int width);
float* AccessScreenBuffer(int row, int col, float* memory, int buffer_width);
#endif /* ENGINE */
