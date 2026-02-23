#include "minic.h"

int strlen(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {}
    return i;
}

void itoa(int num, char *dst) {
    int index = 0;
    int start = 0;
    int i, j;

    if (num == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return;
    }

    if (num < 0) {
        dst[index++] = '-';
        num = -num;
        start = 1;
    }

    while (num != 0) {
        dst[index++] = (num % 10) + '0';
        num /= 10;
    }

    dst[index] = '\0';

    for (i = start, j = index - 1; i < j; i++, j--) {
        char tmp = dst[i];
        dst[i] = dst[j];
        dst[j] = tmp;
    }
}

void ascii_sum(char *a, char *b, char *result) {
    int len_a = strlen(a) - 1;
    int len_b = strlen(b) - 1;
    int carry = 0;
    char ascii_sum[64];
    int i, length;

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

int calculate_joltage_part1(char *battery_bank) {
    int first_max_idx = 0;
    int first_max = 0, second_max = 0;
    int length = strlen(battery_bank);
    int i;

    for (i = 0; i < length - 1; i++) {
        if (battery_bank[i] - '0' > first_max) {
            first_max_idx = i;
            first_max = battery_bank[i] - '0';
        }
    }

    for (i = first_max_idx + 1; i < length; i++) {
        if (battery_bank[i] - '0' > second_max) {
            second_max = battery_bank[i] - '0';
        }
    }

    return first_max * 10 + second_max;
}

void calculate_joltage_part2(char *battery_bank, char *result) {
    char max_joltage[13], max_joltage_index[13];
    int length = strlen(battery_bank);
    int i, j;

    for (i = 0; i < 13; i++) {
        max_joltage[i] = 0;
        max_joltage_index[i] = 0;
    }

    for (j = 0; j < 12; j++) {
        for (i = j == 0 ? 0 : max_joltage_index[j-1] + 1; i < length - 12 + j + 1; i++) {
            if (battery_bank[i] > max_joltage[j]) {
                max_joltage_index[j] = i;
                max_joltage[j] = battery_bank[i];
            }
        }
    }

    ascii_sum(result, max_joltage, result);
}

int main() {
    char battery_bank[1024];
    char buffer[1024], print_buffer[32];

    int battery_bank_index = 0, bytes_read;

    int joltage_part1 = 0;
    char joltage_part2[32] = {'0', '\0'};

    int fd = open("day3.txt", O_RDONLY, 00700);
    if (fd < 0) {
        write(STDOUT_FILENO, "Error opening file\n", 20);
        return 1;
    }

    do {
        int i;

        bytes_read = read(fd, buffer, 1024);
        for (i = 0; i < bytes_read; i++) {
            if (buffer[i] >= '0' && buffer[i] <= '9') {
                battery_bank[battery_bank_index++] = buffer[i];
            } else if (battery_bank_index > 0) {
                battery_bank[battery_bank_index] = '\0';
                battery_bank_index = 0;

                joltage_part1 += calculate_joltage_part1(battery_bank);
                calculate_joltage_part2(battery_bank, joltage_part2);
            }
        }
    } while (bytes_read > 0);

    itoa(joltage_part1, print_buffer);
    write(STDOUT_FILENO, "Puzzle result (part 1): ", 24);
    write(STDOUT_FILENO, print_buffer, strlen(print_buffer));
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "Puzzle result (part 2): ", 24);
    write(STDOUT_FILENO, joltage_part2, strlen(joltage_part2));
    write(STDOUT_FILENO, "\n", 1);

    close(fd);
    return 0;
}
