__kernel void compute_forces(__global const float* positions_x, __global const float* positions_y, __global float* velocities_x, __global float* velocities_y, const float sigma, const float epsilon, const float threshold, const int N) {
    int p1 = get_global_id(0);
    if (p1 >= N) return;
    
    float x1 = positions_x[p1];
    float y1 = positions_y[p1];
    
    for (int p2 = 0; p2 < N; ++p2) {
        if (p1 == p2) continue;
        
        float dx = positions_x[p2] - x1;
        float dy = positions_y[p2] - y1;
        float dist2 = dx * dx + dy * dy;
        
        if (dist2 < threshold) {
            float invDist2 = 1.0f / dist2;
            float invDist6 = invDist2 * invDist2 * invDist2;
            float invDist12 = invDist6 * invDist6;

            float forceMagnitude = 24 * epsilon * (2 * invDist12 - invDist6) * invDist2;

            atomic_add(&velocities_x[p1], forceMagnitude * dx);
            atomic_add(&velocities_y[p1], forceMagnitude * dy);
            atomic_add(&velocities_x[p2], -forceMagnitude * dx);
            atomic_add(&velocities_y[p2], -forceMagnitude * dy);
        }
    }
}
