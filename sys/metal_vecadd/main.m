// vim: shiftwidth=8 tabstop=8 softtabstop=8

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#import <assert.h>

#import "adder.h"

#define N_ELEMS (4096 * 2)

int main(int argc, const char * argv[]) {
    @autoreleasepool {

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        struct adder * adder = adderNew(device);
        assert(adder != NULL);
        NSLog(@"device prepared and function compiled");

        adderPrepareData(adder, N_ELEMS);
        adderRun(adder, N_ELEMS);

        NSLog(@"execution finished");
exit:
        adderFree(adder);
    }
    return 0;
}
