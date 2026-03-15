/*
 * jsonlite.c — SAX-style JSON parser implementation
 *
 * Recursive-descent, single-pass, zero-allocation-friendly.
 * Conforms to RFC 8259.
 */

#include "jsonlite.h"

#include <stdlib.h>
#include <string.h>

/* ---------- internal parser state ---------- */

typedef struct {
    const char       *json;
    size_t            len;
    size_t            pos;
    size_t            line;
    size_t            col;
    int               depth;
    jsonlite_callback cb;
    void             *user_data;
    jsonlite_result   result;
    char *heap_buf; /* single heap allocation for oversized unescape */
} jsonlite_parser;

/* ---------- forward declarations ---------- */

static int parse_value( jsonlite_parser *p );

/* ---------- helpers ---------- */

static void
set_error( jsonlite_parser *p, jsonlite_error err )
{
    p->result.error  = err;
    p->result.offset = p->pos;
    p->result.line   = p->line;
    p->result.column = p->col;
}

static int
peek( const jsonlite_parser *p )
{
    if ( p->pos >= p->len ) return -1;
    return (unsigned char)p->json[p->pos];
}

static void
advance( jsonlite_parser *p )
{
    if ( p->pos < p->len ) {
        if ( p->json[p->pos] == '\n' ) {
            p->line++;
            p->col = 1;
        } else {
            p->col++;
        }
        p->pos++;
    }
}

static void
skip_whitespace( jsonlite_parser *p )
{
    while ( p->pos < p->len ) {
        char c = p->json[p->pos];
        if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' ) {
            advance( p );
        } else {
            break;
        }
    }
}

static int
emit( jsonlite_parser *p, const jsonlite_event *ev )
{
    if ( p->cb ) {
        if ( p->cb( ev, p->user_data ) != 0 ) {
            set_error( p, JSONLITE_ABORTED );
            return -1;
        }
    }
    return 0;
}

static int
hex_digit( int c )
{
    if ( c >= '0' && c <= '9' ) return c - '0';
    if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
    return -1;
}

