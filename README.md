rm seq; gcc -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c; ./seq
rm openmp; mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c; ./openmp
