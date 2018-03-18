#include "camera.h"

#include <stddef.h>

static cam_log_fptr text_log = NULL;

void cam_set_log(cam_log_fptr l) { text_log = l; }

void cam_free_init(struct cam_free* dst, v3 pos, v3 axis, float angle)
{
    dst->position = pos;
    quat_from_angle(&dst->orientation, axis, angle);
    quat_norm(&dst->orientation);
}

void cam_free_mov(struct cam_free* cam, v3 dir, float dist)
{
    quat_rot(&dir, cam->orientation);
    v3_mult(&dir, dist);
    v3_add(&cam->position, dir);
}

void cam_free_rotate(struct cam_free* cam, v3 axis, float angle)
{
    quat rq;
    quat_from_angle(&rq, axis, angle);
    quat_norm(&rq);
    quat_mult(&cam->orientation, rq, cam->orientation);
    quat_norm(&cam->orientation);
}

void cam_noroll_init(struct cam_noroll* cam, v3 position, float yaw,
                     float pitch)
{
    cam->position = position;
    cam->orientation = (quat){ .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f };
    cam->yaw = yaw;
    cam->pitch = pitch;

    cam_noroll_update(cam, 0.0f, 0.0f, (v3){});
}

void cam_noroll_update(struct cam_noroll* cam, float yaw_d, float pitch_d,
                       v3 pos_d)
{
    v3 up = (v3){ .data = { 0.0f, 1.0f, 0.0f } };
    v3 right = (v3){ .data = { 1.0f, 0.0f, 0.0f } };

    cam->yaw += yaw_d;
    cam->pitch += pitch_d;

    quat yaw, pitch;
    quat_from_angle(&yaw, up, cam->yaw);
    quat_from_angle(&pitch, right, cam->pitch);

    quat_mult(&cam->orientation, pitch, yaw);
    quat_norm(&cam->orientation);

    quat_rot(&pos_d, cam->orientation);
    v3_add(&cam->position, pos_d);
}
