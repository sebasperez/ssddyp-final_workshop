#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <omp.h>
#include <mpi.h>
#include <math.h>

// Number of iterations (program's clock starting at T0)

#define MAX_ITERATIONS	50

// Main matrix size

#define ROWS			4
#define COLUMNS		6

// Opinions representation

#define OPINION_WHITE	0
#define OPINION_RED		1
#define OPINION_GREEN	2
#define OPINION_BLUE		3
#define OPINIONS_COUNT	4

// Render

#define SHOW			0

// MATRIX MANAGMENT

int** create_matrix(int rows, int columns) {

	int** matrix = malloc(rows * sizeof (int*));

	int* cell = calloc(columns * rows, sizeof (int));

	for ( int i = 0; i < rows; ++i ) {

		matrix[i] = cell + i * columns;
	}

	return matrix;
}

void destroy_matrix(int** matrix) {

	free(*matrix);
	free(matrix);
}

void initialize_matrix(int rows, int columns, int** matrix) {

	for ( int i = 0; i < rows; i++ ) {

		for ( int j = 0; j < columns; j++ ) {

			matrix[i][j] = (rand() % OPINIONS_COUNT);
		}
	}
}

// RENDER

void set_color(int color) {

	switch (color) {

		case OPINION_RED:
			printf("\x1b[1;31m");
			break;
		case OPINION_GREEN:
			printf("\x1b[1;32m");
			break;
		case OPINION_BLUE:
			printf("\x1b[1;34m");
			break;
		case OPINION_WHITE:
			printf("\x1b[1;0m");
			break;
	};
}

void print_matrix(int rows, int columns, int** matrix) {

	for ( int i = 0; i < rows; i++ ) {

		printf("  ");
		for ( int j = 0; j < columns; j++ ) {
			set_color(matrix[i][j]);
			printf("\u2588\u2588 ");
		}
		printf("\n");
	}
	printf("\n");

	set_color(OPINION_WHITE);
}

void clear_output() {

	printf("\033[2J\033[1;1H");
}

// NEW OPINIONS

int check_draw(int opinions_by_type[OPINIONS_COUNT], int opinions_len) {

	if (opinions_len < 2) {
		return 0;
	}

	int cant = 0;

	for ( int i = 1; i < OPINIONS_COUNT; i++ ) {

		if ( opinions_by_type[i] != 0 ) {

			if ( cant == 0 ) {
				cant = opinions_by_type[i];
			}

			if ( opinions_by_type[i] != cant ) {
				return 0;
			}
		}
	}
	return 1;
}

int choose_opinion(int draw, int neighbors_count, int* neighbors_opinion) {

	int new_opinion = draw ? OPINION_WHITE : -1;
	
	while((draw && new_opinion == OPINION_WHITE) || (!draw && new_opinion == -1)) {
		// ask randomly a neighbor for a opinion
		int neighbor = rand() % neighbors_count;
		new_opinion = neighbors_opinion[neighbor];
	}

	return new_opinion;
}

int ask_opinion(int rows, int columns, int** opinions, int x, int y) {

	// Inicialize Data Structures

	int neighbors_opinion[9] = {0};
	int opinions_count_by_type[OPINIONS_COUNT] = {0};	
	int no_white_opinions_count = 0;
	int neighbors_count = 0;

	// Proccess Matrix and fill Data Structures

	int opinion;

	for ( int i = x - 1, ii = 0; ii < 3; i++, ii++ ) {

		for ( int j = y - 1, jj = 0; jj < 3; j++, jj++ ) {

			opinion = (i > -1 && j > -1 && i < rows && j < columns) ? opinions[i][j] : -1;

			if (opinion > -1) {
				neighbors_opinion[neighbors_count] = opinion;
				neighbors_count++;
				opinions_count_by_type[opinion]++;
				if (opinion != OPINION_WHITE) {
					no_white_opinions_count++;
				}
			}
		}
	}

	// Actually choose the opinion

	if ( check_draw(opinions_count_by_type, no_white_opinions_count) ) {

		return ((rand() % 100) <= 25) ? OPINION_WHITE : choose_opinion(1, neighbors_count, neighbors_opinion);
	}
	else {
		return choose_opinion(0, neighbors_count, neighbors_opinion);
	}
}

