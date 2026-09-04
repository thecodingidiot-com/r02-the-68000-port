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

/* A solid-colour tile is just the same 4-bit palette index repeated
** for all 64 pixels -- 8 identical bytes per row, so the same byte
** value written 4 times makes one row regardless of CPU endianness.
** No image file, no rescomp, no VRAM budget beyond two tiles. */
static const u32 SKY_TILE[8] =
{
    0x44444444, 0x44444444, 0x44444444, 0x44444444,
    0x44444444, 0x44444444, 0x44444444, 0x44444444,
};

static const u32 GROUND_TILE[8] =
{
    0x55555555, 0x55555555, 0x55555555, 0x55555555,
    0x55555555, 0x55555555, 0x55555555, 0x55555555,
};

void    render_backdrop(void)
{
    u16 horizon_row;

    horizon_row = HORIZON_Y / 8;
    VDP_loadTileData(SKY_TILE, TILE_USERINDEX, 1, DMA);
    VDP_loadTileData(GROUND_TILE, TILE_USERINDEX + 1, 1, DMA);
    VDP_fillTileMapRect(BG_A,
        TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, TILE_USERINDEX),
        0, 0, WINDOW_W / 8, horizon_row);
    VDP_fillTileMapRect(BG_A,
        TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, TILE_USERINDEX + 1),
        0, horizon_row, WINDOW_W / 8, (WINDOW_H / 8) - horizon_row);
}

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
