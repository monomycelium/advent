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
 * Application configuration.
 */
typedef struct app {
    char *input;    /**< path to input file */
    char *cooky;    /**< path to cookie file */
    bool check : 1; /**< check results using advent.fly.dev */
    bool uplod : 1; /**< upload results to adventofcode.com */
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

#endif  // CALLER_H
