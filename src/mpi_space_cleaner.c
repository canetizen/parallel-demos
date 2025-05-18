/*
 * Author: canetizen
 * Created on Sun May 18 2025
 * Description: MPI implementation for cleaning spaces from strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TAG_WORK 1
#define TAG_RESULT 2
#define TAG_TERMINATE 3
#define MAX_STRING_LEN 1024

typedef struct {
    int chunk_len;
    char chunk[MAX_STRING_LEN];
} WorkPackage;

typedef struct {
    int cleaned_len;
    char cleaned[MAX_STRING_LEN];
} Result;

void remove_spaces(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i]; i++) {
        if (input[i] != ' ') {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

void worker_process() {
    MPI_Status status;
    WorkPackage work;
    Result result;

    while (1) {
        MPI_Recv(&work, sizeof(WorkPackage), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_TERMINATE)
            break;

        remove_spaces(work.chunk, result.cleaned);
        result.cleaned_len = strlen(result.cleaned);

        MPI_Send(&result, sizeof(Result), MPI_BYTE, 0, TAG_RESULT, MPI_COMM_WORLD);
    }
}

void farmer_process(const char* input_str, int num_procs) {
    int input_len = strlen(input_str);
    int base = input_len / (num_procs - 1);
    int rem = input_len % (num_procs - 1);
    int offset = 0;

    Result final_result;
    final_result.cleaned_len = 0;
    final_result.cleaned[0] = '\0';

    WorkPackage work;
    Result result;
    MPI_Status status;

    // Send initial work to workers
    for (int i = 1; i < num_procs; i++) {
        int len = base + (i <= rem ? 1 : 0);
        strncpy(work.chunk, input_str + offset, len);
        work.chunk[len] = '\0';
        work.chunk_len = len;

        MPI_Send(&work, sizeof(WorkPackage), MPI_BYTE, i, TAG_WORK, MPI_COMM_WORLD);
        offset += len;
    }

    // Receive results and append
    for (int i = 1; i < num_procs; i++) {
        MPI_Recv(&result, sizeof(Result), MPI_BYTE, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
        strcat(final_result.cleaned, result.cleaned);
        final_result.cleaned_len += result.cleaned_len;
    }

    // Send termination signal
    for (int i = 1; i < num_procs; i++) {
        MPI_Send(NULL, 0, MPI_BYTE, i, TAG_TERMINATE, MPI_COMM_WORLD);
    }

    printf("Original string: \"%s\"\n", input_str);
    printf("Cleaned: \"%s\"\n", final_result.cleaned);
}

int main(int argc, char* argv[]) {
    int rank, num_procs;
    const char* input_str = "H E L L O W O R L D";

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0) {
        farmer_process(input_str, num_procs);
    } else {
        worker_process();
    }

    MPI_Finalize();
    return 0;
}
