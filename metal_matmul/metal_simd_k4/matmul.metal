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
        simdgroup_float8x8 sgMatA;
        simdgroup_float8x8 sgMatB;
        simdgroup_float8x8 sgMatC;

        simdgroup_load(sgMatA, a);// , 8, 0, false);
        simdgroup_load(sgMatB, b);// , 8, 0, false);

        simdgroup_multiply(sgMatC, sgMatA, sgMatB);
        simdgroup_store(sgMatC, c); //, 8, 0, false);

        // const int64_t col_tile_offset = tid.x;
        // const int64_t row_tile_offset = tid.y;

        // const uint64_t group_col = group_in_grid.x;
        // const uint64_t group_row = group_in_grid.y;

        // threadgroup float * a_local_buf = buf;
        // threadgroup float * b_local_buf = ((threadgroup float*)buf) +
        //         GROUPS * GROUP_DIM * GROUP_DIM;

        // // phases are runing along the k dimension, so here no GROUPS.
        // const int64_t ph_count = (k / GROUP_DIM);

        // // according to tests, array is on local reg file.
        // thread float a_reg[GROUPS];
        // thread float b_reg[GROUPS];
        // thread float v[GROUPS * GROUPS] = {0};

        // for (int64_t ph = 0; ph < ph_count; ph++) {

        //         //
        //         // Load GROUPS of rows into local buf for a
        //         // We stack all groups in vertical way.
        //         {

        //                 uint64_t local_pos = row_tile_offset * GROUP_DIM + col_tile_offset;
        //                 uint64_t local_offset = 0;
        //                 uint64_t local_offset_inc = GROUP_DIM * GROUP_DIM;

        //                 uint64_t a_pos = (group_row * GROUPS * GROUP_DIM + row_tile_offset) * k +
        //                         ph * GROUP_DIM + col_tile_offset;
        //                 uint64_t a_offset = 0;
        //                 uint64_t a_offset_inc = GROUP_DIM * k;

        //                 #define emit_inst(i) a_local_buf[local_pos + local_offset] = \
        //                     a[a_pos + a_offset];                                     \
        //                     local_offset += local_offset_inc;                        \
        //                     a_offset     += a_offset_inc;

        //                 UNROLL(GROUPS);
        //                 #undef emit_inst
        //         }

        //         //
        //         // Load GROUPS of cols from b to local buf for b
        //         // We place all groups in horizontal way.
        //         {
        //                 uint64_t local_pos = row_tile_offset * (GROUP_DIM * GROUPS) +
        //                         col_tile_offset;
        //                 uint64_t local_offset = 0;
        //                 uint64_t local_offset_inc = GROUP_DIM; // horizontal shift

        //                 uint64_t b_pos = (ph * GROUP_DIM + row_tile_offset) * n +
        //                         group_col * GROUPS * GROUP_DIM + col_tile_offset;
        //                 uint64_t b_offset = 0;
        //                 uint64_t b_offset_inc = GROUP_DIM;

        //                 #define emit_inst(i) b_local_buf[local_pos + local_offset] = \
        //                     b[b_pos + b_offset];                                     \
        //                     local_offset += local_offset_inc;                        \
        //                     b_offset     += b_offset_inc;

        //                 UNROLL(GROUPS);
        //                 #undef emit_inst
        //         }
        //         threadgroup_barrier(mem_flags::mem_threadgroup);


        //         static_assert(GROUPS == 16 | GROUPS == 8 | GROUPS == 4, "hard coded unroll 4, 8, 16");

        //         for (uint64_t inner_idx = 0; inner_idx < GROUP_DIM; inner_idx++) {
        //                 //
        //                 // Loads from SRAM to local reg files
        //                 uint64_t a_pos = (row_tile_offset) * GROUP_DIM +  inner_idx;
        //                 uint64_t b_pos = inner_idx * GROUPS * GROUP_DIM + col_tile_offset;

//#define // emit_inst(k) \
        //                 a_reg[k] = a_local_buf[a_pos + (k) * GROUP_DIM * GROUP_DIM]; \
        //                 b_reg[k] = b_local_buf[b_pos + (k) * GROUP_DIM];

        //                 UNROLL(GROUPS);
//#undef e// mit_inst

//#define // emit_inst(i) \
        //                          v[(i) * GROUPS + (  0 )] += a_reg[(i)] * b_reg[( 0)];        \
        //                          v[(i) * GROUPS + (  1 )] += a_reg[(i)] * b_reg[( 1)];        \
        //                          v[(i) * GROUPS + (  2 )] += a_reg[(i)] * b_reg[( 2)];        \
        //                          v[(i) * GROUPS + (  3 )] += a_reg[(i)] * b_reg[( 3)];        \
        //                 if (GROUPS > 4) {                                                     \
        //                          v[(i) * GROUPS + (  4 )] += a_reg[(i)] * b_reg[( 4)];        \
        //                          v[(i) * GROUPS + (  5 )] += a_reg[(i)] * b_reg[( 5)];        \
        //                          v[(i) * GROUPS + (  6 )] += a_reg[(i)] * b_reg[( 6)];        \
        //                          v[(i) * GROUPS + (  7 )] += a_reg[(i)] * b_reg[( 7)];        \
        //                 }                                                                     \
        //                                                                                       \
        //                 if (GROUPS > 8) {                                                     \
        //                          v[(i) * GROUPS + (  8 )] += a_reg[(i)] * b_reg[( 8)];        \
        //                          v[(i) * GROUPS + (  9 )] += a_reg[(i)] * b_reg[( 9)];        \
        //                          v[(i) * GROUPS + ( 10 )] += a_reg[(i)] * b_reg[(10)];        \
        //                          v[(i) * GROUPS + ( 11 )] += a_reg[(i)] * b_reg[(11)];        \
        //                          v[(i) * GROUPS + ( 12 )] += a_reg[(i)] * b_reg[(12)];        \
        //                          v[(i) * GROUPS + ( 13 )] += a_reg[(i)] * b_reg[(13)];        \
        //                          v[(i) * GROUPS + ( 14 )] += a_reg[(i)] * b_reg[(14)];        \
        //                          v[(i) * GROUPS + ( 15 )] += a_reg[(i)] * b_reg[(15)];        \
        //                 }                                                                     \
        //                                                                                       \

        //                 UNROLL(GROUPS);
        //         }
        //         threadgroup_barrier(mem_flags::mem_threadgroup);
        // }

        // // write back.
        // {
        //         for (uint64_t v_row = 0; v_row < GROUPS; v_row++) {
        //                 const uint64_t c_pos_base = (group_row * GROUPS * GROUP_DIM +
        //                                 row_tile_offset + v_row * GROUP_DIM) * n;
        //                 const uint64_t c_pos = c_pos_base + (group_col * GROUPS * GROUP_DIM + col_tile_offset);
        //                 const uint64_t c_pos_offset_inc = GROUP_DIM;

        //                 const uint64_t v_pos = v_row * GROUPS;
        //                 for (uint64_t v_col = 0; v_col < GROUPS; v_col++) {
        //                         c[c_pos + v_col * c_pos_offset_inc] = v[v_pos + v_col];
        //                 }
        //         }
        // }
}
