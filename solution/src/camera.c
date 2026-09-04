#include "camera.h"

static void camera_rebuild_axes(t_camera *cam)
{
    cam->forward.x = cosFix32(cam->angle);
    cam->forward.y = sinFix32(cam->angle);
    cam->right.x = cam->forward.y;
    cam->right.y = -cam->forward.x;
}

void    camera_init(t_camera *cam, fix32 x, fix32 y, u16 angle)
{
    cam->pos.x = x;
    cam->pos.y = y;
    cam->angle = angle & 1023;
    camera_rebuild_axes(cam);
}

void    camera_turn(t_camera *cam, s16 delta_angle)
{
    cam->angle = (cam->angle + delta_angle) & 1023;
    camera_rebuild_axes(cam);
}

void    camera_move(t_camera *cam, fix32 delta_forward)
{
    cam->pos = vec2_add(cam->pos, vec2_scale(cam->forward, delta_forward));
}
