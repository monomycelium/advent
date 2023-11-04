# Advent of Code Attempts
Welcome to this repository of my attempts to solve challenges from [Advent of Code](https://adventofcode.com/)!

### Project Structure

In the directory for each year, there will be a source file for each day, named `YYYY/dayDD.<c|zig>`. The source file should compile to a shared object, `dayDD.so`, with the following symbols (with `buf_t` being a simple buffer type defined in `common.h`):
``` c
buf_t solve1(buf_t input);
buf_t solve2(buf_t input);
```

The Makefile can compile Zig and C source files:
``` console
$ make 2022/day04.so
zig build-lib  -dynamic -lc -I. 2022/day04.zig
```

The caller can solve both challenges using the input (trailing newlines should be ommited):
``` console
$ make caller # build the `caller` executable
$ ./caller 2022/day04.so < /tmp/input.txt
Part 1: 448
Part 2: 794
```

The Zig source files are compatible with `zig version` `0.11.0`.
