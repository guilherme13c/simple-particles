typedef struct {
    float x;
    float y;
} vec2f;

__kernel void update_positions(__global vec2f* positions, __global vec2f* velocities, const float elapsed, const vec2f limits) {       
    int gid = get_global_id(0);
                       
    positions[gid].x += velocities[gid].x * elapsed;
    positions[gid].y += velocities[gid].y * elapsed;

    if (positions[gid].x < 0.0f) {
        positions[gid].x = -positions[gid].x;
        velocities[gid].x = -velocities[gid].x;
    } else if (positions[gid].x > limits.x) {
        positions[gid].x = 2.0f * limits.x - positions[gid].x;
        velocities[gid].x = -velocities[gid].x;
    }

    if (positions[gid].y < 0.0f) {
        positions[gid].y = -positions[gid].y;
        velocities[gid].y = -velocities[gid].y;
    } else if (positions[gid].y > limits.y) {
        positions[gid].y = 2.0f * limits.y - positions[gid].y;
        velocities[gid].y = -velocities[gid].y;
    }
};
