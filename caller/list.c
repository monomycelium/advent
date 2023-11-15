#include "list.h"

#include <stdlib.h>
#include <string.h>

// saturating addition
static size_t sadd(size_t a, size_t b) {
    return (SIZE_MAX - a < b) ? SIZE_MAX : a + b;
}

list_t l_init(size_t size) {
    list_t l;

    l.buf.len = 0;
    l.siz = size + 1;
    l.buf.ptr = malloc(l.siz);
    l.buf.ptr[0] = '\0';

    return l;
}

void l_resize(list_t *self, size_t size) {
    uint8_t *ptr;
    self->siz += size;

    ptr = (uint8_t *)realloc(self->buf.ptr, self->siz);
    if (ptr == NULL) {
        l_deinit(self);
        return;
    }

    self->buf.ptr = ptr;
}

void l_append(list_t *self, char item) {
    l_ensure(self, self->buf.len + 1);
    if (self->buf.ptr == NULL) return;
    self->buf.len += 1;
    self->buf.ptr[self->buf.len - 1] = item;
    self->buf.ptr[self->buf.len] = '\0';
}

void l_append_buf(list_t *self, buf_t buf) {
    l_ensure(self, self->buf.len + buf.len);
    if (self->buf.ptr == NULL) return;
    memcpy(self->buf.ptr + self->buf.len, buf.ptr, buf.len);
    self->buf.len += buf.len;
    self->buf.ptr[self->buf.len] = '\0';
}

void l_append_str(list_t *self, const char *restrict str) {
    l_append_buf(self, l_bfroms(str));
}

void l_deinit(list_t *self) {
    free(self->buf.ptr);
    self->buf.ptr = NULL;
}

void l_ensure(list_t *self, size_t size) {
    size_t s = size + 1;
    if (self->siz >= s) return;
    s = l_grow_capacity(self->siz, s);
    l_resize(self, s);
}

buf_t l_buffer(list_t *self) {
    l_resize(self, self->buf.len + 1);
    return self->buf;
}

buf_t l_bfroms(const char *str) {
    return (buf_t){
        .len = strlen(str),
        .ptr = (uint8_t *)str,
    };
}

// adapted from
// https://ziglang.org/documentation/master/std/src/std/array_list.zig.html#L1141
size_t l_grow_capacity(size_t current, size_t minimum) {
    while (true) {
        current = sadd(current, current / 2 + 8);
        if (current >= minimum) return current;
    }
}
