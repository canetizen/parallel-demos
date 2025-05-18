# ============================================================================
# Makefile with automatic MPI detection
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
	@if grep -q '#include[[:space:]]*[<"]mpi.h[">]' $(SRCDIR)/$(TARGET).c; then \
		echo "Detected MPI usage in $(TARGET). Compiling with mpicc..."; \
		$(MPICC) $(CFLAGS) -o $(BINDIR)/$(TARGET) $(SRCDIR)/$(TARGET).c -lm; \
	else \
		echo "No MPI detected in $(TARGET). Compiling with gcc..."; \
		$(CC) $(CFLAGS) -o $(BINDIR)/$(TARGET) $(SRCDIR)/$(TARGET).c; \
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
	@if grep -q '#include[[:space:]]*[<"]mpi.h[">]' $(SRCDIR)/$(TARGET).c; then \
		if [ -z "$(np)" ]; then \
			echo "Error: MPI program requires np=N to be specified."; \
			exit 1; \
		fi; \
		echo "Running with MPI: mpirun -np $(np) $(BINDIR)/$(TARGET) $(args)"; \
		mpirun -np $(np) $(BINDIR)/$(TARGET) $(args); \
	else \
		echo "Running without MPI: ./$(BINDIR)/$(TARGET) $(args)"; \
		$(BINDIR)/$(TARGET) $(args); \
	fi

clean:
	@echo "Cleaning $(BINDIR)/"
	@rm -rf $(BINDIR)/*