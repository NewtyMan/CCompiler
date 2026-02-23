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

void solve_dial_position_part1(char direction, int *dial_position, int dist, int *result) {
    if (direction == 'L') {
        *dial_position = *dial_position - dist;
    } else if (direction == 'R') {
        *dial_position = *dial_position + dist;
    }

    *dial_position = *dial_position % 100;
    if (*dial_position < 0) {
        *dial_position = *dial_position + 100;
    }

    if (*dial_position == 0) {
        *result = *result + 1;
    }
}

void solve_dial_position_part2(char direction, int *dial_position, int dist, int *result) {
    int i;
    for (i = 0; i < dist; i++) {
        if (direction == 'L') {
            *dial_position = *dial_position - 1;
        } else if (direction == 'R') {
            *dial_position = *dial_position + 1;
        }

        *dial_position = (*dial_position + 100) % 100;

        if (*dial_position == 0) {
            *result = *result + 1;
        }
    }
}

int main() {
    char buffer[4096], print_buffer[32];
    int fd, dist = 0, n = 0;

    int dial_position_part1 = 50, dial_position_part2 = 50;
    int result_part1 = 0, result_part2 = 0;
    char direction = 'L';

    fd = open("day1.txt", O_RDONLY, 0644);
    if (fd < 0) {
        write(STDOUT_FILENO, "Error opening file\n", 20);
        return 1;
    }

    do {
        int i;

        n = read(fd, buffer, 4096);
        for (i = 0; i < n; i++) {
            char c = buffer[i];

            if (c == 'L' || c == 'R') {
                direction = c;
                dist = 0;
            } else if (c >= '0' && c <= '9') {
                dist = dist * 10 + (c - '0');
            } else if (c == '\n') {
                solve_dial_position_part1(direction, &dial_position_part1, dist, &result_part1);
                solve_dial_position_part2(direction, &dial_position_part2, dist, &result_part2);
            }
        }
    } while (n > 0);


    itoa(result_part1, print_buffer);
    write(STDOUT_FILENO, "Puzzle result (part 1): ", 24);
    write(STDOUT_FILENO, print_buffer, strlen(print_buffer));
    write(STDOUT_FILENO, "\n", 1);

    itoa(result_part2, print_buffer);
    write(STDOUT_FILENO, "Puzzle result (part 2): ", 24);
    write(STDOUT_FILENO, print_buffer, strlen(print_buffer));
    write(STDOUT_FILENO, "\n", 1);

    close(fd);
    return 0;
}
