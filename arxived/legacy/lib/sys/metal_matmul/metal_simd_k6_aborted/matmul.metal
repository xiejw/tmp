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
                threadgroup float  * buf [[threadgroup(0)]],
                const ushort2 gid [[thread_position_in_grid]],
                const ushort2 group_in_grid [[threadgroup_position_in_grid]],
                const ushort2 tid [[thread_position_in_threadgroup]])
{
        const int64_t wrap_id = tid.x / SIMDGROUP_SIZE;
        const int64_t lane_id = tid.x % SIMDGROUP_SIZE;

        const uint64_t group_col = group_in_grid.x;
        const uint64_t group_row = group_in_grid.y;

        simdgroup_float8x8 sg_a;
        simdgroup_float8x8 sg_b;

        static_assert(SIMDGROUPS_PER_GROUP == 8, "");
        // TODO we know we need 8 wraps. So we need 8 acc for for each wrap to track c
        simdgroup_float8x8 sg_acc_0 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_1 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_2 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_3 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_4 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_5 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_6 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_7 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);

        // TODO consider to shift a little to avoid bank conflicts
        threadgroup float * a_buf = buf;
        threadgroup float * b_buf = buf + N_ELEMS_PER_MATRIX;

        //
        // base ptrs
        device const float * a_base_ptr = a + group_row * TILE_SIZE * k; // change row
        device const float * b_base_ptr = b + group_col * TILE_SIZE;     // change col

        for (int64_t k_idx = 0; k_idx < k; k_idx += TILE_SIZE) {

                //
                // load a and b to buf
                {
                        device const float * a_ptr = a_base_ptr + k_idx;      // change col due to k
                        device const float * b_ptr = b_base_ptr + k_idx * n;  // change row due to k
                                                                              //
                        // we have TILE_SIZE == 64, so we can use one wrap to load one tile row.
                        //
                        static_assert(TILE_SIZE == 64, "64 must be tile size");

                        // wrap 0, 1, 2, 3 working on a
                        // wrap 4, 5, 6, 7 working on b
                        device const float * device_ptr_to_load = (wrap_id / 4 == 0) ? a_ptr : b_ptr;
                        threadgroup  float * buf_ptr_to_store =   (wrap_id / 4 == 0) ? a_buf : b_buf;

                        const uint64_t buf_row_stride = TILE_SIZE;
                        const uint64_t ptr_row_stride = n;  // TODO n == k

#define emit_inst(ROW) \
                        {                                                                                 \
                                /* wrap (0, 1) to load 0, 2, 4, ... */                                      \
                                /* wrap (2, 3) to load 1, 3, 5, ... */                                      \
                                uint64_t row_id = (2 * (ROW) + ((wrap_id % 4) / 2));                            \
                                uint64_t col_id = ((wrap_id % 4) % 2); \
                                *(buf_ptr_to_store + row_id * buf_row_stride +  col_id * SIMDGROUP_SIZE + lane_id) =                 \
                                    *(device_ptr_to_load + row_id * ptr_row_stride +  col_id * SIMDGROUP_SIZE + lane_id);            \
                        }

                        UNROLL(HALF_TILE_SIZE);
#undef emit_inst
                        threadgroup_barrier(mem_flags::mem_threadgroup);
                }

                {
                        //
                        // now we have 8 wraps and 8 8x8 in row and col in each tile.
                        // So we assign each wrap to work on one tiling
                        const uint64_t buf_row_stride = TILE_SIZE;

#define emit_inst(K_TILE_ID) \
                        {                                                                         \
                                threadgroup  float * buf_ptr_to_a = a_buf +                       \
                                    wrap_id * SIMDGROUP_MAT_DIM * buf_row_stride +                \
                                    (K_TILE_ID) * SIMDGROUP_MAT_DIM;                              \
                                threadgroup  float * buf_ptr_to_b = b_buf +                       \
                                    (K_TILE_ID) * SIMDGROUP_MAT_DIM * buf_row_stride;             \
                                                                                                  \
                                simdgroup_load(sg_a, buf_ptr_to_a, buf_row_stride);               \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_0, sg_a, sg_b, sg_acc_0);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_1, sg_a, sg_b, sg_acc_1);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_2, sg_a, sg_b, sg_acc_2);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_3, sg_a, sg_b, sg_acc_3);    \
                                \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_4, sg_a, sg_b, sg_acc_4);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_5, sg_a, sg_b, sg_acc_5);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_6, sg_a, sg_b, sg_acc_6);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride);               \
                                simdgroup_multiply_accumulate(sg_acc_7, sg_a, sg_b, sg_acc_7);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                        }

                        UNROLL(SIMDGROUPS_TILES_PER_K_TILE);
#undef emit_inst

                        threadgroup_barrier(mem_flags::mem_threadgroup);
                }
        }

        // At this stage, each wrap maintain 4 results, we need to write them back
        device float * c_base_ptr = c + group_row * TILE_SIZE * n + group_col * TILE_SIZE;
        device float * c_wrap_ptr = c_base_ptr + wrap_id * SIMDGROUP_MAT_DIM * n;           // change row

        simdgroup_store(sg_acc_0, c_wrap_ptr , n);
        simdgroup_store(sg_acc_1, c_wrap_ptr +     SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_2, c_wrap_ptr + 2 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_3, c_wrap_ptr + 3 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_4, c_wrap_ptr + 4 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_5, c_wrap_ptr + 5 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_6, c_wrap_ptr + 6 * SIMDGROUP_MAT_DIM , n);
        simdgroup_store(sg_acc_7, c_wrap_ptr + 7 * SIMDGROUP_MAT_DIM , n);
}
