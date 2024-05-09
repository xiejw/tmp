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
CopyWithNewName( std::string &Src, const char *DstDir, int Index )
{
    char SrcBuf[1000], DstBuf[1000];

    snprintf( SrcBuf, 1000 - 1, "%s", Src.c_str( ) );

    const char *FileName = Src.c_str( );
    auto        ext      = strrchr( FileName, '.' );
    if ( !ext ) {
        snprintf( DstBuf, 1000 - 1, "%s/%010d", DstDir, Index );
    } else {
        snprintf( DstBuf, 1000 - 1, "%s/%010d%s", DstDir, Index, ext );
    }

    if ( !eve::fs::CopyFile( DstBuf, SrcBuf ) ) {
        ALERT( "unexpected error: %s\n", strerror( errno ) );
        ALERT( "%s -> %s\n", SrcBuf, DstBuf );
        exit( 1 );
    }
    return DstBuf;
}

int
main( int argc, char **argv )
{
    int         StartIndex = argc > 1 ? atoi( argv[1] ) : 0;
    const char *DstDir     = argc > 2 ? argv[2] : "Sorted";

    if ( argc > 3 ) {
        ALERT( "usage: <prog> <start_index> <dest_dir>\n" );
        exit( 2 );
    }

    auto DirReader = std::unique_ptr<eve::fs::FsDir>{ eve::fs::OpenDir( "." ) };
    if ( DirReader == nullptr ) {
        ALERT( "failed to open dir\n" );
        exit( 1 );
    }

    std::vector<std::pair<std::string, time_t>> FileNames{ };
    DirReader->Run( [&]( const char *RegFileName ) {
        struct stat sb;
        if ( stat( RegFileName, &sb ) == -1 ) {
            ALERT( "failed\n" );
        } else {
            time_t BirthTime = sb.st_birthtime;
            INFO( "birthtime for %s: %s", RegFileName, ctime( &BirthTime ) );
            FileNames.push_back( { RegFileName, BirthTime } );
        }
    } );
    INFO( "total %zu files\n", FileNames.size( ) );

    std::sort( FileNames.begin( ), FileNames.end( ),
               []( auto a, auto b ) { return a.second < b.second; } );

    mkdir( DstDir, 0755 );

    for ( auto &NameAndTime : FileNames ) {
        std::string &Src = NameAndTime.first;
        std::string  Dst =
            CopyWithNewName( NameAndTime.first, DstDir, StartIndex++ );

        INFO( "birthtime: %ld: %s <- %s\n", NameAndTime.second, Dst.c_str( ),
              Src.c_str( ) );
    }

    return 0;
}
