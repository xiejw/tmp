/*
 * jsonlite.h — SAX-style JSON parser (public API)
 *
 * Zero-allocation-friendly, callback/event-driven JSON parser.
 * Conforms to RFC 8259.
 */

#ifndef JSONLITE_H
#define JSONLITE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- compile-time limits (overridable via -D) ---------- */

#ifndef JSONLITE_MAX_DEPTH
#define JSONLITE_MAX_DEPTH 512
#endif

#ifndef JSONLITE_UNESCAPE_STACK_SIZE
#define JSONLITE_UNESCAPE_STACK_SIZE 1024
#endif

/* ---------- event types ---------- */

typedef enum {
    JSONLITE_EVENT_OBJECT_START,
    JSONLITE_EVENT_OBJECT_END,
    JSONLITE_EVENT_ARRAY_START,
    JSONLITE_EVENT_ARRAY_END,
    JSONLITE_EVENT_KEY,
    JSONLITE_EVENT_STRING,
    JSONLITE_EVENT_NUMBER,
    JSONLITE_EVENT_BOOL,
    JSONLITE_EVENT_NULL
} jsonlite_event_type;

/* ---------- error codes ---------- */

typedef enum {
    JSONLITE_OK = 0,
    JSONLITE_UNEXPECTED_CHAR,
    JSONLITE_UNEXPECTED_END,
    JSONLITE_INVALID_STRING,
    JSONLITE_INVALID_NUMBER,
    JSONLITE_INVALID_LITERAL,
    JSONLITE_TRAILING_CONTENT,
    JSONLITE_DEPTH_EXCEEDED,
    JSONLITE_INVALID_UNICODE,
    JSONLITE_ALLOC_FAILED,
    JSONLITE_ABORTED
} jsonlite_error;

/* ---------- value types ---------- */

typedef struct {
    const char *ptr;
    size_t      len;
    int         owned; /* 1 if ptr points to a temporary unescape buffer */
} jsonlite_str;

typedef union {
    jsonlite_str str;     /* KEY, STRING */
    double       number;  /* NUMBER */
    int          boolean; /* BOOL */
} jsonlite_value;

/* ---------- event ---------- */

typedef struct {
    jsonlite_event_type type;
    jsonlite_value      value;
    int                 depth;
    size_t              offset; /* byte offset in input */
} jsonlite_event;

/* ---------- callback ---------- */

/* Return 0 to continue parsing, non-zero to abort. */
typedef int ( *jsonlite_callback )( const jsonlite_event *event,
                                    void                 *user_data );

/* ---------- result ---------- */

typedef struct {
    jsonlite_error error;
    size_t         offset;
    size_t         line;
    size_t         column;
} jsonlite_result;

/* ---------- API ---------- */

jsonlite_result jsonlite_parse( const char *json, size_t len,
                                jsonlite_callback cb, void *user_data );

const char *jsonlite_error_str( jsonlite_error err );

#ifdef __cplusplus
}
#endif

#endif /* JSONLITE_H */
