#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

// Number of iterations (program's clock starting at T0)

#define MAX_ITERATIONS	4320

// Opinions representation

#define NO_OPINION		-1
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

int ask_opinion(int rows, int columns, int** opinions, int x, int y, int rnd) {

	// Inicialize Data Structures

	int neighbors_count = 0;
	int neighbors_opinion[9] = {0};
	
	int no_white_opinions_count = 0;
	int neighbors_opinion_sin_blanco[9] = {0};
	
	int opinions_count_by_type[OPINIONS_COUNT] = {0};	

	int new_opinion;

	// Proccess Matrix and fill Data Structures

	int opinion;

	int cant_red = 0;
	int cant_green = 0;
	int cant_blue = 0;

	for ( int i = x - 1, ii = 0; ii < 3; i++, ii++ ) {

		for ( int j = y - 1, jj = 0; jj < 3; j++, jj++ ) {

			opinion = (i > -1 && j > -1 && i < rows && j < columns) ? opinions[i][j] : NO_OPINION;

			if (opinion != NO_OPINION) {
				// sin importar la opinion
				neighbors_opinion[neighbors_count] = opinion;
				neighbors_count++;
			}

			switch(opinion) {
				case OPINION_RED:
					cant_red++;
					neighbors_opinion_sin_blanco[no_white_opinions_count] = opinion;
					no_white_opinions_count++;
					break;

				case OPINION_GREEN:
					cant_green++;
					neighbors_opinion_sin_blanco[no_white_opinions_count] = opinion;
					no_white_opinions_count++;
					break;

				case OPINION_BLUE:
					cant_blue++;
					neighbors_opinion_sin_blanco[no_white_opinions_count] = opinion;
					no_white_opinions_count++;
					break;
			}

		}
	}

	int draw = no_white_opinions_count > 1 &&
		     ((cant_red == cant_blue == cant_green) ||
		     (cant_red == 0 && cant_blue == cant_green)  ||
		     (cant_green == 0 && cant_blue == cant_red)  ||
		     (cant_blue == 0 && cant_red == cant_green));

	// Actually choose the opinion
	if(draw) {
		if((rnd % 100) <= 25){
			new_opinion =  OPINION_WHITE;
		} else {
			new_opinion = neighbors_opinion_sin_blanco[rnd % no_white_opinions_count];
		}
	}
	else {
		new_opinion = neighbors_opinion[rnd % neighbors_count];
	}

	return new_opinion;
}

int** ask_new_opinions(int rows, int columns, int** opinions) {

	int** new_opinions = create_matrix(rows, columns);

	for (int i = 0; i < rows; i++ ) {
		for ( int j = 0; j < columns; j++ ) {

			new_opinions[i][j] = ask_opinion(rows, columns, opinions, i, j, rand());
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

	#if SHOW
		clear_output();
	#endif

	srand((unsigned) time(NULL));

	int n = atoi(argv[1]);

	int** opinions = create_matrix(n, n);
	initialize_matrix(n, n, opinions);

	long long start = millis();

	for ( int t = 0; t < MAX_ITERATIONS; ++t) {

		#if SHOW
			printf("\nT%d's opinions. Left %d times.\n\n", t, MAX_ITERATIONS - t);
			print_matrix(n, n, opinions);
		#endif

		int** new_opinions = ask_new_opinions(n, n, opinions);

		destroy_matrix(opinions);

		opinions = new_opinions;

		#if SHOW
			wait_keypress();
			clear_output();
		#endif
	}

	long long end = millis() - start;

	int opinions_by_type[OPINIONS_COUNT] = {0};

	for (int i = 0; i < n; i++ ) {
		for ( int j = 0; j < n; j++ ) {

			int opinion = opinions[i][j];
			opinions_by_type[opinion]++;
		}
	}

	#if SHOW
		print_matrix(n, n, opinions);
	#endif

	if ( (argc == 3) && (atoi(argv[2]) == 1) ) {

		printf("%lld\n", end);
	}
	else {
		printf("Time: %lld\n", end);

		printf("White opinions: %d\n", opinions_by_type[OPINION_WHITE]);
		printf("Red opinions: %d\n", opinions_by_type[OPINION_RED]);
		printf("Green opinions: %d\n", opinions_by_type[OPINION_GREEN]);
		printf("Blue opinions: %d\n", opinions_by_type[OPINION_BLUE]);		
	}


	return (EXIT_SUCCESS);
}