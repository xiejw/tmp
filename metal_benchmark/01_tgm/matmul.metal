// vim: ft=c

#include <metal_stdlib>
#include <metal_compute>
#include <metal_simdgroup_matrix>

using namespace metal;

#include "const.h"

kernel void matmul_op_tile(
                device const float* a,
                device float* b,
                constant int& len,
                threadgroup float  * buf [[threadgroup(0)]],
                const ushort2 gid [[thread_position_in_grid]],
                const ushort2 group_in_grid [[threadgroup_position_in_grid]],
                const ushort2 tid [[thread_position_in_threadgroup]])
{
        const int lane_id = tid.x % THREADGROUP_SIZE;
        const int group_id = tid.y;

        float c = 0;
        // strategy 1. copy elment from a one each time to b each time and

// #define WORK_TO_DO     (N_DIM / THREADGROUP_SIZE)
//
//         int offset = lane_id;
//         for (int k = 0; k < WORK_TO_DO; k++) {
//                 b[offset ] = a[offset];
//                 offset += THREADGROUP_SIZE;
//         }

#define ELEMENT_TO_CP 4
#define WORK_TO_DO     (N_DIM / THREADGROUP_SIZE_Y / THREADGROUP_SIZE / (ELEMENT_TO_CP))
#define STRIDE     (THREADGROUP_SIZE * (ELEMENT_TO_CP))

        int offset = ELEMENT_TO_CP * lane_id + group_id * (N_DIM / THREADGROUP_SIZE_Y);
        for ( int p = 0; p <  REPEAT_COUNT; p++) {

        #pragma unroll (WORK_TO_DO)
        for (int k = 0; k < WORK_TO_DO; k++) {

#pragma unroll (ELEMENT_TO_CP)
                for (int i = 0; i < ELEMENT_TO_CP; i++) {
                //b[offset + i] = a[offset+ i];
                c += buf[offset+ i];
                }
                offset += STRIDE;
        }
        }

        b[0] = c;
}
