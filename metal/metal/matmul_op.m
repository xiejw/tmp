// vim: shiftwidth=8 tabstop=8 softtabstop=8
#import "matmul_op.h"

//
// prototype
//
static void generateRandomFloatData(id<MTLBuffer> buffer, size_t nbytes);
// static void encodeAddCommand(
//                 struct matmul_op *p,
//                 size_t nelems,
//                 id<MTLComputeCommandEncoder> computeEncoder);
// static void verifyResults(struct matmul_op *p, size_t nelems);

//
// struct def
//
struct matmul_op {
        int m;
        int n;
        int k;

        id<MTLDevice> dev;
        id<MTLCommandQueue> que;
        id<MTLComputePipelineState> pip;

        // Buffers to hold data.
        id<MTLBuffer> buf_a;
        id<MTLBuffer> buf_b;
        id<MTLBuffer> buf_c;
};

struct matmul_op * matmulOpNew(id<MTLDevice> device, int m, int n, int k) {
        struct matmul_op * op = malloc(sizeof(*op));
        op->dev = device;
        op->m = m;
        op->n = n;
        op->k = k;

        NSError* error = nil;

        id<MTLLibrary> defaultLibrary = [
                op->dev newLibraryWithFile: @".build/add.metallib"
                                          error: &error];

        if (defaultLibrary == nil) {
                NSLog(@"Failed to find the default library. %s ",
                                [[error localizedDescription] UTF8String]);
                return nil;
        }
        [defaultLibrary autorelease];

        id<MTLFunction> func = [
                defaultLibrary newFunctionWithName: @"add_arrays"];

        if (func == nil) {
                NSLog(@"Failed to find the matmul_op function.");
                return nil;
        }
        [func autorelease];

        op->pip = [op->dev newComputePipelineStateWithFunction: func
                                                         error: &error];

        if (op->pip == nil) {
                //  If the Metal API validation is enabled, you can find out
                //  more information about what went wrong.  (Metal API
                //  validation is enabled by default when a debug build is run
                //  from Xcode)
                NSLog(@"Failed to created pipeline state object, error %@.",
                                error);
                return nil;
        }

        op->que = [op->dev newCommandQueue];
        if (op->que == nil) {
                NSLog(@"Failed to find the command queue.");
                return nil;
        }

        op->buf_a = nil;
        op->buf_b = nil;
        op->buf_c = nil;

        return op;
}

void matmulOpFree(struct matmul_op* p) {
        if (p == NULL) return;

        [p->buf_a release];
        [p->buf_b release];
        [p->buf_c release];

        [p->que release];
        [p->pip release];
        [p->dev release];
        free(p);
}


void matmulOpPrepareData(struct matmul_op* op) {
        int m = op->m;
        int n = op->n;
        int k = op->k;

        size_t nb_a = sizeof(float) * m * k;
        size_t nb_b = sizeof(float) * n * k;
        size_t nb_c = sizeof(float) * m * n;

        op->buf_a = [op->dev
                newBufferWithLength: nb_a
                            options: MTLResourceStorageModeShared];
        op->buf_b = [op->dev
                newBufferWithLength: nb_b
                            options: MTLResourceStorageModeShared];
        op->buf_c = [op->dev
                newBufferWithLength: nb_c
                            options: MTLResourceStorageModeShared];

        generateRandomFloatData(op->buf_a, m*k);
        generateRandomFloatData(op->buf_b, n*k);
}

// void matmulOpRun(struct matmul_op*p, size_t nelems) {
//         // create a command buffer to hold commands.
//         id<MTLCommandBuffer> commandBuffer = [
//                 p->que commandBuffer];
//         assert(commandBuffer != nil);
//
//         // start a compute pass.
//         id<MTLComputeCommandEncoder> computeEncoder = [
//                 commandBuffer computeCommandEncoder];
//         assert(computeEncoder != nil);
//
//         encodeAddCommand(p, nelems, computeEncoder);
//
//         // end the compute pass.
//         [computeEncoder endEncoding];
//
//         // execute the command.
//         [commandBuffer commit];
//
//         // Normally, you want to do other work in your app while the GPU is running,
//         // but in this example, the code simply blocks until the calculation is complete.
//         [commandBuffer waitUntilCompleted];
//
//         verifyResults(p, nelems);
// }
//
//
//
// //
// // impl of helpers
// //
//
void generateRandomFloatData(id<MTLBuffer> buffer, size_t nbytes) {
        float* dataPtr = buffer.contents;

        for (size_t index = 0; index < nbytes; index++) {
                dataPtr[index] = (float)rand()/(float)(RAND_MAX);
        }
}

// void encodeAddCommand(
//                 struct matmul_op *p,
//                 size_t nelems,
//                 id<MTLComputeCommandEncoder> computeEncoder) {
//
//         // encode the pipeline state object and its parameters.
//         [computeEncoder setComputePipelineState: p->pip];
//         [computeEncoder setBuffer: p->buf_a offset:0 atIndex:0];
//         [computeEncoder setBuffer: p->buf_b offset:0 atIndex:1];
//         [computeEncoder setBuffer: p->buf_c offset:0 atIndex:2];
//
//         MTLSize gridSize = MTLSizeMake(nelems, 1, 1);
//
//         // calculate a threadgroup size.
//         NSUInteger threadGroupSize = p->pip.maxTotalThreadsPerThreadgroup;
//         if (threadGroupSize > nelems) {
//                 threadGroupSize = nelems;
//         }
//         MTLSize threadgroupSize = MTLSizeMake(threadGroupSize, 1, 1);
//
//         // Encode the compute command.
//         [computeEncoder dispatchThreads:gridSize
//                   threadsPerThreadgroup:threadgroupSize];
// }
//
// void verifyResults(struct matmul_op *p, size_t nelems) {
//         float* a = p->buf_a.contents;
//         float* b = p->buf_b.contents;
//         float* c = p->buf_c.contents;
//
//         for (size_t index = 0; index < nelems; index++) {
//                 if (c[index] != (a[index] + b[index])) {
//                         printf("Compute ERROR: index=%lu result=%g vs %g=a+b\n",
//                                         index, c[index], a[index] + b[index]);
//                         assert(c[index] == (a[index] + b[index]));
//                 }
//         }
//         NSLog(@"compute results as expected\n");
// }
