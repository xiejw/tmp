// vim: ft=cpp
#pragma once

#include <functional>
#include <memory>
#include <string>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <eve/base.h>

static const bool DEBUG_MODE = true;

namespace eve::fs {

struct FsDir {
  private:
    std::string DirName;
    DIR        *Dir = nullptr;

  public:
    FsDir( std::string DirName, DIR *Dir ) : DirName( DirName ), Dir( Dir )
    {
        DEBUG( "opening dir: %s", this->DirName.c_str( ) );
    };

    ~FsDir( )
    {
        if ( Dir != nullptr ) {
            closedir( Dir );
            DEBUG( "closing dir: %s", this->DirName.c_str( ) );
        }
    }

    // The name lifetime is limited to the invocation.
    void Run( const std::function<void( const char *Name, bool IsFile )> Fn )
    {
        struct dirent *dp;
        for ( ;; ) {
            errno = 0;
            dp    = readdir( Dir );
            if ( dp == NULL ) break;

            if ( strcmp( dp->d_name, "." ) == 0 ||
                 strcmp( dp->d_name, ".." ) == 0 ) {
                continue;
            }

            if ( dp->d_name[0] == '.' ) {
                continue;
            }

            if ( dp->d_type == DT_REG ) {
                Fn( dp->d_name, true );
            }

            if ( dp->d_type == DT_DIR ) {
                Fn( dp->d_name, false );
            }
        }

        if ( errno != 0 ) {
            ALERT( "error for readir" );
        }
    }
};

// Return nullptr if error.
std::unique_ptr<FsDir> OpenDir( const char *dir );
}  // namespace eve::fs
