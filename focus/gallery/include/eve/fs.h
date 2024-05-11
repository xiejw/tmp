// vim: ft=cpp
#pragma once

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
        DEBUG( "opening dir: %s\n", this->DirName.c_str( ) );
    };

    ~FsDir( )
    {
        if ( Dir != nullptr ) {
            closedir( Dir );
            DEBUG( "closing dir: %s\n", this->DirName.c_str( ) );
        }
    }

    // The name lifetime is limited to the invocation.
    void Run( const std::function<void( const char *RegFileName )> Fn )
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
                Fn( dp->d_name );
            }
        }

        if ( errno != 0 ) {
            ALERT( "error for readir" );
        }
    }
};

// Return nullptr if error.
FsDir *
OpenDir( const char *dir )
{
    DIR *OsDir = opendir( dir );
    if ( dir == nullptr ) {
        return nullptr;
    }
    return new FsDir( /*name=*/dir, OsDir );
}
}  // namespace eve::fs
