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
    while (i < scene->count) {
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
    while (i < scene->count) {
        sid = scene->items[i].sprite_id;
        proj = scaler_project(cam, scene->items[i].pos);
        if (!proj.visible) {
            if (sprites[i]) {
                SPR_releaseSprite(sprites[i]);
                sprites[i] = NULL;
                cur_tier[i] = -1;
            }
            i++;
            continue;
        }
        if (proj.tier != cur_tier[i]) {
            if (sprites[i])
                SPR_releaseSprite(sprites[i]);
            /*
            ** Real bug #1, found by an actual reader: SPR_addSprite
            ** can return NULL from VRAM fragmentation alone, even
            ** with free space -- and constantly releasing/re-adding
            ** different-sized sprites as tiers change is exactly what
            ** fragments it. The old code set cur_tier[i] regardless,
            ** so a failed allocation left sprites[i] NULL while
            ** cur_tier[i] said "up to date" -- next frame's
            ** SPR_setPosition(NULL, ...) corrupted the sprite engine
            ** for a frame before the billboard vanished. SGDK's own
            ** header steers you toward SPR_addSpriteSafe for exactly
            ** this failure mode -- tried it first, and it returned
            ** NULL on the very first sprite of a fresh boot, nothing
            ** else on screen to fragment against, so it was dropped.
            ** Only recording the tier on actual success is the real
            ** fix: a failed allocation retries next frame instead of
            ** calling SPR_setPosition on a NULL sprite.
            **
            ** Real bug #2, same symptom's other half: with that fixed,
            ** a billboard could still flash back at a stale position
            ** for one frame right after being correctly culled --
            ** confirmed by an automated pixel-count scan across 60
            ** frames, comparing this exact loop with and without the
            ** flag below (spikes at isolated frames on the same scan
            ** that's clean throughout with it). Root cause: SGDK
            ** delays a sprite's tile/position update under DMA
            ** pressure by default -- exactly what 8 billboards
            ** releasing and reallocating sprites the same frame
            ** creates -- so a just-released sprite's hardware table
            ** entry could still show its last position for a frame.
            ** SPR_FLAG_DISABLE_DELAYED_FRAME_UPDATE forces the update
            ** to happen immediately instead.
            */
            sprites[i] = SPR_addSpriteEx(DEFS[sid][proj.tier],
                proj.screen_x, proj.screen_y, TILE_ATTR(PAL1, TRUE, FALSE, FALSE), 0,
                SPR_FLAG_AUTO_VRAM_ALLOC | SPR_FLAG_AUTO_SPRITE_ALLOC |
                SPR_FLAG_AUTO_TILE_UPLOAD | SPR_FLAG_AUTO_VISIBILITY |
                SPR_FLAG_DISABLE_DELAYED_FRAME_UPDATE);
            cur_tier[i] = sprites[i] ? proj.tier : -1;
        }
        else
            SPR_setPosition(sprites[i], proj.screen_x, proj.screen_y);
        i++;
    }
    SPR_update();
}
