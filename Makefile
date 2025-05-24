# ============================================================================
# Makefile with automatic MPI and OpenMP detection
#
# Author:  canetizen
# Created: 2025-04-24
#
# Usage:
#   make build TARGET=<program_name>
#   make run TARGET=<program_name> [np=<n>] [args="<arg0 arg1 ...>"] (np is necessary for MPI programs)
#   make clean
# ============================================================================

CC = gcc
MPICC = mpicc
CFLAGS = -Wall -Werror -std=c99 -O3 -march=native -funroll-loops -flto

SRCDIR = src
BINDIR = bin

TARGET ?=
np ?=
args ?=

.PHONY: build run clean

build:
	@mkdir -p $(BINDIR)
	@if [ -z "$(TARGET)" ]; then \
		echo "Usage: make build TARGET=program"; \
		exit 1; \
	fi
	@src_file="$(SRCDIR)/$(TARGET).c"; \
	uses_mpi=$$(grep -q '#include[[:space:]]*[<"]mpi.h[">]' $$src_file && echo yes || echo no); \
	uses_omp=$$(grep -q '#include[[:space:]]*[<"]omp.h[">]' $$src_file && echo yes || echo no); \
	CFLAGS_EXTRA="$(CFLAGS)"; \
	if [ "$$uses_omp" = "yes" ]; then \
		CFLAGS_EXTRA="$$CFLAGS_EXTRA -fopenmp"; \
	fi; \
	if [ "$$uses_mpi" = "yes" ] && [ "$$uses_omp" = "yes" ]; then \
		echo "Compiling with MPI and OpenMP..."; \
	elif [ "$$uses_mpi" = "yes" ]; then \
		echo "Compiling with MPI..."; \
	elif [ "$$uses_omp" = "yes" ]; then \
		echo "Compiling with OpenMP..."; \
	else \
		echo "No MPI or OpenMP detected. Compiling as a plain C program..."; \
	fi; \
	if [ "$$uses_mpi" = "yes" ]; then \
		$(MPICC) $$CFLAGS_EXTRA -o $(BINDIR)/$(TARGET) $$src_file -lm; \
	else \
		$(CC) $$CFLAGS_EXTRA -o $(BINDIR)/$(TARGET) $$src_file; \
	fi

run:
	@if [ -z "$(TARGET)" ]; then \
		echo "Usage: make run TARGET=program [np=N] [args=\"...\"]"; \
		exit 1; \
	fi
	@if [ ! -f $(BINDIR)/$(TARGET) ]; then \
		echo "Executable not found: $(BINDIR)/$(TARGET). Please run 'make build' first."; \
		exit 1; \
	fi
	@src_file="$(SRCDIR)/$(TARGET).c"; \
	uses_mpi=$$(grep -q '#include[[:space:]]*[<"]mpi.h[">]' $$src_file && echo yes || echo no); \
	uses_omp=$$(grep -q '#include[[:space:]]*[<"]omp.h[">]' $$src_file && echo yes || echo no); \
	if [ "$$uses_mpi" = "yes" ] && [ "$$uses_omp" = "yes" ]; then \
		echo "Running with MPI and OpenMP..."; \
	elif [ "$$uses_mpi" = "yes" ]; then \
		echo "Running with MPI..."; \
	elif [ "$$uses_omp" = "yes" ]; then \
		echo "Running with OpenMP..."; \
	else \
		echo "No MPI or OpenMP detected. Running as a plain C program..."; \
	fi; \
	if [ "$$uses_mpi" = "yes" ]; then \
		if [ -z "$(np)" ]; then \
			echo "Error: MPI program requires np=N to be specified."; \
			exit 1; \
		fi; \
		mpirun -np $(np) $(BINDIR)/$(TARGET) $(args); \
	else \
		$(BINDIR)/$(TARGET) $(args); \
	fi

clean:
	@echo "Cleaning $(BINDIR)/"
	@rm -rf $(BINDIR)/*
