#include <stdio.h>

#include "reader.h"

#define CHECK_OK(e)                                          \
        do {                                                 \
                if (e != OK) {                               \
                        printf("unexpected error: %d\n", e); \
                        return -1;                           \
                }                                            \
        } while (0)

int
main(void)
{
        struct eva_reader_t *h = NULL;
        CHECK_OK(evaReaderOpen(&h, "makefile"));
        char buf[EVA_MAX_STR_LINE_LEN];

        // loop until eof or error.
        int line_c = 0;
        int size;
        while (1) {
                size = evaReaderNextLine(h, buf);
                if (size == EEOF) {
                        printf("(eof. quit. total %d lines)\n", line_c);
                        return 0;
                }

                if (size < 0) {
                        printf("failed to read line: %d\n", size);
                        return -1;
                }

                line_c++;
                printf("len %3d:%s\n", size, buf);
        }
        evaReaderClose(h);
}
