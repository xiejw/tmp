// Test driver for generated test_flags.h / test_flags.cc
#include <cstdio>
#include <cstdlib>
#include <string>

#include "test_flags.h"

int
main( int argc, char **argv )
{
    forge::Flags flags;
    forge::FlagsInit( &flags );

    std::string err;
    if ( forge::FlagsParse( &flags, argc, argv, &err ) ) {
        fprintf( stderr, "Error: %s\n", err.c_str( ) );
        return 1;
    }
    if ( forge::FlagsValidate( &flags, &err ) ) {
        fprintf( stderr, "Error: %s\n", err.c_str( ) );
        return 1;
    }

    printf( "output=%s count=%d threshold=%ld\n", flags.output, flags.count,
            flags.threshold );
    return 0;
}
