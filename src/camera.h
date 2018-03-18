#pragma once

#include "math.h"

typedef void (*cam_log_fptr)(const char*, ...);
void cam_set_log(cam_log_fptr l);

struct cam_free {
    v3 position;
    quat orientation;
};

void cam_free_init(struct cam_free* dst, v3 pos, v3 axis, float angle);

void cam_free_mov(struct cam_free* cam, v3 dir, float dist);
void cam_free_rotate(struct cam_free* cam, v3 axis, float angleDeg);

struct cam_noroll {
    v3 position;
    quat orientation;

    float yaw;
    float pitch;
};

void cam_noroll_init(struct cam_noroll* cam, v3 position, float yaw,
                     float pitch);
void cam_noroll_update(struct cam_noroll* cam, float yaw_d, float pitch_d,
                       v3 pos_d);
