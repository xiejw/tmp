// be consistant with other tests.
#define N_DIM                    (4096 * 2)

// kernel configs
#define SIMDGROUP_SIZE                   32
#define SIMDGROUP_MAT_DIM                 8
#define SIMDGROUPS_PER_GROUP              8
#define TILE_SIZE                        64
#define HALF_TILE_SIZE                   32  // TILE_SIZE / 2

#define SIMDGROUPS_TILES_PER_K_TILE       8  // TILE_SIZE / SIMDGROUP_MAT_DIM

static_assert(SIMDGROUP_MAT_DIM * SIMDGROUPS_TILES_PER_K_TILE == TILE_SIZE, "");
static_assert(HALF_TILE_SIZE * 2 == TILE_SIZE, "");

#define N_BYTES                           (sizeof(float))
#define N_ELEMS_PER_MATRIX                (TILE_SIZE * TILE_SIZE)
#define N_BYTES_PER_MATRIX                (N_BYTES * (N_ELEMS_PER_MATRIX))
#define N_BYTES_THREADGROUP               (2 * N_BYTES_PER_MATRIX)


#define THREADGROUP_SIZE_X                (SIMDGROUPS_PER_GROUP * SIMDGROUP_SIZE)
#define THREADGROUP_SIZE_Y                1

#define GRID_SIZE_X                       ((N_DIM / TILE_SIZE) * THREADGROUP_SIZE_X)
#define GRID_SIZE_Y                       ((N_DIM / TILE_SIZE) * THREADGROUP_SIZE_Y)

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

