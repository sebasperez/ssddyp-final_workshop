rm openmp; mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c; ./openmp
