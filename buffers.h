#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct char_buff {
    size_t capacity;
    size_t size;
    char* data;
};

struct char_buff char_buff_create();

bool char_buff_alloc(struct char_buff* buff, size_t new_capacity);

bool char_buff_add(struct char_buff* buff, char c);

void char_buff_memset(struct char_buff* buff, char c);

struct char_buff char_buff_copy_span(char* begin, char* end);

void char_buff_free(struct char_buff* buff);

struct ptr_buff {
    size_t capacity;
    size_t size;
    void** data;
};

struct ptr_buff ptr_buff_create();

bool ptr_buff_alloc(struct ptr_buff* buff, size_t new_capacity);

bool ptr_buff_add(struct ptr_buff* buff, void* ptr);

// nie zwalnia pamieci prowadzacej z pointerow
void ptr_buff_free(struct ptr_buff* buff);

struct uint64_buff {
    size_t capacity;
    size_t size;
    uint64_t* data;
};

struct uint64_buff uint64_buff_create();

bool uint64_buff_alloc(struct uint64_buff* buff, size_t new_capacity);

bool uint64_buff_add(struct uint64_buff* buff, uint64_t ptr);

void uint64_buff_erase(struct uint64_buff* buff, uint64_t index);

void uint64_buff_free(struct uint64_buff* buff);