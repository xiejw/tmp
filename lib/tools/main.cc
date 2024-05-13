// This utility finds all photos in the current folder ("."), and sorts them by
// birthtime (only available in macOS, not Linux).
//
// After that, it copies the file to the destination folder with a new name
// containing the index counter. The extention of the file is kept. The start
// index counter is 0 but can be supplied by the argument of the cmd.
//
// Usage: <prog> <start_index> <dest_dir>
//
// For example, assume we have the following three files in current folder:
//
//   name   birthtime
//   a.jpg  3
//   b.mp4  2
//   c      1
//
//  The cmd runs with
//
//    <prog> 4 ../sorted
//
//  will result to new copies like
//
//    ../sorted/0000000004      -> c     (youngest first)
//    ../sorted/0000000005.mp4  -> b.mp4
//    ../sorted/0000000006.jpg  -> a.jpg
//
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
#define ALERT( fmt, ... ) \
    printf( "ALERT %s:%d: " fmt, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__ )

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
        char        buf[100];
        if ( stat( RegFileName, &sb ) == -1 ) {
            ALERT( "failed\n" );
        } else {
            time_t BirthTime = sb.st_birthtime;
            strcpy( buf, ctime( &BirthTime ) );
            buf[strlen( buf ) - 1] = 0;  // remove the final newline
            INFO( "birthtime %s: %s\n", buf, RegFileName );
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
