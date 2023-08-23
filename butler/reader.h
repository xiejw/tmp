#ifndef READER_H_
#define READER_H_

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// eva typedefs
typedef int error_t;

//
// system level constants
//
#define EVA_MAX_PATH_LEN     100   // maximum path string length.
#define EVA_MAX_STR_LINE_LEN 2048  // maximum string line length.

#define OK           0
#define EUNSPECIFIED -1  // An unspecified general error.
#define ENOTPATH     -2  // Not a valid path.
#define EOPENFILE    -5  // Failure during opening file.
#define EREADFILE    -6  // Failure during reading file.
#define ELINELEN     -7  // Line is too long.
#define EEOF         -8  // EOF.

//
// prototypes
//

// opaque handle for file line reading.
struct eva_reader_t;

error_t evaReaderOpen(struct eva_reader_t **handle, char *path);
error_t evaReaderDopen(struct eva_reader_t **handle, int fd);
void    evaReaderClose(struct eva_reader_t *handle);

// reads one more line with newline '\n' stripped.
//
// assumption: dst must be able to hold EVA_MAX_STR_LINE_LEN chars
//
// returns:
// -  the size of the chars read, upon succeed, which could be 0
// -  EEOF for EOF
// -  (otherwise) <0 for error
//
// result will be stored to `dst`, which is owned by call site.
int evaReaderNextLine(struct eva_reader_t *handle, char *dst);

#endif  // READER_H_
