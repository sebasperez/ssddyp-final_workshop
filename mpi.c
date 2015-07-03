#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <omp.h>
#include <mpi.h>

// Number of iterations (program's clock starting at T0)

#define MAX_ITERATIONS	4320

// Opinions representation

#define NO_OPINION     	-1
#define OPINION_WHITE	0
#define OPINION_RED		1
#define OPINION_GREEN	2
#define OPINION_BLUE	3

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

void initialize_matrix(int rows, int columns, int** matrix, int rank, int* halo1, int* halo2) {

	int opinion;

	for ( int i = 0; i < rows; i++ ) {

		for ( int j = 0; j < columns; j++ ) {

			opinion = (rand() % OPINIONS_COUNT);
			matrix[i][j] = opinion;

			switch(rank){
				
				case 0:
					if(j == columns - 1){
						halo1[i] = opinion;
					}
					if(i == rows - 1){
						halo2[j] = opinion;
					}
					break;
				
				case 1:

					if(i == rows - 1){
						halo1[j] = opinion;
					}
					if(j == 0){
						halo2[i] = opinion;
					}
					break;
				
				case 2:

					if(i == 0){
						halo1[j] = opinion;
					}
					if(j == columns){
						halo2[i] = opinion;
					}
					break;

				case 3:

					if(i == 0){
						halo1[j] = opinion;
					}
					if(j == 0){
						halo2[i] = opinion;
					}
					break;
			}
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

int ask_opinion(int rows, int columns, int** opinions, int x, int y, int rnd, int rank, int* halo1, int* halo2, int* corner) {

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

			if(opinion == NO_OPINION){
				switch(rank){
					
					case 0:
						
						if(i == rows - 1 && j == columns - 1){
							opinion = *corner;
						} else {
							if(j == columns - 1){
								opinion = halo1[i];
							}
							if(i == rows - 1){
								opinion = halo2[j];
							}
						}

						break;
					
					case 1:

						if(i == rows - 1 && j == 0){
							opinion = *corner;
						} else {
							if(i == rows - 1){
								opinion = halo1[j];
							}
							if(j == 0){
								opinion = halo2[i];
							}
						}

						break;
					
					case 2:

						if(i == 0 && j == columns - 1){
							opinion = *corner;
						} else {
							if(i == 0){
								opinion = halo1[j];
							}
							if(j == columns - 1){
								opinion = halo2[i];
							}
						}

						break;

					case 3:

						if(i == 0 && j == 0){
							opinion = *corner;
						} else {
							if(i == 0){
								opinion = halo1[j];
							}
							if(j == 0){
								opinion = halo2[i];
							}
						}

						break;
				}
			}

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

int** ask_new_opinions(int rows, int columns, int** opinions, int rank, int* halo1, int* halo2, int* corner, int* my_halo1, int* my_halo2, int* my_corner) {

	int** new_opinions = create_matrix(rows, columns);
	int new_opinion;

	#pragma omp parallel for shared(new_opinions)
	for (int i = 0; i < rows; i++ ) {
		for ( int j = 0; j < columns; j++ ) {

			new_opinion = ask_opinion(rows, columns, opinions, i, j, rand(), rank, halo1, halo2, corner);
			new_opinions[i][j] = new_opinion;

			switch(rank){
				
				case 0:

					if(j == columns - 1){
						my_halo1[i] = new_opinion;
					}
					if(i == rows - 1){
						my_halo2[j] = new_opinion;
					}
					break;
				
				case 1:

					if(i == rows - 1){
						my_halo1[j] = new_opinion;
					}
					if(j == 0){
						my_halo2[i] = new_opinion;
					}
					break;
				
				case 2:

					if(i == 0){
						my_halo1[j] = new_opinion;
					}
					if(j == columns){
						my_halo2[i] = new_opinion;
					}
					break;

				case 3:

					if(i == 0){
						my_halo1[j] = new_opinion;
					}
					if(j == 0){
						my_halo2[i] = new_opinion;
					}
					break;
			}

		}
	}

	switch(rank){
		case 0:
			*my_corner = new_opinions[rows - 1][columns - 1];
		break;

		case 1:
			*my_corner = new_opinions[rows - 1][0];
		break;

		case 2:
			*my_corner = new_opinions[0][columns - 1];
		break;

		case 3:
			*my_corner = new_opinions[0][0];
		break;

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

	int rank, size;
	int root_id = 0;
	long long start, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	srand((unsigned) time(NULL));

	int n = atoi(argv[1]) / 2;

	omp_set_num_threads(atoi(argv[2]));

	int** opinions = create_matrix(n, n);

	// cada matriz tiene siempre 2 halos para compartir con el resto de los procesos
	int* my_halo1 = calloc(n, sizeof (int));
	int* my_halo2 = calloc(n, sizeof (int));
	
	initialize_matrix(n, n, opinions, rank, my_halo1, my_halo2);

	if ( rank == root_id ) {

		start = millis();	
	}

	// send to & recv from & corner
	int send_receive[3], my_corner;

	switch(rank){
		
		case 0:
			my_corner = opinions[n - 1][n - 1];
			send_receive[0] = 1;
			send_receive[1] = 2;
			send_receive[2] = 3;
		break;

		case 1:
			my_corner = opinions[n - 1][0];
			send_receive[0] = 3;
			send_receive[1] = 0;
			send_receive[2] = 2;
		break;

		case 2:
			my_corner = opinions[0][n - 1];
			send_receive[0] = 0;
			send_receive[1] = 3;
			send_receive[2] = 1;
		break;

		case 3:
			my_corner = opinions[0][0];
			send_receive[0] = 1;
			send_receive[1] = 2;
			send_receive[2] = 0;
		break;

	}

	int* halo1 = calloc(n, sizeof (int));
	int* halo2 = calloc(n, sizeof (int));
	int corner;

	for ( int t = 0; t < MAX_ITERATIONS; ++t) {

		MPI_Send(my_halo1, n, MPI_INT, send_receive[0], 0, MPI_COMM_WORLD);
		MPI_Send(my_halo2, n, MPI_INT, send_receive[1], 0, MPI_COMM_WORLD);
		MPI_Send(&my_corner, 1, MPI_INT, send_receive[2], 0, MPI_COMM_WORLD);

		MPI_Recv(halo1, n, MPI_INT, send_receive[0], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(halo2, n, MPI_INT, send_receive[1], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&corner, 1, MPI_INT, send_receive[2], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		int** new_opinions = ask_new_opinions(n, n, opinions, rank, halo1, halo2, &corner, my_halo1, my_halo2, &my_corner);

		destroy_matrix(opinions);

		opinions = new_opinions;
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if ( rank == root_id ) {

		end = millis() - start;
	}

	int opinions_by_type[OPINIONS_COUNT] = {0};

	for (int i = 0; i < n; i++ ) {
		for ( int j = 0; j < n; j++ ) {

			int opinion = opinions[i][j];
			opinions_by_type[opinion]++;
		}
	}
	
	int opinions_results[OPINIONS_COUNT] = {0};

	MPI_Reduce(opinions_by_type, opinions_results, OPINIONS_COUNT, MPI_INT, MPI_SUM, root_id, MPI_COMM_WORLD);

	if ( rank == root_id ) {

		if ( (argc == 4) && (atoi(argv[3]) == 1) ) {

			printf("%lld\n", end);
		}
		else {
			printf("Time: %lld\n", end);

			printf("White opinions: %d\n", opinions_results[OPINION_WHITE]);
			printf("Red opinions: %d\n", opinions_results[OPINION_RED]);
			printf("Green opinions: %d\n", opinions_results[OPINION_GREEN]);
			printf("Blue opinions: %d\n", opinions_results[OPINION_BLUE]);		
		}
	}

	MPI_Finalize();

	return (EXIT_SUCCESS);
}


// MPI_Send(&matrix[i], columns, MPI_INT, (i % size), 0, MPI_COMM_WORLD);
// MPI_Recv(&current_row, columns, MPI_INT, root_id, 0, MPI_COMM_WORLD, &status);