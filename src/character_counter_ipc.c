/*
 * Author: canetizen
 * Created on Sun May 25 2025
 * Description: Character counter using interprocess communication (IPC) mechanisms.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PROCESSES 8
#define MAX_CHARS 256 // Standard ASCII range

/**
 * Child process function to count character frequencies in a segment.
 * Writes the local count to the pipe and exits.
 */
void count_characters(const char *segment, int length, int fd_write) {
    int local_count[MAX_CHARS] = {0};

    // Count frequency of each character in the assigned segment
    for (int i = 0; i < length; i++) {
        unsigned char c = segment[i];
        local_count[c]++;
    }

    // Write result to pipe and check for errors
    ssize_t bytes_written = write(fd_write, local_count, sizeof(local_count));
    if (bytes_written != sizeof(local_count)) {
        perror("write");
        close(fd_write);
        exit(EXIT_FAILURE);
    }

    close(fd_write);
    exit(EXIT_SUCCESS);
}

int main() {
    // Sample input string (can be replaced with dynamic input)
    const char *input = "There are two Mustafa Kemals. One the flesh-and-blood Mustafa Kemal who now stands before you and who will pass away. The other is you, all of you here who will go to the far corners of our land to spread the ideals which must be defended with your lives if necessary. I stand for the nation's dreams, and my life's work is to make them come true.";
    
    int input_len = strlen(input);
    int segment_size = input_len / MAX_PROCESSES;

    int pipes[MAX_PROCESSES][2];            // Pipes for IPC
    int final_count[MAX_CHARS] = {0};       // Aggregated results

    // Create child processes and pipes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            close(pipes[i][0]); // Close unused read end

            // Determine segment bounds
            int start = i * segment_size;
            int length = (i == MAX_PROCESSES - 1) ? input_len - start : segment_size;

            // Perform character counting
            count_characters(input + start, length, pipes[i][1]);
        } else {
            // Parent process
            close(pipes[i][1]); // Close unused write end
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < MAX_PROCESSES; i++) {
        wait(NULL);
    }

    // Read and aggregate results from each child
    for (int i = 0; i < MAX_PROCESSES; i++) {
        int local_count[MAX_CHARS] = {0};

        ssize_t bytes_read = read(pipes[i][0], local_count, sizeof(local_count));
        if (bytes_read != sizeof(local_count)) {
            perror("read");
            close(pipes[i][0]);
            exit(EXIT_FAILURE);
        }

        close(pipes[i][0]);

        // Merge local count into final count
        for (int j = 0; j < MAX_CHARS; j++) {
            final_count[j] += local_count[j];
        }
    }

    // Print final character frequencies
    for (int i = 0; i < MAX_CHARS; i++) {
        if (final_count[i] > 0) {
            printf("Character '%c' (%d) => %d times\n", i, i, final_count[i]);
        }
    }

    return 0;
}
