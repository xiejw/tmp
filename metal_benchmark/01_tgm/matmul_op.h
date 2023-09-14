#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

struct matmul_op;

struct matmul_op* matmulOpNew(id<MTLDevice> device, int len);
void              matmulOpFree(struct matmul_op* op);
void              matmulOpPrepareData(struct matmul_op* op);
void              matmulOpRun(struct matmul_op* op);

