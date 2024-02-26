#include "string_util.h"
#include <ctype.h>
#include <string.h>

void string_util_tolower(char *str) {
    for (int i = 0; str[i] != 0; i++) {
        str[i] = (char) tolower(str[i]);
    }
}

char *strzcpy(char *dest, const char *ori, size_t size) {
    char *ret = strncpy(dest, ori, size);
    ret[size] = 0;
    return ret;
}

size_t string_util_utf8_strlen(const char *str) {
    size_t count = 0;
    while (*str) {
        if ((*str & 0xC0) != 0x80) { // if it is a utf-8 code point
            count++;
        }
        str++;
    }
    return count;
}