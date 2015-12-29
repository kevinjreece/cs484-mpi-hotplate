#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "time.h"
#include "hotplate.h"

#define EPSILON 0.1
#define THRESHOLD 50
#define LENGTH 16384

hotplate _plate;
float** _curr_plate;
float** _prev_plate;
bool** _lock_plate;

int getNumOver() {
	int num = 0;
	// #pragma omp parallel for
	int i, j;
	for (i = 0; i < LENGTH; i++) {
		for (j = 0; j < LENGTH; j++) {
			if (_curr_plate[i][j] > THRESHOLD) {
				num++;
			}
		}
	}
	return num;
}

bool isCellSteady(int row, int col) {
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
	// cout << (int)val << "\t";
}

bool isPlateSteady() {
	int i, j;
	for (i = 0; i < LENGTH-1; i++) {
		for (j = 0; j < LENGTH-1; j++) {
			if (!(_lock_plate[i][j]) && !isCellSteady(i, j)) {
				return false;
			}
		}
	}
	return true;
}

void swapPlates() {
	float** temp = _curr_plate;
	_curr_plate = _prev_plate;
	_prev_plate = temp;
}

void createSteadyState() {
	int steps = 0;
	bool is_steady = false;

	while (!is_steady && steps < 500) {
		steps++;
		#pragma omp parallel for
		for (int i = 1; i < LENGTH-1; i++) {
			for (int j = 1; j < LENGTH-1; j++) {
				if (!(_lock_plate[i][j])) {
					calcNewCellValue(i, j);
				}
			}
		}
		is_steady = isPlateSteady();
		swapPlates();
	}
	if (steps % 2 == 1) swapPlates();
	printf("Steps: %d\n", steps);
	printf("Over threshold: %d\n", getNumOver());
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

void setUp() {
	_plate = (hotplate) { .size = LENGTH };
	initHotplate(&_plate);
	_curr_plate = _plate.curr_plate;
	_prev_plate = _plate.prev_plate;
	_lock_plate = _plate.lock_plate;
}

void cleanUpMemory() {
	freeHotplate(&_plate);
}

int main(int argc, char* argv[]) {
	double time_b = getTime();

	// cout << plate.toString();
	setUp();
	createSteadyState();
	cleanUpMemory();
	// cout << plate.toString() << "\n";
	// printToFile("c.csv");

	double time_e = getTime();
	printf("Time: %lf seconds\n", time_e - time_b);
	return 0;
}
