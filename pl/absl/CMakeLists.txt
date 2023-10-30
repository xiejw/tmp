# see https://abseil.io/docs/cpp/quickstart-cmake
cmake_minimum_required(VERSION 3.21)

project(demo)

# Abseil requires C++14
set(CMAKE_CXX_STANDARD 14)

# Process Abseil's CMake build system
add_subdirectory(third_party/absl)

add_executable(demo hello_world.cc)

# Declare dependency on the absl::strings library
target_link_libraries(demo absl::strings)
