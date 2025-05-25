/*
 * Author: canetizen
 * Created on Fri May 23 2025
 * Description: MPI implementation of the naive string matching algorithm.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // For INT_MAX sentinel value

/**
 * Computes the start index and length of the chunk assigned to a specific rank.
 * The last rank is assigned all remaining characters to ensure full text coverage.
 */
void compute_chunk_info(int rank, int size, int text_len, int pattern_len, int* start, int* length) {
    int chunk_size = text_len / size;
    *start = rank * chunk_size;
    if (rank == size - 1) {
        *length = text_len - *start;
    } else {
        *length = chunk_size + pattern_len - 1; // overlap to handle boundary matches
    }
}

/**
 * Sends chunk metadata (start index and length) from the master process to all worker processes.
 */
void send_chunk_info(int size, int text_len, int pattern_len) {
    for (int i = 1; i < size; i++) {
        int start, length;
        compute_chunk_info(i, size, text_len, pattern_len, &start, &length);
        int info[2] = {start, length};
        MPI_Send(info, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
}

/**
 * Receives assigned chunk metadata from the master process.
 */
void receive_chunk_info(int* start, int* length) {
    int info[2];
    MPI_Recv(info, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *start = info[0];
    *length = info[1];
}

/**
 * Extracts the assigned substring chunk from the full input text.
 */
void extract_chunk(char* chunk, const char* text, int start, int length) {
    strncpy(chunk, text + start, length);
    chunk[length] = '\0';
}

/**
 * Entry point: performs parallel string matching using naive algorithm.
 * Distributes the workload among MPI processes and finds the earliest match.
 */
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // Get current process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size);   // Get total number of processes

    char* text = NULL;
    char* pattern = NULL;
    int text_len = 0, pattern_len = 0;

    // Master process parses input arguments
    if (rank == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s <text> <pattern>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        text = argv[1];
        pattern = argv[2];
        text_len = strlen(text);
        pattern_len = strlen(pattern);
    }

    // Broadcast lengths of text and pattern to all processes
    MPI_Bcast(&text_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&pattern_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast the pattern string to all processes
    if (rank != 0) pattern = (char*)malloc(pattern_len + 1);
    MPI_Bcast(pattern, pattern_len + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Broadcast the entire input text to all processes
    if (rank != 0) text = (char*)malloc(text_len + 1);
    MPI_Bcast(text, text_len + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Determine chunk assignment for each process
    int start, length;
    if (rank == 0) {
        compute_chunk_info(rank, size, text_len, pattern_len, &start, &length);
        send_chunk_info(size, text_len, pattern_len);
    } else {
        receive_chunk_info(&start, &length);
    }

    // Extract the relevant text chunk
    char* chunk = (char*)malloc(length + 1);
    extract_chunk(chunk, text, start, length);

    /**
     * Perform naive string matching within the assigned chunk.
     * Initialize match_index with INT_MAX to serve as sentinel for 'no match'.
     */
    int match_index = INT_MAX;
    for (int i = 0; i <= length - pattern_len; i++) {
        if (strncmp(&chunk[i], pattern, pattern_len) == 0) {
            match_index = start + i;
            break;
        }
    }

    /**
     * Use MPI_Allreduce with MPI_MIN to find the global earliest match.
     * If no match was found in any process, INT_MAX is preserved.
     */
    int global_match_index;
    MPI_Allreduce(&match_index, &global_match_index, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

    // If this process found the earliest match, it prints the result
    if (match_index == global_match_index && global_match_index != INT_MAX) {
        printf("First match found by process %d at index %d\n", rank, match_index);
    }

    // If no match was found, master process prints message
    if (global_match_index == INT_MAX && rank == 0) {
        printf("No match found in the text.\n");
    }

    // Cleanup dynamic memory allocations
    free(chunk);
    if (rank != 0) {
        free(pattern);
        free(text);
    }

    MPI_Finalize();
    return 0;
}
