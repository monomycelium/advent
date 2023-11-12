#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

typedef struct list {
    size_t siz;  // number of bytes that can be held currently
    buf_t buf;   // items in list
} list_t;

list_t l_init(size_t size);
void l_resize(list_t *self, size_t size);
void l_append(list_t *self, char item);
void l_append_buf(list_t *self, buf_t buf);
void l_append_str(list_t *self, const char *str);
void l_deinit(list_t *self);
void l_ensure(list_t *self, size_t size);
buf_t l_buffer(list_t *self);
buf_t l_bfroms(const char *str);
size_t l_grow_capacity(size_t current, size_t minimum);

#endif  // LIST_H
