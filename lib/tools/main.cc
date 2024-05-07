#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#define INFO( fmt, ... )  printf( "INFO : " fmt __VA_OPT__(, ) __VA_ARGS__ )
#define DEBUG( fmt, ... ) printf( "DEBUG: " fmt __VA_OPT__(, ) __VA_ARGS__ )
#define ALERT( fmt, ... ) printf( "ALERT: " fmt __VA_OPT__(, ) __VA_ARGS__ )

#include "fs.h"

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
    for ( auto &f : FileNames ) {
        INFO( "birthtime: %ld: %s\n", f.second, f.first.c_str( ) );
    }

    return 0;
}
