#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

extern "C" const std::uint16_t year = 2022;
extern "C" const std::uint8_t day = 12;

#include "common.h"

/**
 * Representation of coordinate.
 */
struct Coord {
    std::intmax_t x; /** x-coordinate */
    std::intmax_t y; /** y-coordinate */

    bool operator==(const Coord &c) const;
};

/**
 * Representation of grid.
 */
struct Grid {
    std::uint8_t *grid; /** buffer to hold grid */
    Coord limits;       /** grid size */

    typedef std::vector<bool> Visits;
    typedef std::deque<Coord> Queue;

    /**
     * Append possible coordinates to move to.
     */
    std::size_t append_neighbours(Coord coord, Grid::Queue &queue,
                                  Grid::Visits &visits) const;

    /**
     * Get the pointer to data at the coordinate.
     */
    std::uint8_t *at(Coord coord);

    /**
     * Get the pointer to data at the coordinate.
     */
    const std::uint8_t *at(Coord coord) const;

    /**
     * Get index of data at coordinate.
     */
    std::size_t index(Coord coord) const;

    /**
     * Get the coordinate using the index of data.
     */
    Coord at(std::size_t i) const;

    /**
     * Get the number of coordinates.
     */
    std::size_t size() const;

    /**
     * Get the number of bytes needed to store the grid buffer.
     */
    std::size_t real_size() const;

    /**
     * Find the validity of a coordinate in the grid.
     */
    bool valid(Coord coord) const;
};

/**
 * Representation of input.
 */
struct Data {
    Grid grid;
    Coord start; /** starting coordinate */
    Coord end;   /** ending coordinate */

    /**
     * Find the minimal number of steps needed to reach the end from the start.
     *
     * Breadth-First Search implementation inspired by
     * [Reducible](https://youtu.be/xlVX7dXLS64?si=NIDam-5LxZHFWFj2).
     */
    std::size_t bfs(Grid::Queue &queue) const;

    /**
     * Initialise Data using the given input.
     */
    Data(buf_t input);

    /**
     * De-initialise Data.
     */
    ~Data();
};

static buf_t bfromi(size_t i);

extern "C" buf_t solve1(buf_t input) {
    Data d(input);
    Grid::Queue q({d.start});
    return bfromi(d.bfs(q));
}

/*
 * Initially, I wanted to initialise the queue with all the possible starting
 * coordinates. However, my answers were too low! It must have been a bug in my
 * implementation... Anyway, I decided to call the BFS function for each
 * starting point.
 */
extern "C" buf_t solve2(buf_t input) {
    Data d(input);
    size_t min = (size_t)-1;

    Grid::Queue q;

    size_t e = d.grid.real_size();
    for (size_t i = 0; i < e; i++)
        if (d.grid.grid[i] == 'a') {
            Grid::Queue q({d.grid.at(i)});
            min = std::min(d.bfs(q), min);
        }

    return bfromi(min);
}

std::size_t Data::bfs(Grid::Queue &queue) const {
    size_t real_size = this->grid.real_size();

    size_t count = 0;
    Grid::Visits visits(real_size, 0);

    for (const Coord &c : queue) visits[this->grid.index(c)] = true;

    queue.push_back(Coord{-1, -1});
    while (queue.size() > 0) {
        Coord coord = queue.front();
        queue.pop_front();

        if (coord == Coord{-1, -1}) {
            queue.push_back(coord);
            count++;
            continue;
        }

        if (coord == this->end)
            return count;
        else
            this->grid.append_neighbours(coord, queue, visits);
    }

    return (std::size_t)-1;
}

std::size_t Grid::append_neighbours(Coord coord, Grid::Queue &queue,
                                    Grid::Visits &visits) const {
    uint8_t max = *this->at(coord) + 2;
    size_t count = 0;

    const std::array<Coord, 4> possibilities = {
        Coord{coord.x + 1, coord.y},
        Coord{coord.x - 1, coord.y},
        Coord{coord.x, coord.y + 1},
        Coord{coord.x, coord.y - 1},
    };

    for (const Coord &x : possibilities) {
        size_t i = this->index(x);
        if (this->valid(x) and this->grid[i] < max and !visits.at(i)) {
            count++;
            queue.push_back(x);
            visits[i] = true;
        }
    }

    return count;
}

Data::~Data() { std::free(this->grid.grid); }

Data::Data(buf_t input) {
    this->grid.grid = (std::uint8_t *)std::malloc(input.len + 1);
    std::memcpy(this->grid.grid, input.ptr, input.len);
    this->grid.grid[input.len] = '\0';

    std::size_t x[3];
    const int scalars[3] = {'\n', 'S', 'E'};
    for (std::uint8_t i = 0; i < 3; i++) {  // sobbing at the stupidity!
        x[i] = (size_t)std::memchr(this->grid.grid, scalars[i], input.len);

        if (x[i] == 0)
            throw "invalid input";
        else
            x[i] -= (std::size_t)this->grid.grid;
    }

    this->grid.limits.x = x[0];
    this->grid.limits.y = input.len / this->grid.limits.x;

    Coord *ptrs[] = {&this->start, &this->end};
    for (std::uint8_t i = 0; i < 2; i++) {
        *ptrs[i] = this->grid.at(x[i + 1]);
        this->grid.grid[x[i + 1]] = i == 0 ? 'a' : 'z';
    }
}

std::uint8_t *Grid::at(Coord coord) { return this->grid + this->index(coord); }

const std::uint8_t *Grid::at(Coord coord) const {
    return this->grid + this->index(coord);
}

std::size_t Grid::index(Coord coord) const {
    return coord.x + coord.y * (this->limits.x + 1);
}

bool Grid::valid(Coord coord) const {
    return this->limits.x > coord.x && this->limits.y > coord.y &&
           coord.x >= 0 && coord.y >= 0;
}

static buf_t bfromi(size_t i) { return (buf_t){.len = 0, .ptr = (uint8_t *)i}; }

std::size_t Grid::size() const { return this->limits.x * this->limits.y; }

std::size_t Grid::real_size() const {
    return (this->limits.x + 1) * this->limits.y;
}

Coord Grid::at(std::size_t i) const {
    Coord c;
    c.y = i / this->limits.x;
    c.x = (i - c.y) % this->limits.x;
    return c;
}

bool Coord::operator==(const Coord &c) const {
    return this->x == c.x && this->y == c.y;
}
