#include <Accelerate/Accelerate.h>
#include <stdio.h>
#include <chrono>
#include <iostream>

// be consistent in all tests.
constexpr size_t SIZE = 4096L * 2L;

int
main(void)
{
        int lda  = SIZE;
        float *A = new float[SIZE * SIZE];

        int ldb  = SIZE;
        float *B = new float[SIZE * SIZE];

        int ldc  = SIZE;
        float *C = new float[SIZE * SIZE];

        //
        // warm up
        // compute C = matmul(A, B)
        //

        for (int i = 0; i < 10; i++) {
                cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, SIZE,
                            SIZE, SIZE, 1.0, A, lda, B, ldb, 0.0, C, ldc);
        }

        auto start = std::chrono::system_clock::now();

        auto count = 10;

        //
        // real game
        // compute C = matmul(A, B)
        //

        for (int i = 0; i < count; i++) {
                cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, SIZE,
                            SIZE, SIZE, 1.0, A, lda, B, ldb, 0.0, C, ldc);
        }

        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double, std::milli> fp_ms = end - start;

        std::cout << "c.matmul takes " << fp_ms.count() / count / 1000 << "\n";
        return 0;
}
