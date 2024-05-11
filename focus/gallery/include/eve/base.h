// vim: ft=cpp
#pragma once

#include <cstdio>

#define INFO( fmt, ... )  printf( "INFO : " fmt __VA_OPT__(, ) __VA_ARGS__ )
#define DEBUG( fmt, ... ) printf( "DEBUG: " fmt __VA_OPT__(, ) __VA_ARGS__ )
#define ALERT( fmt, ... ) \
    printf( "ALERT %s:%d: " fmt, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__ )

#define PANIC( fmt, ... )                              \
    do {                                               \
        printf( "ALERT %s:%d: " fmt, __FILE__,         \
                __LINE__ __VA_OPT__(, ) __VA_ARGS__ ); \
        exit( 1 );                                     \
    } while ( 0 )
