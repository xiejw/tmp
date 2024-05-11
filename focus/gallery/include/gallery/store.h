// vim: ft=cpp
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace gallery {
struct Store {
  private:
    std::string BaseDir;

  public:
    Store( std::string_view BaseDir ) : BaseDir( BaseDir ) {};

  public:
    std::optional<std::string> Last( ) noexcept;
};
}  // namespace gallery
