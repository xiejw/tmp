#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>

int main() {
  fmt::print("{}", 1);
  fmt::print("{}", std::vector<int>{1, 2, 3});
  return 0;
}

