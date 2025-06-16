/*
 * Author: canetizen
 * Created on Fri May 23 2025
 * Description: Recursive Bitonic Sort using OpenMP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ASCENDING 1
#define DESCENDING 0

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void bitonic_merge(int *arr, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;

        #pragma omp parallel for
        for (int i = low; i < low + k; i++) {
            if ((dir == ASCENDING && arr[i] > arr[i + k]) || (dir == DESCENDING && arr[i] < arr[i + k])) {
                swap(&arr[i], &arr[i + k]);
            }
        }


        bitonic_merge(arr, low, k, dir);
        bitonic_merge(arr, low + k, k, dir);
    }

}

void bitonic_sort(int *arr, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;

        bitonic_sort(arr, low, k, ASCENDING);
        bitonic_sort(arr, low + k, k, DESCENDING);

        bitonic_merge(arr, low, cnt, dir);
    }
}

void parallel_bitonic_sort(int *arr, int n, int dir) {
    bitonic_sort(arr, 0, n, dir);
}

void print_array(int *arr, int n) {
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main() {
    int arr[] = {3, 7, 4, 8, 6, 2, 1, 5};
    int n = sizeof(arr) / sizeof(arr[0]);

    // n must be a power of 2
    if ((n & (n - 1)) != 0) {
        printf("Array size must be a power of 2.\n");
        return 1;
    }

    printf("Original array:\n");
    print_array(arr, n);

    parallel_bitonic_sort(arr, n, ASCENDING);

    printf("Sorted array:\n");
    print_array(arr, n);

    return 0;
}