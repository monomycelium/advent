/// `caller`: a CLI tool to execute Advent of Code solutions.
/// TODO: integrate automation to submit answers and fetch inputs.

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"

char *symbol_name(part_t part) {
    switch (part) {
        case PART_ONE:
            return "solve1";
        case PART_TWO:
            return "solve2";
        default:
            return NULL;
    }
}

buf_t read_input(FILE *restrict stream) {
    ssize_t result;
    size_t bufsiz;
    buf_t buffer;
    char *ptr;

    ptr = NULL;
    result = getdelim(&ptr, &bufsiz, 0, stream);
    if (result == -1) {
        free(ptr);
        return (buf_t){0};
    }

    buffer.len = (size_t)result;
    buffer.ptr = realloc(ptr, buffer.len + 1);
    if (buffer.ptr == NULL) free(ptr);
    return buffer;
}

int main(int argc, char **argv) {
    void *handle;
    char *filename;
    buf_t buffer;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <SHARED_OBJ>\n", argv[0]);
        return EXIT_FAILURE;
    }

    buffer.ptr = NULL;
    filename = argv[1];

    handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL) goto set;

    buffer = read_input(stdin);
    if (buffer.ptr == NULL) {
        fprintf(stderr, "failed to read input: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    for (part_t part = PART_ONE; part < PART_MAX; part++) {
        solve_func func;
        void *symbol;
        buf_t result;
        char *errorstr;

        symbol = dlsym(handle, symbol_name(part));
        if ((errorstr = dlerror()) != NULL) {
            fprintf(stderr, "Part %u failed: %s\n", part, errorstr);
            continue;
        }

        func = (solve_func)symbol;
        result = func(buffer);
        printf("Part %u: %s\n", (uint8_t)part, result.ptr);
        if (result.ptr != NULL) free(result.ptr);
    }

    free(buffer.ptr);
    if (dlclose(handle) != 0) goto set;
    return EXIT_SUCCESS;

set:
    fprintf(stderr, "%s\n", dlerror());
    if (buffer.ptr) free(buffer.ptr);
    return EXIT_FAILURE;
}
