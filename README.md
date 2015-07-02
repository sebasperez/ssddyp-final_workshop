Especificaciones del problema:

Tamaños de las corridas: 1k, 2,5k, 5k, 10k
cantidad de threads openmp: 2, 4, 6, 8
MPI: 4 procesos
10 corridas promediadas

Como Correr:

seq:
rm seq; gcc -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c; ./seq
openmp
rm openmp; mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c; ./openmp
mpi
rm mpi; mpicc -fopenmp -std=c99 -o mpi mpi.c; mpiexec -np 4 ./mpi 100


Cada proceso de MPI debe generar su propia matriz