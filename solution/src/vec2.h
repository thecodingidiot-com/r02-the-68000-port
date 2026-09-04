#ifndef VEC2_H
# define VEC2_H

# include <genesis.h>

typedef struct s_vec2
{
    fix32   x;
    fix32   y;
}   t_vec2;

t_vec2  vec2_add(t_vec2 a, t_vec2 b);
t_vec2  vec2_sub(t_vec2 a, t_vec2 b);
t_vec2  vec2_scale(t_vec2 a, fix32 s);
fix32   vec2_dot(t_vec2 a, t_vec2 b);

#endif
