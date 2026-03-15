/*
 * example.c — test/demo for jsonlite SAX-style JSON parser
 */

#include "jsonlite.h"

#include <stdio.h>
#include <string.h>

/* ---------- print callback ---------- */

static const char *
event_name( jsonlite_event_type type )
{
    switch ( type ) {
    case JSONLITE_EVENT_OBJECT_START:
        return "OBJECT_START";
    case JSONLITE_EVENT_OBJECT_END:
        return "OBJECT_END";
    case JSONLITE_EVENT_ARRAY_START:
        return "ARRAY_START";
    case JSONLITE_EVENT_ARRAY_END:
        return "ARRAY_END";
    case JSONLITE_EVENT_KEY:
        return "KEY";
    case JSONLITE_EVENT_STRING:
        return "STRING";
    case JSONLITE_EVENT_NUMBER:
        return "NUMBER";
    case JSONLITE_EVENT_BOOL:
        return "BOOL";
    case JSONLITE_EVENT_NULL:
        return "NULL";
    }
    return "???";
}

static int
print_callback( const jsonlite_event *ev, void *user_data )
{
    int i;
    int indent = ev->depth;
    (void)user_data;

    for ( i = 0; i < indent; i++ ) printf( "  " );
    printf( "%-14s", event_name( ev->type ) );

    switch ( ev->type ) {
    case JSONLITE_EVENT_KEY:
    case JSONLITE_EVENT_STRING:
        printf( " \"%.*s\"", (int)ev->value.str.len, ev->value.str.ptr );
        break;
    case JSONLITE_EVENT_NUMBER:
        printf( " %g", ev->value.number );
        break;
    case JSONLITE_EVENT_BOOL:
        printf( " %s", ev->value.boolean ? "true" : "false" );
        break;
    default:
        break;
    }

    printf( "  (depth=%d, offset=%zu)\n", ev->depth, ev->offset );
    return 0;
}

/* ---------- abort callback (stops after 3 events) ---------- */

static int
abort_callback( const jsonlite_event *ev, void *user_data )
{
    int *count = (int *)user_data;
    (void)ev;
    ( *count )++;
    if ( *count >= 3 ) return 1; /* abort */
    return 0;
}

/* ---------- test runner ---------- */

static void
run_test( const char *label, const char *json, size_t len, jsonlite_callback cb,
          void *user_data )
{
    jsonlite_result r;
    printf( "=== %s ===\n", label );
    if ( len == 0 && json ) len = strlen( json );
    r = jsonlite_parse( json, len, cb, user_data );
    if ( r.error != JSONLITE_OK ) {
        printf( "  ERROR: %s at offset %zu (line %zu, col %zu)\n",
                jsonlite_error_str( r.error ), r.offset, r.line, r.column );
    } else {
        printf( "  OK\n" );
    }
    printf( "\n" );
}

/* ---------- main ---------- */

int
main( void )
{
    int abort_count;

    /* 1. Basic object with nested values and escapes */
    {
        const char *json =
            "{\n"
            "  \"name\": \"Alice \\\"Wonderland\\\"\\nLine2\",\n"
            "  \"age\": 30,\n"
            "  \"scores\": [100, 98.5, -3.14e2],\n"
            "  \"active\": true,\n"
            "  \"address\": null,\n"
            "  \"emoji\": \"\\u0048\\u0065\\u006C\\u006C\\u006F\",\n"
            "  \"surrogate\": \"\\uD83D\\uDE00\",\n"
            "  \"nested\": { \"x\": [1, [2, [3]]] }\n"
            "}";
        run_test( "Basic object", json, 0, print_callback, NULL );
    }

    /* 2. Top-level string */
    run_test( "Top-level string", "\"hello world\"", 0, print_callback, NULL );

    /* 3. Top-level number */
    run_test( "Top-level number", "42", 0, print_callback, NULL );

    /* 4. Top-level true */
    run_test( "Top-level true", "true", 0, print_callback, NULL );

    /* 5. Top-level false */
    run_test( "Top-level false", "false", 0, print_callback, NULL );

    /* 6. Top-level null */
    run_test( "Top-level null", "null", 0, print_callback, NULL );

    /* 7. Empty object */
    run_test( "Empty object", "{}", 0, print_callback, NULL );

    /* 8. Empty array */
    run_test( "Empty array", "[]", 0, print_callback, NULL );

    /* --- Error cases --- */

    /* 9. Bad syntax */
    run_test( "Error: bad syntax", "{bad}", 0, print_callback, NULL );

    /* 10. Leading zero */
    run_test( "Error: leading zero", "01", 0, print_callback, NULL );

    /* 11. Empty input */
    run_test( "Error: empty input", "", 0, print_callback, NULL );

    /* 12. Trailing content */
    run_test( "Error: trailing content", "true false", 0, print_callback,
              NULL );

    /* 13. Trailing comma */
    run_test( "Error: trailing comma", "[1,2,]", 0, print_callback, NULL );

    /* 14. Bare plus */
    run_test( "Error: bare +", "+1", 0, print_callback, NULL );

    /* 15. Lone surrogate */
    run_test( "Error: lone surrogate", "\"\\uD800\"", 0, print_callback, NULL );

    /* 16. Callback abort */
    abort_count = 0;
    run_test( "Callback abort after 3 events", "{\"a\":1,\"b\":2}", 0,
              abort_callback, &abort_count );

    printf( "All tests completed.\n" );
    return 0;
}
