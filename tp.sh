#!/bin/sh
rm seq; gcc-4.9 -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c;
rm openmp; gcc-4.9 -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c;
rm mpi; mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o mpi mpi.c;

echo "5 ejecuciones del código openmp. n=1000 threads=2"
for i in {1..5};do ./openmp 1000 2 1 >> output_omp_1k_2; done

echo "5 ejecuciones del código openmp. n=2500 threads=2"
for i in {1..5};do ./openmp 2500 2 1 >> output_omp_2.5k_2; done

echo "5 ejecuciones del código openmp. n=1000 threads=4"
for i in {1..5};do ./openmp 1000 4 1 >> output_omp_1k_4; done

echo "5 ejecuciones del código openmp. n=2500 threads=4"
for i in {1..5};do ./openmp 2500 4 1 >> output_omp_2.5k_4; done

echo "5 ejecuciones del código openmp. n=1000 threads=8"
for i in {1..5};do ./openmp 1000 8 1 >> output_omp_1k_8; done

echo "5 ejecuciones del código openmp. n=2500 threads=8"
for i in {1..5};do ./openmp 2500 8 1 >> output_omp_2.5k_8; done

echo "5 ejecuciones del código secuencial. n=1000"
for i in {1..5};do ./seq 1000 1 >> output_seq_1k; done

echo "5 ejecuciones del código secuencial. n=2500"
for i in {1..5};do ./seq 2500 1 >> output_seq_2.5k; done