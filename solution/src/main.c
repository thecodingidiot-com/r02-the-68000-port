#include <genesis.h>
#include "camera.h"
#include "scaler.h"
#include "scene.h"
#include "render.h"
#include "sprites.h"

#define TURN_SPEED  8
#define MOVE_SPEED  FIX32(0.6)

static void handle_input(t_camera *cam)
{
    u16 state;

    state = JOY_readJoypad(JOY_1);
    if (state & BUTTON_LEFT)
        camera_turn(cam, TURN_SPEED);
    if (state & BUTTON_RIGHT)
        camera_turn(cam, -TURN_SPEED);
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
    SPR_init();
    /* Every one of the eight sprite definitions shares this exact
    ** palette (gen_assets.sh builds them all from the same four
    ** colours) -- load it once, from any one of them. */
    PAL_setPalette(PAL1, tree1.palette->data);
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
