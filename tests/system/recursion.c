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

int fact(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * fact(n - 1);
}

int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

int fact_tail(int n, int acc) {
    if (n <= 1) {
        return acc;
    }
    return fact_tail(n - 1, acc * n);
}

int main() {
    int factorial = fact(8);
    int fibonacci = fib(15);
    int facttail = fact_tail(5, 5);

    print("Factorial of 8 is: ");
    print_int(factorial);
    print("\n");

    print("Fibonacci of 15 is: ");
    print_int(fibonacci);
    print("\n");

    print("Factorial tail of 5 and 5 is: ");
    print_int(facttail);
    print("\n");

    return 0;
}
