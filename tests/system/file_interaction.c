#include "minic.h"

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

int strlen(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {}
    return i;
}

void print(char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

void print_int(int num) {
    char buffer[32];
    itoa(num, buffer);
    print(buffer);
}

int main() {
    char *filename = "file_interaction.txt";
    char buffer[32];
    int vowels = 1;
    int closed, fd;

    fd = open(filename, O_RDONLY, 00700);
    if (fd < 0) {
        print("Failed to open input file, error code: ");
        print_int(fd);
        print("\n");
        return 1;
    }

    print("Successfully opened input file: ");
    print(filename);
    print("\n");

    while (read(fd, buffer, 32) > 0) {
        int i;
        for (i = 0; i < 32; i++) {
            switch (buffer[i]) {
                case 'a':
                case 'e':
                case 'i':
                case 'o':
                case 'u':
                    vowels++;
                    break;
                default:
                    break;
            }
        }
    }

    print("File contains: ");
    print_int(vowels);
    print(" vowels.\n");

    closed = close(fd);
    if (closed == -1) {
        print("Failed to close input file!\n");
        return 1;
    }

    print("Input file closed\n");
    return 0;
}