/* Encode a Unicode codepoint as UTF-8 into buf. Returns bytes written. */
static int
encode_utf8( unsigned long cp, char *buf )
{
    if ( cp <= 0x7F ) {
        buf[0] = (char)cp;
        return 1;
    } else if ( cp <= 0x7FF ) {
        buf[0] = (char)( 0xC0 | ( cp >> 6 ) );
        buf[1] = (char)( 0x80 | ( cp & 0x3F ) );
        return 2;
    } else if ( cp <= 0xFFFF ) {
        buf[0] = (char)( 0xE0 | ( cp >> 12 ) );
        buf[1] = (char)( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
        buf[2] = (char)( 0x80 | ( cp & 0x3F ) );
        return 3;
    } else if ( cp <= 0x10FFFF ) {
        buf[0] = (char)( 0xF0 | ( cp >> 18 ) );
        buf[1] = (char)( 0x80 | ( ( cp >> 12 ) & 0x3F ) );
        buf[2] = (char)( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
        buf[3] = (char)( 0x80 | ( cp & 0x3F ) );
        return 4;
    }
    return 0;
}

/* Parse 4 hex digits for \uXXXX. Returns codepoint or -1 on error. */
static long
parse_hex4( jsonlite_parser *p )
{
    unsigned long val = 0;
    int           i;
    for ( i = 0; i < 4; i++ ) {
        int c = peek( p );
        int d = ( c >= 0 ) ? hex_digit( c ) : -1;
        if ( d < 0 ) {
            set_error( p, JSONLITE_INVALID_UNICODE );
            return -1;
        }
        val = ( val << 4 ) | (unsigned long)d;
        advance( p );
    }
    return (long)val;
}

/* ---------- parse_string ---------- */

static int
parse_string_impl( jsonlite_parser *p, jsonlite_str *out )
{
    size_t start_offset = p->pos;

    /* consume opening quote */
    advance( p ); /* skip '"' */

    /* Fast path: scan for closing quote with no escapes */
    {
        size_t scan = p->pos;
        while ( scan < p->len ) {
            unsigned char c = (unsigned char)p->json[scan];
            if ( c == '"' ) {
                /* no escapes — zero-copy */
                out->ptr   = p->json + p->pos;
                out->len   = scan - p->pos;
                out->owned = 0;
                /* advance parser past string content + closing quote */
                while ( p->pos <= scan ) advance( p );
                return 0;
            }
            if ( c == '\\' ) break; /* has escapes, take slow path */
            if ( c < 0x20 ) {
                /* unescaped control character */
                p->pos = scan;
                set_error( p, JSONLITE_INVALID_STRING );
                return -1;
            }
            scan++;
        }
        if ( scan >= p->len && p->json[scan - 1] != '"' ) {
            /* We went off the end without finding a quote or escape */
            /* Fall through — will be caught below or in slow path */
        }
    }

    /* Slow path: unescape into buffer */
    {
        char   stack_buf[JSONLITE_UNESCAPE_STACK_SIZE];
        char  *buf       = stack_buf;
        size_t buf_cap   = JSONLITE_UNESCAPE_STACK_SIZE;
        size_t buf_len   = 0;
        int    used_heap = 0;

        /* Copy any chars before the first escape that fast-path found */
        /* (parser pos is still at the start of string content) */

        for ( ;; ) {
            int c = peek( p );
            if ( c < 0 ) {
                if ( used_heap ) free( buf );
                set_error( p, JSONLITE_UNEXPECTED_END );
                p->pos = start_offset; /* for better error reporting */
                return -1;
            }
            if ( c == '"' ) {
                advance( p ); /* consume closing quote */
                break;
            }
            if ( (unsigned char)c < 0x20 ) {
                if ( used_heap ) free( buf );
                set_error( p, JSONLITE_INVALID_STRING );
                return -1;
            }

            if ( c == '\\' ) {
                advance( p ); /* consume backslash */
                c = peek( p );
                if ( c < 0 ) {
                    if ( used_heap ) free( buf );
                    set_error( p, JSONLITE_UNEXPECTED_END );
                    return -1;
                }

                /* ensure room for up to 4 bytes (UTF-8) */
                if ( buf_len + 4 >= buf_cap ) {
                    size_t new_cap = buf_cap * 2;
                    char  *new_buf;
                    if ( used_heap ) {
                        new_buf = (char *)realloc( buf, new_cap );
                    } else {
                        new_buf = (char *)malloc( new_cap );
                        if ( new_buf ) memcpy( new_buf, buf, buf_len );
                    }
                    if ( !new_buf ) {
                        if ( used_heap ) free( buf );
                        set_error( p, JSONLITE_ALLOC_FAILED );
                        return -1;
                    }
                    buf       = new_buf;
                    buf_cap   = new_cap;
                    used_heap = 1;
                }

                switch ( c ) {
                case '"':
                    buf[buf_len++] = '"';
                    advance( p );
                    break;
                case '\\':
                    buf[buf_len++] = '\\';
                    advance( p );
                    break;
                case '/':
                    buf[buf_len++] = '/';
                    advance( p );
                    break;
                case 'b':
                    buf[buf_len++] = '\b';
                    advance( p );
                    break;
                case 'f':
                    buf[buf_len++] = '\f';
                    advance( p );
                    break;
                case 'n':
                    buf[buf_len++] = '\n';
                    advance( p );
                    break;
                case 'r':
                    buf[buf_len++] = '\r';
                    advance( p );
                    break;
                case 't':
                    buf[buf_len++] = '\t';
                    advance( p );
                    break;
                case 'u': {
                    long cp;
                    advance( p ); /* consume 'u' */
                    cp = parse_hex4( p );
                    if ( cp < 0 ) {
                        if ( used_heap ) free( buf );
                        return -1;
                    }
                    /* Handle surrogate pairs */
                    if ( cp >= 0xD800 && cp <= 0xDBFF ) {
                        long          lo;
                        unsigned long full;
                        if ( peek( p ) != '\\' ) {
                            if ( used_heap ) free( buf );
                            set_error( p, JSONLITE_INVALID_UNICODE );
                            return -1;
                        }
                        advance( p );
                        if ( peek( p ) != 'u' ) {
                            if ( used_heap ) free( buf );
                            set_error( p, JSONLITE_INVALID_UNICODE );
                            return -1;
                        }
                        advance( p );
                        lo = parse_hex4( p );
                        if ( lo < 0 ) {
                            if ( used_heap ) free( buf );
                            return -1;
                        }
                        if ( lo < 0xDC00 || lo > 0xDFFF ) {
                            if ( used_heap ) free( buf );
                            set_error( p, JSONLITE_INVALID_UNICODE );
                            return -1;
                        }
                        full = (unsigned long)( 0x10000 +
                                                ( ( cp - 0xD800 ) << 10 ) +
                                                ( lo - 0xDC00 ) );
                        buf_len += (size_t)encode_utf8( full, buf + buf_len );
                    } else if ( cp >= 0xDC00 && cp <= 0xDFFF ) {
                        /* lone low surrogate */
                        if ( used_heap ) free( buf );
                        set_error( p, JSONLITE_INVALID_UNICODE );
                        return -1;
                    } else {
                        buf_len += (size_t)encode_utf8( (unsigned long)cp,
                                                        buf + buf_len );
                    }
                    break;
                }
                default:
                    if ( used_heap ) free( buf );
                    set_error( p, JSONLITE_INVALID_STRING );
                    return -1;
                }
            } else {
                /* ensure room */
                if ( buf_len + 1 >= buf_cap ) {
                    size_t new_cap = buf_cap * 2;
                    char  *new_buf;
                    if ( used_heap ) {
                        new_buf = (char *)realloc( buf, new_cap );
                    } else {
                        new_buf = (char *)malloc( new_cap );
                        if ( new_buf ) memcpy( new_buf, buf, buf_len );
                    }
                    if ( !new_buf ) {
                        if ( used_heap ) free( buf );
                        set_error( p, JSONLITE_ALLOC_FAILED );
                        return -1;
                    }
                    buf       = new_buf;
                    buf_cap   = new_cap;
                    used_heap = 1;
                }
                buf[buf_len++] = (char)c;
                advance( p );
            }
        }

        out->ptr = buf;
        out->len = buf_len;
        if ( used_heap ) {
            /* Track for cleanup */
            free( p->heap_buf );
            p->heap_buf = buf;
            out->owned  = 1;
        } else {
            out->owned = 0;
        }
        return 0;
    }
}

/* ---------- parse_number ---------- */

static int
parse_number( jsonlite_parser *p )
{
    size_t         start = p->pos;
    char           num_buf[64];
    size_t         num_len;
    char          *endptr;
    double         val;
    jsonlite_event ev;

    /* optional minus */
    if ( peek( p ) == '-' ) advance( p );

    /* integer part */
    if ( peek( p ) < 0 ) {
        set_error( p, JSONLITE_UNEXPECTED_END );
        return -1;
    }
    if ( peek( p ) == '0' ) {
        advance( p );
        /* no leading zeros: next must not be digit */
        if ( peek( p ) >= '0' && peek( p ) <= '9' ) {
            set_error( p, JSONLITE_INVALID_NUMBER );
            return -1;
        }
    } else if ( peek( p ) >= '1' && peek( p ) <= '9' ) {
        advance( p );
        while ( peek( p ) >= '0' && peek( p ) <= '9' ) advance( p );
    } else {
        set_error( p, JSONLITE_INVALID_NUMBER );
        return -1;
    }

    /* fractional part */
    if ( peek( p ) == '.' ) {
        advance( p );
        if ( peek( p ) < '0' || peek( p ) > '9' ) {
            set_error( p, JSONLITE_INVALID_NUMBER );
            return -1;
        }
        while ( peek( p ) >= '0' && peek( p ) <= '9' ) advance( p );
    }

    /* exponent */
    if ( peek( p ) == 'e' || peek( p ) == 'E' ) {
        advance( p );
        if ( peek( p ) == '+' || peek( p ) == '-' ) advance( p );
        if ( peek( p ) < '0' || peek( p ) > '9' ) {
            set_error( p, JSONLITE_INVALID_NUMBER );
            return -1;
        }
        while ( peek( p ) >= '0' && peek( p ) <= '9' ) advance( p );
    }

    num_len = p->pos - start;
    if ( num_len >= sizeof( num_buf ) ) {
        set_error( p, JSONLITE_INVALID_NUMBER );
        return -1;
    }
    memcpy( num_buf, p->json + start, num_len );
    num_buf[num_len] = '\0';

    val = strtod( num_buf, &endptr );
    if ( endptr != num_buf + num_len ) {
        set_error( p, JSONLITE_INVALID_NUMBER );
        return -1;
    }

    ev.type         = JSONLITE_EVENT_NUMBER;
    ev.value.number = val;
    ev.depth        = p->depth;
    ev.offset       = start;
    return emit( p, &ev );
}

/* ---------- parse_literal ---------- */

static int
parse_literal( jsonlite_parser *p )
{
    size_t         start = p->pos;
    jsonlite_event ev;

    if ( p->len - p->pos >= 4 && memcmp( p->json + p->pos, "true", 4 ) == 0 ) {
        ev.type          = JSONLITE_EVENT_BOOL;
        ev.value.boolean = 1;
        ev.depth         = p->depth;
        ev.offset        = start;
        p->pos += 4;
        p->col += 4;
        return emit( p, &ev );
    }
    if ( p->len - p->pos >= 5 && memcmp( p->json + p->pos, "false", 5 ) == 0 ) {
        ev.type          = JSONLITE_EVENT_BOOL;
        ev.value.boolean = 0;
        ev.depth         = p->depth;
        ev.offset        = start;
        p->pos += 5;
        p->col += 5;
        return emit( p, &ev );
    }
    if ( p->len - p->pos >= 4 && memcmp( p->json + p->pos, "null", 4 ) == 0 ) {
        ev.type   = JSONLITE_EVENT_NULL;
        ev.depth  = p->depth;
        ev.offset = start;
        p->pos += 4;
        p->col += 4;
        return emit( p, &ev );
    }

    set_error( p, JSONLITE_INVALID_LITERAL );
    return -1;
}

/* ---------- parse_string (event-emitting wrappers) ---------- */

static int
parse_string_event( jsonlite_parser *p, jsonlite_event_type type )
{
    jsonlite_str   str;
    jsonlite_event ev;
    size_t         start = p->pos;
    if ( parse_string_impl( p, &str ) != 0 ) return -1;
    ev.type      = type;
    ev.value.str = str;
    ev.depth     = p->depth;
    ev.offset    = start;
    return emit( p, &ev );
}

/* ---------- parse_object ---------- */

static int
parse_object( jsonlite_parser *p )
{
    jsonlite_event ev;
    int            first = 1;

    if ( p->depth >= JSONLITE_MAX_DEPTH ) {
        set_error( p, JSONLITE_DEPTH_EXCEEDED );
        return -1;
    }

    ev.type   = JSONLITE_EVENT_OBJECT_START;
    ev.depth  = p->depth;
    ev.offset = p->pos;
    memset( &ev.value, 0, sizeof( ev.value ) );
    if ( emit( p, &ev ) != 0 ) return -1;

    advance( p ); /* consume '{' */
    p->depth++;

    skip_whitespace( p );
    if ( peek( p ) == '}' ) {
        advance( p );
        p->depth--;
        ev.type   = JSONLITE_EVENT_OBJECT_END;
        ev.depth  = p->depth;
        ev.offset = p->pos - 1;
        return emit( p, &ev );
    }

    for ( ;; ) {
        if ( !first ) {
            skip_whitespace( p );
            if ( peek( p ) != ',' ) break;
            advance( p ); /* consume ',' */
        }
        first = 0;

        skip_whitespace( p );
        if ( peek( p ) != '"' ) {
            set_error( p, JSONLITE_UNEXPECTED_CHAR );
            return -1;
        }
        if ( parse_string_event( p, JSONLITE_EVENT_KEY ) != 0 ) return -1;

        skip_whitespace( p );
        if ( peek( p ) != ':' ) {
            set_error( p, JSONLITE_UNEXPECTED_CHAR );
            return -1;
        }
        advance( p ); /* consume ':' */

        skip_whitespace( p );
        if ( parse_value( p ) != 0 ) return -1;
    }

    if ( peek( p ) != '}' ) {
        set_error( p, JSONLITE_UNEXPECTED_CHAR );
        return -1;
    }
    advance( p );
    p->depth--;

    ev.type   = JSONLITE_EVENT_OBJECT_END;
    ev.depth  = p->depth;
    ev.offset = p->pos - 1;
    return emit( p, &ev );
}

/* ---------- parse_array ---------- */

static int
parse_array( jsonlite_parser *p )
{
    jsonlite_event ev;
    int            first = 1;

    if ( p->depth >= JSONLITE_MAX_DEPTH ) {
        set_error( p, JSONLITE_DEPTH_EXCEEDED );
        return -1;
    }

    ev.type   = JSONLITE_EVENT_ARRAY_START;
    ev.depth  = p->depth;
    ev.offset = p->pos;
    memset( &ev.value, 0, sizeof( ev.value ) );
    if ( emit( p, &ev ) != 0 ) return -1;

    advance( p ); /* consume '[' */
    p->depth++;

    skip_whitespace( p );
    if ( peek( p ) == ']' ) {
        advance( p );
        p->depth--;
        ev.type   = JSONLITE_EVENT_ARRAY_END;
        ev.depth  = p->depth;
        ev.offset = p->pos - 1;
        return emit( p, &ev );
    }

    for ( ;; ) {
        if ( !first ) {
            skip_whitespace( p );
            if ( peek( p ) != ',' ) break;
            advance( p ); /* consume ',' */
        }
        first = 0;

        skip_whitespace( p );
        if ( parse_value( p ) != 0 ) return -1;
    }

    if ( peek( p ) != ']' ) {
        set_error( p, JSONLITE_UNEXPECTED_CHAR );
        return -1;
    }
    advance( p );
    p->depth--;

    ev.type   = JSONLITE_EVENT_ARRAY_END;
    ev.depth  = p->depth;
    ev.offset = p->pos - 1;
    return emit( p, &ev );
}

/* ---------- parse_value ---------- */

static int
parse_value( jsonlite_parser *p )
{
    int c = peek( p );
    if ( c < 0 ) {
        set_error( p, JSONLITE_UNEXPECTED_END );
        return -1;
    }
    switch ( c ) {
    case '{':
        return parse_object( p );
    case '[':
        return parse_array( p );
    case '"':
        return parse_string_event( p, JSONLITE_EVENT_STRING );
    case 't':
    case 'f':
    case 'n':
        return parse_literal( p );
    case '-':
        return parse_number( p );
    default:
        if ( c >= '0' && c <= '9' ) return parse_number( p );
        set_error( p, JSONLITE_UNEXPECTED_CHAR );
        return -1;
    }
}

/* ---------- public API ---------- */

jsonlite_result
jsonlite_parse( const char *json, size_t len, jsonlite_callback cb,
                void *user_data )
{
    jsonlite_parser p;
    memset( &p, 0, sizeof( p ) );
    p.json         = json;
    p.len          = len;
    p.pos          = 0;
    p.line         = 1;
    p.col          = 1;
    p.depth        = 0;
    p.cb           = cb;
    p.user_data    = user_data;
    p.heap_buf     = NULL;
    p.result.error = JSONLITE_OK;

    skip_whitespace( &p );
    if ( peek( &p ) < 0 ) {
        set_error( &p, JSONLITE_UNEXPECTED_END );
        free( p.heap_buf );
        return p.result;
    }

    if ( parse_value( &p ) != 0 ) {
        free( p.heap_buf );
        return p.result;
    }

    skip_whitespace( &p );
    if ( peek( &p ) >= 0 ) {
        set_error( &p, JSONLITE_TRAILING_CONTENT );
    }

    free( p.heap_buf );
    return p.result;
}

const char *
jsonlite_error_str( jsonlite_error err )
{
    switch ( err ) {
    case JSONLITE_OK:
        return "ok";
    case JSONLITE_UNEXPECTED_CHAR:
        return "unexpected character";
    case JSONLITE_UNEXPECTED_END:
        return "unexpected end of input";
    case JSONLITE_INVALID_STRING:
        return "invalid string";
    case JSONLITE_INVALID_NUMBER:
        return "invalid number";
    case JSONLITE_INVALID_LITERAL:
        return "invalid literal";
    case JSONLITE_TRAILING_CONTENT:
        return "trailing content after value";
    case JSONLITE_DEPTH_EXCEEDED:
        return "maximum nesting depth exceeded";
    case JSONLITE_INVALID_UNICODE:
        return "invalid unicode escape";
    case JSONLITE_ALLOC_FAILED:
        return "memory allocation failed";
    case JSONLITE_ABORTED:
        return "parsing aborted by callback";
    }
    return "unknown error";
}
