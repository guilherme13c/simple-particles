typedef struct {
    float x;
    float y;
} vec2f;

__kernel void compute_forces(const unsigned long size, __global const vec2f* distances, __global vec2f* velocities, const float elapsed) {
    const float G = 100;

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

    velocities[i].x -= G / norm * elapsed * direction.x;
    velocities[i].y -= G / norm * elapsed * direction.y;

    velocities[j].x += G / norm * elapsed * direction.x;
    velocities[j].y += G / norm * elapsed * direction.y;
}