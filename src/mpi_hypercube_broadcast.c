/*
 * Author: canetizen
 * Created on Sun May 18 2025
 * Description: MPI implementation for one-to-all broadcast on hypercube.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

void hypercube_broadcast(int d, int my_id, int source, int *X, int n) {
    int virtual_id = my_id ^ source;
    int mask = (1 << d) - 1;

    for (int i = d - 1; i >= 0; i--) {
        mask = mask ^ (1 << i); // Set bit i of mask to 0
        if ((virtual_id & mask) == 0) {
            if ((virtual_id & (1 << i)) == 0) {
                int virtual_dest = virtual_id ^ (1 << i);
                int physical_dest = virtual_dest ^ source;
                printf("Rank %d: Sending message to %d at step %d\n", my_id, physical_dest, i);
                MPI_Send(X, n, MPI_INT, physical_dest, 0, MPI_COMM_WORLD);
            } else {
                int virtual_source = virtual_id ^ (1 << i);
                int physical_source = virtual_source ^ source;
                printf("Rank %d: Receiving message from %d at step %d\n", my_id, physical_source, i);
                MPI_Recv(X, n, MPI_INT, physical_source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}

int main(int argc, char **argv) {
    int d, source, n = 1, X[1];
    MPI_Init(&argc, &argv);

    int my_id, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (argc < 3) {
        if (my_id == 0) printf("Usage: %s <dimension d> <source node>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    d = atoi(argv[1]);
    source = atoi(argv[2]);

    // Example: one integer message
    if (my_id == source)
        X[0] = 42; // Or any value you want to broadcast

    hypercube_broadcast(d, my_id, source, X, n);

    printf("Rank %d: Final value X = %d\n", my_id, X[0]);

    MPI_Finalize();
    return 0;
}
