#include "md2html.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * ErrorStack
 * ========================================================================= */

#define ERRSTACK_INIT_CAP 256

void forge_error_stack_init(ErrorStack *stk) {
    stk->buf = NULL;
    stk->len = 0;
    stk->cap = 0;
}

void forge_error_stack_free(ErrorStack *stk) {
    free(stk->buf);
    stk->buf = NULL;
    stk->len = 0;
    stk->cap = 0;
}

void forge_emit_error(ErrorStack *stk, const char *fmt, ...) {
    if (stk->cap == 0) {
        stk->buf = malloc(ERRSTACK_INIT_CAP);
        if (!stk->buf) return;
        stk->cap = ERRSTACK_INIT_CAP;
    }

    for (;;) {
        size_t  avail = stk->cap - stk->len;
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(stk->buf + stk->len, avail, fmt, ap);
        va_end(ap);

        if (n < 0) return; /* encoding error */

        if ((size_t)n < avail) {
            stk->len += (size_t)n;
            /* append newline separator */
            if (stk->len < stk->cap) {
                stk->buf[stk->len++] = '\n';
            }
            return;
        }

        /* not enough room — double the buffer */
        size_t new_cap = stk->cap * 2 + (size_t)n + 2;
        char  *nb      = realloc(stk->buf, new_cap);
        if (!nb) return;
        stk->buf = nb;
        stk->cap = new_cap;
    }
}

const char *forge_error_stack_get(const ErrorStack *stk) {
    if (!stk->buf || stk->len == 0) return NULL;
    return stk->buf;
}

/* =========================================================================
 * HtmlTemplate
 * ========================================================================= */

/* Read entire file into heap buffer (caller frees). Returns NULL on error. */
static char *read_file(const char *path, size_t *out_len, ErrorStack *stk) {
    FILE *f = fopen(path, "r");
    if (!f) {
        forge_emit_error(stk, "cannot open '%s'", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);

    if (sz < 0) {
        forge_emit_error(stk, "cannot stat '%s'", path);
        fclose(f);
        return NULL;
    }

    char *buf = malloc((size_t)sz + 1);
    if (!buf) {
        forge_emit_error(stk, "out of memory reading '%s'", path);
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)sz, f);
    buf[got]   = '\0';
    *out_len   = got;
    fclose(f);
    return buf;
}

/* Extract the content between "key: <<EOF\n" and "\nEOF" (or "EOF" at end).
 * Returns a newly malloc'd string, or NULL on error. */
static char *extract_heredoc(const char *buf, const char *key, ErrorStack *stk) {
    /* Build search prefix: "key: <<EOF\n" */
    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%s: <<EOF\n", key);
    size_t plen = strlen(prefix);

    const char *start = strstr(buf, prefix);
    if (!start) {
        forge_emit_error(stk, "template: missing '%s: <<EOF' block", key);
        return NULL;
    }
    start += plen; /* first char after the opening line */

    /* Find closing "\nEOF" or "EOF" at start of line */
    const char *end = strstr(start, "\nEOF");
    if (!end) {
        /* EOF right at the start of content (empty block) */
        if (strncmp(start, "EOF", 3) == 0) {
            end = start;
        } else {
            forge_emit_error(stk, "template: missing closing EOF for '%s'", key);
            return NULL;
        }
    } else {
        end += 1; /* skip the '\n' before EOF */
    }

    size_t len    = (size_t)(end - start);
    char  *result = malloc(len + 1);
    if (!result) {
        forge_emit_error(stk, "out of memory");
        return NULL;
    }
    memcpy(result, start, len);
    result[len] = '\0';
    return result;
}

int forge_parse_template(const char *path, HtmlTemplate *tmpl, ErrorStack *stk) {
    tmpl->begin_html = NULL;
    tmpl->end_html   = NULL;

    size_t len;
    char  *buf = read_file(path, &len, stk);
    if (!buf) return 1;

    tmpl->begin_html = extract_heredoc(buf, "begin", stk);
    tmpl->end_html   = extract_heredoc(buf, "end", stk);
    free(buf);

    if (!tmpl->begin_html || !tmpl->end_html) {
        forge_template_free(tmpl);
        return 1;
    }
    return 0;
}

