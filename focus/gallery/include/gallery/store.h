// vim: ft=cpp
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <eve/base/error.h>

namespace gallery {

struct DirEntry {
    std::string              Path{ };
    bool                     Expanded = false;
    bool                     Pointer  = 0;  // Positive is Dirs
    std::vector<DirEntry>    Dirs{ };
    std::vector<std::string> Files{ };

    eve::base::Error           expand( );
    std::optional<std::string> getLast( );
};

struct Store {
  private:
    std::string               BaseDir;
    std::unique_ptr<DirEntry> Root = nullptr;

  public:
    Store( std::string_view BaseDir ) : BaseDir( BaseDir ) {};

  public:
    std::optional<std::string> getLast( ) noexcept;
};
}  // namespace gallery
