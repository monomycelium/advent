// TODO: implement `check` using advent.fly.dev.

#include "check.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fetch.h"

const char *const outcome_sym[] = {
    "\xE2\x9C\x85",      // Check mark
    "\xE2\x9D\x8C",      // Cross mark
    "\xE2\xAC\x87",      // Downwards arrow
    "\xE2\xAC\x86",      // Upwards arrow
    "\xE2\x8F\xA6",      // Hourglass
    "\xF0\x9F\x9A\xA7",  // Construction sign
    "\xE2\x9A\xA0",      // Question mark
};

outcome_t upload(const day_t *day, part_t part, buf_t answer) {
    buf_t post;           /**< POST data */
    res_t res;            /**< HTTP response */
    char url[URL_LENGTH]; /**< URL */
    char *ptr;            /**< temporary pointer */
    outcome_t out;        /**< outcome */

    if (access(day->app.cooky, R_OK | W_OK) == -1) {
        fputs("cookie file not available\n", stderr);
        return INVALID;
    }

    post.len = (sizeof "level=&answer=") + answer.len +
               (size_t)floor(log10(PART_MAX - 1)) + 1;
    post.ptr = malloc(post.len);
    if (post.ptr == NULL) return INVALID;

    post.len = snprintf((char *)post.ptr, post.len, "level=%u&answer=%s",
                        (unsigned int)part % PART_MAX, answer.ptr);
    snprintf(url, URL_LENGTH, URL_FORMAT, day->year % 10000, day->day % 100);

    res = fetch(url, day->app.cooky, post);
    if (res.status != 200) {
        if (res.status > 0)
            fprintf(stderr, "HTTP status: %u\n", (unsigned int)res.status);
        else
            fputs("CURL(3) failed\n", stderr);

        fputs((char *)res.buffer.ptr, stderr);
        fputc('\n', stderr);
        goto inv;
    }

    ptr = memmem(res.buffer.ptr, res.buffer.len, "<main>", 6);
    // ptr = strstr((char *)res.buffer.ptr, "<main>");
    if (ptr == NULL) goto inv;

    /*
     * Possible cases:
     * - That's the right answer!
     * - That's not the right answer; your answer is too high.
     * - That's not the right answer; your answer is too low.
     * - That's not the right answer.
     * - You don't seem to be solving the right level
     * - You gave an answer too recently;
     */

    if (res.buffer.ptr + res.buffer.len <= (uint8_t *)(19 + 7 + ptr)) goto inv;

    ptr += 19;  // skip HTML tags
    switch (ptr[7]) {
        case 't':
            out = CORRECT;
            goto end;
        case 'n':
            if (res.buffer.ptr + res.buffer.len <= (uint8_t *)(48 + ptr))
                goto inv;

            switch (ptr[48]) {
                case 'l':
                    out = TOO_LOW;
                    goto end;
                case 'h':
                    out = TOO_HIGH;
                    goto end;
                default:
                    out = WRONG;  // answer not numeric
                    goto end;
            }
        case 'e':
            out = WAIT;
            goto end;
        case '\'':
            out = LEVEL;
            break;
        default:
            goto inv;
    }

end:
    if (res.buffer.ptr != NULL) free(res.buffer.ptr);
    free(post.ptr);
    return out;

inv:
    out = INVALID;
    goto end;
};
