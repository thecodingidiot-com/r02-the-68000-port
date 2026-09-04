#ifndef SCENE_H
# define SCENE_H

# include "vec2.h"

# define SPRITE_TREE    0
# define SPRITE_ROCK    1

typedef struct s_billboard
{
    t_vec2  pos;
    int     sprite_id;
}   t_billboard;

typedef struct s_scene
{
    t_billboard const  *items;
    int                 count;
}   t_scene;

void    scene_load(t_scene *scene);

#endif
