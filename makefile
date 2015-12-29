.PHONY: all project clean

all: project

project: omp pthread mpi

omp: src/ompMain.c
	@echo "Compiling OpenMP"
	@gcc -std=c99 -O3 -fopenmp src/ompMain.c -o bin/ompMain.o

pthread: src/pthreadMain.c
	@echo "Compiling Pthread"
	@gcc -lpthread -std=gnu99 -O3 src/pthreadMain.c -o bin/pthreadMain.o

mpi: src/mpiMain.c
	@echo "Compiling MPI"
	@mpicc -std=c99 -lm src/mpiMain.c -o bin/mpiMain.o

clean:
	@rm -f bin/*
