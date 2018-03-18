#include "math.h"

void v3_copy(v3* dst, v3 src)
{
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
}

void v3_neg(v3* v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
}

void v3_add(v3* dst, v3 src)
{
    dst->x += src.x;
    dst->y += src.y;
    dst->z += src.z;
}

void v3_sub(v3* dst, v3 src)
{
    dst->x -= src.x;
    dst->y -= src.y;
    dst->z -= src.z;
}

void v3_norm(v3* v)
{
    float len = v3_len(*v);
    v->x /= len;
    v->y /= len;
    v->z /= len;
}

float v3_len_sq(v3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float v3_len(v3 v)
{
    return sqrt(v3_len_sq(v));
}

float v3_dot(v3 left, v3 right)
{
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

void v3_mult(v3* v, float s)
{
    v->x = v->x * s;
    v->y = v->y * s;
    v->z = v->z * s;
}

void v3_cross(v3* dst, v3 src)
{
    float x = dst->y * src.z - dst->z * src.y;
    float y = dst->z * src.x - dst->x * src.z;
    float z = dst->x * src.y - dst->y * src.x;
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

void v4_copy(v4* dst, v4 src)
{
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
    dst->w = src.w;
}

void v4_neg(v4* v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
    v->w = -v->w;
}

void v4_add(v4* dst, v4 src)
{
    dst->x += src.x;
    dst->y += src.y;
    dst->z += src.z;
    dst->w += src.w;
}

void v4_sub(v4* dst, v4 src)
{
    dst->x -= src.x;
    dst->y -= src.y;
    dst->z -= src.z;
    dst->w -= src.w;
}

void v4_norm(v4* v)
{
    float ilen = 1.0f / v4_len(*v);
    v4_mult(v, ilen);
}

void v4_mult(v4* dst, float s)
{
    dst->x *= s;
    dst->y *= s;
    dst->z *= s;
    dst->w *= s;
}

float v4_len(v4 v)
{
    return sqrt(v4_len_sq(v));
}

float v4_len_sq(v4 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

float v4_dot(v4 left, v4 right)
{
    return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
}

void quat_from_angle(quat* dst, v3 axis, float angle)
{
    v3_norm(&axis);

    float ha = angle * 0.5f;
    float s = sinf(ha);

    dst->x = axis.x * s;
    dst->y = axis.y * s;
    dst->z = axis.z * s;
    dst->w = cosf(ha);
}

void quat_copy(quat* dst, quat src)
{
    dst->x = src.x;
    dst->y = src.y;
    dst->z = src.z;
    dst->w = src.w;
}

void quat_norm(quat* dst)
{
    float il = 1.0f / quat_len(*dst);

    dst->x *= il;
    dst->y *= il;
    dst->z *= il;
    dst->w *= il;
}

void quat_conj(quat* q)
{
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
}

void quat_mult(quat* res, quat left, quat right)
{
    res->x = left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y;
    res->y = left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x;
    res->z = left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w;
    res->w = left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z;
}

float quat_len(quat q)
{
    return sqrt(quat_len_sq(q));
}

float quat_len_sq(quat q)
{
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

void quat_rot(v3* v, quat q)
{
    quat tmp = {};
    v3_copy((v3*)&tmp, *v);

    quat conj;
    quat_copy(&conj, q);
    quat_conj(&conj);
    quat_norm(&conj);

    quat_mult(&tmp, conj, tmp);
    quat_mult(&tmp, tmp, q);

    v->x = tmp.x;
    v->y = tmp.y;
    v->z = tmp.z;
}

void m4_copy(m4* dst, const m4* src)
{
    for (uint32_t i = 0; i < 16; ++i) {
        dst->data[i] = src->data[i];
    }
}

void m4_unit(m4* dst)
{
    for (uint32_t i = 0; i < 16; ++i) {
        dst->data[i] = 0.0f;
    }

    for (uint32_t i = 0; i < 4; ++i) {
        *m4_at(dst, i, i) = 1.0f;
    }
}

void m4_dot(m4* dst, const m4* right)
{
    m4 res;
    for (uint32_t row = 0; row < 4; ++row) {
        for (uint32_t col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (uint32_t col_left = 0; col_left < 4; ++col_left) {
                float a = *m4_at(dst, row, col_left);
                float b = *m4_at(right, col_left, col);
                sum += a * b;
            }
            *m4_at(&res, row, col) = sum;
        }
    }
    m4_copy(dst, &res);
}

void m4_inverse(m4* m)
{
    // TODO
}

void m4_transform(m4 m, v3* v)
{
    v3 res;
    res.data[0] = *m4_at(&m, 0, 0) * v->x + *m4_at(&m, 0, 1) * v->y + *m4_at(&m, 0, 2) * v->z + *m4_at(&m, 0, 3);
    res.data[1] = *m4_at(&m, 1, 0) * v->x + *m4_at(&m, 1, 1) * v->y + *m4_at(&m, 1, 2) * v->z + *m4_at(&m, 1, 3);
    res.data[2] = *m4_at(&m, 2, 0) * v->x + *m4_at(&m, 2, 1) * v->y + *m4_at(&m, 2, 2) * v->z + *m4_at(&m, 2, 3);
    v3_copy(v, res);
}

void m4_from_quat(m4* dst, quat q)
{
    quat_norm(&q);

    float xx = 2 * q.x * q.x;
    float yy = 2 * q.y * q.y;
    float zz = 2 * q.z * q.z;
    float xy = 2 * q.x * q.y;
    float xz = 2 * q.x * q.z;
    float yz = 2 * q.y * q.z;
    float wx = 2 * q.x * q.w;
    float wy = 2 * q.y * q.w;
    float wz = 2 * q.w * q.z;

    *dst = (m4){};
    *m4_at(dst, 0, 0) = 1 - yy - zz;
    *m4_at(dst, 0, 1) = xy - wz;
    *m4_at(dst, 0, 2) = xz + wy;
    *m4_at(dst, 1, 0) = xy + wz;
    *m4_at(dst, 1, 1) = 1 - xx - zz;
    *m4_at(dst, 1, 2) = yz - wx;
    *m4_at(dst, 2, 0) = xz - wy;
    *m4_at(dst, 2, 1) = yz + wx;
    *m4_at(dst, 2, 2) = 1 - xx - yy;
    *m4_at(dst, 3, 3) = 1;
}

void m4_rotation(m4* dst, v3 angles)
{
    m4 x_rotation = {};

    *m4_at(&x_rotation, 3, 3) = 1.0f;
    *m4_at(&x_rotation, 0, 0) = 1.0f;
    *m4_at(&x_rotation, 1, 1) = cosf(angles.x);
    *m4_at(&x_rotation, 1, 2) = -sinf(angles.x);
    *m4_at(&x_rotation, 2, 1) = sinf(angles.x);
    *m4_at(&x_rotation, 2, 2) = cosf(angles.x);

    m4 y_rotation = {};
    *m4_at(&y_rotation, 3, 3) = 1.0f;
    *m4_at(&y_rotation, 0, 0) = cosf(angles.y);
    *m4_at(&y_rotation, 1, 1) = 1.0f;
    *m4_at(&y_rotation, 0, 2) = sinf(angles.y);
    *m4_at(&y_rotation, 2, 0) = -sinf(angles.y);
    *m4_at(&y_rotation, 2, 2) = cosf(angles.y);

    m4 z_rotation = {};
    *m4_at(&z_rotation, 3, 3) = 1.0f;
    *m4_at(&z_rotation, 0, 0) = cosf(angles.z);
    *m4_at(&z_rotation, 0, 1) = -sinf(angles.z);
    *m4_at(&z_rotation, 1, 0) = sinf(angles.z);
    *m4_at(&z_rotation, 1, 1) = cosf(angles.z);
    *m4_at(&z_rotation, 2, 2) = 1.0f;

    m4_dot(&y_rotation, &z_rotation);
    m4_dot(&x_rotation, &y_rotation);

    m4_copy(dst, &x_rotation);
}

void m4_translation(m4* dst, v3 vec)
{
    *dst = (m4){};
    for (uint32_t idx = 0; idx < 4; ++idx) {
        *m4_at(dst, idx, idx) = 1.0f;
    }
    *m4_at(dst, 3, 0) = vec.x;
    *m4_at(dst, 3, 1) = vec.y;
    *m4_at(dst, 3, 2) = vec.z;
}

void m4_set_translation(m4* dst, v3 vec)
{
    *m4_at(dst, 3, 0) = vec.x;
    *m4_at(dst, 3, 1) = vec.y;
    *m4_at(dst, 3, 2) = vec.z;
}

void m4_scale(m4* dst, v3 vec)
{
    *dst = (m4){};
    for (uint32_t idx = 0; idx < 3; ++idx) {
        *m4_at(dst, idx, idx) = vec.data[idx];
    }
    *m4_at(dst, 3, 3) = 1.0f;
}

void m4_set_scale(m4* dst, v3 vec)
{
    for (uint32_t idx = 0; idx < 3; ++idx) {
        *m4_at(dst, idx, idx) = vec.data[idx];
    }
}

void m4_look_at(m4* dst, v3 from, v3 to, v3 up)
{
    *dst = (m4){};

    v3 zaxis;
    v3_copy(&zaxis, from);
    v3_sub(&zaxis, to);
    v3_norm(&zaxis);

    v3 xaxis;
    v3_copy(&xaxis, up);
    v3_cross(&xaxis, zaxis);
    v3_norm(&xaxis);

    v3 yaxis;
    v3_copy(&yaxis, zaxis);
    v3_cross(&yaxis, xaxis);
    v3_norm(&yaxis);

    v3_neg(&from);
    *m4_at(dst, 0, 0) = xaxis.x;
    *m4_at(dst, 1, 0) = xaxis.y;
    *m4_at(dst, 2, 0) = xaxis.z;
    *m4_at(dst, 3, 0) = v3_dot(xaxis, from);
    *m4_at(dst, 0, 1) = yaxis.x;
    *m4_at(dst, 1, 1) = yaxis.y;
    *m4_at(dst, 2, 1) = yaxis.z;
    *m4_at(dst, 3, 1) = v3_dot(yaxis, from);
    *m4_at(dst, 0, 2) = zaxis.x;
    *m4_at(dst, 1, 2) = zaxis.y;
    *m4_at(dst, 2, 2) = zaxis.z;
    *m4_at(dst, 3, 2) = v3_dot(zaxis, from);
    *m4_at(dst, 3, 3) = 1.0f;
}

void m4_perspective(m4* dst, float fov, float aspect, float near, float far)
{
    *dst = (m4){};

    float depth = near - far;
    const float DEG_TO_RAD = 0.0174532925f;
    float tan_half_fov = tanf(0.5f * fov * DEG_TO_RAD);

    *m4_at(dst, 0, 0) = 1.0f / tan_half_fov;
    *m4_at(dst, 1, 1) = aspect / tan_half_fov;
    *m4_at(dst, 2, 2) = (far + near) / depth;
    *m4_at(dst, 3, 2) = 2.0f * (far * near) / depth;
    *m4_at(dst, 2, 3) = -1.0f;
}

void m4_view_from_quat(m4* dst, quat q, v3 pos)
{
    float xy = 2.0f * q.x * q.y;
    float xz = 2.0f * q.x * q.z;
    float xw = 2.0f * q.x * q.w;
    float yz = 2.0f * q.y * q.z;
    float yw = 2.0f * q.y * q.w;
    float zw = 2.0f * q.z * q.w;

    float cost = 2.0f * q.w * q.w - 1.0f;

    v3 xaxis, yaxis, zaxis;
    xaxis.x = cost + 2.0f * q.x * q.x;
    xaxis.y = xy - zw;
    xaxis.z = xz + yw;

    yaxis.x = xy + zw;
    yaxis.y = cost + 2.0f * q.y * q.y;
    yaxis.z = yz - xw;

    zaxis.x = xz - yw;
    zaxis.y = yz + xw;
    zaxis.z = cost + 2.0f * q.z * q.z;

    *dst = (m4){};

    *m4_at(dst, 0, 0) = xaxis.x;
    *m4_at(dst, 1, 0) = xaxis.y;
    *m4_at(dst, 2, 0) = xaxis.z;

    *m4_at(dst, 0, 1) = yaxis.x;
    *m4_at(dst, 1, 1) = yaxis.y;
    *m4_at(dst, 2, 1) = yaxis.z;

    *m4_at(dst, 0, 2) = zaxis.x;
    *m4_at(dst, 1, 2) = zaxis.y;
    *m4_at(dst, 2, 2) = zaxis.z;

    *m4_at(dst, 3, 3) = 1.0f;

    *m4_at(dst, 3, 0) = -v3_dot(xaxis, pos);
    *m4_at(dst, 3, 1) = -v3_dot(yaxis, pos);
    *m4_at(dst, 3, 2) = -v3_dot(zaxis, pos);
}
