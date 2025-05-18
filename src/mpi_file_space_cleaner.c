/*
 * Author: canetizen
 * Created on Sun May 18 2025
 * Description: MPI implementation for cleaning spaces from files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TAG_WORK_LEN     10
#define TAG_WORK_DATA    11
#define TAG_RESULT_LEN   20
#define TAG_RESULT_DATA  21
#define TAG_TERMINATE    99

void remove_spaces(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i]; i++) {
        if (input[i] != ' ' && input[i] != '\n') {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

char* read_input_file(const char* filename, int* out_len) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Dosya açılamadı");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    rewind(file);

    char* buffer = malloc((fsize + 1) * sizeof(char));
    if (!buffer) {
        perror("Bellek ayırılamadı");
        fclose(file);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1, fsize, file);
    if (read_bytes != fsize) {
        fprintf(stderr, "Dosya eksik okundu: %zu / %ld bayt\n", read_bytes, fsize);
        free(buffer);
        fclose(file);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return NULL;
    }

    buffer[fsize] = '\0';
    fclose(file);

    *out_len = (int)fsize;
    return buffer;
}

void worker_process() {
    MPI_Status status;

    while (1) {
        int chunk_len;

        MPI_Recv(&chunk_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == TAG_TERMINATE)
            break;

        char* chunk = malloc((chunk_len + 1) * sizeof(char));
        if (!chunk) {
            perror("chunk malloc failed");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        MPI_Recv(chunk, chunk_len, MPI_CHAR, 0, TAG_WORK_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        chunk[chunk_len] = '\0';

        char* cleaned = malloc((chunk_len + 1) * sizeof(char));
        if (!cleaned) {
            free(chunk);
            perror("cleaned malloc failed");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        remove_spaces(chunk, cleaned);
        int cleaned_len = strlen(cleaned);

        MPI_Send(&cleaned_len, 1, MPI_INT, 0, TAG_RESULT_LEN, MPI_COMM_WORLD);
        MPI_Send(cleaned, cleaned_len, MPI_CHAR, 0, TAG_RESULT_DATA, MPI_COMM_WORLD);

        free(chunk);
        free(cleaned);
    }
}

void farmer_process(const char* filename, int num_procs) {
    int input_len;
    char* input_str = read_input_file(filename, &input_len);
    if (!input_str) return;

    int base = input_len / (num_procs - 1);
    int rem = input_len % (num_procs - 1);
    int offset = 0;

    char* final_result = malloc((input_len + 1) * sizeof(char));
    if (!final_result) {
        perror("final_result malloc failed");
        free(input_str);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    final_result[0] = '\0';

    // Send chunks to workers
    for (int i = 1; i < num_procs; i++) {
        int len = base + (i <= rem ? 1 : 0);
        MPI_Send(&len, 1, MPI_INT, i, TAG_WORK_LEN, MPI_COMM_WORLD);
        MPI_Send(input_str + offset, len, MPI_CHAR, i, TAG_WORK_DATA, MPI_COMM_WORLD);
        offset += len;
    }

    // Receive cleaned results
    for (int i = 1; i < num_procs; i++) {
        int cleaned_len;
        MPI_Status status;

        MPI_Recv(&cleaned_len, 1, MPI_INT, MPI_ANY_SOURCE, TAG_RESULT_LEN, MPI_COMM_WORLD, &status);

        char* cleaned_part = malloc((cleaned_len + 1) * sizeof(char));
        if (!cleaned_part) {
            perror("cleaned_part malloc failed");
            free(input_str);
            free(final_result);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        MPI_Recv(cleaned_part, cleaned_len, MPI_CHAR, status.MPI_SOURCE, TAG_RESULT_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cleaned_part[cleaned_len] = '\0';

        strcat(final_result, cleaned_part);
        free(cleaned_part);
    }

    // Terminate workers
    for (int i = 1; i < num_procs; i++) {
        int dummy = 0;
        MPI_Send(&dummy, 1, MPI_INT, i, TAG_TERMINATE, MPI_COMM_WORLD);
    }

    // Print result
    printf("Original string: \"%s\"\n\n", input_str);
    printf("Cleaned result : \"%s\"\n", final_result);

    free(input_str);
    free(final_result);
}

int main(int argc, char* argv[]) {
    int rank, num_procs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0) {
        farmer_process(argv[1], num_procs);
    } else {
        worker_process();
    }

    MPI_Finalize();
    return 0;
}
