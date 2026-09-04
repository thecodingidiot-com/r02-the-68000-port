#ifndef SCALER_H
# define SCALER_H

# include <genesis.h>
# include "vec2.h"
# include "camera.h"

# define WINDOW_W       320
# define WINDOW_H       224
# define HORIZON_Y      160
# define NEAR_PLANE     FIX32(2)

/* The VDP can size a sprite from 1x1 to 4x4 tiles -- 8, 16, 24 or 32
** pixels a side. Nothing in between exists in hardware. */
# define SPRITE_TIERS   4

typedef struct s_projection
{
    fix32   depth;
    fix32   side;
    int     visible;
    int     tier;
    int     size;
    int     screen_x;
    int     screen_y;
}   t_projection;

int             scaler_tier_size(int tier);
t_projection    scaler_project(t_camera const *cam, t_vec2 world_pos);

#endif
