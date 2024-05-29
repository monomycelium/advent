#include <algorithm>
#include <cassert>
#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <unordered_set>
#include <vector>

#include "common.h"

#define NUMBER_PARTS 2 /** number of part numbers (for part two) */

extern "C" const uint8_t day = 03;
extern "C" const uint16_t year = 2023;

struct Coord {
    intmax_t x;
    intmax_t y;
};

struct Grid {
    uint8_t *ptr; /** pointer to input */
    Coord dim;    /** dimensions of grid */

    Grid(buf_t input);
    uint8_t *at(Coord c);
};

Grid::Grid(buf_t input) {
    this->ptr = input.ptr;
    this->dim.y = 0;
    this->dim.x = static_cast<uint8_t *>(std::memchr(
                      input.ptr, '\n', static_cast<size_t>(input.len))) -
                  input.ptr;

    if (this->dim.x == 0) return;

    this->dim.y = input.len / (this->dim.x + 1);
    assert(input.len % (this->dim.x + 1) == 0);
}

uint8_t *Grid::at(Coord c) {
    uint8_t *ret = this->ptr + c.x + (this->dim.x + 1) * c.y;
    return (c.y >= this->dim.y || c.x >= this->dim.x || c.y < 0 || c.x < 0)
               ? nullptr
               : ret;
}

static bool issym(uint8_t u) { return (std::isdigit(u) || u == '.') ^ 1; }

extern "C" buf_t solve1(buf_t input) {
    std::unordered_set<uint8_t *> set;
    Grid grid(input);
    Coord c = {.x = 0, .y = 0};

    for (c.y = 0; c.y < grid.dim.y; c.y++)
        for (c.x = 0; c.x < grid.dim.x; c.x++)
            if (issym(*(grid.at(c))))
                for (int8_t i = -1; i < 2; i++)
                    for (int8_t j = -1; j < 2; j++) {
                        Coord addr = {.x = c.x + j, .y = c.y + i};
                        uint8_t *p = grid.at(addr);

                        for (; p != nullptr && std::isdigit(*p);
                             addr.x--, p = grid.at(addr));
                        if (addr.x == c.x + j) continue;

                        addr.x += 1;
                        set.insert(grid.at(addr));
                    }

    uintmax_t sum = 0;
    for (const auto &elem : set) sum += strtoumax((char *)elem, nullptr, 10);
    return (buf_t){.len = 0, .ptr = reinterpret_cast<uint8_t *>(sum)};
}

extern "C" buf_t solve2(buf_t input) {
    Grid grid(input);
    Coord c = {.x = 0, .y = 0};
    uintmax_t sum = 0;

    for (c.y = 0; c.y < grid.dim.y; c.y++)
        for (c.x = 0; c.x < grid.dim.x; c.x++)
            if (*(grid.at(c)) == '*') {
                std::unordered_set<uint8_t *> set;

                for (int8_t i = -1; i < 2; i++)
                    for (int8_t j = -1; j < 2; j++) {
                        Coord addr = {.x = c.x + j, .y = c.y + i};
                        uint8_t *p = grid.at(addr);

                        for (; p != nullptr && std::isdigit(*p);
                             addr.x--, p = grid.at(addr));
                        if (addr.x == c.x + j) continue;

                        addr.x += 1;
                        set.insert(grid.at(addr));
                    }

                if (set.size() == NUMBER_PARTS) {
                    uintmax_t num = 1;
                    for (const auto &elem : set)
                        num *= strtoumax(reinterpret_cast<char *>(elem),
                                         nullptr, 10);
                    sum += num;
                }
            }

    return (buf_t){.len = 0, .ptr = reinterpret_cast<uint8_t *>(sum)};
}
