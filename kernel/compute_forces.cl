typedef struct {
    float x;
    float y;
} vec2f;

// TODO: TEST
__kernel void compute_forces(const unsigned long size, __global const vec2f* distances, __global vec2f* velocities) {
    const float G = 100000;

    int gid = get_global_id(0);

    int i = gid / size;
    int j = gid % size;

    velocities[i].x += G / (distances[gid].x * distances[gid].x);
    velocities[i].y += G / (distances[gid].y * distances[gid].y);
}