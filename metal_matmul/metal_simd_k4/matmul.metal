// vim: ft=c

#include <metal_stdlib>
#include <metal_compute>
#include <metal_simdgroup_matrix>

using namespace metal;

#include "const.h"

kernel void matmul_op_tile(
                device const float* a,
                device const float* b,
                device float* c,
                constant int64_t& m,
                constant int64_t& n,
                constant int64_t& k,
                // threadgroup float  * buf [[threadgroup(0)]],
                const ushort2 gid [[thread_position_in_grid]],
                const ushort2 group_in_grid [[threadgroup_position_in_grid]],
                const ushort2 tid [[thread_position_in_threadgroup]])
{
        const int64_t tile_row_idx = tid.y;

        const uint64_t group_col = group_in_grid.x;
        const uint64_t group_row = group_in_grid.y;

        simdgroup_float8x8 sg_a;
        simdgroup_float8x8 sg_b;
        simdgroup_float8x8 sg_acc = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);

        // contracting dim, following SIMD GROUP size
        const uint64_t phase_count = k / SIMDGROUP_MAT_DIM;

        const uint64_t global_row_idx = (group_row * GROUP_SIZE_Y + tile_row_idx) * SIMDGROUP_MAT_DIM;
        const uint64_t global_col_idx = (group_col) * SIMDGROUP_MAT_DIM;

        for (uint64_t ph_idx = 0; ph_idx < phase_count; ph_idx++) {

                const uint64_t a_pos = global_row_idx * k + ph_idx * SIMDGROUP_MAT_DIM;
                const uint64_t b_pos = ph_idx * SIMDGROUP_MAT_DIM * n + global_col_idx;

                simdgroup_load(sg_a, a + a_pos, k);
                simdgroup_load(sg_b, b + b_pos, n);

                simdgroup_multiply_accumulate(sg_acc, sg_a, sg_b, sg_acc);
        }

        simdgroup_store(sg_acc, c + global_row_idx * n + global_col_idx, n);
}
