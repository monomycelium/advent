#ifndef CALLER_H
#define CALLER_H

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

/**
 * Level of solution.
 */
typedef enum part {
    PART_ONE = 1,
    PART_TWO = 2,
    PART_MAX = 3,
} part_t;

/**
 * Check method.
 */
typedef enum check {
    LEAVE, /**< none */
    CHECK, /**< check results using advent.fly.dev */
    UPLOD, /**< upload results to adventofcode.com */
} check_t;

/**
 * Application configuration.
 */
typedef struct app {
    char *input;                           /**< path to input file */
    char *cooky;                           /**< path to cookie file */
    check_t check;                         /**< check method */
    uint8_t parts : (PART_MAX - PART_ONE); /**< bitfield of parts to execute */
    char *objct;                           /**< path to shared object */
} app_t;

/**
 * Required data for each day.
 */
typedef struct day {
    app_t app;
    uint16_t year;
    uint8_t day;
    buf_t input;
    void *handle;
} day_t;

bool parseargs(int argc, char **argv, app_t *app);
char *symbol_name(part_t part);
void usage(int code, char *arg0);
void solve(const day_t *day, part_t part);

#endif  // CALLER_H
