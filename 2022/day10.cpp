#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bitset>
#include <unordered_map>

#include "common.h"

#define COLS 40 /**< columns in device screen */
#define ROWS 6  /**< rows in device screen */
#define WIDTH 3 /**< sprite width */

#define CW 5 /**< character width */
#define CH 6 /**< character height */

extern "C" const uint16_t year = 2022;
extern "C" const uint8_t day = 10;

typedef std::bitset<CH * CW> Block;
typedef std::unordered_map<Block, uint8_t> BlockMap;

typedef enum {
    NoOp,
    AddX,
    Invalid,
} ExeKind;

typedef struct {
    ExeKind kind;
    intmax_t arg;
} Exe;

typedef struct {
    uintmax_t cycles; /**< cycle count */
    intmax_t X;       /**< register of CPU */
    void *data;       /**< data for callback */
} State;

/**
 * Parse an instruction.
 */
static Exe parse_exe(buf_t line);

/**
 * Execute an instruction.
 *
 * @param self      machine state.
 * @param exe       instruction to execute.
 * @param inc_fn    callback to increment cycle.
 */
static void exec_exe(State *self, Exe exe, void (*inc_fn)(State *, uintmax_t));

/**
 * Split buffer using a single delimeter.
 *
 * @param input:    Buffer to split.
 * @param delim:    Delimiting character to split using.
 * @param ptr:      Internal pointer (should initially point to input.ptr).
 */
static buf_t strspl(buf_t input, uint8_t delim, uint8_t **ptr);

/**
 * Print number to a heap-allocated buffer in Base-10.
 */
static buf_t strfrommax(uintmax_t max);

/**
 * Cycle increment function for Part One.
 */
static void inc1(State *self, uintmax_t amt);

/**
 * Cycle increment function for Part Two.
 */
static void inc2(State *self, uintmax_t amt);

/**
 * Wrapper to solve each part.
 */
static void solve(State *self, buf_t input, void (*inc_fn)(State *, uintmax_t));

/**
 * Parse nth character of alphabet from rendered art.
 */
static uint8_t parse_char(std::bitset<COLS * ROWS> set, size_t n);

extern "C" buf_t solve1(buf_t input) {
    State state;
    uintmax_t sum;

    sum = 0;
    state.cycles = 0;
    state.X = 1;
    state.data = &sum;

    solve(&state, input, inc1);
    return strfrommax(sum);
}

extern "C" buf_t solve2(buf_t input) {
    State state;
    std::bitset<ROWS * COLS> bit;
    buf_t ret;

    state.cycles = 0;
    state.X = 1;
    state.data = &bit;

    solve(&state, input, inc2);

    ret.len = bit.size() / (CW * CH);
    ret.ptr = (uint8_t *)malloc(ret.len);
    if (ret.ptr == NULL) return ret;
    for (uint8_t i = 0; i < ret.len; i++) ret.ptr[i] = parse_char(bit, i);

    return ret;
}

static void inc1(State *self, uintmax_t amt) {
    for (uintmax_t i = 0; i < amt; i++) {
        self->cycles += 1;

        if (self->cycles % 40 == 20)
            *(uintmax_t *)self->data += self->cycles * self->X;
    }
}

static void inc2(State *self, uintmax_t amt) {
    std::bitset<ROWS * COLS> *data;
    data = (std::bitset<ROWS * COLS> *)self->data;

    for (uintmax_t i = 0; i < amt; i++) {
        uintmax_t pos;

        pos = self->cycles;
        self->cycles += 1;

        data->set(pos, llabs((long long)(pos % COLS) - (long long)self->X) <=
                           (WIDTH / 2));
    }
}

