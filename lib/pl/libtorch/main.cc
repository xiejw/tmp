#include <torch/torch.h>
#include <iostream>

#ifdef CPU_DEVICE_ONLY
const auto deviceToUse = torch::kCPU;
#else
const auto deviceToUse = torch::kMPS;
#endif  // CPU;

int
main( )
{
    auto opt =
        torch::TensorOptions( ).dtype( torch::kFloat32 ).device( deviceToUse );
    torch::Tensor tensor = torch::rand( { 2, 3 }, opt );
    std::cout << tensor << std::endl;
}
