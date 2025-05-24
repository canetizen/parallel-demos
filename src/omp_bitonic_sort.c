/*
 * Author: canetizen
 * Created on Sat May 23 2025
 * Description: Iterative Bitonic Sort using OpenMP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Compare and swap two elements if they are out of order based on dir (1 = ascending, 0 = descending)
void compare_and_swap(int* arr, int i, int j, int dir) {
    if ((dir && arr[i] > arr[j]) || (!dir && arr[i] < arr[j])) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// Iterative Bitonic Sort
void bitonic_sort_iterative(int* arr, int n, int dir) {
    for (int size = 2; size <= n; size <<= 1) {
        for (int stride = size >> 1; stride > 0; stride >>= 1) {
            // Parallel compare and swap within each stage
            #pragma omp parallel for
            for (int i = 0; i < n; i++) {
                int j = i ^ stride;  // Paired index using XOR trick
                if (j > i) {
                    int same_block = (i & size) == (j & size); // Ensure both in same block
                    int direction = ((i & size) == 0) ? dir : !dir; // Asc/desc based on bit
                    if (same_block) {
                        compare_and_swap(arr, i, j, direction);
                    }
                }
            }
        }
    }
}

int main() {
    // Example array (must be power of 2 in length)
    int arr[] = {10, 30, 11, 20, 4, 330, 21, 110};
    int n = sizeof(arr) / sizeof(arr[0]);

    // Bitonic sort in ascending order
    bitonic_sort_iterative(arr, n, 1);

    // Output result
    printf("Sorted array:\n");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}
