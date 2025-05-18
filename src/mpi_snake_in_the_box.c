/*
 * Author: canetizen
 * Created on Sun May 18 2025
 * Description: MPI implementation for the Snake-in-the-box problem.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

#define MAX_DIMENSION 10
#define TAG_WORK 1
#define TAG_RESULT 2
#define TAG_TERMINATE 3

typedef struct {
    int current_path[1 << MAX_DIMENSION];
    int path_length;
    int dimension;
    int start_vertex;
} WorkPackage;

typedef struct {
    int best_path[1 << MAX_DIMENSION];
    int path_length;
} Result;

// Check if two vertices differ by exactly one bit (are neighbors in hypercube)
bool are_neighbors(int a, int b) {
    int xor_result = a ^ b;
    return (xor_result & (xor_result - 1)) == 0 && xor_result != 0;
}

// Check if adding vertex v to path creates a chord
bool creates_chord(int path[], int path_length, int v) {
    // Check only against non-adjacent vertices in path
    for (int i = 0; i < path_length - 1; i++) {
        if (are_neighbors(path[i], v) && i < path_length - 2) {
            return true;
        }
    }
    return false;
}

// Check if vertex v is already in the path
bool in_path(int path[], int path_length, int v) {
    for (int i = 0; i < path_length; i++) {
        if (path[i] == v) {
            return true;
        }
    }
    return false;
}

// Print the binary representation of a vertex
void print_vertex(int v, int dimension) {
    for (int i = dimension - 1; i >= 0; i--) {
        printf("%d", (v >> i) & 1);
    }
}

// Extend the current path recursively (used by worker)
void extend_path(int path[], int path_length, int dimension, int max_depth, Result* best_result) {
    // Update best result if current path is longer
    if (path_length > best_result->path_length) {
        best_result->path_length = path_length;
        memcpy(best_result->best_path, path, path_length * sizeof(int));
    }
    
    // Stop if we've reached the maximum depth for this search
    if (path_length >= max_depth) {
        return;
    }
    
    int current = path[path_length - 1];
    int neighbor;
    
    // Try all possible neighbors
    for (int i = 0; i < dimension; i++) {
        neighbor = current ^ (1 << i);  // Flip one bit to get a neighbor
        
        // Check if this neighbor can be added
        if (!in_path(path, path_length, neighbor) && !creates_chord(path, path_length, neighbor)) {
            path[path_length] = neighbor;
            extend_path(path, path_length + 1, dimension, max_depth, best_result);
        }
    }
}

// Worker process function
void worker_process(int rank) {
    WorkPackage work;
    Result local_result, result_to_send;
    MPI_Status status;
    
    while (1) {
        // Receive work from farmer
        MPI_Recv(&work, sizeof(WorkPackage), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        // If received terminate signal, exit loop
        if (status.MPI_TAG == TAG_TERMINATE) {
            break;
        }
        
        // Initialize local result
        local_result.path_length = work.path_length;
        memcpy(local_result.best_path, work.current_path, work.path_length * sizeof(int));
        
        // Extend path from the current state
        extend_path(work.current_path, work.path_length, work.dimension, 1 << work.dimension, &local_result);
        
        // Send result back to farmer
        result_to_send = local_result;
        MPI_Send(&result_to_send, sizeof(Result), MPI_BYTE, 0, TAG_RESULT, MPI_COMM_WORLD);
    }
}

// Farmer process function
void farmer_process(int dimension, int num_procs) {
    WorkPackage work;
    Result global_best, received_result;
    MPI_Status status;
    int workers_available = num_procs - 1;
    int next_start_vertex = 0;
    const int total_vertices = 1 << dimension;
    
    // Initialize global best result
    global_best.path_length = 0;
    
    // Prepare initial work packages and send to workers
    for (int worker = 1; worker < num_procs && next_start_vertex < total_vertices; worker++, next_start_vertex++) {
        work.dimension = dimension;
        work.path_length = 1;
        work.current_path[0] = next_start_vertex;
        work.start_vertex = next_start_vertex;
        
        MPI_Send(&work, sizeof(WorkPackage), MPI_BYTE, worker, TAG_WORK, MPI_COMM_WORLD);
    }
    
    // Receive results and send more work until all starting vertices are processed
    while (workers_available > 0) {
        // Receive result from any worker
        MPI_Recv(&received_result, sizeof(Result), MPI_BYTE, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
        
        // Update global best if needed
        if (received_result.path_length > global_best.path_length) {
            global_best = received_result;
        }
        
        // If more work is available, send it to the worker
        if (next_start_vertex < total_vertices) {
            work.dimension = dimension;
            work.path_length = 1;
            work.current_path[0] = next_start_vertex;
            work.start_vertex = next_start_vertex;
            next_start_vertex++;
            
            MPI_Send(&work, sizeof(WorkPackage), MPI_BYTE, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);
        } else {
            // No more work, send terminate signal
            MPI_Send(NULL, 0, MPI_BYTE, status.MPI_SOURCE, TAG_TERMINATE, MPI_COMM_WORLD);
            workers_available--;
        }
    }
    
    // Print results
    printf("Dimension: %d\n", dimension);
    printf("Best path length: %d\n", global_best.path_length);
    printf("Best path: ");
    for (int i = 0; i < global_best.path_length; i++) {
        print_vertex(global_best.best_path[i], dimension);
        if (i < global_best.path_length - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int rank, num_procs, dimension = 4;  // Default dimension is 4
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    // Parse command line arguments
    if (argc > 1) {
        dimension = atoi(argv[1]);
        if (dimension <= 0 || dimension > MAX_DIMENSION) {
            if (rank == 0) {
                printf("Invalid dimension. Must be between 1 and %d.\n", MAX_DIMENSION);
            }
            MPI_Finalize();
            return 1;
        }
    }
    
    // Start timer
    double start_time = MPI_Wtime();
    
    if (rank == 0) {
        // Farmer process
        farmer_process(dimension, num_procs);
        
        // End timer and print execution time
        double end_time = MPI_Wtime();
        printf("Execution time: %.2f seconds\n", end_time - start_time);
    } else {
        // Worker process
        worker_process(rank);
    }
    
    MPI_Finalize();
    return 0;
}