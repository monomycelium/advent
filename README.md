# Advent of Code Attempts
Welcome to this repository of my attempts to solve challenges from [Advent of Code](https://adventofcode.com/)!

### Project Structure

There will be a source file for each day in each year's event, named `YYYY/dayDD.<c|zig>`, which should compile to a shared object, `YYYY/dayDD.so`, with the following symbols:
``` c
#include "common.h" /**< Includes definiton for buf_t */

const uint8_t day; /**< Day of solution. */
const uint16_t year; /**< Year of event. */

buf_t solve1(buf_t input); /**< Solution to Part One */
buf_t solve2(buf_t input); /**< Solution to Part Two */
```

Possible return values:
- a string of characters allocated using malloc(3) and null-terminated; len indicates the number of characters and ptr contains the address of the string.
- a `uintmax_t` number; len is set to 0 and ptr contains the number to format.

The Makefile can compile Zig and C source files (with a preference for C):
``` console
$ make 2022/day04.so
zig build-lib -dynamic -lc -Icaller/ 2022/day04.zig
```

The caller can solve both challenges using the input:
``` console
$ make caller # build the `caller` executable to `./caller/caller`
$ ./caller/caller -i/tmp/input 2022/day04.so
Part 1: 448
Part 2: 794
```

Finally, the caller can even upload the results using your session cookie (which must be saved using the format [described by CURL](https://curl.se/docs/http-cookies.html)):
``` console
$ ./caller/caller -u -b .cookie -p2 2022/day04.so < /tmp/input
Part 2: 794 ✅
```

The Zig source files are compatible with version `0.11.0`.
