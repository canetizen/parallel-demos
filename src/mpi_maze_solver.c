/*
 * Author: canetizen
 * Created on Sat May 24 2025
 * Description: MPI implementation for maze solving. 
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 5
#define START_X 0
#define START_Y 0
#define END_X 4
#define END_Y 4
#define MAX_PATH_LEN 100

// Maze: 0 = free, 1 = wall
int maze[N][N] = {
    {0, 1, 0, 0, 0},
    {0, 1, 0, 1, 0},
    {0, 0, 0, 1, 0},
    {0, 1, 1, 1, 0},
    {1, 0, 0, 0, 0}
};

// Direction vectors: up, down, left, right
int dx[4] = {-1, 1, 0, 0};
int dy[4] = {0, 0, -1, 1};
const char* dir_names[4] = {"Up", "Down", "Left", "Right"};

// Validity check for next move
int is_valid(int x, int y, int visited[N][N]) {
    return (x >= 0 && x < N && y >= 0 && y < N &&
            maze[x][y] == 0 && visited[x][y] == 0);
}

// Recursive DFS with path tracking
int dfs(int x, int y, int visited[N][N], int path[], int depth) {
    if (x == END_X && y == END_Y) {
        return 1;  // Goal reached
    }

    visited[x][y] = 1;

    for (int d = 0; d < 4; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];

        if (is_valid(nx, ny, visited)) {
            path[depth] = d;
            if (dfs(nx, ny, visited, path, depth + 1)) {
                return 1;
            }
        }
    }

    visited[x][y] = 0;
    return 0;
}

int main(int argc, char** argv) {
    int rank, size;
    int found = 0;
    int global_found;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("MPI Maze Solver with %d processes\n", size);
    }

    if (rank < 4) {  // Only 4 directions possible from start
        int visited[N][N] = {0};
        int path[MAX_PATH_LEN];
        memset(path, -1, sizeof(path));

        int nx = START_X + dx[rank];
        int ny = START_Y + dy[rank];

        if (is_valid(nx, ny, visited)) {
            visited[START_X][START_Y] = 1;
            path[0] = rank;  // First move

            if (dfs(nx, ny, visited, path, 1)) {
                found = 1;
                printf("Rank %d: Path found!\nPath: ", rank);
                for (int i = 0; i < MAX_PATH_LEN && path[i] != -1; i++) {
                    printf("%s ", dir_names[path[i]]);
                }
                printf("\n");
            }
        }
    }

    MPI_Reduce(&found, &global_found, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if (global_found)
            printf("A solution was found.\n");
        else
            printf("No solution exists.\n");
    }

    MPI_Finalize();
    return 0;
}
