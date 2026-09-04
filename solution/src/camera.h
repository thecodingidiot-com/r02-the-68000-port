#ifndef CAMERA_H
# define CAMERA_H

# include <genesis.h>
# include "vec2.h"

/* Angle is an integer in [0, 1023] -- 1024 steps around a full circle,
** matching SGDK's sinFix32()/cosFix32() lookup table convention. There
** is no radians here; there is no float here at all. */
typedef struct s_camera
{
    t_vec2  pos;
    u16     angle;
    t_vec2  forward;
    t_vec2  right;
}   t_camera;

void    camera_init(t_camera *cam, fix32 x, fix32 y, u16 angle);
void    camera_turn(t_camera *cam, s16 delta_angle);
void    camera_move(t_camera *cam, fix32 delta_forward);

#endif
