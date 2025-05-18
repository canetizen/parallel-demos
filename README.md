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

  
  

---

## Repository Structure

```plaintext
parallel-demos/
├── src/      # Source code for each project (e.g., mpi_snake_in_the_box.c)
├── bin/      # Compiled binaries will be placed here
├── Makefile  # Smart Makefile for building and running projects
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

**Future Work**

* Recursive exploration can be pruned by estimating the maximum achievable path length from the current state and comparing it to the best result found so far.

* To avoid redundant computation of symmetric paths, workers may be restricted to initiate paths from vertex 0 and proceed with a single fixed neighbor in the second step.

* Communication overhead can be reduced by partitioning the search space into non-overlapping regions, enabling local computation of maximal paths before a global aggregation step at the root process.

* Disjoint regional assignments help eliminate duplicated effort by ensuring that each path is explored by only one worker.

* Maintaining a hash table of previously explored path signatures can prevent revisiting equivalent paths generated in different orders.

* Once an optimal or sufficiently long path is discovered, an early termination signal can be propagated to halt remaining workers and conserve resources.

* Storing precomputed adjacency lists or bitwise neighbor maps in memory can significantly reduce access latency during traversal.

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

---

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

---

**Output:**

* **Broadcast:** Each process logs when it sends/receives messages at every step. All nodes print the final value they received.
* **Reduce:** Each process logs message passing steps. The destination node prints the final reduced result (e.g., sum of all process ranks).

---