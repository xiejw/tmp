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
                constant int& m,
                constant int& n,
                constant int64_t& k,
                const ushort2 gid [[thread_position_in_grid]],
                const ushort2 group_in_grid [[threadgroup_position_in_grid]],
                const ushort2 tid [[thread_position_in_threadgroup]])
{
        const int wrap_id = tid.x / SIMDGROUP_SIZE;
        // const int lane_id = tid.x % SIMDGROUP_SIZE;

        const int group_col = group_in_grid.x;
        const int group_row = group_in_grid.y;

        simdgroup_float8x8 sg_a;
        simdgroup_float8x8 sg_b;

        // TODO we know we need 8 wraps. So we need 8 acc for for each wrap to track c
        static_assert(SIMDGROUPS_PER_GROUP == 8, "");
        simdgroup_float8x8 sg_acc_0 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_1 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_2 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_3 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_4 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_5 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_6 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_7 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);

        // simdgroup_float8x8 sg_acc_8 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        // simdgroup_float8x8 sg_acc_9 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        // simdgroup_float8x8 sg_acc_a = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        // simdgroup_float8x8 sg_acc_b = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        // simdgroup_float8x8 sg_acc_c = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        // simdgroup_float8x8 sg_acc_d = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        // simdgroup_float8x8 sg_acc_e = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        // simdgroup_float8x8 sg_acc_f = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);

        //
        // base ptrs
        device const float * a_base_ptr = a + group_row * TILE_SIZE * k; // change row
        device const float * b_base_ptr = b + group_col * TILE_SIZE;     // change col

        for (int k_idx = 0; k_idx < k; k_idx += K_TILE_SIZE) {

                //static_assert(TILE_SIZE == 32, "32 must be tile size");
                device const float * a_ptr = a_base_ptr + k_idx;      // change col due to k
                device const float * b_ptr = b_base_ptr + k_idx * n;  // change row due to k

                //
                // now we have 4 wraps and 4 8x8 in row and col in each tile.
                // So we assign each wrap to work on one tiling
                const int buf_row_stride = n;

                #pragma unroll(SIMDGROUPS_TILES_PER_K_TILE)
                for (int K_TILE_ID = 0; K_TILE_ID < SIMDGROUPS_TILES_PER_K_TILE; K_TILE_ID++) {
                        device const float * buf_ptr_to_a = a_ptr +
                                wrap_id * SIMDGROUP_MAT_DIM * buf_row_stride +
                                (K_TILE_ID) * SIMDGROUP_MAT_DIM;
                        device const float * buf_ptr_to_b = b_ptr +
                                (K_TILE_ID) * SIMDGROUP_MAT_DIM * buf_row_stride;

                        simdgroup_load(sg_a, buf_ptr_to_a, buf_row_stride);
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_0, sg_a, sg_b, sg_acc_0);

                        buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_1, sg_a, sg_b, sg_acc_1);

                        buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_2, sg_a, sg_b, sg_acc_2);

                        buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_3, sg_a, sg_b, sg_acc_3);

                        buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_4, sg_a, sg_b, sg_acc_4);

                        buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_5, sg_a, sg_b, sg_acc_5);

                        buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_6, sg_a, sg_b, sg_acc_6);

                        buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        simdgroup_multiply_accumulate(sg_acc_7, sg_a, sg_b, sg_acc_7);

                        /////

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_8, sg_a, sg_b, sg_acc_8);

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_9, sg_a, sg_b, sg_acc_9);

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_a, sg_a, sg_b, sg_acc_a);

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_b, sg_a, sg_b, sg_acc_b);

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_c, sg_a, sg_b, sg_acc_c);

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_d, sg_a, sg_b, sg_acc_d);

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_e, sg_a, sg_b, sg_acc_e);

                        // buf_ptr_to_b += SIMDGROUP_MAT_DIM;
                        // simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);
                        // simdgroup_multiply_accumulate(sg_acc_f, sg_a, sg_b, sg_acc_f);
                }

                // NOTE: This is needed to avoid cache miss.
                threadgroup_barrier(mem_flags::mem_threadgroup);
        }

        device float * c_base_ptr = c + group_row * TILE_SIZE * n + group_col * TILE_SIZE;
        device float * c_wrap_ptr = c_base_ptr + wrap_id * SIMDGROUP_MAT_DIM * n;

        simdgroup_store(sg_acc_0, c_wrap_ptr , n);
        simdgroup_store(sg_acc_1, c_wrap_ptr +     SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_2, c_wrap_ptr + 2 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_3, c_wrap_ptr + 3 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_4, c_wrap_ptr + 4 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_5, c_wrap_ptr + 5 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_6, c_wrap_ptr + 6 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_7, c_wrap_ptr + 7 * SIMDGROUP_MAT_DIM , n);

        //
        // simdgroup_store(sg_acc_8, c_wrap_ptr + 8 * SIMDGROUP_MAT_DIM , n);
        // simdgroup_store(sg_acc_9, c_wrap_ptr + 9 * SIMDGROUP_MAT_DIM , n);
        // simdgroup_store(sg_acc_a, c_wrap_ptr + 10 * SIMDGROUP_MAT_DIM , n);
        // simdgroup_store(sg_acc_b, c_wrap_ptr + 11 * SIMDGROUP_MAT_DIM , n);
        // simdgroup_store(sg_acc_c, c_wrap_ptr + 12 * SIMDGROUP_MAT_DIM , n);
        // simdgroup_store(sg_acc_d, c_wrap_ptr + 13 * SIMDGROUP_MAT_DIM , n);
        // simdgroup_store(sg_acc_e, c_wrap_ptr + 14 * SIMDGROUP_MAT_DIM , n);
        // simdgroup_store(sg_acc_f, c_wrap_ptr + 15 * SIMDGROUP_MAT_DIM , n);
}
