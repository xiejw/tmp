/*
 * sjson.h — SAX-style JSON parser (public API)
 *
 * Zero-allocation-friendly, callback/event-driven JSON parser.
 * Conforms to RFC 8259.
 */

#ifndef SJSON_H
#define SJSON_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- compile-time limits (overridable via -D) ---------- */

#ifndef SJSON_MAX_DEPTH
#define SJSON_MAX_DEPTH 512
#endif

#ifndef SJSON_UNESCAPE_STACK_SIZE
#define SJSON_UNESCAPE_STACK_SIZE 1024
#endif

/* ---------- event types ---------- */

typedef enum {
    SJSON_EVENT_OBJECT_START,
    SJSON_EVENT_OBJECT_END,
    SJSON_EVENT_ARRAY_START,
    SJSON_EVENT_ARRAY_END,
    SJSON_EVENT_KEY,
    SJSON_EVENT_STRING,
    SJSON_EVENT_NUMBER,
    SJSON_EVENT_BOOL,
    SJSON_EVENT_NULL
} sjson_event_type;

/* ---------- error codes ---------- */

typedef enum {
    SJSON_OK = 0,
    SJSON_UNEXPECTED_CHAR,
    SJSON_UNEXPECTED_END,
    SJSON_INVALID_STRING,
    SJSON_INVALID_NUMBER,
    SJSON_INVALID_LITERAL,
    SJSON_TRAILING_CONTENT,
    SJSON_DEPTH_EXCEEDED,
    SJSON_INVALID_UNICODE,
    SJSON_ALLOC_FAILED,
    SJSON_ABORTED
} sjson_error;

/* ---------- value types ---------- */

typedef struct {
    const char *ptr;
    size_t      len;
    int         owned; /* 1 if ptr points to a temporary unescape buffer */
} sjson_str;

typedef union {
    sjson_str str;    /* KEY, STRING */
    double    number; /* NUMBER */
    int       boolean;/* BOOL */
} sjson_value;

/* ---------- event ---------- */

typedef struct {
    sjson_event_type type;
    sjson_value      value;
    int              depth;
    size_t           offset; /* byte offset in input */
} sjson_event;

/* ---------- callback ---------- */

/* Return 0 to continue parsing, non-zero to abort. */
typedef int (*sjson_callback)(const sjson_event *event, void *user_data);

/* ---------- result ---------- */

typedef struct {
    sjson_error error;
    size_t      offset;
    size_t      line;
    size_t      column;
} sjson_result;

/* ---------- API ---------- */

sjson_result sjson_parse(const char *json, size_t len,
                         sjson_callback cb, void *user_data);

const char *sjson_error_str(sjson_error err);

#ifdef __cplusplus
}
#endif

#endif /* SJSON_H */
