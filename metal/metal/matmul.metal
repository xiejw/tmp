// vim: ft=c

#include <metal_stdlib>
using namespace metal;

kernel void matmul_op(
                device const float* a,
                device const float* b,
                device float* c,
                constant int64_t& m,
                constant int64_t& n,
                constant int64_t& k,
                ushort2 gid [[thread_position_in_grid]])
{
        const int64_t row = gid.x;
        const int64_t col = gid.y;
        float v = 0;
        for (int64_t i = 0; i < k; i++) {
                v += a[row * k + i] * b[i * n + col];
        }
        c[row * n + col] = v;
}
