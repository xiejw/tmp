// vim: ft=c

#include <metal_stdlib>
#include <metal_compute>
using namespace metal;

// TODO
// assume m == n == k == 4096*2 for now
// assume both a and b are row marjo
//
// launch thread group as (16, 16)
// threadgroup is 8 (16 * 16) group
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
        const int64_t col = gid.x;

        const int64_t col_tile_offset = tid.x;
        const int64_t row_tile_offset = tid.y;

        threadgroup float * a_buf = buf;
        threadgroup float * b_buf = ((threadgroup float*)buf) + GROUPS * GROUP_DIM * GROUP_DIM;

        // phases are runing along the k dimension, so here
        const int64_t ph_count = (k / GROUP_DIM);

        // according to tests, array is on local reg file.
        float v[GROUPS] = {0};

        for (int64_t ph = 0; ph < ph_count; ph++ ) {

                uint64_t group_y = group_in_grid.y;

                {
                        // #define emit_load(i) a_buf[(row_tile_offset + i * GROUP_DIM ) * GROUP_DIM + col_tile_offset] = a[ ( group_y*  GROUPS * GROUP_DIM  + row_tile_offset + i * GROUP_DIM) * k + ph * GROUP_DIM + col_tile_offset]

                        // this rewriting improves 140ms; wtf>>>
                        uint64_t offset  = 0;
                        uint64_t offset_a = 0;
                        uint64_t a_buf_pos= (row_tile_offset ) * GROUP_DIM + col_tile_offset;
                        uint64_t a_pos = ( group_y*  GROUPS * GROUP_DIM  + row_tile_offset  ) * k + ph * GROUP_DIM + col_tile_offset;

#define emit_load(i) a_buf[ a_buf_pos + offset ] = a[a_pos  + offset_a]; \
                        offset += GROUP_DIM * GROUP_DIM; \
                        offset_a += GROUP_DIM * k;

                        static_assert(GROUPS==8, "group size is not expected");

                        //emit_load(15);
                        //emit_load(14);
                        //emit_load(13);
                        //emit_load(12);
                        //emit_load(11);
                        //emit_load(10);
                        //emit_load( 9);
                        //emit_load( 8);
                        emit_load( 7);
                        emit_load( 6);
                        emit_load( 5);
                        emit_load( 4);
                        emit_load( 3);
                        emit_load( 2);
                        emit_load( 1);
                        emit_load( 0);
                }

                b_buf[row_tile_offset * GROUP_DIM + col_tile_offset] = b[(ph * GROUP_DIM + row_tile_offset) * n + col];
                threadgroup_barrier(mem_flags::mem_threadgroup);

                // loop GROUPs
#define emit_inst(i) { \
                float tmp = b_buf[ i * GROUP_DIM + col_tile_offset];                            \
                                                                                                \
                int64_t pos = (row_tile_offset) * GROUP_DIM +  i;                 \
                int64_t offset = GROUP_DIM * GROUP_DIM;    \
                v[0] += a_buf[pos] * tmp;  pos += offset;       \
                v[1] += a_buf[pos] * tmp;  pos += offset;        \
                v[2] += a_buf[pos] * tmp;  pos += offset;        \
                v[3] += a_buf[pos] * tmp;  pos += offset;        \
                v[4] += a_buf[pos] * tmp;  pos += offset;        \
                v[5] += a_buf[pos] * tmp;  pos += offset;        \
                v[6] += a_buf[pos] * tmp;  pos += offset;        \
                v[7] += a_buf[pos] * tmp;  pos += offset;        \
}

                // GROUP_DIM
                emit_inst( 0);
                emit_inst( 1);
                emit_inst( 2);
                emit_inst( 3);
                emit_inst( 4);
                emit_inst( 5);
                emit_inst( 6);
                emit_inst( 7);
                emit_inst( 8);
                emit_inst( 9);
                emit_inst(10);
                emit_inst(11);
                emit_inst(12);
                emit_inst(13);
                emit_inst(14);
                emit_inst(15);

#undef emit_inst

                threadgroup_barrier(mem_flags::mem_threadgroup);
        }

        {
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 0 * GROUP_DIM + row_tile_offset)* n + col] = v[0];
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 1 * GROUP_DIM + row_tile_offset)* n + col] = v[1];
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 2 * GROUP_DIM + row_tile_offset)* n + col] = v[2];
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 3 * GROUP_DIM + row_tile_offset)* n + col] = v[3];
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 4 * GROUP_DIM + row_tile_offset)* n + col] = v[4];
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 5 * GROUP_DIM + row_tile_offset)* n + col] = v[5];
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 6 * GROUP_DIM + row_tile_offset)* n + col] = v[6];
                // c[(group_in_grid.y *  GROUPS * GROUP_DIM  + 7 * GROUP_DIM + row_tile_offset)* n + col] = v[7];


                // did not improve
                uint64_t c_pos = (group_in_grid.y *  GROUPS * GROUP_DIM  + row_tile_offset)* n + col;
                uint64_t offset = GROUP_DIM * n;
                thread float *vp = v;

#define emit_inst(i) {c[c_pos] = *vp; c_pos += offset; vp++;}

                emit_inst( 0);
                emit_inst( 1);
                emit_inst( 2);
                emit_inst( 3);
                emit_inst( 4);
                emit_inst( 5);
                emit_inst( 6);
                emit_inst( 7);

#undef emit_inst
        }
}
