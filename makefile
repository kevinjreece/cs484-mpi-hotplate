.PHONY: all project clean

all: project

project: omp pthread mpilin mpilog

omp: src/ompMain.c
	@echo "Compiling OpenMP"
	@gcc -std=c99 -O3 -fopenmp src/ompMain.c -o bin/ompMain.o

pthread: src/pthreadMain.c
	@echo "Compiling Pthread"
	@gcc -lpthread -std=gnu99 -O3 src/pthreadMain.c -o bin/pthreadMain.o

mpilin: src/mpiLinMain.c
	@echo "Compiling MPI linear"
	@mpicc -std=c99 -lm src/mpiLinMain.c -o bin/mpiLinMain.o

mpilog: src/mpiLogMain.c
	@echo "Compiling MPI log"
	@mpicc -std=c99 -lm src/mpiLogMain.c -o bin/mpiLogMain.o

clean:
	@rm -f bin/*
