// vim: ft=cpp
#pragma once

#include <gallery/store.h>

namespace gallery {
struct App {
  public:
    Store &Store;

  public:
    int Run( );

  private:
    int RunFltk( );
};
}  // namespace gallery
