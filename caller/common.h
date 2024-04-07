#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <sys/types.h>

/**
 * Heap-allocated buffer.
 */
typedef struct buf {
    ssize_t len;  /**< Length of data. */
    uint8_t *ptr; /**< Pointer to null-terminated data. */
} buf_t;

/**
 * Solver function type.
 */
typedef buf_t (*solve_func)(buf_t);

#endif
