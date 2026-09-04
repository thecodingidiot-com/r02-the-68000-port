#ifndef RENDER_H
# define RENDER_H

# include <genesis.h>
# include "scene.h"
# include "camera.h"

# define MAX_BILLBOARDS 8

void    render_backdrop(void);
void    render_init(t_scene const *scene);
void    render_scene(t_scene const *scene, t_camera const *cam);

#endif
