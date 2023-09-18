#include "reader.h"

//
// internal data structure
//

struct eva_reader_t {
        //
        // internal fields
        //
        int            fd;
        unsigned char *buf;
        char           flag_eof;
        ssize_t        buf_pos;
        ssize_t        buf_cap;
};

//
// implementation
//

#define MAX_BUF_SIZE (4096 * 16)

//
// private prototypes
//

// One side effect. reader->cap is set to read returning size. So if it is end
// of file, cap is 0. To improve readability, use ldNextBufEof to check it.
static error_t    ldNextBuf(struct eva_reader_t *handle);
static inline int ldNextBufEof(struct eva_reader_t *handle);

error_t
evaReaderOpen(struct eva_reader_t **h, char *path)
{
        int fd = open(path, O_RDONLY);
        if (fd == -1) return EOPENFILE;
        return evaReaderDopen(h, fd);
}

error_t
evaReaderDopen(struct eva_reader_t **h, int fd)
{
        // allocate resources.
        *h        = malloc(sizeof(struct eva_reader_t));
        (*h)->buf = malloc(MAX_BUF_SIZE * sizeof(unsigned char));

        (*h)->fd       = fd;
        (*h)->flag_eof = 0;
        (*h)->buf_pos  = 0;
        (*h)->buf_cap  = 0;
        return OK;
}

void
evaReaderClose(struct eva_reader_t *h)
{
        if (h == NULL) return;
        close(h->fd);
        free(h->buf);
        free(h);
}

int
evaReaderNextLine(struct eva_reader_t *handle, char *dst)
{
        //
        // special case, returns immediately if EOF.
        if (handle->flag_eof) return EEOF;

        // allocates the buffer for current line.
        //
        // It is 1 larger than the EVA_MAX_STR_LINE_LEN.  Also note this is not
        // power of 2. bad for performance.
        unsigned char line[EVA_MAX_STR_LINE_LEN + 1];
        int           current_len = 0;
        int           i;

        // The loop ends in any of the following conditions.
        // 1. EOF.
        // 2. find an EOL.
        // 3. reach maximul line length limit, i.e., error.
        for (;;) {
                // If the current buf is fully used, read next buffer.
                if (handle->buf_pos >= handle->buf_cap) {
                        error_t err = ldNextBuf(handle);
                        if (OK != err) return err;
                }

                if (ldNextBufEof(handle)) {
                        handle->flag_eof = 1;
                        if (current_len == 0) {
                                // Nothing left after the last EOL. This is a
                                // quite tricky case as lots of editors (vim)
                                // and cmds (echo) automatically insert newline
                                // for text file. To debug in hex, use xxd
                                // <filename>.
                                dst[current_len] = '\0';
                                return 0;
                        } else {
                                line[current_len] = '\0';
                                strcpy(dst, (const char *)line);
                                return current_len;
                        }
                }

                // Tries to find end-of-line, i.e., `\n`.
                for (i = handle->buf_pos; i < handle->buf_cap; i++) {
                        if (handle->buf[i] != '\n') {
                                continue;
                        }

                        int length = i - handle->buf_pos;

                        if (current_len + length >= EVA_MAX_STR_LINE_LEN)
                                return ELINELEN;

                        // Move the data from line to handle buffer.
                        memcpy(line + current_len,
                               handle->buf + handle->buf_pos, length);
                        current_len += length;
                        handle->buf_pos = i + 1;

                        line[current_len] = '\0';
                        strcpy(dst, (const char *)line);
                        return current_len;
                }

                // We reach the end of the buffer.
                int length = handle->buf_cap - handle->buf_pos;
                if (current_len + length >= EVA_MAX_STR_LINE_LEN)
                        return ELINELEN;

                memcpy(line + current_len, handle->buf + handle->buf_pos,
                       length);
                current_len += length;
                handle->buf_pos = handle->buf_cap;

                // ready for next iteration.
        }
}

//
// impl of private methods
//
static error_t
ldNextBuf(struct eva_reader_t *handle)
{
        handle->buf_pos = 0L;
        handle->buf_cap = read(handle->fd, handle->buf, MAX_BUF_SIZE);

        if (handle->buf_cap == -1) return EREADFILE;

        return OK;
}

int
ldNextBufEof(struct eva_reader_t *handle)
{
        return handle->buf_cap == 0;
}
