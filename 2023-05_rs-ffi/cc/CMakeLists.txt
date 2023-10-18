cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(test_app)
include(CMakePrintHelpers)


find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_library(test_app test_app.cc)
# add_executable(test_app test_app.cc)
target_link_libraries(test_app "${TORCH_LIBRARIES}")
set_property(TARGET test_app PROPERTY CXX_STANDARD 14)
cmake_print_variables(TORCH_LIBRARIES)