void forge_template_free(HtmlTemplate *tmpl) {
    free(tmpl->begin_html);
    free(tmpl->end_html);
    tmpl->begin_html = NULL;
    tmpl->end_html   = NULL;
}

/* =========================================================================
 * Parser state
 * ========================================================================= */

typedef struct {
    const char **lines;
    size_t       line_count;
    size_t       pos;
    FILE        *out;
    HeadingHooks *hooks;
    int           in_paragraph;
    int           list_depth;
    int           list_ordered[8]; /* 0=ul 1=ol per depth */
    int           in_code_fence;
    char          fence_char;
    int           fence_len;
    ErrorStack   *stk;
} md_parser;

/* =========================================================================
 * Predicates (pure, no I/O)
 * ========================================================================= */

static int is_blank(const char *line) {
    while (*line == ' ' || *line == '\t') line++;
    return *line == '\0';
}

static int is_heading(const char *line, int *level, const char **text) {
    int lv = 0;
    while (line[lv] == '#') lv++;
    if (lv == 0 || lv > 6) return 0;
    if (line[lv] != ' ') return 0;
    *level = lv;
    *text  = line + lv + 1;
    return 1;
}

static int is_fence_open(const char *line, char *fc, int *flen) {
    char c = line[0];
    if (c != '`' && c != '~') return 0;
    int n = 0;
    while (line[n] == c) n++;
    if (n < 3) return 0;
    *fc   = c;
    *flen = n;
    return 1;
}

static int is_fence_close(const char *line, char fc, int flen) {
    int n = 0;
    while (line[n] == fc) n++;
    if (n < flen) return 0;
    /* rest of the line must be blank */
    const char *rest = line + n;
    while (*rest == ' ' || *rest == '\t') rest++;
    return *rest == '\0';
}

static int is_blockquote(const char *line) {
    return line[0] == '>' && (line[1] == ' ' || line[1] == '\0');
}

/* indent is number of leading spaces; text points past them and the marker */
static int is_unordered_item(const char *line, int *indent, const char **text) {
    int i = 0;
    while (line[i] == ' ') i++;
    char c = line[i];
    if (c != '-' && c != '*' && c != '+') return 0;
    if (line[i + 1] != ' ') return 0;
    *indent = i;
    *text   = line + i + 2;
    return 1;
}

static int is_ordered_item(const char *line, int *indent, int *num,
                            const char **text) {
    int i = 0;
    while (line[i] == ' ') i++;
    if (line[i] < '0' || line[i] > '9') return 0;
    int n = 0;
    while (line[i] >= '0' && line[i] <= '9') { n = n * 10 + (line[i] - '0'); i++; }
    if (line[i] != '.' && line[i] != ')') return 0;
    if (line[i + 1] != ' ') return 0;
    *indent = i - (i - (int)(line + i - line - (i - (int)(line + i - line)))); /* leading spaces */
    /* recalculate indent cleanly */
    int lead = 0;
    while (line[lead] == ' ') lead++;
    *indent = lead;
    *num    = n;
    *text   = line + i + 2;
    return 1;
}

static int is_table_row(const char *line) {
    const char *p = line;
    while (*p == ' ') p++;
    return *p == '|';
}

static int is_table_separator(const char *line) {
    const char *p = line;
    while (*p == ' ') p++;
    if (*p == '|') p++;
    /* expect only |, -, :, space */
    while (*p) {
        if (*p != '|' && *p != '-' && *p != ':' && *p != ' ' && *p != '\t')
            return 0;
        p++;
    }
    /* must contain at least one '-' */
    return strchr(line, '-') != NULL;
}

/* =========================================================================
 * Inline renderer
 * ========================================================================= */

