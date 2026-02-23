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

int resolve_toiled_paper_part1(char *input_data, int rows, int columns, int remove_toilet_paper) {
    int result = 0;
    int x, y, i, j;
    for (y = 0; y < rows; y++) {
        for (x = 0; x < columns; x++) {
            int surrounding = 0;

            int current_idx = y * columns + x;
            if (input_data[current_idx] != '@') {
                continue;
            }

            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    int ny, nx;

                    if (i == 0 && j == 0) {
                        continue;
                    }

                    ny = y + i;
                    nx = x + j;

                    if (ny >= 0 && ny < rows && nx >= 0 && nx < columns) {
                        int adjacent_idx = ny * columns + nx;
                        if (input_data[adjacent_idx] == '@') {
                            surrounding++;
                        }
                    }
                }
            }

            if (surrounding < 4) {
                if (remove_toilet_paper == 1) {
                    input_data[current_idx] = '.';
                }
                result++;
            }
        }
    }
    return result;
}

int resolve_toiled_paper_part2(char *input_data, int rows, int columns) {
    int intermediate_result = 0;
    int result = 0;

    do {
        intermediate_result = resolve_toiled_paper_part1(input_data, rows, columns, 1);
        result += intermediate_result;
    } while (intermediate_result > 0);

    return result;
}

int main() {
    int buffer_idx = 0, bytes_read = 0, rows = 0, columns = 0;
    char buffer[4096], input_data[32768];
    int i, result_part1, result_part2;

    int fd = open("day4.txt", O_RDONLY, 00700);
    if (fd < 0) {
        write(STDOUT_FILENO, "Error opening file\n", 20);
        return 1;
    }

    do {
        int tmp_columns = 0;

        bytes_read = read(fd, buffer, 4096);
        for (i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                if (columns == 0) {
                    columns = tmp_columns;
                }
                rows++;
            } else if (buffer[i] == '.' || buffer[i] == '@') {
                input_data[buffer_idx++] = buffer[i];
                tmp_columns++;
            }
        }
    } while (bytes_read > 0);

    itoa(rows, buffer);
    write(STDOUT_FILENO, "Rows: ", 6);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    itoa(columns, buffer);
    write(STDOUT_FILENO, "Columns: ", 9);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    result_part1 = resolve_toiled_paper_part1(input_data, rows, columns, 0);
    result_part2 = resolve_toiled_paper_part2(input_data, rows, columns);

    itoa(result_part1, buffer);
    write(STDOUT_FILENO, "Puzzle result (part 1): ", 24);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    itoa(result_part2, buffer);
    write(STDOUT_FILENO, "Puzzle result (part 2): ", 24);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    close(fd);
    return 0;
}
