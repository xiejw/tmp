// vim: shiftwidth=8 tabstop=8 softtabstop=8
#import "matmul_op.h"

#include <time.h>

#include "const.h"

//
// prototype
//
static void generateRandomFloatData(id<MTLBuffer> buffer, size_t nbytes);
static void encodeAddCommand(
                struct matmul_op* op,
                id<MTLComputeCommandEncoder> computeEncoder);
static void verifyResults(struct matmul_op *op);

static double eva_gettime_in_secs() {
        struct timespec req;
        clock_gettime(CLOCK_MONOTONIC, &req);
        return (double)req.tv_sec + (double) req.tv_nsec / 1000 / 1000 / 1000;
}

//
// struct def
//
struct matmul_op {
        int m;
        int n;
        int k;

        // heavy metal objects.
        id<MTLDevice> dev;
        id<MTLCommandQueue> que;
        id<MTLComputePipelineState> pip;

        // buffers to hold data.
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
                op->dev newLibraryWithFile: @".build/matmul.metallib"
                                     error: &error];

        if (defaultLibrary == nil) {
                NSLog(@"Failed to find the default library. %s ",
                                [[error localizedDescription] UTF8String]);
                return nil;
        }
        [defaultLibrary autorelease];

        id<MTLFunction> func = [
                defaultLibrary newFunctionWithName: @"matmul_op_tile"];

        if (func == nil) {
                NSLog(@"Failed to find the matmul_op_tile function.");
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

void matmulOpRun(struct matmul_op* op) {
        id<MTLCommandBuffer> commandBuffer = [op->que commandBuffer];
        assert(commandBuffer != nil);

        id<MTLComputeCommandEncoder> computeEncoder = [
                commandBuffer computeCommandEncoder];
        assert(computeEncoder != nil);
        NSLog(@"status: %lu", [commandBuffer status]);

        encodeAddCommand(op, computeEncoder);
        [computeEncoder endEncoding];

        double start = eva_gettime_in_secs();
        [commandBuffer commit];
        NSLog(@"status: %lu", [commandBuffer status]);

        [commandBuffer waitUntilCompleted];
        NSLog(@"status: %lu", [commandBuffer status]);
        double end = eva_gettime_in_secs();

        NSLog(@"matmul takes %g secs", end - start);
        NSLog(@"metal commit END\n");
        verifyResults(op);
}

//
// impl of helpers
//

void generateRandomFloatData(id<MTLBuffer> buffer, size_t nbytes) {
        float* dataPtr = buffer.contents;

        for (size_t index = 0; index < nbytes; index++) {
                dataPtr[index] = (float)rand()/(float)(RAND_MAX);
        }
}

void encodeAddCommand(
                struct matmul_op *op,
                id<MTLComputeCommandEncoder> computeEncoder) {

        int m = op->m;
        int n = op->n;
        int k = op->k;
        size_t len = sizeof(int);

        // encode the pipeline state object and its parameters.
        [computeEncoder setComputePipelineState: op->pip];
        [computeEncoder setBuffer: op->buf_a offset:0 atIndex:0];
        [computeEncoder setBuffer: op->buf_b offset:0 atIndex:1];
        [computeEncoder setBuffer: op->buf_c offset:0 atIndex:2];
        [computeEncoder setBytes: &m length:len atIndex:3];
        [computeEncoder setBytes: &n length:len atIndex:4];
        [computeEncoder setBytes: &k length:len atIndex:5];

        [computeEncoder setThreadgroupMemoryLength: N_BYTES_THREADGROUP atIndex:0];

        MTLSize gridSize = MTLSizeMake(GRID_SIZE_X, GRID_SIZE_Y, 1);
        NSLog(@"GridSize %lux%lu", gridSize.width, gridSize.height);

        NSUInteger threadGroupSize = op->pip.maxTotalThreadsPerThreadgroup;
        NSUInteger simdGroupSize = op->pip.threadExecutionWidth;
        NSUInteger threadGroupMemSize = op->pip.staticThreadgroupMemoryLength;
        NSLog(@"max thread group size: %d\n", (int) threadGroupSize);
        NSLog(@"simd group size      : %d\n", (int) simdGroupSize);
        NSLog(@"thread group mem size: %d\n", (int) threadGroupMemSize);

        if (THREADGROUP_SIZE_X * THREADGROUP_SIZE_Y > threadGroupSize) {
                NSLog(@"======================");
                NSLog(@"!!!ERROR group size!!!");
                NSLog(@"======================");
        }

        MTLSize threadgroupSize = MTLSizeMake(THREADGROUP_SIZE_X, THREADGROUP_SIZE_Y, 1);
        NSLog(@"Thread group size %lux%lu", threadgroupSize.width, threadgroupSize.height);


        if (gridSize.width < threadgroupSize.width) {
                NSLog(@"======================");
                NSLog(@"!!!ERROR grid size!!!");
                NSLog(@"======================");
        }
        if (gridSize.height < threadgroupSize.height) {
                NSLog(@"======================");
                NSLog(@"!!!ERROR grid size!!!");
                NSLog(@"======================");
        }
        [computeEncoder dispatchThreads:gridSize
                  threadsPerThreadgroup:threadgroupSize];
}

#include <Accelerate/Accelerate.h>

static  void matmul_op_host(
                float* a,
                float* b,
                float* c,
                int64_t m,
                int64_t n,
                int64_t k) {

        // The following naive code is tooooo slow.
        //        for (int64_t row = 0; row < m; row++) {
        //                for (int64_t col = 0; col < n; col++) {
        //                        float v = 0;
        //                        for (int64_t i = 0; i < k; i++) {
        //                                v += a[row * k + i] + b[i * n + col];
        //                        }
        //                        c[row * n + col] = v;
        //                }
        //        }
        cblas_sgemm(
                        CblasRowMajor,
                        CblasNoTrans,
                        CblasNoTrans,
                        m,
                        n,
                        k,
                        1.0,
                        a,
                        /*lda=*/m,
                        b,
                        /*ldb=*/k,
                        0.0,
                        c,
                        /*ldc=*/m);
}

void verifyResults(struct matmul_op *op) {
        float* a = op->buf_a.contents;
        float* b = op->buf_b.contents;
        float* c = op->buf_c.contents;

        const int64_t m = op->m;
        const int64_t n = op->n;
        const int64_t k = op->k;

        NSLog(@"generate expeced results on host\n");
        float* c_host = malloc(sizeof(float) * m * n);
        matmul_op_host(a, b, c_host, m, n, k);
        NSLog(@"generate expeced results on host DONE\n");

        size_t count = 0;

        for (size_t index = 0; index < m * n; index++) {
                if (c_host[index] != c[index]) {
                        NSLog(@"index: %zu - %f vs %f", index, c_host[index], c[index]);
                        if (count ++ > 10) {
                                NSLog(@"too many errors. abort checking.");
                                goto exit;
                        }
                }
        }
        free(c_host);
        NSLog(@"compute results as expected\n");
        return;
exit:
        free(c_host);
        NSLog(@"compute results failed to pass\n");
}
