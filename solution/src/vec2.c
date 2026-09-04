#include "vec2.h"

t_vec2  vec2_add(t_vec2 a, t_vec2 b)
{
    t_vec2  r;

    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return (r);
}

t_vec2  vec2_sub(t_vec2 a, t_vec2 b)
{
    t_vec2  r;

    r.x = a.x - b.x;
    r.y = a.y - b.y;
    return (r);
}

t_vec2  vec2_scale(t_vec2 a, fix32 s)
{
    t_vec2  r;

    r.x = fix32Mul(a.x, s);
    r.y = fix32Mul(a.y, s);
    return (r);
}

fix32   vec2_dot(t_vec2 a, t_vec2 b)
{
    return (fix32Mul(a.x, b.x) + fix32Mul(a.y, b.y));
}
