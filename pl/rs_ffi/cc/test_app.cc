#include <torch/torch.h>
#include <iostream>

// int main() {
extern "C" void hello() {
        torch::Tensor tensor = torch::rand({2, 3});
        std::cout << tensor << std::endl;
}
