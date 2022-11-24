#include "dml.h"
#include <math.h>

float lerp(float v0, float v1, float t) {
    if(t > 1) return v1;
    return (1 - t) * v0 + t * v1;
}

Vector lerp_2d(Vector start, Vector end, float t) {
    return (Vector){
        lerp(start.x, end.x, t),
        lerp(start.y, end.y, t),
    };
}

Vector quadratic_2d(Vector start, Vector control, Vector end, float t) {
    return lerp_2d(lerp_2d(start, control, t), lerp_2d(control, end, t), t);
}

Vector vector_add(Vector a, Vector b) {
    return (Vector){a.x + b.x, a.y + b.y};
}

Vector vector_sub(Vector a, Vector b) {
    return (Vector){a.x - b.x, a.y - b.y};
}

Vector vector_mul_components(Vector a, Vector b) {
    return (Vector){a.x * b.x, a.y * b.y};
}

Vector vector_div_components(Vector a, Vector b) {
    return (Vector){a.x / b.x, a.y / b.y};
}

Vector vector_normalized(Vector a) {
    float length = vector_magnitude(a);
    return (Vector){a.x / length, a.y / length};
}

float vector_magnitude(Vector a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

float vector_distance(Vector a, Vector b) {
    return vector_magnitude(vector_sub(a, b));
}

float vector_dot(Vector a, Vector b) {
    Vector _a = vector_normalized(a);
    Vector _b = vector_normalized(b);
    return _a.x * _b.x + _a.y * _b.y;
}