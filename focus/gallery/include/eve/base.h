// vim: ft=cpp
#pragma once

#include <cstdio>

#define INFO( fmt, ... ) printf( "INFO : " fmt "\n" __VA_OPT__(, ) __VA_ARGS__ )
#define DEBUG( fmt, ... ) \
    printf( "DEBUG: " fmt "\n" __VA_OPT__(, ) __VA_ARGS__ )
#define ALERT( fmt, ... )                       \
    printf( "ALERT %s:%d: " fmt "\n", __FILE__, \
            __LINE__ __VA_OPT__(, ) __VA_ARGS__ )

#define PANIC( fmt, ... )                              \
    do {                                               \
        printf( "ALERT %s:%d: " fmt "\n", __FILE__,    \
                __LINE__ __VA_OPT__(, ) __VA_ARGS__ ); \
        exit( 1 );                                     \
    } while ( 0 )
