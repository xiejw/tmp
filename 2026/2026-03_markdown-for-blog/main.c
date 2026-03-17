#include "md2html.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s -i input.md [-o output.html] [-t template.tmpl]\n",
            prog);
}

int main(int argc, char **argv) {
    const char *input_path    = NULL;
    const char *output_path   = NULL;
    const char *template_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_path = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            template_path = argv[++i];
        } else {
            fprintf(stderr, "unknown argument: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }

    if (!input_path) {
        fputs("error: -i input.md is required\n", stderr);
        usage(argv[0]);
        return 1;
    }

    ErrorStack stk = {0};
    forge_error_stack_init(&stk);

    HtmlTemplate tmpl     = {0};
    int          has_tmpl = 0;

    if (template_path) {
        if (forge_parse_template(template_path, &tmpl, &stk)) {
            fprintf(stderr, "template error:\n%s\n", forge_error_stack_get(&stk));
            forge_error_stack_free(&stk);
            return 1;
        }
        has_tmpl = 1;
    }

    FILE *out = stdout;
    if (output_path) {
        out = fopen(output_path, "w");
        if (!out) {
            fprintf(stderr, "error: cannot open output '%s'\n", output_path);
            if (has_tmpl) forge_template_free(&tmpl);
            forge_error_stack_free(&stk);
            return 1;
        }
    }

    int rc = forge_convert(input_path, out, has_tmpl ? &tmpl : NULL, NULL, &stk);

    if (output_path) fclose(out);
    if (has_tmpl) forge_template_free(&tmpl);

    const char *errors = forge_error_stack_get(&stk);
    if (errors) {
        fprintf(stderr, "errors during conversion:\n%s\n", errors);
    }

    forge_error_stack_free(&stk);
    return rc;
}
