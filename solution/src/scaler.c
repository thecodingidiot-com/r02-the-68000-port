#include "scaler.h"

static const int TIER_SIZES[SPRITE_TIERS] = {8, 16, 24, 32};

int scaler_tier_size(int tier)
{
    return (TIER_SIZES[tier]);
}

/* r01's continuous h = WINDOW_H / depth still gets computed here, in
** full fixed-point precision -- it just can't be the sprite's real
** size any more. This picks whichever of the four hardware sizes it
** landed closest to. */
static int  nearest_tier(int ideal)
{
    int best;
    int best_diff;
    int diff;
    int i;

    best = 0;
    best_diff = ideal - TIER_SIZES[0];
    if (best_diff < 0)
        best_diff = -best_diff;
    i = 1;
    while (i < SPRITE_TIERS)
    {
        diff = ideal - TIER_SIZES[i];
        if (diff < 0)
            diff = -diff;
        if (diff < best_diff)
        {
            best_diff = diff;
            best = i;
        }
        i++;
    }
    return (best);
}

t_projection    scaler_project(t_camera const *cam, t_vec2 world_pos)
{
    t_projection    proj;
    t_vec2          rel;
    int             ideal;

    rel = vec2_sub(world_pos, cam->pos);
    proj.depth = vec2_dot(rel, cam->forward);
    proj.side = vec2_dot(rel, cam->right);
    if (proj.depth < NEAR_PLANE)
    {
        proj.visible = 0;
        return (proj);
    }
    proj.visible = 1;
    ideal = fix32ToInt(fix32Div(intToFix32(WINDOW_H), proj.depth));
    proj.tier = nearest_tier(ideal);
    proj.size = TIER_SIZES[proj.tier];
    proj.screen_x = WINDOW_W / 2
        + fix32ToInt(fix32Div(fix32Mul(intToFix32(WINDOW_H), proj.side), proj.depth))
        - proj.size / 2;
    proj.screen_y = HORIZON_Y - proj.size;
    return (proj);
}
