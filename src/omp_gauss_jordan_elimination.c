/*
 * Author: canetizen
 * Created on Sat May 24 2025
 * Description: OpenMP implementation for Gauss-Jordan Elimination.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 4  // Matrix size (NxN)

// Helper function to print the augmented matrix [A | b]
void print_matrix(double A[N][N], double b[N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%8.4f ", A[i][j]);
        printf("| %8.4f\n", b[i]);
    }
    printf("\n");
}

int main() {
    // Augmented matrix A|b
    double A[N][N] = {
        {2, 1, -1, -3},
        {-3, -1, 2, -1},
        {-2, 1, 2, -3},
        {1, 2, -1, -1}
    };

    double b[N] = {8, -11, -3, 4};  // Right-hand side vector
    double x[N]; // Solution vector

    printf("Initial Matrix:\n");
    print_matrix(A, b);

    // Gauss-Jordan Elimination
    // Outer loop over each pivot position (row and column index)
    for (int k = 0; k < N; k++) {
        // Step 1: Normalize the pivot row (row k) so that A[k][k] becomes 1

        double pivot = A[k][k];

        // 'j' iterates over columns in row k
        // Parallelize normalization of the pivot row
        #pragma omp parallel for
        for (int j = 0; j < N; j++) {
            A[k][j] /= pivot;
        }

        // Normalize the corresponding value in vector b (scalar operation)
        b[k] /= pivot;

        // Step 2: Eliminate all other entries in column k
        // so that A[i][k] becomes 0 for i != k

        // 'i' iterates over rows (excluding the pivot row)
        // Inner loop uses 'j' again to go across columns
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            if (i != k) {
                double factor = A[i][k];  // Multiplier to eliminate A[i][k]
                for (int j = 0; j < N; j++) {
                    A[i][j] -= factor * A[k][j];  // Subtract scaled pivot row
                }
                b[i] -= factor * b[k];  // Update corresponding value in b
            }
        }
    }

    // Extract the final solution: since A is now identity, x = b
    #pragma omp parallel for
    for (int i = 0; i < N; i++)
        x[i] = b[i];

    // Output the solution vector
    printf("Solution vector (x):\n");
    for (int i = 0; i < N; i++)
        printf("x[%d] = %8.4f\n", i, x[i]);

    return 0;
}
