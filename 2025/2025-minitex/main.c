#include <fcntl.h>     // for open()
#include <stdio.h>     // for perror()
#include <stdlib.h>    // for malloc(), free()
#include <sys/stat.h>  // for fstat()
#include <unistd.h>    // for read(), close()

// Reads entire file into memory.
//
// Returns a malloc'd buffer containing the file data, or NULL on error.
// Sets *size to file length (if size != NULL).
//
// Caller must free() the buffer.
static char *
read_file_to_buffer( const char *path, size_t *size )
{
    int fd = open( path, O_RDONLY );
    if ( fd < 0 ) {
        perror( "open" );
        return NULL;
    }

    struct stat st;
    if ( fstat( fd, &st ) < 0 ) {
        perror( "fstat" );
        close( fd );
        return NULL;
    }

    // File size might be zero (e.g. /proc files), so handle carefully
    if ( st.st_size == 0 ) {
        close( fd );
        if ( size ) *size = 0;
        char *empty = malloc( 1 );
        if ( empty ) empty[0] = '\0';
        return empty;
    }

    off_t file_size = st.st_size;
    char *buffer    = malloc( file_size + 1 );
    if ( !buffer ) {
        perror( "malloc" );
        close( fd );
        return NULL;
    }

    ssize_t total_read = 0;
    while ( total_read < file_size ) {
        ssize_t bytes = read( fd, buffer + total_read, file_size - total_read );
        if ( bytes < 0 ) {
            perror( "read" );
            free( buffer );
            close( fd );
            return NULL;
        }
        if ( bytes == 0 ) break;  // EOF
        total_read += bytes;
    }

    buffer[total_read] = '\0';  // Null-terminate (useful for text)
    if ( size ) *size = total_read;

    close( fd );
    return buffer;
}

int
main( void )
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
    return 0;
}
