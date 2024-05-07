#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define INFO( fmt, ... )  printf( "INFO : " fmt __VA_OPT__(, ) __VA_ARGS__ )
#define DEBUG( fmt, ... ) printf( "DEBUG: " fmt __VA_OPT__(, ) __VA_ARGS__ )
#define ALERT( fmt, ... ) printf( "ALERT: " fmt __VA_OPT__(, ) __VA_ARGS__ )

#include "fs.h"

std::string
Rename( std::string &Src, int Index )
{
    char        Buf[1000];
    const char *FileName = Src.c_str( );
    auto        ext      = strrchr( FileName, '.' );
    if ( !ext ) {
        snprintf( Buf, 1000 - 1, "%010d", Index );
    } else {
        snprintf( Buf, 1000 - 1, "%010d%s", Index, ext );
    }
    return Buf;
}

int
main( )
{
    auto dir = std::unique_ptr<eve::fs::FsDir>{ eve::fs::OpenDir( "." ) };
    if ( dir == nullptr ) {
        ALERT( "failed to open dir\n" );
        exit( 1 );
    }

    std::vector<std::pair<std::string, time_t>> FileNames{ };
    int                                         count = 0;
    dir->Run( [&]( const char *RegFileName ) {
        struct stat sb;
        if ( stat( RegFileName, &sb ) == -1 ) {
            ALERT( "failed\n" );
        } else {
            time_t BirthTime = sb.st_birthtime;
            INFO( "birthtime for %s: %s", RegFileName, ctime( &BirthTime ) );
            FileNames.push_back( { RegFileName, BirthTime } );
        }
        count++;
    } );
    INFO( "total %d files\n", count );

    std::sort( FileNames.begin( ), FileNames.end( ),
               []( auto a, auto b ) { return a.second < b.second; } );

    int Index = 0;
    for ( auto &f : FileNames ) {
        std::string &Src = f.first;
        std::string  Dst = Rename( f.first, Index++ );
        INFO( "birthtime: %ld: %s\n", f.second, Src.c_str( ) );
        INFO( "Rename to %s\n", Dst.c_str( ) );
        if ( -1 == symlink( Src.c_str( ), Dst.c_str( ) ) ) {
            ALERT( "unexpected error\n" );
            exit( 1 );
        }
    }

    return 0;
}
