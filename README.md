# parallel-demos

**Description:**
A collection of example projects demonstrating parallel computing concepts and tools (such as MPI, OpenMP, etc.).
Each project is located in the `src/` directory, and the repository includes a versatile Makefile that automatically detects the use of MPI and compiles/runs each project accordingly.

---

## Table of Contents

* [Repository Structure](#repository-structure)
* [Build and Run Instructions](#build-and-run-instructions)
* [Projects](#projects)

  * [mpi\_snake\_in\_the\_box](#mpi\_snake\_in\_the\_box)
  * [mpi\_space\_cleaner](#mpi\_space\_cleaner)
  * [mpi\_file\_space\_cleaner](#mpi\_file\_space\_cleaner) 
  * [mpi\_hypercube\_broadcast-and-mpi\_hypercube\_reduce](#mpi\_hypercube\_broadcast-and-mpi\_hypercube\_reduce) 
  * [omp\_pi\_estimation](#omp\_pi\_estimation) 
  * [mpi\_maze\_solver](#mpi\_maze\_solver) 
  * [omp\_gauss\_jordan\_elimination](#omp\_gauss\_jordan\_elimination) 
  * [mpi\_naive\_string\_matcher](#mpi\_naive\_string\_matcher) 
  * [omp\_bitonic\_sort](#omp\_bitonic\_sort) 

---

## Repository Structure

```plaintext
parallel-demos/
├── src/                # Source code for each project (e.g., mpi_snake_in_the_box.c).
├── bin/                # Compiled binaries will be placed here.
├── notes_in_turkish/   # Study notes in Turkish.
├── Makefile            # Smart Makefile for building and running projects.
└── README.md
```

---

## Build and Run Instructions

This repository uses a single Makefile to manage building and running all demo projects.

### 1. Build a Project

To build a project, run:

```sh
make build TARGET=<project_name>
```

* Replace `<project_name>` with the source file name (without `.c`) located in `src/`.
* The Makefile will automatically use `mpicc` if the source includes `mpi.h`, otherwise `gcc`.

### 2. Run a Project

#### For MPI Programs:

```sh
make run TARGET=<project_name> np=<number_of_processes> args="<arguments>"
```

* `np` **must** be specified for MPI programs.
* `args` is optional, pass any command-line arguments required by the program.

#### For Non-MPI Programs:

```sh
make run TARGET=<project_name> args="<arguments>"
```

* `np` is ignored.

### 3. Clean Build Artifacts

```sh
make clean
```

* Removes all files in `bin/`.

---

## Projects

### mpi\_snake\_in\_the\_box

**Description:**
Parallel implementation (using MPI) to search for long “snake-in-the-box” paths in an n-dimensional hypercube.
The problem is distributed using a simple farmer-worker pattern:

* The farmer (rank 0) distributes search tasks (starting vertices) to worker processes.
* Workers recursively explore non-chorded paths, returning their best results to the farmer.
* The farmer collects and reports the longest path found.

---

**How to Build:**

```sh
make build TARGET=mpi_snake_in_the_box
```

**How to Run:**

```sh
make run TARGET=mpi_snake_in_the_box np=<number_of_processes> args="<dimension>"
```

* `dimension` is the hypercube’s dimension.
* Example for a 6-dimensional cube with 8 MPI processes:

  ```sh
  make run TARGET=mpi_snake_in_the_box np=8 args="6"
  ```

**Output:**
* Prints the longest snake path found, its binary sequence, and execution time.

**Future Work**
* Recursive exploration can be pruned by estimating the maximum achievable path length from the current state and comparing it to the best result found so far.

* To avoid redundant computation of symmetric paths, workers may be restricted to initiate paths from vertex 0 and proceed with a single fixed neighbor in the second step.

* Communication overhead can be reduced by partitioning the search space into non-overlapping regions, enabling local computation of maximal paths before a global aggregation step at the root process.

* Disjoint regional assignments help eliminate duplicated effort by ensuring that each path is explored by only one worker.

* Maintaining a hash table of previously explored path signatures can prevent revisiting equivalent paths generated in different orders.

* Once an optimal or sufficiently long path is discovered, an early termination signal can be propagated to halt remaining workers and conserve resources.

* Storing precomputed adjacency lists or bitwise neighbor maps in memory can significantly reduce access latency during traversal.

---

### mpi\_space\_cleaner

**Description:**
Parallel MPI program that removes spaces from a given input string using a farmer-worker pattern. The input string is divided into chunks and distributed to worker processes. Each worker cleans its chunk by removing spaces and returns the result. The farmer collects and concatenates the cleaned chunks in correct order.

---

**How to Build:**

```sh
make build TARGET=mpi_space_cleaner
```

**How to Run:**

```sh
make run TARGET=mpi_space_cleaner np=<number_of_processes>
```

* `input_str` is hardcoded inside the source (e.g., `"H E L L O W O R L D"`), but can be modified.
* Example run with 4 processes:

  ```sh
  make run TARGET=mpi_space_cleaner np=4
  ```

**Output:**

```
Original string: "H E L L O W O R L D"
Cleaned: "HELLOWORLD"
```
---

### mpi\_file\_space\_cleaner

**Description:**
A parallel MPI program that removes **spaces and newlines** from a text file using a farmer-worker model. The input file is divided into chunks and sent to worker processes. Each worker removes `' '` and `'\n'` characters from its chunk and returns the cleaned data. The farmer process collects the cleaned chunks and combines them into the final output.

---

**How to Build:**

```sh
make build TARGET=mpi_file_space_cleaner
```

**How to Run:**

```sh
make run TARGET=mpi_file_space_cleaner np=<number_of_processes> args="path/to/input_file.txt"
```

* Replace `path/to/input_file.txt` with the actual file path.
* Example with 8 MPI processes:

```sh
make run TARGET=mpi_file_space_cleaner np=8 args="resource/long_string_with_many_spaces.txt"
```

**Output Example:**

```
Original string: "This is a
test input   with

many    spaces."

Cleaned result : "Thisisatestinputwithmanyspaces."
```
---

### mpi\_hypercube\_broadcast-and-mpi\_hypercube\_reduce

**Description:**
Parallel MPI implementations of **one-to-all broadcast** and **all-to-one reduction** operations on a *d*-dimensional hypercube.

* **Broadcast:** A single source node broadcasts a message (or array) to all other nodes, efficiently using hypercube communication.
* **Reduce:** All nodes send a message (or array) which are combined (e.g., summed) at a destination node.
  Both algorithms log step-by-step message passing between nodes for educational and debugging purposes.

---

**How to Build:**

```sh
make build TARGET=mpi_hypercube_broadcast
make build TARGET=mpi_hypercube_reduce
```

**How to Run:**

#### Broadcast (one-to-all):

```sh
make run TARGET=mpi_hypercube_broadcast np=<number_of_processes> args="<dimension> <source>"
```

* `dimension`: Hypercube's dimension (e.g., 3 for 8 processes)
* `source`: Node that starts the broadcast (typically 0)
* **Example:**

  ```sh
  make run TARGET=mpi_hypercube_broadcast np=8 args="3 0"
  ```

#### Reduce (all-to-one):

```sh
make run TARGET=mpi_hypercube_reduce np=<number_of_processes> args="<dimension> <destination>"
```

* `dimension`: Hypercube's dimension (e.g., 3 for 8 processes)
* `destination`: Node where the reduction result is collected (typically 0)
* **Example:**

  ```sh
  make run TARGET=mpi_hypercube_reduce np=8 args="3 0"
  ```

**Output:**

* **Broadcast:** Each process logs when it sends/receives messages at every step. All nodes print the final value they received.
* **Reduce:** Each process logs message passing steps. The destination node prints the final reduced result (e.g., sum of all process ranks).

---

### omp\_pi\_estimation

**Description:**
An OpenMP-based parallel implementation of π (pi) estimation using the numerical integration method. The algorithm divides the \[0, 1] interval into `n` steps and approximates the integral of `4 / (1 + x²)` using the midpoint rule.

Each thread computes a portion of the sum in parallel using OpenMP's `reduction` clause.

---

**How to Build:**

```sh
make build TARGET=omp_pi_estimation
```

**How to Run:**

```sh
make run TARGET=omp_pi_estimation args="<num_steps>"
```

* Replace `<num_steps>` with the number of intervals used to approximate π.
* Example:

  ```sh
  make run TARGET=omp_pi_estimation args="100000000"
  ```

**Output:**

```
Approximate value of pi: 3.141592653589882
```
---

### mpi\_maze\_solver

**Description:**
A parallel MPI-based implementation of a maze solver using depth-first search (DFS). The maze is represented as a 2D grid, where `0` indicates open paths and `1` indicates walls. Each MPI process independently explores a unique direction from the maze’s entry point. If a valid path from the start to the goal exists, the responsible process outputs the sequence of directional moves taken (e.g., `Down → Right → Right → ...`).

---

**How to Build:**

```sh
make build TARGET=mpi_maze_solver
```

**How to Run:**

```sh
make run TARGET=mpi_maze_solver np=4
```

* You **must** run this with exactly 4 processes since the algorithm assigns one of the four directions (Up, Down, Left, Right) to each process from the start cell.
* The maze is currently hardcoded but can be modified inside the source.

**Output Example:**

```
MPI Maze Solver with 4 processes
Rank 1: Path found!
Path: Down Down Right Right Down Down Right Right 
A solution was found.
```

If no path exists:

```
MPI Maze Solver with 4 processes
No solution exists.
```

**Limitations and Future Work:**

* **Static Maze Definition:**
  The maze is statically defined within the source code. In future iterations, it can be loaded dynamically from external files or passed as input arguments for greater flexibility.

* **Fixed Process Count (4):**
  Only the first 4 MPI processes are used, with each assigned to a unique direction (Up, Down, Left, Right) from the starting cell. Additional processes remain idle. A more scalable solution would dynamically partition the search space among arbitrary numbers of processes.

* **No Shortest Path Guarantee:**
  The DFS-based search returns the first valid path it finds, not necessarily the shortest one. This could be improved by switching to or integrating with parallel BFS or A\* search.

* **Lack of Shared State or Pruning:**
  Each process explores independently without knowledge of others’ visited nodes, potentially duplicating work. Shared visited sets, coordinated pruning, or work stealing mechanisms could enhance efficiency and reduce redundant computation.

* **Fixed Maze Size and Grid Representation:**
  Maze dimensions and layout are hardcoded with a fixed-size grid. Adapting the solver to support arbitrary-sized mazes and alternative data representations (e.g., adjacency lists) would improve generality.

* **Sequential Output and Termination:**
  Once a path is found, only the discovering process prints the solution. A central coordination mechanism (e.g., rank 0 reporting) and early termination broadcast to halt all workers could improve resource usage and clarity.

* **Limited Real-World Applicability Without Enhancements:**
  While suitable as a learning tool for parallel exploration and MPI, the algorithm requires improvements to be competitive with modern parallel pathfinding systems.

---

### omp\_gauss\_jordan\_elimination

**Description:**
An OpenMP-based implementation of **Gauss-Jordan Elimination** to solve systems of linear equations of the form `Ax = b`.
The algorithm converts the augmented matrix `[A | b]` into reduced row-echelon form using elementary row operations, fully parallelizing:

* The normalization of each pivot row.
* The elimination of pivot column entries from other rows.

---

**How to Build:**

```sh
make build TARGET=omp_gauss_jordan_elimination
```

**How to Run:**

```sh
make run TARGET=omp_gauss_jordan_elimination
```

**Output Example:**

```
Initial Matrix:
  2.0000   1.0000  -1.0000  -3.0000 |   8.0000
 -3.0000  -1.0000   2.0000  -1.0000 | -11.0000
 -2.0000   1.0000   2.0000  -3.0000 |  -3.0000
  1.0000   2.0000  -1.0000  -1.0000 |   4.0000

Solution vector (x):
x[0] =   6.0000
x[1] =   1.3333
x[2] =   4.3333
x[3] =   0.3333
```
---
### mpi\_naive\_string\_matcher

**Description:**
The input string (text) and the pattern (pattern) are provided via command-line arguments. The full input string and pattern are broadcast to all MPI processes. Each process then receives an overlapping chunk of the text to ensure matches spanning chunk boundaries are not missed. Each process performs naive pattern matching on its assigned chunk independently. After all processes complete their search, the global minimum match index is determined using MPI_Allreduce with the MPI_MIN operation. All processes compare their local match index to this global minimum, and any process that finds a match at that index prints the result. If no match is found, the master process (rank 0) reports it.

---

**How to Build:**

```sh
make build TARGET=mpi_naive_string_matcher
```

**How to Run:**

```sh
make run TARGET=mpi_naive_string_matcher np=<number_of_processes> args="<text> <pattern>"
```

* Example:

```sh
make run TARGET=mpi_naive_string_matcher np=4 args="bnabananaananabanana ana"
```

**Output Example:**

```
First match found by process 2 at index 4
```

**Limitations**
This MPI-based parallel implementation of the naive string matching algorithm improves scalability by distributing the workload across multiple processes, but it inherently preserves several limitations of the naive approach. While overlapping chunks ensure correctness at process boundaries, they introduce redundant computations, and the use of MPI_Allreduce requires all processes to complete their search before the result is determined—preventing true early termination. Additionally, the algorithm retains the naive method’s inefficiencies, such as its O(n·m) time complexity, lack of skipping logic, and poor performance on repetitive inputs. Thus, while parallelization offers better utilization of resources, the underlying algorithm's simplicity continues to limit overall efficiency.

---

### omp\_bitonic\_sort

**Description:**
A parallel sorting algorithm implemented using OpenMP to demonstrate the Bitonic Sort approach. Bitonic Sort is a comparison-based sorting algorithm particularly well-suited for parallel architectures due to its regular structure and predictable communication pattern. This implementation uses an iterative (non-recursive) approach to construct and merge bitonic sequences using nested loops. Parallelism is applied to the compare-and-swap operations within each stage using OpenMP directives for improved performance on multi-core systems.

* The input is an unsorted array (currently hardcoded).
* Sorting is performed in ascending order using bitwise operations to control the merging logic.
* The implementation assumes the input size is a power of two.

---

**How to Build:**

```sh
make build TARGET=omp_bitonic_sort
```

**How to Run:**

```sh
make run TARGET=omp_bitonic_sort
```

* No command-line arguments are needed.
* The input array is defined in the source and can be modified for testing.

**Output Example:**

```
Sorted array:
4 10 11 20 21 30 110 330
```

---