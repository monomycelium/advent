# Advent of Code Attempts
Welcome to this repository of my attempts to solve challenges from [Advent of Code](https://adventofcode.com/)!

### Project Structure

There will be a source file for each day in each year's event, named `YYYY/dayDD.<c|cpp|zig>`, which should compile to a shared object, `YYYY/dayDD.so`, with the following symbols exported:
``` c
#include "common.h" /**< Includes definiton for buf_t */

const uint8_t day; /**< Day of solution. */
const uint16_t year; /**< Year of event. */

buf_t solve1(buf_t input); /**< Solution to Part One */
buf_t solve2(buf_t input); /**< Solution to Part Two */
```
Ensure the symbols can be resolved using `dlsym(3)` by declaring them with `extern "C"` in C++ and `export` in Zig.

Possible return values of `solveX` functions:
- a result string; len set to number of bytes and ptr set to address of null-terminated string allocated using malloc(3).
- a `uintmax_t` number; len set to 0 and ptr set to number to be formatted.
- an error; len set to -1 and ptr set to either NULL or address of null-terminated error string allocated using malloc(3).

The Makefile can compile source files (order of source language preference: {C, C++, Zig}):
``` console
$ make 2022/day04.so
zig build-lib -dynamic ... 2022/day04.zig
```

The caller can solve both challenges using the input:
``` console
$ make caller # build the `caller` executable in `/caller/`
$ ./caller/caller -i /tmp/input ./2022/day04.so
Part 1: 448
Part 2: 794
```

Finally, the caller can upload the results using your session cookie (which must be saved using the format [described by cURL](https://curl.se/docs/http-cookies.html)):
``` console
$ ./caller/caller -i /tmp/input -u -b ./.cookie -p2 ./2022/day04.so
Part 2: 794 ✅
```
