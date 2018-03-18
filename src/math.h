#pragma once

#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct v3 {
    union {
        float data[3];
        struct {
            float x;
            float y;
            float z;
        };
    };
} v3;

void v3_copy(v3* dst, v3 src);
void v3_neg(v3* v);
void v3_add(v3* dst, v3 src);
void v3_sub(v3* dst, v3 src);
void v3_norm(v3* v);
void v3_mult(v3* dst, float s);
void v3_cross(v3* dst, v3 src);
float v3_len(v3 v);
float v3_len_sq(v3 v);
float v3_dot(v3 left, v3 right);

typedef struct v4 {
    union {
        float data[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
} v4;

void v4_copy(v4* dst, v4 src);
void v4_neg(v4* v);
void v4_add(v4* dst, v4 src);
void v4_sub(v4* dst, v4 src);
void v4_norm(v4* v);
void v4_mult(v4* dst, float s);
void v4_cross(v4* dst, v4 src);

float v4_len(v4 v);
float v4_len_sq(v4 v);
float v4_dot(v4 left, v4 right);

typedef struct quat {
    union {
        float data[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
} quat;

void quat_from_angle(quat* dst, v3 axis, float angle);
void quat_copy(quat* dst, quat src);
void quat_norm(quat* dst);
void quat_conj(quat* q);
void quat_mult(quat* res, quat left, quat right);
void quat_rot(v3* v, quat q);

float quat_len(quat q);
float quat_len_sq(quat q);

typedef struct m4 {
    float data[16];
} m4;

#define m4_at(matrix, row, column) (&(matrix)->data[4 * (row) + (column)])

void m4_copy(m4* dst, const m4* src);
void m4_unit(m4* dst);
void m4_dot(m4* dst, const m4* right);
void m4_inverse(m4* dst);
void m4_transform(m4 m, v3* v);
void m4_from_quat(m4* dst, quat q);

void m4_rotation(m4* dst, v3 angles);
void m4_translation(m4* dst, v3 v);
void m4_set_translation(m4* dst, v3 v);
void m4_scale(m4* dst, v3 v);
void m4_set_scale(m4* dst, v3 v);
void m4_look_at(m4* dst, v3 from, v3 to, v3 up);
void m4_perspective(m4* dst, float fov, float aspect, float near_clipping,
                    float far_clipping);
void m4_view_from_quat(m4* dst, quat q, v3 pos);

#ifdef __cplusplus
}
#endif
