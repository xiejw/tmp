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
Rename( const char *SrcDir, std::string &Src, const char *DstDir, int Index )
{
    char SrcBuf[1000], DstBuf[1000];

    snprintf( SrcBuf, 1000 - 1, "%s/%s", SrcDir, Src.c_str( ) );

    const char *FileName = Src.c_str( );
    auto        ext      = strrchr( FileName, '.' );
    if ( !ext ) {
        snprintf( DstBuf, 1000 - 1, "%s/%010d", DstDir, Index );
    } else {
        snprintf( DstBuf, 1000 - 1, "%s/%010d%s", DstDir, Index, ext );
    }

    if ( -1 == symlink( SrcBuf, DstBuf ) ) {
        ALERT( "unexpected error: %s\n", strerror( errno ) );
        ALERT( "%s -> %s\n", SrcBuf, DstBuf );
        exit( 1 );
    }
    return DstBuf;
}

int
main( int argc, char **argv )
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

    int         Index  = argc > 1 ? atoi( argv[1] ) : 0;
    const char *SrcDir = argc > 2 ? argv[2] : "..";
    const char *DstDir = argc > 3 ? argv[3] : "Sorted";

    mkdir( DstDir, 0755 );

    for ( auto &f : FileNames ) {
        std::string &Src = f.first;
        std::string  Dst = Rename( SrcDir, f.first, DstDir, Index++ );

        INFO( "birthtime: %ld: %s -> %s\n", f.second, Src.c_str( ),
              Dst.c_str( ) );
    }

    return 0;
}
