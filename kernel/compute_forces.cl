typedef struct {
    float x;
    float y;
} vec2f;

__kernel void compute_forces(const unsigned long size, __global const vec2f* distances, __global vec2f* velocities, const float elapsed) {
    const float G = 10000000;

    int gid = get_global_id(0);

    int i = gid / size;
    int j = gid % size;

    float norm_x = distances[gid].x * distances[gid].x;
    float norm_y = distances[gid].y * distances[gid].y;
    float norm = max(0.001f, sqrt(norm_x + norm_y));

    vec2f direction = {
        .x = distances[gid].x / norm,
        .y = distances[gid].y / norm
    };

    vec2f resulting_force = {
        .x = G / norm * elapsed * direction.x,
        .y = G / norm * elapsed * direction.y
    };

    velocities[i].x -= resulting_force.x;
    velocities[i].y -= resulting_force.y;

    velocities[j].x += resulting_force.x;
    velocities[j].y += resulting_force.y;
}