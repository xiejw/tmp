#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

struct adder;

struct adder* adderNew(id<MTLDevice> device);
void          adderFree(struct adder*);
void          adderPrepareData(struct adder* p, size_t nelems);
void          adderRun(struct adder*p, size_t nelems);

