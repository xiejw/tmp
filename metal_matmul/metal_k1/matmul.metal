// vim: ft=c

#include <metal_stdlib>
#include <metal_compute>
using namespace metal;

// TODO
// assume m == n == k == 4096*2 for now
// assume both a and b are row marjo
//
// launch thread group as (16, 16)
// threadgroup is 2 (16 * 16) group
#define GROUP_DIM 16
kernel void matmul_op_tile(
                device const float* a,
                device const float* b,
                device float* c,
                constant int64_t& m,
                constant int64_t& n,
                constant int64_t& k,
                threadgroup float  * buf [[threadgroup(0)]],
                ushort2 gid [[thread_position_in_grid]],
                ushort2 tid [[thread_position_in_threadgroup]])
{
        const int64_t col = gid.x;
        const int64_t row = gid.y;

        const int64_t col_tile_offset = tid.x;
        const int64_t row_tile_offset = tid.y;

        threadgroup float * a_buf = buf;
        threadgroup float * b_buf = ((threadgroup float*)buf) + GROUP_DIM * GROUP_DIM;

        const int64_t ph_count = (4096 * 2 / GROUP_DIM);

        float v = 0;
        for (int64_t ph = 0; ph < ph_count; ph++ ) {
                a_buf[row_tile_offset * GROUP_DIM + col_tile_offset] = a[row * k + ph * GROUP_DIM + col_tile_offset];
                b_buf[row_tile_offset * GROUP_DIM + col_tile_offset] = b[(ph * GROUP_DIM + row_tile_offset) * n + col];
                threadgroup_barrier(mem_flags::mem_threadgroup);

                for (int64_t i = 0; i < GROUP_DIM; i++) {
                        v += a_buf[row_tile_offset * GROUP_DIM + i] * b_buf[i * GROUP_DIM + col_tile_offset];
                }

                threadgroup_barrier(mem_flags::mem_threadgroup);
        }

        c[row * n + col] = v;
}
