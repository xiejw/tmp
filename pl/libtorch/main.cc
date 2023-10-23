#include <torch/torch.h>
#include <iostream>

const auto deviceToUse = torch::kMPS;

int main() {
        auto opt =
                torch::TensorOptions().dtype(torch::kFloat32).device(deviceToUse);
        torch::Tensor tensor = torch::rand({2, 3}, opt);
        std::cout << tensor << std::endl;
}
