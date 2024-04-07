/**
 * Day 8: Treetop Tree House
 *
 * The functions in this source assume each row has the same amount of trees
 * followed by a single newline byte.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <string>

#include "common.h"

extern "C" const int16_t year = 2022;
extern "C" const int8_t day = 8;

struct Grid {
    const uint8_t *buffer;
    size_t width;
    size_t height;

    Grid() { memset(this, 0, sizeof *this); }

    Grid(buf_t input) {
        void *ptr;

        ptr = memchr(input.ptr, '\n', input.len);
        if (ptr == NULL) *this = Grid();

        this->width = (size_t)ptr - (size_t)input.ptr;
        this->height = (size_t)input.len / (this->width + 1);
        this->buffer = (const uint8_t *)input.ptr;
    }

    uint8_t value(size_t row, size_t col) {
        return this->buffer[row * (this->width + 1) + col];
    }

    size_t view_distance(size_t row, size_t col, ssize_t rv, ssize_t cv) {
        size_t d;
        uint8_t h;

        d = 0;
        h = this->value(row, col);

        while (true) {
            uint8_t t;

            row += rv;
            col += cv;

            t = this->value(row, col);
            d++;

            if (t >= h) break;
            if (rv >= 0 && ((row + rv) >= this->height)) break;
            if (cv >= 0 && ((col + cv) >= this->width)) break;
            if (rv < 0 && ((size_t)-rv > row)) break;
            if (cv < 0 && ((size_t)-cv > col)) break;
        }

        return d;
    }

    size_t scenic_score(size_t row, size_t col) {
        return this->view_distance(row, col, 1, 0) *
               this->view_distance(row, col, -1, 0) *
               this->view_distance(row, col, 0, 1) *
               this->view_distance(row, col, 0, -1);
    }
};

extern "C" buf_t solve1(buf_t input) {
    Grid grid;                      /**< grid representation */
    boost::dynamic_bitset<> bitset; /**< bitset of tree visibility */
    std::string str;                /**< result as string */
    buf_t result;                   /**< result as buf_t */

    result.len = -1;
    result.ptr = NULL;

    grid = Grid(input);
    bitset = boost::dynamic_bitset<>(grid.width * grid.height);

    for (size_t row = 0; row < grid.height; row++) {
        uint8_t lmax, rmax; /**< max from left and right */

        lmax = rmax = 0;
        for (size_t col = 0; col < grid.width; col++) {
            size_t li, ri;  /**< left and right index */
            uint8_t lv, rv; /**< left and right value */
            bool lg, rg;    /**< whether {left, right} > {lmax, rmax} */

            li = col;
            ri = grid.width - col - 1;

            lv = grid.value(row, li);
            rv = grid.value(row, ri);

            lg = lv > lmax;
            rg = rv > rmax;

            if (lg) lmax = lv;
            if (rg) rmax = rv;

            bitset[row * grid.width + li] |= lg;
            bitset[row * grid.width + ri] |= rg;
        }
    }

    for (size_t col = 0; col < grid.width; col++) {
        uint8_t tmax, bmax; /**< max from top and bottom */

        tmax = bmax = 0;
        for (size_t row = 0; row < grid.height; row++) {
            size_t ti, bi;  /**< top and bottom index */
            uint8_t tv, bv; /**< top and bottom value */
            bool tg, bg;    /**< whether {top, bottom} > {tmax, bmax} */

            ti = row;
            bi = grid.height - row - 1;

            tv = grid.value(ti, col);
            bv = grid.value(bi, col);

            tg = tv > tmax;
            bg = bv > bmax;

            if (tg) tmax = tv;
            if (bg) bmax = bv;

            bitset[ti * grid.width + col] |= tg;
            bitset[bi * grid.width + col] |= bg;
        }
    }

    result.ptr = (uint8_t *)bitset.count();
    result.len = 0;

    return result;
}

extern "C" buf_t solve2(buf_t input) {
    size_t max;      /**< highest scenic score */
    Grid grid;       /**< grid representation */
    std::string str; /**< result as string */
    buf_t result;    /**< result as buf_t */

    result.len = -1;
    result.ptr = NULL;

    max = 0;
    grid = Grid(input);
    if (grid.buffer == NULL) return result;

    for (size_t row = 0; row < grid.height; row++)
        for (size_t col = 0; col < grid.width; col++)
            max = std::max(max, grid.scenic_score(row, col));

    result.ptr = (uint8_t *)max;
    result.len = 0;

    return result;
}
