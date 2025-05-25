/*
 * Author: canetizen
 * Created on Sun May 25 2025
 * Description: MPI implementation for removing zeros from an array.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int rank, size;

    const int n = 10;
    int data[10] = {5, 4, 0, 0, 0, 1, 0, 0, 2, 0};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0 && n % size != 0) {
        printf("Error: Array size must be divisible by number of processes.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int chunk_size = n / size;
    int* local_chunk = (int*)malloc(chunk_size * sizeof(int));

    MPI_Scatter(data, chunk_size, MPI_INT,
                local_chunk, chunk_size, MPI_INT,
                0, MPI_COMM_WORLD);

    // Each process builds fixed-size buffer (max chunk_size pairs)
    int* sendbuf = (int*)malloc(chunk_size * 2 * sizeof(int));
    for (int i = 0; i < chunk_size * 2; i++)
        sendbuf[i] = -1; // mark as empty

    int pair_index = 0;
    for (int i = 0; i < chunk_size; i++) {
        if (local_chunk[i] != 0) {
            int global_index = rank * chunk_size + i;
            sendbuf[2 * pair_index]     = global_index;
            sendbuf[2 * pair_index + 1] = local_chunk[i];
            pair_index++;
        }
    }

    // Gather all fixed-size buffers
    int* recvbuf = NULL;
    if (rank == 0)
        recvbuf = (int*)malloc(size * chunk_size * 2 * sizeof(int));

    MPI_Gather(sendbuf, chunk_size * 2, MPI_INT,
               recvbuf, chunk_size * 2, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Index-Value pairs (non-zero elements):\n");
        for (int i = 0; i < size * chunk_size; i++) {
            int index = recvbuf[2 * i];
            int value = recvbuf[2 * i + 1];
            if (index != -1) {
                printf("%d,%d ", index, value);
            }
        }
        printf("\n");
        free(recvbuf);
    }

    free(local_chunk);
    free(sendbuf);
    MPI_Finalize();
    return 0;
}
