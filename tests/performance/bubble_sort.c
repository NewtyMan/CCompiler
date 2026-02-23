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

void fill_array(int *dst, int n) {
    int i;
    for (i = 0; i < n; i++) {
        int tmp = 0;
        getrandom(&tmp, 4, 0);
        dst[i] = tmp % 100;
    }
}

int main() {
    int n = 1000000;
    int x1[1000000];

    fill_array(x1, n);
    bubble_sort(x1, n);

    return 0;
}
