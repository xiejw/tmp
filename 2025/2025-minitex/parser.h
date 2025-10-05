#include <ctype.h>
#include <string.h>

// === --- Parser ---------------------------------------------------------- ===

// ===== Data Structures =====

typedef struct Command {
    char            *name;
    char            *content;
    struct Command **children;
    size_t           child_count;
} Command;

// ===== Utility =====

// Alloc-safe substring
static char *
substr( const char *start, size_t len )
{
    char *s = malloc( len + 1 );
    if ( !s ) return NULL;
    memcpy( s, start, len );
    s[len] = '\0';
    return s;
}

// Free recursively
void
cmd_free( Command *cmd )
{
    if ( !cmd ) return;
    free( cmd->name );
    free( cmd->content );
    for ( size_t i = 0; i < cmd->child_count; i++ )
        cmd_free( cmd->children[i] );
    free( cmd->children );
    free( cmd );
}

void
cmd_print( const Command *cmd, int depth )
{
    for ( int i = 0; i < depth; i++ ) printf( "  " );
    printf( "\\%s { %s }\n", cmd->name, cmd->content ? cmd->content : "" );
    for ( size_t i = 0; i < cmd->child_count; i++ )
        cmd_print( cmd->children[i], depth + 1 );
}

// ===== Recursive Parser =====

// Forward declaration
static Command *parse_command( const char **p );

// Reads content until matching brace (handles nested \cmd{...})
static char *
parse_braced_content( const char **p, Command *parent )
{
    const char *start   = *p;
    const char *out     = *p;
    char       *result  = malloc( 1 );
    size_t      out_len = 0;
    result[0]           = '\0';

    while ( **p ) {
        // Handle escape sequences
        if ( **p == '\\' ) {
            if ( ( *p )[1] == '\\' ) {  // literal backslash
                result            = realloc( result, out_len + 2 );
                result[out_len++] = '\\';
                result[out_len]   = '\0';
                *p += 2;
                continue;
            }

            // Try parse nested command
            const char *save  = *p;
            Command    *child = parse_command( p );
            if ( child ) {
                parent->children = realloc(
                    parent->children,
                    sizeof( Command * ) * ( parent->child_count + 1 ) );
                parent->children[parent->child_count++] = child;
                continue;
            } else {
                *p     = save;  // not a valid command, treat '\' literally
                result = realloc( result, out_len + 2 );
                result[out_len++] = *( *p )++;
                result[out_len]   = '\0';
                continue;
            }
        }

        if ( **p == '{' ) {
            // Nested braces inside content — copy as-is
            result            = realloc( result, out_len + 2 );
            result[out_len++] = *( *p )++;
            result[out_len]   = '\0';
            continue;
        }

        if ( **p == '}' ) {
            ( *p )++;  // consume closing brace
            break;
        }

        // Normal character
        result            = realloc( result, out_len + 2 );
        result[out_len++] = *( *p )++;
        result[out_len]   = '\0';
    }

    return result;
}

// Parse \cmd{content}
static Command *
parse_command( const char **p )
{
    if ( **p != '\\' ) {
        PANIC( "expect \\, got %s", *p );
    };

    const char *start = *p;
    ( *p )++;  // skip '\'

    const char *name_start = *p;
    while ( isalpha( (unsigned char)**p ) || **p == '_' ) ( *p )++;

    size_t name_len = *p - name_start;
    if ( name_len == 0 ) return NULL;  // not a valid command

    // Skip whitespace
    while ( isspace( (unsigned char)**p ) ) ( *p )++;

    if ( **p != '{' ) {
        *p = start;  // rollback if not followed by '{'
        return NULL;
    }

    ( *p )++;  // skip '{'

    Command *cmd = calloc( 1, sizeof( Command ) );
    cmd->name    = substr( name_start, name_len );

    // Parse content (recursive)
    cmd->content = parse_braced_content( p, cmd );

    return cmd;
}

// === --- Public API ------------------------------------------------------ ===

void
parse_all( const char *buf, Command ***out_cmds, size_t *out_count )
{
    *out_cmds     = NULL;
    *out_count    = 0;
    const char *p = buf;

    while ( *p ) {
        if ( *p == '\\' ) {
            Command *cmd = parse_command( &p );
            if ( cmd ) {
                *out_cmds = realloc( *out_cmds,
                                     sizeof( Command * ) * ( *out_count + 1 ) );

                ( *out_cmds )[( *out_count )++] = cmd;
                continue;
            }
        }
        PANIC( "expected cmd" );
    }
}
