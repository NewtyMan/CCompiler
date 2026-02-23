#include "minic.h"

int strlen(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {}
    return i;
}

int strcmp(char *s1, char *s2) {
    int i;

    if (strlen(s1) != strlen(s2)) {
        return 0;
    }

    for (i = 0; s1[i] != '\0'; i++) {
        if (s1[i] != s2[i]) {
            return 0;
        }
    }

    return 1;
}

void ascii_sum(char *a, char *b, char *result) {
    int len_a = strlen(a) - 1;
    int len_b = strlen(b) - 1;
    int carry = 0;
    char ascii_sum[64];
    int length, i;

    for (i = 0; len_a >= 0 || len_b >= 0; i++, len_a--, len_b--) {
        int da = len_a >= 0 ? a[len_a] - '0' : 0;
        int db = len_b >= 0 ? b[len_b] - '0' : 0;

        int sum = da + db + carry;
        carry = sum / 10;

        ascii_sum[i] = (sum % 10) + '0';
    }

    if (carry != 0) {
        ascii_sum[i++] = carry + '0';
    }
    ascii_sum[i] = '\0';

    length = i;
    for (i = 0; i < length; i++) {
        result[i] = ascii_sum[length - i - 1];
    }
    result[i] = '\0';
}

char is_invalid_product_id_part2(char *product_id) {
    int length = strlen(product_id);
    char pattern[32];
    int i, j;

    for (i = 0; i < length / 2; i++) {
        char match_failed;

        if (length % (i + 1) != 0) {
            continue;
        }

        for (j = 0; j <= i; j++) {
            pattern[j] = product_id[j];
        }

        match_failed = 0;
        for (j = 0; j < length; j++) {
            if (pattern[j % (i + 1)] != product_id[j]) {
                match_failed = 1;
                break;
            }
        }

        if (match_failed == 0) {
            return 1;
        }
    }

    return 0;
}

char is_invalid_product_id_part1(char *product_id) {
    int length = strlen(product_id);
    int i, half;

    if (length % 2 != 0) {
        return 0;
    }

    half = length / 2;
    for (i = 0; i < half; i++) {
        if (product_id[i] != product_id[i + half]) {
            return 0;
        }
    }

    return 1;
}

void sum_invalid_ids(char *start, char *end, char *result_part1, char *result_part2) {
    ascii_sum(end, "1", end);
    while (strcmp(start, end) != 1) {
        if (is_invalid_product_id_part1(start)) {
            ascii_sum(result_part1, start, result_part1);
        }
        if (is_invalid_product_id_part2(start)) {
            ascii_sum(result_part2, start, result_part2);
        }
        ascii_sum(start, "1", start);
    }
}

int main() {
    char result_part1[64] = {'0', '\0'};
    char result_part2[64] = {'0', '\0'};
    char buffer[64], start[64], end[64];

    int bytes_read, i, index = 0;
    char direction = 'L';

    int fd = open("day2.txt", O_RDONLY, 0644);
    if (fd < 0) {
        write(STDOUT_FILENO, "Cannot open file!\n", 19);
        return 1;
    }

    do {
        bytes_read = read(fd, buffer, 64);
        for (i = 0; i < bytes_read; i++) {
            char c = buffer[i];
            if (c == '-') {
                start[index] = '\0';
                direction = 'R';
                index = 0;
            } else if (c >= '0' && c <= '9') {
                if (direction == 'L') {
                    start[index++] = c;
                } else {
                    end[index++] = c;
                }
            } else if (c == ',' || c == '\n') {
                end[index] = '\0';
                direction = 'L';
                index = 0;
                sum_invalid_ids(start, end, result_part1, result_part2);
            }
        }
    } while (bytes_read > 0);

    write(STDOUT_FILENO, "Puzzle result (part 1): ", 24);
    write(STDOUT_FILENO, result_part1, strlen(result_part1));
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "Puzzle result (part 2): ", 24);
    write(STDOUT_FILENO, result_part2, strlen(result_part2));
    write(STDOUT_FILENO, "\n", 1);

    close(fd);
    return 0;
}
