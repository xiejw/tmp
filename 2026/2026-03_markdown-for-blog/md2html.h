#ifndef MD2HTML_H
#define MD2HTML_H

#include <stddef.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * ErrorStack -- dynamic char buffer accumulating error messages
 * ------------------------------------------------------------------------- */

typedef struct {
    char  *buf;
    size_t len;
    size_t cap;
} ErrorStack;

void        forge_error_stack_init(ErrorStack *stk);
void        forge_error_stack_free(ErrorStack *stk);
void        forge_emit_error(ErrorStack *stk, const char *fmt, ...);
const char *forge_error_stack_get(const ErrorStack *stk); /* NULL if empty */

/* -------------------------------------------------------------------------
 * HtmlTemplate -- begin/end HTML blocks read from a template file
 *
 * Template file format:
 *
 *   begin: <<EOF
 *   ...html...
 *   EOF
 *
 *   end: <<EOF
 *   ...html...
 *   EOF
 * ------------------------------------------------------------------------- */

typedef struct {
    char *begin_html;
    char *end_html;
} HtmlTemplate;

int  forge_parse_template(const char *path, HtmlTemplate *tmpl, ErrorStack *stk);
void forge_template_free(HtmlTemplate *tmpl);

/* -------------------------------------------------------------------------
 * HeadingHooks -- optional per-level custom renderers
 *
 * If hooks[i] is NULL the default <h{i+1}>...</h{i+1}> is emitted.
 * Return 0 on success, 1 on error (emit into stk).
 * ------------------------------------------------------------------------- */

typedef int (*heading_hook_fn)(int level, const char *text, size_t text_len,
                               FILE *out, ErrorStack *stk);

typedef struct {
    heading_hook_fn hooks[6]; /* index 0 = h1 … index 5 = h6 */
} HeadingHooks;

/* -------------------------------------------------------------------------
 * forge_convert -- main entry point
 *
 * Reads input_path, writes HTML to out.
 * tmpl and hooks may be NULL.
 * Returns 0 on success, 1 if any error was emitted into stk.
 * ------------------------------------------------------------------------- */

int forge_convert(const char *input_path, FILE *out,
                  const HtmlTemplate *tmpl, HeadingHooks *hooks,
                  ErrorStack *stk);

#endif /* MD2HTML_H */