int** ask_new_opinions(int rows, int columns, int** opinions) {

	int** new_opinions = create_matrix(rows, columns);

	#pragma omp parallel for
	for ( int i = 0; i < rows; i++ ) {

		for ( int j = 0; j < columns; j++ ) {

			new_opinions[i][j] = ask_opinion(rows, columns, opinions, i, j);
		}
	}
	return new_opinions;
}

// USER INTERACTION

void wait_keypress() {

	struct termios oldt, newt;

	printf("Press any to continue...");

	// get console mode
	tcgetattr(STDIN_FILENO, &oldt);

	// set console into non-canonical mode
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	// wait for input
	getchar();

	// restore old terminal mode
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

// MEASURE

long long millis() {

	struct timeval tp;
	gettimeofday(&tp, NULL);
	return (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000; //get current timestamp in milliseconds
}

// MAIN

int main(int argc, char** argv) {

	long long start;

	// MPI
	int rank, size, root_id = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// inicialize random seed
	srand((unsigned) time(NULL));

	if (root_id == rank) {

		start = millis();

		double lado = floor(sqrt((ROWS*COLUMNS)/size));

		int sub_rows = (int) lado;
		int sub_columns = (int) lado;

		int acumulado_x = 0;
		int acumulado_y = 0;

		// for ( int i = 0; i < ROWS ; i+sub_rows ) {

		// 	for ( int j = 0; j < COLUMNS ; j+sub_columns ) {

			for ( int j = 0; j < size ; j++ ) {
			// if ( i == size-1 ) {
			// 	sub_rows += ROWS % size;
			// 	sub_columns += COLUMNS % size;
			// }
				if(acumulado_x < COLUMNS){
					acumulado_x += lado;
				}

				if(acumulado_x + lado > COLUMNS){
					sub_columns += COLUMNS % size;
					acumulado_x = 0; // reset x
					acumulado_y += lado; // proximo proc baja de lina
				}

				if(acumulado_y < ROWS){
					acumulado_y += lado;
				}

				if(acumulado_y + lado > ROWS){
					sub_rows += ROWS % size;
					// no hago mas nada xq deberiamos estar en la esquina inferior derecha (la ultima matriz)
				}

				int** sub_opinions = create_matrix(sub_rows, sub_columns);
				initialize_matrix(sub_rows, sub_columns, sub_opinions);

				print_matrix(sub_rows, sub_columns, sub_opinions);

				sub_columns = lado;
				sub_rows = lado;
			}
		// }
	}

	// if (SHOW) { clear_output(); }
	// int** opinions = create_matrix(ROWS, COLUMNS);
	// initialize_matrix(ROWS, COLUMNS, opinions);

	// for ( int t = 0; t <= MAX_ITERATIONS; ++t) {

	// 	if (SHOW) {
	// 		printf("\nT%d's opinions. Left %d times.\n\n", t, MAX_ITERATIONS - t);
	// 		print_matrix(ROWS, COLUMNS, opinions);
	// 	}

	// 	int** new_opinions = ask_new_opinions(ROWS, COLUMNS, opinions);

	// 	// from T to T+1
	// 	destroy_matrix(opinions);
	// 	opinions = new_opinions;

	// 	if (SHOW) {
	// 		wait_keypress();
	// 		clear_output();
	// 	}
	// }

	// if (!SHOW) { print_matrix(ROWS, COLUMNS, opinions); }

	MPI_Finalize();

	long long end = millis() - start;

	printf("Time: %lld\n", end);

	return (EXIT_SUCCESS);
}