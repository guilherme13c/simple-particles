typedef struct {
    float x;
    float y;
} vec2f;

__kernel void pair_differences(const unsigned long size, __global const vec2f* p, __global vec2f* dist) {
    int gid = get_global_id(0);

    int i = gid / size;
    int j = gid % size;

    dist[i * size + j].x = p[i].x - p[j].x;
    dist[i * size + j].y = p[i].y - p[j].y;
}