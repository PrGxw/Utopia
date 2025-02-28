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

bool ContainedInTriangleArea(Point *p1, Point *p2, Point *p3, Point *targetp)
{
    float A = TriangleArea(p1, p2, p3);
    float A1 = TriangleArea(p1, p2, targetp);
    float A2 = TriangleArea(p1, p3, targetp);
    float A3 = TriangleArea(p2, p3, targetp);
    return (A == (A1 + A2 + A3));
}

float EdgeFunction(Point *p1, Point *p2, Point *targetp) {
    return (targetp->x - p1->x) * (p2->y - p1->y) - (targetp->y - p1->y) * (p2->x - p1->x);
}

bool ContainedInTriangleEdgeFunction(Point *p1, Point *p2, Point *p3, Point *targetp)
{
    Point* points[3] = {p1, p2, p3};
    bool contains = true;

    for (int i = 0; i < 3; i++){
        int j = (i+1) % 3;
        contains &= (EdgeFunction(points[i], points[j], targetp) > 0);
    }
    return contains;
}

bool ContainedInTriangle(Shape *triangle, Point *targetp)
{
    Point *p1 = ((Point*)triangle->geometry)+0;
    Point *p2 = ((Point*)triangle->geometry)+1;
    Point *p3 = ((Point*)triangle->geometry)+2;
    // return ContainedInTriangleArea(p1, p2, p3, targetp);
    return ContainedInTriangleEdgeFunction(p1, p2, p3, targetp);
}

float TriangleArea(Point *p1, Point *p2, Point *p3)
{
    return abs((p1->x * (p2->y - p3->y) + p2->x * (p3->y - p1->y) + p3->x * (p1->y - p2->y)) / 2.0f);
}

void FindTriangleBoudningBox(Shape *vertices, Point *bounding_top_left, Point *bounding_bot_right)
{
    if (vertices->num_elements != 3) throw -1;
    for (int i = 0; i < vertices->num_elements; i++)
    {
        Point *vertex = ((Point*)vertices->geometry)+i;
        if ((bounding_top_left->x == NULL) || (vertex->x < bounding_top_left->x))
            bounding_top_left->x = vertex->x;
        if ((bounding_top_left->y == NULL) || (vertex->y < bounding_top_left->y))
            bounding_top_left->y = vertex->y;
        if ((bounding_bot_right->x == NULL) || (vertex->x > bounding_bot_right->x))
            bounding_bot_right->x = vertex->x;
        if ((bounding_bot_right->y == NULL) || (vertex->y > bounding_bot_right->y))
            bounding_bot_right->y = vertex->y;
    }
}


void PointToImage(Point *image_point, Point *vertex, float focal_distance){
    float x = vertex->x;
    float y = vertex->y;
    float z = vertex->z;
    image_point->x = (x / -z) * focal_distance;
    image_point->y = (y / -z) * focal_distance;
}

void ImageToPoint(){
    throw -1;
}


void ImageToNDC(Point *image_point, Point *ndc_point, float frame_height, float frame_width){
    ndc_point->x = (image_point->x + frame_width / 2) / frame_width;
    ndc_point->y = (image_point->y + frame_height / 2) / frame_height;
}

void NDCToImage(){
    throw -1;
}

void NDCToRaster(Point *ndc_point, Point *raster_point, int height, int width){
    raster_point->x = (int)(ndc_point->x * width);
    raster_point->y = (int)(ndc_point->y * height);
    // raster_point.z = image_point.z;
}

void RasterToNDC(){
    throw -1;
}


float* AccessScreenBuffer(int row, int col, float* memory, int buffer_width){
    return memory + row + buffer_width*col;
}
