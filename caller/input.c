#include "input.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

buf_t read_input(FILE *restrict stream) {
    buf_t buffer;
    uint8_t *ptr = NULL;
    size_t bufsiz = 0;

    buffer.ptr = NULL;
    buffer.len = 0;

    // use fread(3) if stream is seekable
    if (fseek(stream, 0L, SEEK_END) == 0) {
        long pos;
        size_t len;

        pos = ftell(stream);
        if (pos == -1) return buffer;

        bufsiz = pos + 1;
        ptr = malloc(bufsiz);
        if (ptr == NULL) return buffer;

        if (fseek(stream, 0L, SEEK_SET) == -1) return buffer;

        len = fread(ptr, 1, pos, stream);
        if (len != (size_t)pos && feof(stream) == 0) return buffer;

        buffer.ptr = ptr;
        buffer.len = len;
        return buffer;
    } else {  // or else, use getdelim(3)
        ssize_t result;

        if (errno != ESPIPE) return buffer;  // check fseek(3) error

        result = getdelim((char **)&ptr, &bufsiz, 0, stream);
        if (result == -1) {
            free(ptr);
            return buffer;
        }

        buffer.len = (size_t)result;
        buffer.ptr = realloc(ptr, buffer.len + 1);
        if (buffer.ptr == NULL) free(ptr);
        return buffer;
    }
}
