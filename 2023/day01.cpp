#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <charconv>
#include <limits>
#include <string>

#include "common.h"

extern "C" const uint8_t day = 01;
extern "C" const uint16_t year = 2023;

struct DigitResult {
    uint8_t *pos;  /** original address of the digit (in input buffer) */
    uint8_t digit; /** parsed digit */
};

typedef uint8_t (*digit_func)(bool, uint8_t *);

buf_t solver(buf_t input, digit_func func);
DigitResult func1(bool rev, uint8_t *ptr);
DigitResult func2(bool rev, uint8_t *ptr);

DigitResult func1(bool rev, uint8_t *ptr) {  // gets first or last numeric digit
    int8_t inc = rev ? -1 : 1;

    for (; *ptr != '\n'; ptr += inc)
        if (isdigit((int)*ptr) != 0)
            return {.pos = ptr, .digit = (uint8_t)(*ptr - '0')};

    return {.pos = NULL, .digit = (uint8_t)-1};
}

DigitResult func2(bool rev, uint8_t *ptr) {  // gets first or last written digit
    static const char *const digits[] = {
        "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    constexpr uint8_t length = sizeof digits / sizeof digits[0];
    int8_t inc = rev ? -1 : 1;

    for (; *ptr != '\n'; ptr += inc)
        for (uint8_t i = 0; i < length; i++)
            if (memcmp(digits[i], ptr, strlen(digits[i])) == 0)
                return {.pos = ptr, .digit = (uint8_t)(i + 1)};

    return {.pos = NULL, .digit = (uint8_t)-1};
}

buf_t solver(buf_t input, digit_func func) {
    uint8_t *ptr;
    uintmax_t sum;
    size_t count;

    ptr = input.ptr;
    sum = 0;
    count = 0;

    while (*ptr != 0) {
        uint8_t num;
        uint8_t *end;

        end = (uint8_t *)memchr(ptr, '\n', input.ptr + input.len - ptr);
        if (end == NULL) {  // would this happen?
            *end = 0;
            fprintf(stderr, "weird format in line %zu: '%s'\n", count, ptr);
            goto next;
        }

        num = func(false, ptr);
        if (num == (uint8_t)-1) {
            *end = 0;
            fprintf(stderr, "no digits in line %zu: '%s'\n", count, ptr);
            goto next;  // no digits in line.
        }
        num *= 10;
        num += func(true, end - 1);

        sum += num;
    next:
        ptr = end + 1;  // set pointer to next line
        count++;
    }

    return (buf_t){.len = 0, .ptr = (uint8_t *)sum};
}

extern "C" buf_t solve1(buf_t input) {
    return solver(input, [](auto x, auto y) { return func1(x, y).digit; });
}

extern "C" buf_t solve2(buf_t input) {
    return solver(input, [](auto x, auto y) {
        DigitResult one = func1(x, y);
        DigitResult two = func2(x, y);

        return one.pos == NULL     ? two.digit
               : two.pos == NULL   ? one.digit
               : one.pos < two.pos ? (x ? two.digit : one.digit)
                                   : (x ? one.digit : two.digit);
    });
}
