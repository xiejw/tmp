// vim: ft=c

#include <metal_stdlib>
#include <metal_compute>
using namespace metal;

#include "const.h"

// beautiful c macro unrolling. yay!
#define UNROLL(N) _UNROLL_IMPL(N)
#define _UNROLL_IMPL(N) UNROLL_ ## N(0)

#define UNROLL_32(i) UNROLL_16(i); UNROLL_16(16)

#define UNROLL_16(i) emit_inst(i); UNROLL_15(i+1)
#define UNROLL_15(i) emit_inst(i); UNROLL_14(i+1)
#define UNROLL_14(i) emit_inst(i); UNROLL_13(i+1)
#define UNROLL_13(i) emit_inst(i); UNROLL_12(i+1)
#define UNROLL_12(i) emit_inst(i); UNROLL_11(i+1)
#define UNROLL_11(i) emit_inst(i); UNROLL_10(i+1)
#define UNROLL_10(i) emit_inst(i); UNROLL_9(i+1)
#define UNROLL_9(i)  emit_inst(i); UNROLL_8(i+1)

#define UNROLL_8(i) emit_inst(i); UNROLL_7(i+1)
#define UNROLL_7(i) emit_inst(i); UNROLL_6(i+1)
#define UNROLL_6(i) emit_inst(i); UNROLL_5(i+1)
#define UNROLL_5(i) emit_inst(i); UNROLL_4(i+1)
#define UNROLL_4(i) emit_inst(i); UNROLL_3(i+1)
#define UNROLL_3(i) emit_inst(i); UNROLL_2(i+1)
#define UNROLL_2(i) emit_inst(i); UNROLL_1(i+1)
#define UNROLL_1(i) emit_inst(i)

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

        const int64_t col = gid.x;
        const uint64_t group_y = group_in_grid.y;

        threadgroup float * a_local_buf = buf;
        threadgroup float * b_local_buf = ((threadgroup float*)buf) +
                GROUPS * GROUP_DIM * GROUP_DIM;

        // phases are runing along the k dimension, so here
        const int64_t ph_count = (k / GROUP_DIM);

        // according to tests, array is on local reg file.
        float v[GROUPS] = {0};

        for (int64_t ph = 0; ph < ph_count; ph++ ) {

                // load GROUPS of rows into local buf for a
                {
                        // this rewriting improves 140ms; wtf
                        uint64_t local_offset = 0;
                        uint64_t a_offset = 0;

                        uint64_t local_pos = row_tile_offset * GROUP_DIM +
                                col_tile_offset;

                        uint64_t a_pos = (group_y * GROUPS * GROUP_DIM + row_tile_offset) * k +
                                ph * GROUP_DIM + col_tile_offset;

                        #define emit_inst(i) a_local_buf[local_pos + local_offset] = \
                            a[a_pos + a_offset];                                     \
                            local_offset += GROUP_DIM * GROUP_DIM;                   \
                            a_offset     += GROUP_DIM * k;

                        UNROLL(GROUPS);

                        #undef emit_inst

                }

                // load element from b to local buf for b
                b_local_buf[row_tile_offset * GROUP_DIM + col_tile_offset] = b[
                        (ph * GROUP_DIM + row_tile_offset) * n + col];
                threadgroup_barrier(mem_flags::mem_threadgroup);

                // take one element from col and work on all rows in this thread.
                #define emit_inst(i) {                                                 \
                        float tmp = b_local_buf[(i) * GROUP_DIM + col_tile_offset];    \
                                                                                       \
                        int64_t pos = (row_tile_offset) * GROUP_DIM + (i);             \
                        int64_t offset = GROUP_DIM * GROUP_DIM;                        \
                        /* loop GROUPs */                                              \
                        for (int64_t k = 0; k < GROUPS; k++) {                         \
                                v[k] += a_local_buf[pos] * tmp;  pos += offset;        \
                        }                                                              \
                }

                UNROLL(GROUP_DIM);
                #undef emit_inst

                threadgroup_barrier(mem_flags::mem_threadgroup);
        }

        // write back.
        {
                uint64_t c_pos = (group_in_grid.y * GROUPS * GROUP_DIM +
                                row_tile_offset) * n + col;
                uint64_t offset = GROUP_DIM * n;
                thread float *vp = v;

                #define emit_inst(i) {c[c_pos] = *vp; c_pos += offset; vp++;}
                UNROLL(GROUPS);
                #undef emit_inst
        }
}
