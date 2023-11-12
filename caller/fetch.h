#ifndef FETCH_H
#define FETCH_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

/**
 * HTTP Response.
 */
typedef struct res {
    /**
     * Response status.
     *
     * If `curl_easy` fails, `result` will be the negative of the `CURLcode`.
     * Otherwise, `result` will be the HTTP response status code.
     *
     * For more information:
     * - https://curl.se/libcurl/c/libcurl-errors.html
     * - https://developer.mozilla.org/en-US/docs/Web/HTTP/status
     */
    int16_t status;
    /**
     * Raw response.
     *
     * If `curl_easy` fails, `buffer` will be a heap-allocated error buffer.
     *
     * For more information:
     * - https://curl.se/libcurl/c/CURLOPT_ERRORBUFFER.html
     */
    buf_t buffer;
} res_t;

/**
 * Make an HTTP Request using `curl_easy`.
 *
 * @param url           URL to make request to.
 * @param cookiefile    Cookie file to read and write cookies.
 * @param data          Data to upload.
 */
res_t fetch(const char *url, const char *cookiefile, buf_t data);

#endif  // FETCH_H
