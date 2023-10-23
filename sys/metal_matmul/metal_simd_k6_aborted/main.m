// vim: shiftwidth=8 tabstop=8 softtabstop=8

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#import <assert.h>

#import "matmul_op.h"
#include "const.h"


int main(int argc, const char * argv[]) {
        @autoreleasepool {

                id<MTLDevice> device = MTLCreateSystemDefaultDevice();
                struct matmul_op * op = matmulOpNew(
                                device,
                                /*m=*/N_DIM,
                                /*n=*/N_DIM,
                                /*k=*/N_DIM);
                assert(op != NULL);
                NSLog(@"device prepared and function compiled");

                matmulOpPrepareData(op);
                matmulOpRun(op);

                NSLog(@"execution finished");
exit:
                matmulOpFree(op);
        }
        return 0;
}
