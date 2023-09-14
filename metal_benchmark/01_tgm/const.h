// be consistant with other tests.
#define N_DIM                    (1024 * 2)  // 8k
#define THREADGROUP_SIZE                    (32)
#define THREADGROUP_SIZE_X                (32)
#define THREADGROUP_SIZE_Y                (4)
#define N_BYTES_THREADGROUP     (sizeof(float) * N_DIM)

#define REPEAT_COUNT  (4096 * 512)

static_assert(THREADGROUP_SIZE == THREADGROUP_SIZE_X, "");

#define GRID_SIZE_X                       (THREADGROUP_SIZE_X)
#define GRID_SIZE_Y                       (THREADGROUP_SIZE_Y)

