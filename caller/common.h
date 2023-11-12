#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/**
 * Heap-allocated buffer.
 */
typedef struct buf {
    size_t len; /**< Length of data. */
    char *ptr;  /**< Pointer to null-terminated data. */
} buf_t;

/**
 * Solver function type.
 */
typedef buf_t (*solve_func)(buf_t);

#endif
