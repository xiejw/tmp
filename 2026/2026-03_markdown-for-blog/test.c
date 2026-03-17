#include "md2html.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
 * Minimal test harness
 * ------------------------------------------------------------------------- */

static int g_passed = 0;
static int g_failed = 0;

static void check(const char *name, const char *got, const char *want) {
    if (strcmp(got, want) == 0) {
        printf("  PASS  %s\n", name);
        g_passed++;
    } else {
        printf("  FAIL  %s\n", name);
        printf("        want: %s\n", want);
        printf("        got:  %s\n", got);
        g_failed++;
    }
}

/* Convert a markdown string to HTML string (heap-allocated, caller frees).
 * Returns NULL on fatal error. */
static char *md_to_str(const char *md) {
    /* write md to a temp file */
    char in_path[]  = "/tmp/md2html_test_in_XXXXXX";
    char out_path[] = "/tmp/md2html_test_out_XXXXXX";

    int in_fd = mkstemp(in_path);
    if (in_fd < 0) return NULL;
    write(in_fd, md, strlen(md));
    close(in_fd);

    int out_fd = mkstemp(out_path);
    if (out_fd < 0) { unlink(in_path); return NULL; }
    close(out_fd);

    ErrorStack stk = {0};
    forge_error_stack_init(&stk);

    FILE *out = fopen(out_path, "w");
    int   rc  = forge_convert(in_path, out, NULL, NULL, &stk);
    fclose(out);

    unlink(in_path);

    char  *result = NULL;
    if (rc == 0) {
        FILE *f = fopen(out_path, "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            rewind(f);
            result = malloc((size_t)sz + 1);
            if (result) {
                size_t got = fread(result, 1, (size_t)sz, f);
                result[got] = '\0';
            }
            fclose(f);
        }
    }

    unlink(out_path);
    forge_error_stack_free(&stk);
    return result;
}

#define RUN(name, md, want) do { \
    char *got = md_to_str(md);   \
    check(name, got ? got : "", want); \
    free(got); \
} while (0)

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_heading(void) {
    RUN("h1", "# Hello\n",       "<h1>Hello</h1>\n");
    RUN("h2", "## World\n",      "<h2>World</h2>\n");
    RUN("h3", "### Deep\n",      "<h3>Deep</h3>\n");
}

static void test_paragraph(void) {
    RUN("paragraph",
        "Hello world\n",
        "<p>Hello world</p>\n");

    RUN("paragraph blank sep",
        "Line one\n\nLine two\n",
        "<p>Line one</p>\n<p>Line two</p>\n");
}

static void test_inline_link(void) {
    RUN("link",
        "[click](https://example.com)\n",
        "<p><a href=\"https://example.com\">click</a></p>\n");
}

static void test_html_escape(void) {
    RUN("escape",
        "a < b & c > d\n",
        "<p>a &lt; b &amp; c &gt; d</p>\n");
}

static void test_fenced_code(void) {
    RUN("fenced code",
        "```\nfoo bar\n```\n",
        "<pre><code>foo bar\n</code></pre>\n");

    RUN("fenced code with lang",
        "```c\nint x;\n```\n",
        "<pre><code class=\"language-c\">int x;\n</code></pre>\n");
}

static void test_blockquote(void) {
    RUN("blockquote",
        "> hello\n",
        "<blockquote>\n<p>hello</p>\n</blockquote>\n");
}

static void test_unordered_list(void) {
    RUN("ul basic",
        "- a\n- b\n- c\n",
        "<ul>\n<li>a</li>\n<li>b</li>\n<li>c</li>\n</ul>\n");
}

static void test_ordered_list(void) {
    RUN("ol basic",
        "1. one\n2. two\n3. three\n",
        "<ol>\n<li>one</li>\n<li>two</li>\n<li>three</li>\n</ol>\n");
}

static void test_nested_list(void) {
    RUN("nested ul",
        "- parent\n  - child\n",
        "<ul>\n<li>parent\n<ul>\n<li>child</li>\n</ul>\n</li>\n</ul>\n");
}

static void test_table(void) {
    RUN("table",
        "| A | B |\n|---|---|\n| 1 | 2 |\n",
        "<table>\n<thead>\n<tr><th>A</th><th>B</th></tr>\n</thead>\n"
        "<tbody>\n<tr><td>1</td><td>2</td></tr>\n</tbody>\n</table>\n");
}

static void test_template(void) {
    /* write a tiny template to a temp file */
    char tmpl_path[] = "/tmp/md2html_test_tmpl_XXXXXX";
    int  fd          = mkstemp(tmpl_path);
    if (fd < 0) { printf("  FAIL  template (mkstemp)\n"); g_failed++; return; }

    const char *tmpl_content =
        "begin: <<EOF\n<html><body>\nEOF\n\nend: <<EOF\n</body></html>\nEOF\n";
    write(fd, tmpl_content, strlen(tmpl_content));
    close(fd);

    /* write md input */
    char in_path[] = "/tmp/md2html_test_in2_XXXXXX";
    int  in_fd     = mkstemp(in_path);
    const char *md = "# Hi\n";
    write(in_fd, md, strlen(md));
    close(in_fd);

    ErrorStack   stk  = {0};
    HtmlTemplate tmpl = {0};
    forge_error_stack_init(&stk);
    forge_parse_template(tmpl_path, &tmpl, &stk);

    char out_path[] = "/tmp/md2html_test_out2_XXXXXX";
    int  out_fd     = mkstemp(out_path);
    close(out_fd);
    FILE *out = fopen(out_path, "w");
    forge_convert(in_path, out, &tmpl, NULL, &stk);
    fclose(out);

    /* read result */
    FILE *f = fopen(out_path, "r");
    char  buf[512] = {0};
    if (f) { fread(buf, 1, sizeof(buf) - 1, f); fclose(f); }

    unlink(tmpl_path);
    unlink(in_path);
    unlink(out_path);
    forge_template_free(&tmpl);
    forge_error_stack_free(&stk);

    check("template wrapping",
          buf,
          "<html><body>\n<h1>Hi</h1>\n</body></html>\n");
}

static void test_error_stack(void) {
    ErrorStack stk = {0};
    forge_error_stack_init(&stk);

    if (forge_error_stack_get(&stk) != NULL) {
        printf("  FAIL  error_stack empty\n"); g_failed++;
    } else {
        printf("  PASS  error_stack empty\n"); g_passed++;
    }

    forge_emit_error(&stk, "oops %d", 42);
    const char *msg = forge_error_stack_get(&stk);
    if (msg && strstr(msg, "oops 42")) {
        printf("  PASS  error_stack emit\n"); g_passed++;
    } else {
        printf("  FAIL  error_stack emit (got: %s)\n", msg ? msg : "(null)");
        g_failed++;
    }

    forge_error_stack_free(&stk);
}

/* -------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */

int main(void) {
    printf("=== md2html tests ===\n");

    test_error_stack();
    test_heading();
    test_paragraph();
    test_inline_link();
    test_html_escape();
    test_fenced_code();
    test_blockquote();
    test_unordered_list();
    test_ordered_list();
    test_nested_list();
    test_table();
    test_template();

    printf("=====================\n");
    printf("%d passed, %d failed\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