static void html_escape_char(char c, FILE *out) {
    switch (c) {
    case '<': fputs("&lt;",  out); break;
    case '>': fputs("&gt;",  out); break;
    case '&': fputs("&amp;", out); break;
    default:  fputc(c, out); break;
    }
}

static int render_inline(const char *src, size_t len, FILE *out,
                          ErrorStack *stk) {
    (void)stk;
    size_t i = 0;
    while (i < len) {
        /* link: [text](url) */
        if (src[i] == '[') {
            const char *text_start = src + i + 1;
            const char *bracket    = memchr(text_start, ']', len - i - 1);
            if (bracket && bracket[1] == '(' ) {
                const char *url_start = bracket + 2;
                size_t      rem       = len - (size_t)(url_start - src);
                const char *paren     = memchr(url_start, ')', rem);
                if (paren) {
                    fputs("<a href=\"", out);
                    for (const char *p = url_start; p < paren; p++)
                        html_escape_char(*p, out);
                    fputs("\">", out);
                    for (const char *p = text_start; p < bracket; p++)
                        html_escape_char(*p, out);
                    fputs("</a>", out);
                    i = (size_t)(paren - src) + 1;
                    continue;
                }
            }
        }
        html_escape_char(src[i], out);
        i++;
    }
    return 0;
}

/* =========================================================================
 * close_paragraph helper
 * ========================================================================= */

static void close_paragraph(md_parser *p) {
    if (p->in_paragraph) {
        fputs("</p>\n", p->out);
        p->in_paragraph = 0;
    }
}

/* =========================================================================
 * Block parsers
 * ========================================================================= */

static int parse_heading(md_parser *p) {
    int         level;
    const char *text;
    if (!is_heading(p->lines[p->pos], &level, &text)) return 0;

    close_paragraph(p);

    if (p->hooks && p->hooks->hooks[level - 1]) {
        int rc = p->hooks->hooks[level - 1](level, text, strlen(text),
                                             p->out, p->stk);
        if (rc) return 1;
    } else {
        fprintf(p->out, "<h%d>", level);
        render_inline(text, strlen(text), p->out, p->stk);
        fprintf(p->out, "</h%d>\n", level);
    }

    p->pos++;
    return 0;
}

static int parse_fenced_code(md_parser *p) {
    char fc;
    int  flen;
    if (!is_fence_open(p->lines[p->pos], &fc, &flen)) return 0;

    close_paragraph(p);

    /* optional language tag after the fence markers */
    const char *lang = p->lines[p->pos] + flen;
    while (*lang == ' ') lang++;

    if (*lang)
        fprintf(p->out, "<pre><code class=\"language-%s\">", lang);
    else
        fputs("<pre><code>", p->out);

    p->pos++;

    while (p->pos < p->line_count) {
        const char *line = p->lines[p->pos];
        if (is_fence_close(line, fc, flen)) {
            p->pos++;
            break;
        }
        /* raw output inside code block — only escape HTML chars */
        for (const char *c = line; *c; c++)
            html_escape_char(*c, p->out);
        fputc('\n', p->out);
        p->pos++;
    }

    fputs("</code></pre>\n", p->out);
    return 0;
}

static int parse_blockquote(md_parser *p) {
    if (!is_blockquote(p->lines[p->pos])) return 0;

    close_paragraph(p);
    fputs("<blockquote>\n", p->out);

    while (p->pos < p->line_count && is_blockquote(p->lines[p->pos])) {
        const char *text = p->lines[p->pos] + 2; /* skip "> " */
        if (p->lines[p->pos][1] == '\0') text = ""; /* bare ">" line */
        fputs("<p>", p->out);
        render_inline(text, strlen(text), p->out, p->stk);
        fputs("</p>\n", p->out);
        p->pos++;
    }

    fputs("</blockquote>\n", p->out);
    return 0;
}

/* Forward declaration for mutual recursion between list and nested list */
static int parse_list_at(md_parser *p, int base_indent, int ordered);

