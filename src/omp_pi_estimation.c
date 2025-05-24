/*
 * Author: canetizen
 * Created on Sat May 24 2025
 * Description: OpenMP implementation for parallel pi estimation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    // Check if user provided the number of steps
    if (argc != 2) {
        printf("Usage: %s <num_steps>\n", argv[0]);
        return 1;
    }

    // Convert input argument to long
    long num_steps = atol(argv[1]);
    if (num_steps <= 0) {
        printf("Error: num_steps must be a positive integer.\n");
        return 1;
    }

    double step, pi, sum = 0.0;
    step = 1.0 / (double) num_steps;

    // Start of parallel region
    #pragma omp parallel
    {
        double x;

        // Each thread computes a portion of the sum
        #pragma omp for reduction(+:sum)
        for (int i = 0; i < num_steps; i++) {
            x = (i + 0.5) * step;                  // Midpoint of interval
            sum += 4.0 / (1.0 + x * x);            // Add to local sum
        }
    }

    // Final result
    pi = step * sum;
    printf("Approximate value of pi: %.15f\n", pi);

    return 0;
}
