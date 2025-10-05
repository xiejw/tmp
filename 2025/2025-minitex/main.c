#include <fcntl.h>     // for open()
#include <stdio.h>     // for perror()
#include <stdlib.h>    // for malloc(), free()
#include <sys/stat.h>  // for fstat()
#include <unistd.h>    // for read(), close()

#define PANIC( ... )                     \
    do {                                 \
        printf( "abort: " __VA_ARGS__ ); \
        printf( "\n" );                  \
        exit( 1 );                       \
    } while ( 0 )

#define MIN( x, y ) ( ( x ) <= ( y ) ? ( x ) : ( y ) )

#include "io.h"

#include "parser.h"

void
test_read_file( )
{
    size_t size;
    char  *data = read_file_to_buffer( "data/1.mt", &size );
    if ( data ) {
        printf( "File size: %zu bytes\n", size );
        printf( "Contents:\n%s\n", data );
        free( data );
    } else {
        printf( " Failed. \n" );
    }
}

void
test_parser( )
{
    const char *data =
        "\\p{Text before \\outer{some text and \\inner{deep content with "
        "\\\\escaped slash} inside} end.} continue";

    printf( "=== Input ===\n%s\n===      ===\n", data );

    Command **cmds  = NULL;
    size_t    count = 0;
    parse_all( data, &cmds, &count );

    for ( size_t i = 0; i < count; i++ ) cmd_print( cmds[i], 0 );
    for ( size_t i = 0; i < count; i++ ) cmd_free( cmds[i] );
    free( cmds );
}

int
main( void )
{
    test_read_file( );
    test_parser( );
    return 0;
}
