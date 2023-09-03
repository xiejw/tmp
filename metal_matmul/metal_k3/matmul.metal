// vim: ft=c

#include <metal_stdlib>
#include <metal_compute>
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
        const int64_t col_tile_offset = tid.x;
        const int64_t row_tile_offset = tid.y;

        const uint64_t group_col = group_in_grid.x;
        const uint64_t group_row = group_in_grid.y;

        threadgroup float * a_local_buf = buf;
        threadgroup float * b_local_buf = ((threadgroup float*)buf) +
                GROUPS * GROUP_DIM * GROUP_DIM;

        // phases are runing along the k dimension, so here no GROUPS.
        const int64_t ph_count = (k / GROUP_DIM);

        // according to tests, array is on local reg file.
        thread float a_reg[GROUPS];
        thread float b_reg[GROUPS];
        thread float v[GROUPS * GROUPS] = {0};

        for (int64_t ph = 0; ph < ph_count; ph++) {

                //
                // Load GROUPS of rows into local buf for a
                // We stack all groups in vertical way.
                {

                        uint64_t local_pos = row_tile_offset * GROUP_DIM + col_tile_offset;
                        uint64_t local_offset = 0;
                        uint64_t local_offset_inc = GROUP_DIM * GROUP_DIM;

                        uint64_t a_pos = (group_row * GROUPS * GROUP_DIM + row_tile_offset) * k +
                                ph * GROUP_DIM + col_tile_offset;
                        uint64_t a_offset = 0;
                        uint64_t a_offset_inc = GROUP_DIM * k;

                        #define emit_inst(i) a_local_buf[local_pos + local_offset] = \
                            a[a_pos + a_offset];                                     \
                            local_offset += local_offset_inc;                        \
                            a_offset     += a_offset_inc;

                        UNROLL(GROUPS);
                        #undef emit_inst
                }

                //
                // Load GROUPS of cols from b to local buf for b
                // We place all groups in horizontal way.
                {
                        uint64_t local_pos = row_tile_offset * (GROUP_DIM * GROUPS) +
                                col_tile_offset;
                        uint64_t local_offset = 0;
                        uint64_t local_offset_inc = GROUP_DIM; // horizontal shift

                        uint64_t b_pos = (ph * GROUP_DIM + row_tile_offset) * n +
                                group_col * GROUPS * GROUP_DIM + col_tile_offset;
                        uint64_t b_offset = 0;
                        uint64_t b_offset_inc = GROUP_DIM;

                        #define emit_inst(i) b_local_buf[local_pos + local_offset] = \
                            b[b_pos + b_offset];                                     \
                            local_offset += local_offset_inc;                        \
                            b_offset     += b_offset_inc;

                        UNROLL(GROUPS);
                        #undef emit_inst
                }
                threadgroup_barrier(mem_flags::mem_threadgroup);


                for (uint64_t inner_idx = 0; inner_idx < GROUP_DIM; inner_idx++) {
                //{
                        //uint64_t inner_idx = 0;
                        //
                        // Loads from SRAM to local reg files
                        for (uint64_t k = 0; k < GROUPS; k++) {
                                //a_reg[k] = a_local_buf[(row_tile_offset) * GROUP_DIM + (k) * GROUP_DIM * GROUP_DIM + inner_idx];
                                //b_reg[k] = b_local_buf[inner_idx * GROUPS * GROUP_DIM + col_tile_offset + (k) * GROUP_DIM];
                                //
                                a_reg[k] = a_local_buf[(row_tile_offset) * GROUP_DIM + (k) * GROUP_DIM * GROUP_DIM + inner_idx];
                                b_reg[k] = b_local_buf[inner_idx * GROUPS * GROUP_DIM + col_tile_offset + (k) * GROUP_DIM];

                                //a_reg[k] = a_local_buf[(row_tile_offset * GROUPS + k ) * GROUP_DIM + inner_idx];
                                //b_reg[k] = b_local_buf[inner_idx * GROUPS * GROUP_DIM + col_tile_offset * GROUPS + (k)];
                        }

                        for (uint64_t v_row = 0; v_row < GROUPS; v_row++) {
                                const uint64_t v_pos = v_row * GROUPS;
                                for (int64_t v_col = 0; v_col < GROUPS; v_col++) {
                                        v[v_pos + v_col] += a_reg[v_row] * b_reg[v_col];
                                }
                        }
                }
                threadgroup_barrier(mem_flags::mem_threadgroup);
        }

        // write back.
        {
                for (uint64_t v_row = 0; v_row < GROUPS; v_row++) {
                        const uint64_t c_pos_base = (group_row * GROUPS * GROUP_DIM +
                                        row_tile_offset + v_row * GROUP_DIM) * n;
                        const uint64_t c_pos = c_pos_base + (group_col * GROUPS * GROUP_DIM + col_tile_offset);
                        const uint64_t c_pos_offset_inc = GROUP_DIM;

                        const uint64_t v_pos = v_row * GROUPS;
                        for (uint64_t v_col = 0; v_col < GROUPS; v_col++) {
                                c[c_pos + v_col * c_pos_offset_inc] = v[v_pos + v_col];
                        }
                }
        }
}
