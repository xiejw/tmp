// vim: ft=cpp
#pragma once

#include <functional>
#include <memory>
#include <string>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <eve/base/error.h>
#include <eve/base/log.h>
#include <eve/base/result.h>

namespace eve::fs {

struct DirReader {
  private:
    std::string DirName;
    DIR        *Dir = nullptr;

  public:
    DirReader( std::string DirName, DIR *Dir )
        : DirName( DirName ), Dir( Dir ) {};
    ~DirReader( );

  public:
    // The name lifetime is limited to the invocation.
    eve::base::Error Run(
        const std::function<void( const char *Name, bool IsFile )> Fn );
};

eve::base::Result<std::unique_ptr<DirReader>> OpenDir( const char *dir );
}  // namespace eve::fs
