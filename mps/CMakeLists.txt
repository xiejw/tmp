cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(torch_mps_cc)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_executable(torch_mps_cc matmul_torch.cc)
target_link_libraries(torch_mps_cc "${TORCH_LIBRARIES}")
set_property(TARGET torch_mps_cc PROPERTY CXX_STANDARD 17)
