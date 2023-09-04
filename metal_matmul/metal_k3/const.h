#define TILE_SIZE 8
#define GROUP_DIM TILE_SIZE
#define GROUPS 8

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

