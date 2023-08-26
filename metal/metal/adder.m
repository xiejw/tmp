// vim: shiftwidth=8 tabstop=8 softtabstop=8
#import "adder.h"

//
// prototype
//
static void generateRandomFloatData(id<MTLBuffer> buffer, size_t nbytes);
static void encodeAddCommand(
                struct adder *p,
                size_t nelems,
                id<MTLComputeCommandEncoder> computeEncoder);
static void verifyResults(struct adder *p, size_t nelems);

struct adder {
        id<MTLDevice> _mDevice;

        // The compute pipeline generated from the compute kernel in the .metal
        // shader file.
        id<MTLComputePipelineState> _mAddFunctionPSO;

        // The command queue used to pass commands to the device.
        id<MTLCommandQueue> _mCommandQueue;

        // Buffers to hold data.
        id<MTLBuffer> _mBufferA;
        id<MTLBuffer> _mBufferB;
        id<MTLBuffer> _mBufferC;
};

struct adder * adderNew(id<MTLDevice> device) {
        struct adder * adder = malloc(sizeof(*adder));
        adder->_mDevice = device;

        NSError* error = nil;

        // Load the shader files with a '.metal' file extension

        id<MTLLibrary> defaultLibrary = [
                adder->_mDevice newLibraryWithFile: @".build/add.metallib"
                                             error: &error];

        if (defaultLibrary == nil) {
                NSLog(@"Failed to find the default library. %s ",
                                [[error localizedDescription] UTF8String]);
                return nil;
        }

        // load function
        id<MTLFunction> addFunction = [
                defaultLibrary newFunctionWithName: @"add_arrays"];

        if (addFunction == nil) {
                NSLog(@"Failed to find the adder function.");
                return nil;
        }

        // create a compute pipeline state object.
        adder->_mAddFunctionPSO = [
                adder->_mDevice newComputePipelineStateWithFunction: addFunction
                                                              error: &error];

        if (adder->_mAddFunctionPSO == nil)
        {
                //  If the Metal API validation is enabled, you can find out
                //  more information about what went wrong.  (Metal API
                //  validation is enabled by default when a debug build is run
                //  from Xcode)
                NSLog(@"Failed to created pipeline state object, error %@.",
                                error);
                return nil;
        }

        adder->_mCommandQueue = [adder->_mDevice newCommandQueue];
        if (adder->_mCommandQueue == nil)
        {
                NSLog(@"Failed to find the command queue.");
                return nil;
        }

        adder->_mBufferA = nil;
        adder->_mBufferB = nil;
        adder->_mBufferC = nil;

        return adder;
}

void adderFree(struct adder* p) {
        if (p == NULL) return;
        // read https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/MemoryMgmt/Articles/MemoryMgmt.html#//apple_ref/doc/uid/10000011i

        [p->_mBufferA release];
        [p->_mBufferB release];
        [p->_mBufferC release];

        [p->_mCommandQueue release];
        [p->_mAddFunctionPSO release];
        [p->_mDevice release];
        free(p);
}


void adderPrepareData(struct adder* p, size_t nelems) {

        size_t nbytes = sizeof(float) * nelems;
        // Allocate three buffers to hold our initial data and the result.
        p->_mBufferA = [p->_mDevice
                newBufferWithLength: nbytes
                            options: MTLResourceStorageModeShared];
        p->_mBufferB = [p->_mDevice
                newBufferWithLength: nbytes
                            options: MTLResourceStorageModeShared];
        p->_mBufferC = [p->_mDevice
                newBufferWithLength: nbytes
                            options: MTLResourceStorageModeShared];

        generateRandomFloatData(p->_mBufferA, nelems);
        generateRandomFloatData(p->_mBufferB, nelems);
}

void adderRun(struct adder*p, size_t nelems) {
        // create a command buffer to hold commands.
        id<MTLCommandBuffer> commandBuffer = [
                p->_mCommandQueue commandBuffer];
        assert(commandBuffer != nil);

        // start a compute pass.
        id<MTLComputeCommandEncoder> computeEncoder = [
                commandBuffer computeCommandEncoder];
        assert(computeEncoder != nil);

        encodeAddCommand(p, nelems, computeEncoder);

        // end the compute pass.
        [computeEncoder endEncoding];

        // execute the command.
        [commandBuffer commit];

        // Normally, you want to do other work in your app while the GPU is running,
        // but in this example, the code simply blocks until the calculation is complete.
        [commandBuffer waitUntilCompleted];

        verifyResults(p, nelems);
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
                struct adder *p,
                size_t nelems,
                id<MTLComputeCommandEncoder> computeEncoder) {

        // encode the pipeline state object and its parameters.
        [computeEncoder setComputePipelineState: p->_mAddFunctionPSO];
        [computeEncoder setBuffer: p->_mBufferA offset:0 atIndex:0];
        [computeEncoder setBuffer: p->_mBufferB offset:0 atIndex:1];
        [computeEncoder setBuffer: p->_mBufferC offset:0 atIndex:2];

        MTLSize gridSize = MTLSizeMake(nelems, 1, 1);

        // calculate a threadgroup size.
        NSUInteger threadGroupSize = p->_mAddFunctionPSO.maxTotalThreadsPerThreadgroup;
        if (threadGroupSize > nelems) {
                threadGroupSize = nelems;
        }
        MTLSize threadgroupSize = MTLSizeMake(threadGroupSize, 1, 1);

        // Encode the compute command.
        [computeEncoder dispatchThreads:gridSize
                  threadsPerThreadgroup:threadgroupSize];
}

void verifyResults(struct adder *p, size_t nelems) {
        float* a = p->_mBufferA.contents;
        float* b = p->_mBufferB.contents;
        float* c = p->_mBufferC.contents;

        for (size_t index = 0; index < nelems; index++) {
                if (c[index] != (a[index] + b[index])) {
                        printf("Compute ERROR: index=%lu result=%g vs %g=a+b\n",
                                        index, c[index], a[index] + b[index]);
                        assert(c[index] == (a[index] + b[index]));
                }
        }
        NSLog(@"compute results as expected\n");
}