static int parse_list(md_parser *p) {
    int         indent;
    const char *text;
    int         num;

    if (is_unordered_item(p->lines[p->pos], &indent, &text))
        return parse_list_at(p, indent, 0);
    if (is_ordered_item(p->lines[p->pos], &indent, &num, &text))
        return parse_list_at(p, indent, 1);
    return 0;
}

static int parse_list_at(md_parser *p, int base_indent, int ordered) {
    close_paragraph(p);

    const char *tag = ordered ? "ol" : "ul";
    fprintf(p->out, "<%s>\n", tag);

    while (p->pos < p->line_count) {
        const char *line = p->lines[p->pos];
        if (is_blank(line)) { p->pos++; break; }

        int         indent;
        const char *text;
        int         num;
        int         this_ordered;

        if (is_unordered_item(line, &indent, &text))      { this_ordered = 0; }
        else if (is_ordered_item(line, &indent, &num, &text)) { this_ordered = 1; }
        else break;

        if (indent < base_indent) break;           /* dedented — caller handles */
        if (indent > base_indent) {
            /* deeper indent: open nested list inside previous <li> */
            parse_list_at(p, indent, this_ordered);
            fputs("</li>\n", p->out);
            continue;
        }

        /* same indent */
        fputs("<li>", p->out);
        render_inline(text, strlen(text), p->out, p->stk);
        p->pos++;

        /* peek: if next line is deeper, nest immediately */
        if (p->pos < p->line_count) {
            const char *next = p->lines[p->pos];
            int ni; const char *nt; int nn;
            if (is_unordered_item(next, &ni, &nt) && ni > base_indent) {
                fputc('\n', p->out);
                parse_list_at(p, ni, 0);
                fputs("</li>\n", p->out);
            } else if (is_ordered_item(next, &ni, &nn, &nt) && ni > base_indent) {
                fputc('\n', p->out);
                parse_list_at(p, ni, 1);
                fputs("</li>\n", p->out);
            } else {
                fputs("</li>\n", p->out);
                continue;
            }
        } else {
            fputs("</li>\n", p->out);
        }
    }

    fprintf(p->out, "</%s>\n", tag);
    return 0;
}

/* Split a table row into cells. cells[] receives pointers into a scratch
 * buffer (which must be large enough). Returns cell count. */
static int split_table_row(const char *line, char *scratch, size_t scratch_sz,
                            const char **cells, int max_cells) {
    /* copy line into scratch so we can mutate it */
    strncpy(scratch, line, scratch_sz - 1);
    scratch[scratch_sz - 1] = '\0';

    char *p = scratch;
    /* trim leading/trailing spaces */
    while (*p == ' ') p++;
    if (*p == '|') p++; /* skip leading pipe */

    int count = 0;
    while (*p && count < max_cells) {
        /* find next '|' */
        char *end = strchr(p, '|');
        if (end) *end = '\0';

        /* trim whitespace around cell */
        while (*p == ' ' || *p == '\t') p++;
        char *tail = p + strlen(p);
        while (tail > p && (tail[-1] == ' ' || tail[-1] == '\t')) tail--;
        *tail = '\0';

        cells[count++] = p;
        if (!end) break;
        p = end + 1;
    }
    return count;
}

