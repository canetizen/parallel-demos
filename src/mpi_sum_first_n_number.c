/*
 * Author: canetizen
 * Created on Sun May 25 2025
 * Description: MPI implementation for sum of the first n number.
 */ 

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int info[2]; // [0] = start, [1] = chunk

    if (rank == 0) {
        if (argc != 2) {
            printf("Usage: %s <n>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int n = atoi(argv[1]);
        if (n % size != 0) {
            printf("Error: n must be divisible by the number of processes.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int chunk = n / size;

        // Prepare start/chunk pairs for each process
        int* sendbuf = (int*)malloc(2 * size * sizeof(int));
        for (int i = 0; i < size; i++) {
            sendbuf[2 * i]     = i * chunk + 1; // start
            sendbuf[2 * i + 1] = chunk;         // chunk
        }

        // Scatter [start, chunk] pairs
        MPI_Scatter(sendbuf, 2, MPI_INT,
                    info, 2, MPI_INT,
                    0, MPI_COMM_WORLD);

        free(sendbuf);
    } else {
        // Receive [start, chunk]
        MPI_Scatter(NULL, 2, MPI_INT,
                    info, 2, MPI_INT,
                    0, MPI_COMM_WORLD);
    }

    int my_start = info[0];
    int chunk = info[1];
    int my_end = my_start + chunk - 1;

    // Compute local sum
    int local_sum = 0;
    for (int i = my_start; i <= my_end; i++) {
        local_sum += i;
    }

    // Gather local sums to root
    int* local_sums = NULL;
    if (rank == 0) {
        local_sums = (int*)malloc(size * sizeof(int));
    }

    MPI_Gather(&local_sum, 1, MPI_INT,
               local_sums, 1, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        int total_sum = 0;
        for (int i = 0; i < size; i++) {
            total_sum += local_sums[i];
        }
        printf("Sum of the first %d positive integers is %d\n", chunk * size, total_sum);
        free(local_sums);
    }

    MPI_Finalize();
    return 0;
}
