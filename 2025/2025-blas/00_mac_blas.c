#include <Accelerate/Accelerate.h>
#include <stdio.h>

int
main( void )
{
    // 2x3
    float a[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    // 2x3
    float b[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    float c[4];

    cblas_sgemm( CblasRowMajor, CblasNoTrans, CblasTrans, 2, 2, 3, 1.0,
                 /*A=*/a, 3,
                 /*B=*/b, 3, 0.0, /*C=*/c, 2 );

    printf( "c %f\n", c[0] );
    printf( "c %f\n", c[1] );
    printf( "c %f\n", c[2] );
    printf( "c %f\n", c[3] );
}