static int parse_table(md_parser *p) {
    if (!is_table_row(p->lines[p->pos])) return 0;
    /* need at least a header row + separator */
    if (p->pos + 1 >= p->line_count) return 0;
    if (!is_table_separator(p->lines[p->pos + 1])) return 0;

    close_paragraph(p);

    char        scratch[4096];
    const char *cells[64];

    fputs("<table>\n<thead>\n<tr>", p->out);
    int ncols = split_table_row(p->lines[p->pos], scratch, sizeof(scratch),
                                cells, 64);
    for (int i = 0; i < ncols; i++) {
        fputs("<th>", p->out);
        render_inline(cells[i], strlen(cells[i]), p->out, p->stk);
        fputs("</th>", p->out);
    }
    fputs("</tr>\n</thead>\n", p->out);
    p->pos += 2; /* skip header + separator */

    fputs("<tbody>\n", p->out);
    while (p->pos < p->line_count && is_table_row(p->lines[p->pos])) {
        int n = split_table_row(p->lines[p->pos], scratch, sizeof(scratch),
                                cells, 64);
        fputs("<tr>", p->out);
        for (int i = 0; i < n; i++) {
            fputs("<td>", p->out);
            render_inline(cells[i], strlen(cells[i]), p->out, p->stk);
            fputs("</td>", p->out);
        }
        fputs("</tr>\n", p->out);
        p->pos++;
    }
    fputs("</tbody>\n</table>\n", p->out);
    return 0;
}

static int parse_paragraph(md_parser *p) {
    const char *line = p->lines[p->pos];
    if (is_blank(line)) {
        close_paragraph(p);
        p->pos++;
        return 0;
    }

    if (!p->in_paragraph) {
        fputs("<p>", p->out);
        p->in_paragraph = 1;
    } else {
        fputc('\n', p->out); /* soft line break within paragraph */
    }

    render_inline(line, strlen(line), p->out, p->stk);
    p->pos++;
    return 0;
}

/* =========================================================================
 * forge_convert
 * ========================================================================= */

int forge_convert(const char *input_path, FILE *out,
                  const HtmlTemplate *tmpl, HeadingHooks *hooks,
                  ErrorStack *stk) {
    /* load file */
    size_t flen;
    char  *fbuf = read_file(input_path, &flen, stk);
    if (!fbuf) return 1;

    /* replace \n with \0 and build line pointer array */
    size_t line_count = 0;
    for (size_t i = 0; i < flen; i++)
        if (fbuf[i] == '\n') line_count++;
    /* last line may have no trailing newline */
    if (flen > 0 && fbuf[flen - 1] != '\n') line_count++;

    const char **lines = malloc((line_count + 1) * sizeof(char *));
    if (!lines) {
        forge_emit_error(stk, "out of memory");
        free(fbuf);
        return 1;
    }

    size_t idx = 0;
    lines[idx++] = fbuf;
    for (size_t i = 0; i < flen; i++) {
        if (fbuf[i] == '\n') {
            fbuf[i] = '\0';
            if (i + 1 < flen) lines[idx++] = fbuf + i + 1;
        }
    }
    line_count = idx;

    /* emit template begin */
    if (tmpl && tmpl->begin_html)
        fputs(tmpl->begin_html, out);

    /* parse */
    md_parser p = {0};
    p.lines      = lines;
    p.line_count = line_count;
    p.pos        = 0;
    p.out        = out;
    p.hooks      = hooks;
    p.stk        = stk;

    while (p.pos < p.line_count) {
        const char *line = p.lines[p.pos];

        if (parse_heading(&p))      goto done;
        if (p.pos >= p.line_count)  break;
        line = p.lines[p.pos];

        if (is_fence_open(line, &(char){0}, &(int){0})) {
            if (parse_fenced_code(&p)) goto done;
        } else if (is_blockquote(line)) {
            if (parse_blockquote(&p)) goto done;
        } else {
            int ind; const char *txt; int num;
            if (is_unordered_item(line, &ind, &txt) ||
                is_ordered_item(line, &ind, &num, &txt)) {
                if (parse_list(&p)) goto done;
            } else if (is_table_row(line) &&
                       p.pos + 1 < p.line_count &&
                       is_table_separator(p.lines[p.pos + 1])) {
                if (parse_table(&p)) goto done;
            } else {
                if (parse_paragraph(&p)) goto done;
            }
        }
    }

done:
    close_paragraph(&p);

    /* emit template end */
    if (tmpl && tmpl->end_html)
        fputs(tmpl->end_html, out);

    free(lines);
    free(fbuf);
    return forge_error_stack_get(stk) ? 1 : 0;
}
