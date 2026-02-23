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
    quick_sort(x1, 0, n - 1);

    return 0;
}
