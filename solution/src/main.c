#include <genesis.h>
#include "camera.h"
#include "scaler.h"
#include "scene.h"
#include "render.h"

#define STRAFE_SPEED    FIX32(0.3)
#define MOVE_SPEED      FIX32(0.6)
#define MIN_SIDE        FIX32(-10)
#define MAX_SIDE        FIX32(10)

/*
** Real bug, found by an actual reader running the compiled ROM: this
** used to be PAL_setPalette(PAL1, tree1.palette->data) -- borrowing
** whichever sprite's own bundled palette happened to be handy.
** rescomp trims each sprite's palette to only the colours ITS OWN
** pixels use, so tree1's copy stopped at index 2 (its trunk/leaf
** colours) -- indices 4/5, the backdrop's sky/ground, were never in
** it at all. Every sprite drew fine (each one's own colours were
** always present); the backdrop stayed black regardless of draw
** order, because the colours it needed were silently missing from
** CRAM. Fixed with an explicit palette that owns every colour this
** program actually uses, not borrowed from any one resource.
*/
static const u16 PALETTE[16] =
{
    RGB24_TO_VDPCOLOR(0x000000),
    RGB24_TO_VDPCOLOR(0x5a3a1e),
    RGB24_TO_VDPCOLOR(0x2e8b57),
    RGB24_TO_VDPCOLOR(0x7a7a7a),
    RGB24_TO_VDPCOLOR(0x5c9de8),
    RGB24_TO_VDPCOLOR(0x4a4a4a),
};

/*
** This used to call camera_turn() here -- true rotation, the same
** model r01-the-scaler's own first draft used before its own research
** (actually playing a real Space Harrier ROM, in g03-the-getaway)
** caught that none of the cabinets this project's Part III is modelled
** on ever rotate the camera to steer. r01 fixed it first; this port
** carries the same fix over rather than re-deriving it: cam->right
** never changes once nothing here rotates the camera, so left/right
** becomes a plain vector add instead, fenced by MIN_SIDE/MAX_SIDE so
** there's an actual edge either side of the road. camera_turn() is
** untouched and still real -- it just isn't what steering means here
** any more, same as r01.
*/
static void handle_input(t_camera *cam)
{
    u16 state;

    state = JOY_readJoypad(JOY_1);
    if (state & BUTTON_LEFT)
        cam->pos = vec2_add(cam->pos, vec2_scale(cam->right, -STRAFE_SPEED));
    if (state & BUTTON_RIGHT)
        cam->pos = vec2_add(cam->pos, vec2_scale(cam->right, STRAFE_SPEED));
    if (cam->pos.y > MAX_SIDE)
        cam->pos.y = MAX_SIDE;
    if (cam->pos.y < MIN_SIDE)
        cam->pos.y = MIN_SIDE;
    if (state & BUTTON_UP)
        camera_move(cam, MOVE_SPEED);
    if (state & BUTTON_DOWN)
        camera_move(cam, -MOVE_SPEED);
}

int main(bool hardReset)
{
    t_scene     scene;
    t_camera    cam;

    VDP_setScreenWidth320();
    PAL_setPalette(PAL1, PALETTE);
    render_backdrop();
    SPR_init();
    scene_load(&scene);
    camera_init(&cam, 0, 0, 0);
    render_init(&scene);
    while (1)
    {
        handle_input(&cam);
        render_scene(&scene, &cam);
        SYS_doVBlankProcess();
    }
    return (0);
}
