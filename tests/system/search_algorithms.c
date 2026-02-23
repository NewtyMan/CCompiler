#include "minic.h"

/*****************************
 *     UTILITY FUNCTIONS     *
 *****************************/

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

void print_array(int *arr, int n) {
    int i;
    write(STDOUT_FILENO, "[", 1);
    for (i = 0; i < n; i++) {
        char buffer[32];
        itoa(arr[i], buffer);
        write(STDOUT_FILENO, buffer, strlen(buffer));
        if (i < n - 1) {
            write(STDOUT_FILENO, ", ", 2);
        }
    }
    write(STDOUT_FILENO, "]", 1);
}

/*****************************
 *     SEARCH ALGORITHMS     *
 *****************************/

int linear_search(int *arr, int n, int key) {
    int i;
    for (i = 0; i < n; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return -1;
}

int binary_search(int *arr, int n, int key) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid] == key) {
            return mid;
        }
        if (arr[mid] < key) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

int interpolation_search(int *arr, int lo, int hi, int x) {
    int pos;
    if (lo <= hi && x >= arr[lo] && x <= arr[hi]) {
        pos = lo + (hi - lo) / (arr[hi] - arr[lo]) * (x - arr[lo]);
        if (arr[pos] == x) {
            return pos;
        }
        if (arr[pos] < x) {
            return interpolation_search(arr, pos + 1, hi, x);
        }
        if (arr[pos] > x) {
            return interpolation_search(arr, lo, pos - 1, x);
        }
    }
    return -1;
}

int main() {
    int n = 10;
    int key = 85;
    int x[10] = {-50, -23, 0, 5, 18, 32, 55, 60, 85, 99};
    int index;
    char buffer[32];

    write(STDOUT_FILENO, "Array: ", 7);
    print_array(x, n);
    write(STDOUT_FILENO, "\n", 1);

    index = linear_search(x, n, key);
    itoa(index, buffer);
    write(STDOUT_FILENO, "Linear search index: ", 21);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    index = binary_search(x, n, key);
    itoa(index, buffer);
    write(STDOUT_FILENO, "Binary search index: ", 21);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    index = interpolation_search(x, 0, n - 1, key);
    itoa(index, buffer);
    write(STDOUT_FILENO, "Interpolation search index: ", 28);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}
