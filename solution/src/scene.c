#include "scene.h"
#include "scaler.h"

/*
** No fopen() here -- there is no filesystem on a cartridge. r01's
** scene1.txt becomes plain compiled-in data: an array baked into the
** ROM at build time, exactly like any other const. This is the same
** four-stations-of-two-billboards layout r01 shipped with, just
** written as C instead of parsed from a text file.
*/
static const t_billboard ROAD[] =
{
    {{FIX32(20), FIX32(-6)}, SPRITE_TREE},
    {{FIX32(20), FIX32(6)}, SPRITE_ROCK},
    {{FIX32(45), FIX32(-6)}, SPRITE_ROCK},
    {{FIX32(45), FIX32(6)}, SPRITE_TREE},
    {{FIX32(75), FIX32(-6)}, SPRITE_TREE},
    {{FIX32(75), FIX32(6)}, SPRITE_ROCK},
    {{FIX32(110), FIX32(-6)}, SPRITE_ROCK},
    {{FIX32(110), FIX32(6)}, SPRITE_TREE},
};

void    scene_load(t_scene *scene)
{
    scene->items = ROAD;
    scene->count = sizeof(ROAD) / sizeof(ROAD[0]);
}
