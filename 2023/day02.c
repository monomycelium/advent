#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "common.h"

const uint8_t day = 02;
const uint16_t year = 2023;

typedef enum {
    COLOUR_RED,
    COLOUR_GREEN,
    COLOUR_BLUE,
    COLOUR_MAX,
} Colour;

typedef struct {
    uintmax_t id; /** game ID */
    uintmax_t sets[COLOUR_MAX];
} Subset;

typedef struct {
    uintmax_t sum;
    Subset game;               /** using Subset to represent a game */
    uintmax_t max[COLOUR_MAX]; /** (for part one) maximum number of cubes for
                                  each colour */
} Data;

static const char *const colours[] = {"red", "green", "blue"};
static const Subset first = {.id = 0x00, .sets = {0x00, 0x00, 0x00}};
static const Subset lasts = {.id = UINTMAX_MAX,
                             .sets = {UINTMAX_MAX, UINTMAX_MAX, UINTMAX_MAX}};

void solver(buf_t input, void (*func)(Subset, void *), void *data);

void cb1(Subset set, void *data);
void cb2(Subset set, void *data);

void cb1(Subset set, void *info) {
    Data *data = (Data *)info;

    // new game
    if (memcmp(set.sets, first.sets, sizeof set.sets) == 0) {
        memcpy(&(data->game), &set, sizeof set);
        return;
    }

    if (set.id != data->game.id) return;

    // end of game
    if (memcmp(set.sets, lasts.sets, sizeof set.sets) == 0) {
        bool no = false;

        // printf("%ju: %ju, %ju, %ju\n", data->game.id, data->game.sets[0],
        // data->game.sets[1], data->game.sets[2]);

        for (Colour i = 0; i < COLOUR_MAX; i++)
            no |= (data->game.sets[i] > data->max[i]);

        // if (no) printf("%ju\n", data->game.id);
        data->sum += no ? 0 : data->game.id;
        return;
    }

    for (Colour i = 0; i < COLOUR_MAX; i++)
        data->game.sets[i] = MAX(data->game.sets[i], set.sets[i]);

    // printf("%ju: %ju, %ju, %ju\n", set.id, set.sets[0], set.sets[1],
    // set.sets[2]);
}

void cb2(Subset set, void *info) {
    Data *data = (Data *)info;

    // new game
    if (memcmp(set.sets, first.sets, sizeof set.sets) == 0) {
        memcpy(&(data->game), &set, sizeof set);
        return;
    }

    if (set.id != data->game.id) return;

    // end of game
    if (memcmp(set.sets, lasts.sets, sizeof set.sets) == 0) {
        uintmax_t power = 1;

        for (Colour i = 0; i < COLOUR_MAX; i++) power *= data->game.sets[i];

        data->sum += power;
        return;
    }

    for (Colour i = 0; i < COLOUR_MAX; i++)
        data->game.sets[i] = MAX(data->game.sets[i], set.sets[i]);
}

void solver(buf_t input, void (*func)(Subset, void *), void *data) {
    char *ptr = (char *)input.ptr;

    while (*ptr != '\0') {
        char *end;
        Subset set = first;

        assert(memcmp(ptr, "Game ", 5) == 0);
        ptr += 5;

        set.id = strtoumax((char *)ptr, &end, 10);
        assert(end != ptr);
        assert(*end == ':');
        ptr = end;

        func(first, data);

        while (*ptr != '\n') {
            uintmax_t val;
            Colour col;

            // printf("%zu\n", (size_t)ptr - (size_t)input.ptr);

            if (*ptr == ':' || *ptr == ';') {
                func(set, data);
                memset(set.sets, 0, sizeof set.sets);
            } else
                assert(*ptr == ',');

            assert(*(ptr + 1) == ' ');
            ptr += 2;

            val = strtoumax((char *)ptr, &end, 10);
            assert(end != ptr);
            assert(*end == ' ');
            ptr = end + 1;

            // TODO: handle colour.
            for (col = 0; col < COLOUR_MAX &&
                          memcmp(colours[col], ptr, strlen(colours[col])) != 0;
                 col++)
                ;

            assert(col != COLOUR_MAX);
            set.sets[col] += val;
            ptr += strlen(colours[col]);
        }

        func(set, data);
        memcpy(set.sets, lasts.sets, sizeof set.sets);
        func(set, data);
        ptr++;
    }

    func(lasts, data);
}

buf_t solve1(buf_t input) {
    Data data = {
        .game = first,
        .sum = 0,
        .max = {[COLOUR_RED] = 12, [COLOUR_GREEN] = 13, [COLOUR_BLUE] = 14},
    };

    solver(input, cb1, &data);
    return (buf_t){.len = 0, .ptr = (uint8_t *)data.sum};
}

buf_t solve2(buf_t input) {
    Data data = {0};
    solver(input, cb2, &data);
    return (buf_t){.len = 0, .ptr = (uint8_t *)data.sum};
}
