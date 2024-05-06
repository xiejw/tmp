#include <cstdio>
#include <memory>
#include <string>

#include <dirent.h>
#include <stdlib.h>

#define DEBUG( fmt, ... ) printf( fmt __VA_OPT__(, ) __VA_ARGS__ )

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
};

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

int
main( )
{
    printf( "Hello, opendir(\".\") \n" );
    auto dir = std::unique_ptr<eve::fs::FsDir>{ eve::fs::OpenDir( "." ) };
    if ( dir == nullptr ) {
        printf( "failed to open dir\n" );
        exit( 1 );
    }

    return 0;
}
