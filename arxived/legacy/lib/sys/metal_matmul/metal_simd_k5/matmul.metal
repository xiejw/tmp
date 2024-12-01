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
                threadgroup float  * buf [[threadgroup(0)]],
                const ushort2 gid [[thread_position_in_grid]],
                const ushort2 group_in_grid [[threadgroup_position_in_grid]],
                const ushort2 tid [[thread_position_in_threadgroup]])
{
        const int wrap_id = tid.x / SIMDGROUP_SIZE;
        const int lane_id = tid.x % SIMDGROUP_SIZE;

        const int group_col = group_in_grid.x;
        const int group_row = group_in_grid.y;

        simdgroup_float8x8 sg_a;
        simdgroup_float8x8 sg_b;

        // TODO we know we need 4 wraps. So we need 4 acc for for each wrap to track c
        simdgroup_float8x8 sg_acc_0 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_1 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_2 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
        simdgroup_float8x8 sg_acc_3 = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);

        // TODO consider to shift a little to avoid bank conflicts
        threadgroup float * a_buf = buf;
        threadgroup float * b_buf = buf + N_ELEMS_PER_MATRIX;

        //
        // base ptrs
        device const float * a_base_ptr = a + group_row * TILE_SIZE * k; // change row
        device const float * b_base_ptr = b + group_col * TILE_SIZE;     // change col

        for (int k_idx = 0; k_idx < k; k_idx += TILE_SIZE) {

                //
                // load a and b to buf
                //
                // Strategy each kernel load 4 float (by levearing vec)
                // So if we have 4 wraps, then
                // wrap 0 is loading a with row 0,1,2,3
                // wrap 1 is loading a with row 4,5,6,7
                // wrap 2 is loading b with row 0,1,2,3
                // wrap 3 is loading b with row 4,5,6,7
                //
                //  repeat 4 times
                {
                        // we have TILE_SIZE == 32, so we can use one wrap to load one tile row.
                        //
                        static_assert(TILE_SIZE == 32, "32 must be tile size");
                        device const float * a_ptr = a_base_ptr + k_idx;      // change col due to k
                        device const float * b_ptr = b_base_ptr + k_idx * n;  // change row due to k

                        // wrap 0 and 1 working on a
                        // wrap 2 and 3 working on b
                        device const float * device_ptr_to_load_base = (wrap_id / 2 == 0) ? a_ptr : b_ptr;
                        threadgroup  float * buf_ptr_to_store_base =   (wrap_id / 2 == 0) ? a_buf : b_buf;

                        const int buf_row_stride = TILE_SIZE;
                        const int ptr_row_stride = n;  // TODO n == k

                        const int row_inc_due_to_lane = lane_id / 8;
                        const int lane_id_to_load = (lane_id % 8) * 4;

                        device const float * device_ptr_to_load ;
                        threadgroup  float * buf_ptr_to_store ;

#define emit_inst(ROW) \
                        {                                                                                 \
                                int row_id = (8 * (ROW) +  4* (wrap_id % 2) + row_inc_due_to_lane);                            \
                                device_ptr_to_load = device_ptr_to_load_base + row_id * ptr_row_stride + lane_id_to_load;\
                                buf_ptr_to_store = buf_ptr_to_store_base + row_id * buf_row_stride + lane_id_to_load;  \
                                buf_ptr_to_store[0] = device_ptr_to_load[0]; \
                                buf_ptr_to_store[1] = device_ptr_to_load[1]; \
                                buf_ptr_to_store[2] = device_ptr_to_load[2]; \
                                buf_ptr_to_store[3] = device_ptr_to_load[3]; \
                        }

                        UNROLL(QUARTER_TILE_SIZE);
#undef emit_inst
                        threadgroup_barrier(mem_flags::mem_threadgroup);
                }

                {
                        //
                        // now we have 4 wraps and 4 8x8 in row and col in each tile.
                        // So we assign each wrap to work on one tiling
                        const int buf_row_stride = TILE_SIZE;

// consider to tune this to improve bank conflicts
#define SHIFT 0

#define emit_inst(K_TILE_ID) \
                        {                                                                         \
                                threadgroup  float * buf_ptr_to_a = a_buf +                       \
                                    wrap_id * SIMDGROUP_MAT_DIM * buf_row_stride +                \
                                    (K_TILE_ID) * SIMDGROUP_MAT_DIM;                              \
                                threadgroup  float * buf_ptr_to_b = b_buf +                       \
                                    (K_TILE_ID) * SIMDGROUP_MAT_DIM * buf_row_stride;             \
                                                                                                  \
                                simdgroup_load(sg_a, buf_ptr_to_a, buf_row_stride + SHIFT);               \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride + SHIFT);               \
                                simdgroup_multiply_accumulate(sg_acc_0, sg_a, sg_b, sg_acc_0);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride + SHIFT);               \
                                simdgroup_multiply_accumulate(sg_acc_1, sg_a, sg_b, sg_acc_1);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride + SHIFT);               \
                                simdgroup_multiply_accumulate(sg_acc_2, sg_a, sg_b, sg_acc_2);    \
                                simdgroup_barrier(mem_flags::mem_none); \
                                                                                                  \
                                buf_ptr_to_b += SIMDGROUP_MAT_DIM;                                \
                                simdgroup_load(sg_b, buf_ptr_to_b, buf_row_stride + SHIFT);               \
                                simdgroup_multiply_accumulate(sg_acc_3, sg_a, sg_b, sg_acc_3);    \
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
}
