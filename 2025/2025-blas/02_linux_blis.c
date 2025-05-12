#include <blis.h>
#include <stdio.h>

int
main( void )
{
    // 2x3
    float a[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    // 2x3
    float b[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    // 2x2
    float c[4];

    float alpha = 1.0, beta = 0.0;

    // c = matmul(a, trans(b));
    // Read api of https://github.com/flame/blis/blob/master/docs/BLISTypedAPI.md#level-3-operations
    // The api is different from BLAS.
    bli_sgemm( BLIS_NO_TRANSPOSE, BLIS_TRANSPOSE, 2, 2, 3, &alpha,
               /*A=*/a, 3, 1,
               /*B=*/b, 3, 1, &beta, /*C=*/c, 2, 1 );

    printf( "c %f\n", c[0] );
    printf( "c %f\n", c[1] );
    printf( "c %f\n", c[2] );
    printf( "c %f\n", c[3] );
}
