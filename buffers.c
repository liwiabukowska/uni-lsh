#include "buffers.h"
#include "lsh_exit.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct char_buff char_buff_create()
{
    struct char_buff buff = { 0, 0, NULL };
    return buff;
}

bool char_buff_alloc(struct char_buff* buff, size_t new_capacity)
{
    char* ptr = realloc(buff->data, new_capacity);
    if (!ptr) {
        exit_with_message(1, "malloc error!!!\n");
        return false;
    }

    buff->capacity = new_capacity;
    buff->data = ptr;
    return true;
}

bool char_buff_add(struct char_buff* buff, char c)
{
    if (buff->size == buff->capacity) {
        if (!char_buff_alloc(buff, 2 * buff->capacity)) {
            return false;
        }
    }

    buff->data[buff->size] = c;
    ++buff->size;
    return true;
}

void char_buff_memset(struct char_buff* buff, char c)
{
    memset(buff->data, c, buff->capacity);
}

struct char_buff char_buff_copy_span(char* begin, char* end)
{
    struct char_buff buff = char_buff_create();
    char_buff_alloc(&buff, end - begin);

    while (begin < end) {
        char_buff_add(&buff, *begin);
        ++begin;
    }

    return buff;
}

void char_buff_free(struct char_buff* buff)
{
    free(buff->data);
    buff->data = NULL;
    buff->capacity = 0;
    buff->size = 0;
}

struct ptr_buff ptr_buff_create()
{
    struct ptr_buff buff = { 0, 0, NULL };
    return buff;
}

bool ptr_buff_alloc(struct ptr_buff* buff, size_t new_capacity)
{
    void** ptr = realloc(buff->data, new_capacity * sizeof(void*));
    if (!ptr) {
        exit_with_message(1, "malloc error!!!\n");
        return false;
    }

    buff->capacity = new_capacity;
    buff->data = ptr;
    return true;
}

bool ptr_buff_add(struct ptr_buff* buff, void* ptr)
{
    if (buff->size == buff->capacity) {
        if (!ptr_buff_alloc(buff, 2 * buff->capacity)) {
            return false;
        }
    }

    buff->data[buff->size] = ptr;
    ++buff->size;
    return true;
}

void ptr_buff_free(struct ptr_buff* buff)
{
    free(buff->data);
    buff->data = NULL;
    buff->capacity = 0;
    buff->size = 0;
}

struct uint64_buff uint64_buff_create()
{
    struct uint64_buff buff = {
        0, 0,
        NULL
    };

    return buff;
}

bool uint64_buff_alloc(struct uint64_buff* buff, size_t new_capacity)
{
    uint64_t* ptr = realloc(buff->data, new_capacity * sizeof(uint64_t));
    if (!ptr) {
        exit_with_message(1, "malloc error!!!\n");
        return false;
    }

    buff->capacity = new_capacity;
    buff->data = ptr;
    return true;
}

bool uint64_buff_add(struct uint64_buff* buff, uint64_t num)
{
    if (buff->size == buff->capacity) {
        if (!uint64_buff_alloc(buff, 2 * buff->capacity)) {
            return false;
        }
    }

    buff->data[buff->size] = num;
    ++buff->size;
    return true;
}

void uint64_buff_erase(struct uint64_buff* buff, uint64_t index)
{
    buff->data[index] = buff->data[buff->size - 1];
    --buff->size;
}

void uint64_buff_free(struct uint64_buff* buff)
{
    free(buff->data);
    buff->data = NULL;
    buff->capacity = 0;
    buff->size = 0;
}