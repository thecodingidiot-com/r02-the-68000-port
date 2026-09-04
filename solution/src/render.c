#include "render.h"
#include "scaler.h"
#include "sprites.h"

/*
** One SpriteDefinition per (subject, hardware size) pair -- rescomp
** cannot vary a single SpriteDefinition's VDP size frame to frame, so
** "the same billboard at a different size" really is a different
** sprite here, not a different frame of one.
*/
static const SpriteDefinition *DEFS[2][SPRITE_TIERS] =
{
    { &tree1, &tree2, &tree3, &tree4 },
    { &rock1, &rock2, &rock3, &rock4 },
};

static Sprite  *sprites[MAX_BILLBOARDS];
static s8       cur_tier[MAX_BILLBOARDS];

void    render_init(t_scene const *scene)
{
    int i;

    i = 0;
    while (i < scene->count)
    {
        sprites[i] = NULL;
        cur_tier[i] = -1;
        i++;
    }
}

void    render_scene(t_scene const *scene, t_camera const *cam)
{
    t_projection    proj;
    int             i;
    int             sid;

    i = 0;
    while (i < scene->count)
    {
        sid = scene->items[i].sprite_id;
        proj = scaler_project(cam, scene->items[i].pos);
        if (!proj.visible)
        {
            if (sprites[i])
            {
                SPR_releaseSprite(sprites[i]);
                sprites[i] = NULL;
                cur_tier[i] = -1;
            }
            i++;
            continue;
        }
        if (proj.tier != cur_tier[i])
        {
            if (sprites[i])
                SPR_releaseSprite(sprites[i]);
            sprites[i] = SPR_addSprite(DEFS[sid][proj.tier],
                proj.screen_x, proj.screen_y, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
            cur_tier[i] = proj.tier;
        }
        else
        {
            SPR_setPosition(sprites[i], proj.screen_x, proj.screen_y);
        }
        i++;
    }
    SPR_update();
}
