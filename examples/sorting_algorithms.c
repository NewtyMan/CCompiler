#include "minic.h"

/*************************
 *   SORTING ALGORITHMS   *
 *************************/

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    int j;
    for (j = low; j <= high - 1; j = j + 1) {
        if (arr[j] < pivot) {
            i = i + 1;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i+1], &arr[high]);
    return i + 1;
}

void quick_sort(int *arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quick_sort(arr, low, pi-1);
        quick_sort(arr, pi+1, high);
    }
}

void insertion_sort(int *arr, int n) {
    int i;
    for (i = 1; i < n; i++) {
        int tmp = arr[i];
        int j = i - 1;
        while (tmp < arr[j] && j >= 0) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = tmp;
    }
}

void selection_sort(int *arr, int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        int temp = arr[i];
        arr[i] = arr[min_idx];
        arr[min_idx] = temp;
    }
}

void bubble_sort(int *arr, int n) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/*************************
 *   UTILITY FUNCTIONS   *
 *************************/

void itoa(int num, char *dst) {
    int index = 0;
    int start = 0;

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

    int i, j;
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
    write(STDOUT_FILENO, "[", 1);
    int i;
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

void clone_array(int *dst, int *src, int n) {
    int i;
    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

void fill_array(int *dst, int n) {
    int i;
    for (i = 0; i < n; i++) {
        int tmp = 0;
        getrandom(&tmp, 4, 0);
        dst[i] = tmp % 100;
    }
}

/************************
 *         MAIN         *
 ************************/

int main() {
    int n = 10;
    int x1[10], x2[10], x3[10], x4[10];

    fill_array(x1, n);
    clone_array(x2, x1, n);
    clone_array(x3, x1, n);
    clone_array(x4, x1, n);

    write(STDOUT_FILENO, "Unsorted array:", 15);
    print_array(x1, n);
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "Bubble sort: ", 13);
    bubble_sort(x1, n);
    print_array(x1, n);
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "Insertion sort: ", 16);
    insertion_sort(x2, n);
    print_array(x2, n);
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "Selection sort: ", 16);
    selection_sort(x3, n);
    print_array(x3, n);
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "Quick sort: ", 12);
    quick_sort(x4, 0, n - 1);
    print_array(x4, n);
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}
