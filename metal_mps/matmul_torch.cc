#include <chrono>
#include <iostream>

#include <torch/mps.h>
#include <torch/torch.h>

// be consistent in all tests.
constexpr size_t SIZE = 4096L * 2L;

const auto deviceToUse = torch::kMPS;
// const auto deviceToUse = torch::kCPU;

int
main()
{
        auto opt =
            torch::TensorOptions().dtype(torch::kFloat32).device(deviceToUse);
        torch::Tensor a = torch::randn({SIZE, SIZE}, opt);
        torch::Tensor b = torch::randn({SIZE, SIZE}, opt);
        torch::Tensor c = torch::randn({SIZE, SIZE}, opt);

        //
        // warm up
        // compute C = matmul(A, B)
        //
        torch::mps::synchronize();

        auto count = 10;
        for (int i = 0; i < count; i++) {
                at::matmul_out(c, a, b);
        }

        torch::mps::synchronize();
        //
        // real game
        // compute C = matmul(A, B)
        //
        auto start = std::chrono::system_clock::now();

        for (int i = 0; i < count; i++) {
                at::matmul_out(c, a, b);
        }

        torch::mps::synchronize();
        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double, std::milli> fp_ms = end - start;

        std::cout << "libtorch device " << deviceToUse << "\n";
        std::cout << "c.matmul takes " << fp_ms.count() / count / 1000 << "\n";
        return 0;
}
