/*
See LICENSE folder for this sample’s licensing information.

Abstract:
An app that performs a simple calculation on a GPU.
*/

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#import <assert.h>

#import "adder.h"

// This is the C version of the function that the sample
// implements in Metal Shading Language.
void add_arrays(const float* inA,
                const float* inB,
                float* result,
                int length)
{
    for (int index = 0; index < length ; index++)
    {
        result[index] = inA[index] + inB[index];
    }
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        struct adder * adder = adderNew(device);
        assert(adder != NULL);
        NSLog(@"device prepared and function compiled");

        // // Create buffers to hold data
        // [adder prepareData];

        // // Send a command to the GPU to perform the calculation.
        // [adder sendComputeCommand];

        NSLog(@"execution finished");
exit:
        adderFree(adder);
    }
    return 0;
}
