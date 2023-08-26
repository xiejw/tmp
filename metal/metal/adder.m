/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A class to manage all of the Metal objects this app creates.
*/

#import "adder.h"

struct adder {
    id<MTLDevice> _mDevice;

    // The compute pipeline generated from the compute kernel in the .metal shader file.
    id<MTLComputePipelineState> _mAddFunctionPSO;

    // The command queue used to pass commands to the device.
    id<MTLCommandQueue> _mCommandQueue;

    // Buffers to hold data.
    id<MTLBuffer> _mBufferA;
    id<MTLBuffer> _mBufferB;
    id<MTLBuffer> _mBufferResult;
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
    //  If the Metal API validation is enabled, you can find out more
    //  information about what went wrong.  (Metal API validation is enabled by
    //  default when a debug build is run from Xcode)
    NSLog(@"Failed to created pipeline state object, error %@.", error);
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
  adder->_mBufferResult = nil;

  return adder;
}

void adderFree(struct adder* p) {
  if (p == NULL) return;
  // read https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/MemoryMgmt/Articles/MemoryMgmt.html#//apple_ref/doc/uid/10000011i

  [p->_mBufferA release];
  [p->_mBufferB release];
  [p->_mBufferResult release];

  [p->_mCommandQueue release];
  [p->_mAddFunctionPSO release];
  [p->_mDevice release];
  free(p);
}


// - (void) prepareData
// {
//     // Allocate three buffers to hold our initial data and the result.
//     _mBufferA = [_mDevice newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
//     _mBufferB = [_mDevice newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
//     _mBufferResult = [_mDevice newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
//
//     [self generateRandomFloatData:_mBufferA];
//     [self generateRandomFloatData:_mBufferB];
// }
//
// - (void) sendComputeCommand
// {
//     // Create a command buffer to hold commands.
//     id<MTLCommandBuffer> commandBuffer = [_mCommandQueue commandBuffer];
//     assert(commandBuffer != nil);
//
//     // Start a compute pass.
//     id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
//     assert(computeEncoder != nil);
//
//     [self encodeAddCommand:computeEncoder];
//
//     // End the compute pass.
//     [computeEncoder endEncoding];
//
//     // Execute the command.
//     [commandBuffer commit];
//
//     // Normally, you want to do other work in your app while the GPU is running,
//     // but in this example, the code simply blocks until the calculation is complete.
//     [commandBuffer waitUntilCompleted];
//
//     [self verifyResults];
// }
//
// - (void)encodeAddCommand:(id<MTLComputeCommandEncoder>)computeEncoder {
//
//     // Encode the pipeline state object and its parameters.
//     [computeEncoder setComputePipelineState:_mAddFunctionPSO];
//     [computeEncoder setBuffer:_mBufferA offset:0 atIndex:0];
//     [computeEncoder setBuffer:_mBufferB offset:0 atIndex:1];
//     [computeEncoder setBuffer:_mBufferResult offset:0 atIndex:2];
//
//     MTLSize gridSize = MTLSizeMake(arrayLength, 1, 1);
//
//     // Calculate a threadgroup size.
//     NSUInteger threadGroupSize = _mAddFunctionPSO.maxTotalThreadsPerThreadgroup;
//     if (threadGroupSize > arrayLength)
//     {
//         threadGroupSize = arrayLength;
//     }
//     MTLSize threadgroupSize = MTLSizeMake(threadGroupSize, 1, 1);
//
//     // Encode the compute command.
//     [computeEncoder dispatchThreads:gridSize
//               threadsPerThreadgroup:threadgroupSize];
// }
//
// - (void) generateRandomFloatData: (id<MTLBuffer>) buffer
// {
//     float* dataPtr = buffer.contents;
//
//     for (unsigned long index = 0; index < arrayLength; index++)
//     {
//         dataPtr[index] = (float)rand()/(float)(RAND_MAX);
//     }
// }
// - (void) verifyResults
// {
//     float* a = _mBufferA.contents;
//     float* b = _mBufferB.contents;
//     float* result = _mBufferResult.contents;
//
//     for (unsigned long index = 0; index < arrayLength; index++)
//     {
//         if (result[index] != (a[index] + b[index]))
//         {
//             printf("Compute ERROR: index=%lu result=%g vs %g=a+b\n",
//                    index, result[index], a[index] + b[index]);
//             assert(result[index] == (a[index] + b[index]));
//         }
//     }
//     printf("Compute results as expected\n");
// }
// @end
