#ifndef kjr_hotplate
#define kjr_hotplate

#include <stdbool.h>

typedef struct {
	int size;
	float** curr_plate;
	float** prev_plate;
	bool** lock_plate;
} hotplate;

void printPlate(float** plate, int size) {
	int i, j;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			printf("%.1f, ", plate[i][j]);
		}
		printf("\n");
	}
}

void printLockPlate(bool** plate, int size) {
	int i, j;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			printf("%d, ", plate[i][j]);
		}
		printf("\n");
	}
}

void hotplate_initRegular(float** curr_plate, float** prev_plate, bool** lock_plate, int size) {
	// printf("initRegularCells\n");
	int i, j;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			curr_plate[i][j] = 50;
			prev_plate[i][j] = 50;
			lock_plate[i][j] = false;
		}
	}
}

void hotplate_initLocked(float** curr_plate, float** prev_plate, bool** lock_plate, int size) {
	// printf("initLockedCells\n");
	int i, t_row, l_col, b_row, r_col;
	t_row = l_col = 0;
	b_row = r_col = size - 1;
	
	
	for(i = 0; i < size; i++) {
		// top row = 0
		curr_plate[t_row][i] = 0;
		prev_plate[t_row][i] = 0;
		lock_plate[t_row][i] = true;
		// bottom row = 100
		curr_plate[b_row][i] = 100;
		prev_plate[b_row][i] = 100;
		lock_plate[b_row][i] = true;
		// left col = 0
		curr_plate[i][l_col] = 0;
		prev_plate[i][l_col] = 0;
		lock_plate[i][l_col] = true;
		// right col = 0
		curr_plate[i][r_col] = 0;
		prev_plate[i][r_col] = 0;
		lock_plate[i][r_col] = true;
	}
}

void initSubArrays(float** curr_plate, float** prev_plate, bool** lock_plate, int size) {
	for (int i = 0; i < size; i++) {
		curr_plate[i] = malloc(size * sizeof(float));
		prev_plate[i] = malloc(size * sizeof(float));
		lock_plate[i] = malloc(size * sizeof(bool));
	}
}

void initHotplate(hotplate* plate) {
	int size = plate->size;
	float** curr_plate = malloc(size * sizeof(float*));
	float** prev_plate = malloc(size * sizeof(float*));
	bool** lock_plate = malloc(size * sizeof(bool*));

	// initialize sub-arrays
	initSubArrays(curr_plate, prev_plate, lock_plate, size);
	// initialize regular cells
	hotplate_initRegular(curr_plate, prev_plate, lock_plate, size);
	// initialize locked cells
	hotplate_initLocked(curr_plate, prev_plate, lock_plate, size);

	plate->curr_plate = curr_plate;
	plate->prev_plate = prev_plate;
	plate->lock_plate = lock_plate;
	
	return;
}

void freeHotplate(hotplate* plate) {
	int size = plate->size;

	for (int i = 0; i < size; i++) {
		free(plate->curr_plate[i]);
		free(plate->prev_plate[i]);
		free(plate->lock_plate[i]);
	}

	free(plate->curr_plate);
	free(plate->prev_plate);
	free(plate->lock_plate);

	return;
}



#endif

















