#include "caller.h"

#include <dlfcn.h>
#include <errno.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "check.h"
#include "input.h"

void solve(const day_t *day, part_t part) {
    char *errorstr;
    solve_func func;
    void *symbol;
    buf_t result;

    symbol = dlsym(day->handle, symbol_name(part));
    if ((errorstr = dlerror()) != NULL) {
        result.ptr = (uint8_t *)errorstr;
        symbol = NULL;
        goto print;
    }

    func = (solve_func)symbol;
    result = func(day->input);

    if (result.len == 0 && result.ptr != NULL) {
        uintmax_t value = (uintmax_t)result.ptr;
        result.len = (size_t)snprintf(NULL, 0, "%ju", value);
        result.ptr = malloc(result.len + 1);

        if (result.ptr == NULL) {
            result.len = 0;
            goto print;
        }

        snprintf((char *)result.ptr, result.len + 1, "%ju", value);
    }

print:
    printf("Part %u: %s ", (unsigned int)part, result.ptr);
    fputs("\033[90m", stdout);

    if (symbol != NULL) {
        if (day->app.check != LEAVE)
            fputs(outcome_sym[(day->app.check == UPLOD ? upload : check)(
                      day, part, result)],
                  stdout);

        free(result.ptr);
    }

    fputs("\033[m", stdout);
    putchar('\n');
}

int main(int argc, char **argv) {
    day_t day;      /**< day data */
    char *errorstr; /**< error string */
    FILE *inputptr; /**< input file pointer */
    void *ptr;      /**< temporary pointer */

    day.app.input = NULL;
    day.app.check = LEAVE;
    day.app.parts = 0b11;
    day.app.cooky = ".cookie";
    day.app.objct = NULL;
    day.input.ptr = NULL;

    if (parseargs(argc, argv, &day.app) == false) usage(EXIT_FAILURE, argv[0]);

    if (day.app.input != NULL) {
        inputptr = fopen(day.app.input, "r");

        if (inputptr == NULL) {
            errorstr = "open";
            goto err;
        }
    }

    day.input = read_input(day.app.input != NULL ? inputptr : stdin);
    if (day.input.ptr == NULL) {
        int old = errno;
        fclose(inputptr);
        errno = old;
        errorstr = "read";
        goto err;
    }

    if (day.app.input != NULL)
        if (fclose(inputptr) != 0) {
            free(day.input.ptr);
            errorstr = "close";
            goto err;
        }

    day.handle = dlopen(day.app.objct, RTLD_LAZY);
    if (day.handle == NULL) goto set;

    ptr = dlsym(day.handle, "year");
    if ((errorstr = dlerror()) != NULL) goto die;
    day.year = *(uint16_t *)ptr;
    ptr = dlsym(day.handle, "day");
    if ((errorstr = dlerror()) != NULL) goto die;
    day.day = *(uint8_t *)ptr;

    for (part_t part = PART_ONE; part < PART_MAX; part++)
        if ((day.app.parts >> (part - PART_ONE)) & 1) solve(&day, part);

    free(day.input.ptr);
    if (dlclose(day.handle) != 0) goto set;
    return EXIT_SUCCESS;

err:
    fprintf(stderr, "failed to %s input: %s\n", errorstr, strerror(errno));
    return EXIT_FAILURE;

set:
    errorstr = dlerror();
die:
    fprintf(stderr, "%s\n", errorstr);
    if (day.input.ptr) free(day.input.ptr);
    return EXIT_FAILURE;
}

void usage(int code, char *arg0) {
    fprintf(stderr, "usage: %s [OPTIONS] <SHARED_OBJ>\n", arg0);
    fputs(
        "  -c\t\t\tcheck answer using advent.fly.dev\n"
        "  -u\t\t\tupload answer to adventofcode.com\n"
        "  -p <PART: uint>\texecute PART (default: all)\n"
        "  -i <PATH: str>\tread input from file (default: stdin)\n"
        "  -b <PATH: str>\tcookie file (default: .cookie)\n",
        stderr);
    exit(code);
}

char *symbol_name(part_t part) {
    switch (part) {
        case PART_ONE:
            return "solve1";
        case PART_TWO:
            return "solve2";
        default:
            return NULL;
    }
}

bool parseargs(int argc, char **argv, app_t *app) {
    int c;
    app_t old = *app;
    app->parts = 0;

    while ((c = getopt(argc, argv, "cui:b:p:")) != -1) switch (c) {
            case 'c':
                app->check = CHECK;
                break;
            case 'u':
                app->check = UPLOD;
                break;
            case 'b':
                app->cooky = optarg;
                break;
            case 'i':
                app->input = optarg;
                break;
            case 'p': {
                unsigned char u = optarg[0] - '0';

                if (u >= PART_MAX || u < PART_ONE) {
                    fprintf(stderr, "invalid part `%s'\n", optarg);
                    return false;
                };

                app->parts ^= 1 << (u - PART_ONE);
                break;
            }
            case '?':
            default:
                return false;
        }

    if (optind == argc) {
        fputs("missing argument: <SHARED_OBJ>\n", stderr);
        return false;
    }

    app->objct = argv[optind];                    // set shared object
    if (app->parts == 0) app->parts = old.parts;  // restore parts if unchanged

    return true;
}
