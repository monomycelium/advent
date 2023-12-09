#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <format>
#include <functional>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "common.h"

extern "C" const uint16_t year = 2022;
extern "C" const uint8_t day = 9;

/**
 * Direction of movement.
 */
enum Dir {
    MU = 'U', /**< up */
    MD = 'D', /**< down */
    ML = 'L', /**< left */
    MR = 'R', /**< right */
    MI = 0,   /**< invalid */
};

/**
 * Representation of movement.
 */
struct Mov {
    Dir dir;     /**< direction */
    uint8_t amt; /**< steps */
};

/**
 * Representation of coordinate.
 */
struct Cor {
    intmax_t x; /**< x-coordinate */
    intmax_t y; /**< y-coordinate */

    struct Hash {
        std::size_t operator()(const Cor &c) const;
    };

    bool operator==(const Cor &c) const;

    /**
     * Whether `this` is touching `tail`.
     */
    bool is_touching(const Cor *head) const;

    /**
     * Advance tail one step closer to head.
     *
     * Tail moves horizontally, vertically or diagonally.
     */
    void advance_to(const Cor *head);

    /**
     * Move head one step to `Dir`.
     */
    void move(Dir move);
};

/**
 * Parse input and call a callback function for each `Mov`.
 *
 * @param input     Solution input.
 * @param data      Data pointer to pass to `callback`.
 * @param callback  Callback to call for each line.
 *
 * @return          Number of lines parsed.
 */
static size_t parse(buf_t input, std::function<bool(Mov)> callback);

/**
 * Split buffer using a single delimeter.
 *
 * @param input:    Buffer to split.
 * @param delim:    Delimiting character to split using.
 * @param ptr:      Internal pointer (should initially point to input.ptr).
 */
static buf_t strspl(buf_t input, uint8_t delim, uint8_t **ptr);

/**
 * Convert `size_t` to heap-allocated buffer.
 */
static buf_t bfromi(size_t i);

/**
 * Solves for n knots.
 */
static std::unordered_set<Cor, Cor::Hash> solve(buf_t input, size_t n);

extern "C" buf_t solve1(buf_t input) { return bfromi(solve(input, 2).size()); }

extern "C" buf_t solve2(buf_t input) { return bfromi(solve(input, 10).size()); }

static std::unordered_set<Cor, Cor::Hash> solve(buf_t input, size_t n) {
    std::unordered_set<Cor, Cor::Hash> unique{{0, 0}};
    if (n < 2) return unique;
    std::vector<Cor> knots(n, {0, 0});

    parse(input, [&](Mov mov) -> bool {
        if (mov.dir == MI || mov.amt == 0) return true;  // return false to stop

        for (uint8_t a = 0; a < mov.amt; a++) {
            bool moved = false;
            knots[0].move(mov.dir);

            for (size_t i = 1; i < n; i++) {
                Cor *head = &knots[i - 1];
                Cor *tail = &knots[i];

                if (head->is_touching(tail)) break;
                tail->advance_to(head);
                moved = i + 1 == n;
            }

            if (moved) unique.insert(knots[n - 1]);
        }

        return true;
    });

    return unique;
}

void Cor::advance_to(const Cor *head) {
    Cor dif = {
        .x = head->x - this->x,
        .y = head->y - this->y,
    };

    Cor mov = {
        .x = (dif.x > 0)   ? 1
             : (dif.x < 0) ? -1
                           : 0,
        .y = (dif.y > 0)   ? 1
             : (dif.y < 0) ? -1
                           : 0,
    };

    this->x += mov.x;
    this->y += mov.y;
}

bool Cor::is_touching(const Cor *tail) const {
    return std::abs(this->x - tail->x) <= 1 && std::abs(this->y - tail->y) <= 1;
}

void Cor::move(Dir dir) {
    switch (dir) {
        case MU:
            (this->y)++;
            break;
        case MD:
            (this->y)--;
            break;
        case ML:
            (this->x)--;
            break;
        case MR:
            (this->x)++;
            break;
        default:
            break;
    }
}

static size_t parse(buf_t input, std::function<bool(Mov)> callback) {
    buf_t sub;
    size_t lines = 0;
    uint8_t *ptr = input.ptr;
    bool run = true;

    while (run && (sub = strspl(input, '\n', &ptr)).ptr != NULL) {
        char *end;
        unsigned long num;
        Mov mov;

        mov.dir = MI;
        mov.amt = 0;

        if (sub.len < 3) goto call;

        mov.dir = (Dir)sub.ptr[0];  // map first character
        switch (mov.dir) {
            case (uint8_t)MU:
            case (uint8_t)MD:
            case (uint8_t)ML:
            case (uint8_t)MR:
                break;
            default:
                mov.dir = MI;
                break;
        }

        num = strtoul((char *)sub.ptr + 2, &end, 10);
        if ((num != ULONG_MAX || errno != ERANGE) &&
            (*end == '\n' || *end == '\0'))
            mov.amt = (uint8_t)num;

    call:
        run = callback(mov);
        lines++;
    }

    return lines;
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

std::size_t Cor::Hash::operator()(const Cor &c) const {
    return std::hash<intmax_t>()(c.x) ^ (std::hash<intmax_t>()(c.y) << 1);
}

bool Cor::operator==(const Cor &c) const {
    return this->x == c.x && this->y == c.y;
}

static buf_t bfromi(size_t i) {
    buf_t ret;
    std::string result = std::to_string(i);

    ret.len = result.length();
    ret.ptr = (uint8_t *)strndup(&result[0], ret.len);
    return ret;
}