static void solve(State *self, buf_t input,
                  void (*inc_fn)(State *, uintmax_t)) {
    buf_t sub;      /**< substring for each line */
    uint8_t *ptr;   /**< internal pointer for strspl */
    uintmax_t line; /**< line number */

    ptr = input.ptr;
    line = 0;
    while ((sub = strspl(input, '\n', &ptr)).ptr != NULL) {
        Exe exec;

        errno = 0;
        exec = parse_exe(sub);

        if (((exec.arg == INTMAX_MIN || exec.arg == INTMAX_MAX) &&
             errno == ERANGE) ||
            errno == EINVAL)
            fprintf(stderr, "bad input (line %ju): %s\n", line,
                    strerror(errno));

        exec_exe(self, exec, inc_fn);
        line += 1;
    }
}

static Exe parse_exe(buf_t line) {
    Exe ret;

    ret.kind = Invalid;
    ret.arg = -1;

    if (line.len < strlen("noop")) return ret;

    switch (line.ptr[0]) {
        case 'n':
            ret.kind = NoOp;
            ret.arg = 0;
            return ret;
        case 'a': {
            char *endptr;
            if (line.len < 6) return ret;

            ret.kind = AddX;
            ret.arg = strtoimax((const char *)line.ptr + strlen("addx "),
                                &endptr, 10);
            if ((uint8_t *)endptr != (uint8_t *)(line.ptr + line.len))
                errno = EINVAL;

            return ret;
        }
        default:
            return ret;
    }
}

static buf_t strspl(buf_t input, uint8_t delim, uint8_t **ptr) {
    uint8_t *end;
    buf_t ret;

    ret.ptr = NULL;
    ret.len = 0;
    if (*ptr == NULL) return ret;

loop:
    if (*ptr >= (input.ptr + input.len)) {
        *ptr = NULL;
        return ret;
    }

    if (**ptr == delim) {
        (*ptr)++;
        goto loop;
    }

    ret.ptr = *ptr;

find:
    (*ptr)++;

    if (*ptr >= input.ptr + input.len) {
        end = input.ptr + input.len;
        *ptr = NULL;
    } else if (**ptr == delim) {
        end = *ptr;
        (*ptr)++;
    } else
        goto find;

    ret.len = end - ret.ptr;
    return ret;
}

static uint8_t parse_char(std::bitset<COLS * ROWS> set, size_t n) {
    Block ch;

    if (n >= set.size() / (CW * CH)) return 0;
    const size_t total_width = set.size() / CH;

    for (size_t h = 0; h < CH; h++)
        for (size_t i = 0; i < CW; i++)
            ch[ch.size() - 1 - (i + h * CW)] =
                set[i + h * total_width + n * CW];

    // this made me cry
    const BlockMap map = {
        {Block(0b011001001010010111101001010010), 'A'},
        {Block(0b111001001011100100101001011100), 'B'},
        {Block(0b011001001010000100001001001100), 'C'},
        {Block(0b111101000011100100001000011110), 'E'},
        {Block(0b111101000011100100001000010000), 'F'},
        {Block(0b011001001010000101101001001110), 'G'},
        {Block(0b100101001011110100101001010010), 'H'},
        {Block(0b011100010000100001000010001110), 'I'},
        {Block(0b001100001000010000101001001100), 'J'},
        {Block(0b100101010011000101001010010010), 'K'},
        {Block(0b100001000010000100001000011110), 'L'},
        {Block(0b011001001010010100101001001100), 'O'},
        {Block(0b111001001010010111001000010000), 'P'},
        {Block(0b111001001010010111001010010010), 'R'},
        {Block(0b011101000010000011000001011100), 'S'},
        {Block(0b100101001010010100101001001100), 'U'},
        {Block(0b100011000101010001000010000100), 'Y'},
        {Block(0b111100001000100010001000011110), 'Z'},
    };

    BlockMap::const_iterator search = map.find(ch);
    return search != map.end() ? search->second : 0;
}

static void exec_exe(State *self, Exe exe, void (*inc_fn)(State *, uintmax_t)) {
    switch (exe.kind) {
        case AddX:
            inc_fn(self, 2);
            self->X += exe.arg;
            break;
        case NoOp:
            inc_fn(self, 1);
            break;
        default:
            break;
    }
}

static buf_t strfrommax(uintmax_t max) {
    return (buf_t){.len = 0, .ptr = (uint8_t *)max};
}
