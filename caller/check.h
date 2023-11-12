#ifndef CHECK_H
#define CHECK_H

#include <stdint.h>

#include "caller.h"
#include "common.h"

#define URL_FORMAT "https://adventofcode.com/%u/day/%u/answer"
#define URL_LENGTH 44 /**< maximum URL length */

/**
 * Outcome of solution.
 */
typedef enum {
    CORRECT,
    WRONG,
    TOO_LOW,
    TOO_HIGH,
    WAIT,
    LEVEL, /**< wrong level */
    INVALID,
} outcome_t;

/**
 * Unicode symbols to represent the outcome.
 * For example: outcome_sym[outcome_t outcome].
 */
extern const char *const outcome_sym[];

/**
 * Upload answer to adventofcode.com and return result.
 *
 * @param day       day data for solution.
 * @param part    solution part.
 * @param answer    answer to solution.
 */
outcome_t upload(const day_t *day, part_t part, buf_t answer);

/**
 * Check answer with advent.fly.dev and return result.
 *
 * @param day      day data for solution.
 * @param part      solution part.
 * @param answer    answer to solution.
 */
outcome_t check(const day_t *day, part_t part, buf_t answer);

#endif  // CHECK_H
