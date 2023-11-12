#include "fetch.h"

#include <curl/curl.h>

#include "list.h"

static size_t write(char *ptr, size_t size, size_t nmemb, list_t *list);

res_t fetch(const char *url, const char *cookiefile, buf_t data) {
    char errbuf[CURL_ERROR_SIZE]; /**< error buffer */
    list_t list;                  /**< raw response */
    CURLcode c;                   /**< CURL code */
    CURL *curl;                   /**< CURL instance */
    res_t res;                    /**< HTTP response */

    errbuf[0] = '\0';
    curl = NULL;
    list = l_init(0);
    res = (res_t){
        .status = 0,
        .buffer =
            (buf_t){
                .len = 0,
                .ptr = NULL,
            },
    };

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl == NULL) goto defer;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    /* cache the CA cert bundle in memory for a week */
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    /* set callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    if (cookiefile != NULL) {
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiefile);
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiefile);
    }

    if (data.ptr != NULL) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.len);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.ptr);
    }

    // execute
    c = curl_easy_perform(curl);
    if (c != CURLE_OK) {
        const char *err;
        err = (errbuf[0] == '\0') ? curl_easy_strerror(c) : errbuf;
        res.status = -(int16_t)c;
        list.buf.len = 0;
        l_append_str(&list, err);
        goto save;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status);

save:
    res.buffer = l_buffer(&list);

defer:
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return res;
}

static size_t write(char *ptr, size_t size, size_t nmemb, list_t *list) {
    size_t realsize;
    realsize = size * nmemb;

    l_append_buf(list, (buf_t){
                           .len = realsize,
                           .ptr = ptr,
                       });

    return (list->buf.ptr == NULL) ? 0 : realsize;
}
