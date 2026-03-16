// Usage: flagsc <input.flags> <output_basename>
//
// Reads a .flags DSL file and generates <output_basename>.h and
// <output_basename>.cc in the current directory.
//
// Example:
//   flagsc my_flags.flags my_flags

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "codegen.h"
#include "dsl.h"

static const int kPathSize = 512;

int
main( int argc, char **argv )
{
    if ( argc != 3 ) {
        fprintf( stderr, "Usage: flagsc <input.flags> <output_basename>\n" );
        fprintf( stderr, "\n" );
        fprintf(
            stderr,
            "Reads a .flags DSL file and generates <output_basename>.h and\n" );
        fprintf( stderr, "<output_basename>.cc in the current directory.\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "Example:\n" );
        fprintf( stderr, "  flagsc my_flags.flags my_flags\n" );
        return 1;
    }

    const char *input_path = argv[1];
    const char *basename   = argv[2];

    // Read entire input file
    FILE *fin = fopen( input_path, "r" );
    if ( fin == nullptr ) {
        fprintf( stderr, "flagsc: cannot open input file: %s\n", input_path );
        return 1;
    }

    fseek( fin, 0, SEEK_END );
    long file_size = ftell( fin );
    fseek( fin, 0, SEEK_SET );

    char *buf   = new char[file_size + 1];
    long  nread = (long)fread( buf, 1, (size_t)file_size, fin );
    fclose( fin );

    if ( nread != file_size ) {
        fprintf( stderr, "flagsc: failed to read input file: %s\n",
                 input_path );
        delete[] buf;
        return 1;
    }
    buf[file_size] = '\0';

    // Parse DSL
    forge::FlagDef def;
    std::string    err;
    if ( forge::FlagDefParseDsl( &def, buf, &err ) ) {
        fprintf( stderr, "flagsc: parse error: %s\n", err.c_str( ) );
        delete[] buf;
        return 1;
    }
    delete[] buf;

    // Build output file paths
    char h_path[kPathSize];
    char cc_path[kPathSize];
    snprintf( h_path, sizeof( h_path ), "%s.h", basename );
    snprintf( cc_path, sizeof( cc_path ), "%s.cc", basename );

    FILE *out_h  = fopen( h_path, "w" );
    FILE *out_cc = fopen( cc_path, "w" );

    if ( out_h == nullptr ) {
        fprintf( stderr, "flagsc: cannot open output file: %s\n", h_path );
        return 1;
    }
    if ( out_cc == nullptr ) {
        fclose( out_h );
        fprintf( stderr, "flagsc: cannot open output file: %s\n", cc_path );
        return 1;
    }

    if ( forge::CodegenWrite( &def, basename, out_h, out_cc, &err ) ) {
        fclose( out_h );
        fclose( out_cc );
        fprintf( stderr, "flagsc: codegen error: %s\n", err.c_str( ) );
        return 1;
    }

    fclose( out_h );
    fclose( out_cc );

    return 0;
}
