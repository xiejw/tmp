// be consistant with other tests.
#define N_DIM                    (4096 * 4096)  // 64MiB, larger than all caches

#define THREADGROUP_SIZE                    (32)
#define THREADGROUP_SIZE_X                (32)
#define THREADGROUP_SIZE_Y                (32)

static_assert(THREADGROUP_SIZE == THREADGROUP_SIZE_X, "");

#define GRID_SIZE_X                       (THREADGROUP_SIZE_X)
#define GRID_SIZE_Y                       (THREADGROUP_SIZE_Y)

