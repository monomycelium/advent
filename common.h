#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdio.h>

typedef struct buf {
    size_t len;
    char *ptr;
} buf_t;

typedef enum part {
    PART_ONE = 1,
    PART_TWO = 2,
    PART_MAX = 3,
} part_t;

typedef buf_t (*solve_func)(buf_t);

char *symbol_name(part_t part);
buf_t read_input(FILE *restrict stream);

#endif
