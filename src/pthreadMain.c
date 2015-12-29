#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include "time.h"
#include "hotplate.h"

#define EPSILON 0.1
#define THRESHOLD 50
#define LENGTH 16384
#define MAX_THREADS 64

hotplate _plate;
float** _curr_plate;
float** _prev_plate;
bool** _lock_plate;
bool* _is_section_steady;
int* _above_threshold;
int _nthreads;
int _iterations;
bool _is_steady;
int _section_size;
FILE* fp;

pthread_t threadstructs[MAX_THREADS];
pthread_mutex_t _is_steady_lock;
pthread_barrier_t _barr;
const pthread_barrierattr_t *restrict _attr = 0;

int getNumOverInPlate(float threshold) {
	int num = 0;
	int i, j;
	for (i = 0; i < _nthreads; i++) {
		num += _above_threshold[i];
	}
	return num;
}

int getNumOverInSection(int tid, float threshold) {
	int num = 0;
	int i, row, col;
	for (i = 0; i < _section_size; i++) {
		row = (tid * _section_size) + i;
		for (col = 0; col < LENGTH; col++) {
			if (_curr_plate[row][col] > threshold) {
				num++;
			}
		}
	}
	return num;
}

char isCellSteady(int row, int col) {
	float up = _curr_plate[row-1][col];
	float left = _curr_plate[row][col-1];
	float center = _curr_plate[row][col];
	float right = _curr_plate[row][col+1];
	float down = _curr_plate[row+1][col];
	return fabs(center - (up + left + right + down) / 4.0f) < EPSILON;
}

void calcNewCellValue(int row, int col) {
	float up = _prev_plate[row-1][col];
	float left = _prev_plate[row][col-1];
	float center = _prev_plate[row][col];
	float right = _prev_plate[row][col+1];
	float down = _prev_plate[row+1][col];
	float val = (up + left + right + down + (center * 4.0f)) / 8.0f;
	_curr_plate[row][col] = val;
}

bool isSectionSteady(int tid) {
	int i, row, col;
	for (i = 0; i < _section_size; i++) {
		row = (tid * _section_size) + i;
		for (col = 0; col < LENGTH; col++) {
			if (!(_lock_plate[row][col]) && !isCellSteady(row, col)) {
				return false;
			}
		}
	}
	return true;
}

bool isPlateSteady() {
	bool is_steady = true;
	int i;
	for (i = 0; i < _nthreads; i++) {
		if (_is_section_steady[i]) {
			_is_section_steady[i] = false;
		}
		else {
			is_steady = false;
		}
	}
	return is_steady;
}

void advanceSection(int tid) {
	int i, row, col;
	for (i = 0; i < _section_size; i++) {
		row = (tid * _section_size) + i;
		for (col = 0; col < LENGTH; col++) {
			if (!(_lock_plate[row][col])) {
				calcNewCellValue(row, col);
			}
		}
	}
}

void swapPlates() {
	float** temp = _curr_plate;
	_curr_plate = _prev_plate;
	_prev_plate = temp;
}

void prepNextIteration() {
	_iterations++;
	_is_steady = isPlateSteady();
	if (!_is_steady) swapPlates();
	fprintf(fp, "%d,\n", getNumOverInPlate(THRESHOLD));
}

void* createSteadyStateSection(void* arg) {
	int tid = (int)(uintptr_t)arg;
	printf("createSteadyStateSection in thread %d\n", tid);

	while (!_is_steady && _iterations < 500) {
		advanceSection(tid);
		_is_section_steady[tid] = isSectionSteady(tid);
		_above_threshold[tid] = getNumOverInSection(tid, THRESHOLD);

		pthread_barrier_wait(&_barr);
		if (!tid) prepNextIteration();
		pthread_barrier_wait(&_barr);
	}
	
	return 0;
}

void createSteadyState() {
	int thread;

	for (thread = 0; thread < _nthreads; thread++) {
		pthread_create(&threadstructs[thread], 0, createSteadyStateSection, (void *)(uintptr_t)thread);
	}

	for (thread = 0; thread < _nthreads; thread++) {
		pthread_join(threadstructs[thread], 0);
	}

	if (_iterations % 2 == 1) swapPlates();
	printf("Steps: %d\n", _iterations);
	printf("Over threshold: %d\n", getNumOverInPlate(THRESHOLD));
}

void printToFile(char* filename) {
	FILE* fp;
	fp = fopen(filename, "w");
	int i, j;
	for (i = 0; i < LENGTH; i++) {
		for (j = 0; j < LENGTH; j++) {
			fprintf(fp, "%lf, ", _curr_plate[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void initBarrierArrays() {
	int i;
	for (i = 0; i < _nthreads; i++) {
		_is_section_steady[i] = false;
		_above_threshold[i] = 0;
	}
}

void cleanUpMemory() {
	freeHotplate(&_plate);

	free(_is_section_steady);
	free(_above_threshold);

	pthread_barrier_destroy(&_barr);

	fclose(fp);
}

void setUp() {
	_plate = (hotplate) { .size = LENGTH };
	initHotplate(&_plate);
	_curr_plate = _plate.curr_plate;
	_prev_plate = _plate.prev_plate;
	_lock_plate = _plate.lock_plate;

	_is_section_steady = malloc(_nthreads * sizeof(bool));
	_above_threshold = malloc(_nthreads * sizeof(int));
	_section_size = LENGTH / _nthreads;
	initBarrierArrays();

	pthread_barrier_init(&_barr, _attr, _nthreads);

	fp = fopen("aboveThreshold.csv", "w");
}

int main(int argc, char* argv[]) {
	if (argc < 2) { printf("Include the number of threads you want as the first parameter\n"); return 0; }

	double time_b = getTime();
	_nthreads = atoi(argv[1]);

	setUp();
	// printPlate(_curr_plate, LENGTH);
	// printLockPlate(_lock_plate, LENGTH);
	createSteadyState();
	cleanUpMemory();

	double time_e = getTime();
	printf("Time: %lf seconds\n", time_e - time_b);
	return 0;
}














