// vim: ft=cpp
#pragma once

#include <vector>

namespace policy {

template <class X, class Y, class W, class LossFn>
struct Policy {
  public:
  public:
    auto predict( X &t ) -> int                                     = 0;
    auto update( std::vector<std::tuple<X, Y, W>> &inputs ) -> void = 0;
};

}  // namespace policy
