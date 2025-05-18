/*
 * Author: canetizen
 * Created on Sun May 18 2025
 * Description: MPI implementation for all-to-one reduce on hypercube.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

void hypercube_reduce(int d, int my_id, int dest, int *X, int *sum, int n) {
    int mask = 0;
    for (int j = 0; j < n; j++)
        sum[j] = X[j];

    for (int i = 0; i < d; i++) {
        if ((my_id & mask) == 0) {
            if ((my_id & (1 << i)) != 0) {
                int msg_dest = my_id ^ (1 << i);
                printf("Rank %d: Sending sum to %d at step %d\n", my_id, msg_dest, i);
                MPI_Send(sum, n, MPI_INT, msg_dest, 0, MPI_COMM_WORLD);
            } else {
                int msg_source = my_id ^ (1 << i);
                int recv_buf[n];
                printf("Rank %d: Receiving sum from %d at step %d\n", my_id, msg_source, i);
                MPI_Recv(recv_buf, n, MPI_INT, msg_source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (int j = 0; j < n; j++)
                    sum[j] += recv_buf[j];
            }
        }
        mask = mask ^ (1 << i); // Set bit i of mask to 1
    }
}

int main(int argc, char **argv) {
    int d, dest, n = 1, X[1], sum[1];
    MPI_Init(&argc, &argv);

    int my_id, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (argc < 3) {
        if (my_id == 0) printf("Usage: %s <dimension d> <destination node>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    d = atoi(argv[1]);
    dest = atoi(argv[2]);

    // Example: each node contributes its own rank
    X[0] = my_id;

    hypercube_reduce(d, my_id, dest, X, sum, n);

    if (my_id == dest)
        printf("Rank %d: Final reduced sum = %d\n", my_id, sum[0]);

    MPI_Finalize();
    return 0;
}
