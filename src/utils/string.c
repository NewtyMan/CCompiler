#include <utils/string.h>
#include <stdlib.h>

int string_cmp(char *a, char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}

int string_len(char *s) {
    int len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

void string_cpy(char *dst, char *src) {
    while (*src != '\0') {
        *dst++ = *src++;
    }
    *dst = '\0';
}

int power(int base, int exponent) {
    int result = 1;
    while (exponent > 0) {
        result *= base;
        exponent--;
    }
    return result;
}

int get_hex_char(char hex) {
    switch (hex) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            return 0;
    }
}

int string_atoi(char *str, int base) {
    int str_len = string_len(str);
    int result = 0;

    int i;
    for (i = 0; i < str_len; i++) {
        if (base == 16) {
            result += get_hex_char(str[i]) * power(16, str_len - i - 1);
            continue;
        }

        result += (str[i] - '0') * power(base, str_len - i - 1);
    }

    return result;
}

char* string_substr(char *str, int start, int end) {
    char *result = calloc(end - start + 2, sizeof(char));

    int i;
    for (i = start; i <= end; i++) {
        result[i-start] = str[i];
    }
    result[i-start] = '\0';

    return result;
}
