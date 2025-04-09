typedef struct {
    float x;
    float y;
} vec2f;

__kernel void update_positions(__global vec2f* p, __global const vec2f* v, const float elapsed) {       
    int gid = get_global_id(0);
                       
    p[gid].x += v[gid].x * elapsed;
    p[gid].y += v[gid].y * elapsed;
};
