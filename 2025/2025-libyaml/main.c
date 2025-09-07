#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

int
main( void )
{
    const char *yaml_input =
        "config:\n"
        "  database:\n"
        "    host: localhost\n"
        "    port: 5432\n"
        "  features:\n"
        "    enable: true\n"
        "    version: 1\n";

    yaml_parser_t parser;
    yaml_event_t  event;

    if ( !yaml_parser_initialize( &parser ) ) {
        fprintf( stderr, "Failed to initialize parser!\n" );
        return 1;
    }
    yaml_parser_set_input_string( &parser, (const unsigned char *)yaml_input,
                                  strlen( yaml_input ) );

    int   done     = 0;
    int   indent   = 0;     // Track map nesting level
    char *last_key = NULL;  // Store the most recent key

    while ( !done ) {
        if ( !yaml_parser_parse( &parser, &event ) ) {
            fprintf( stderr, "Parser error %d\n", parser.error );
            return 1;
        }

        switch ( event.type ) {
        case YAML_MAPPING_START_EVENT:
            indent++;
            break;

        case YAML_MAPPING_END_EVENT:
            indent--;
            break;

        case YAML_SCALAR_EVENT: {
            char *value = (char *)event.data.scalar.value;
            if ( !last_key ) {
                // First scalar is a key
                last_key = strdup( value );
            } else {
                // Second scalar is a value
                for ( int i = 0; i < indent; i++ )
                    printf( "  " );  // Indentation
                printf( "%s: %s\n", last_key, value );
                free( last_key );
                last_key = NULL;
            }
            break;
        }

        case YAML_STREAM_END_EVENT:
            done = 1;
            break;

        default:
            break;
        }

        yaml_event_delete( &event );
    }

    yaml_parser_delete( &parser );
    return 0;
}
