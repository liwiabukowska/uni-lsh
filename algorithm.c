#include "algorithm.h"

char* find_char(char* const begin, char* const end, char c)
{
    char* ptr = begin;
    while (ptr < end) {
        if (*ptr == c) {
            return ptr;
        }

        ++ptr;
    }

    return end;
}

char* find_substr(char* const begin, char* const end, char* const substr)
{
    char* position = begin;
    while (position < end) {
        char* i = position;
        char* j = substr;
        while (i < end && *i == *j) {
            ++i;
            ++j;
        }
        if (i < end && *j == '\0') {
            return position;
        }

        ++position;
    }

    return end;
}