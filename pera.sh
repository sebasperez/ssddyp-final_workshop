#!/bin/sh
rm seq; gcc-4.9 -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c;
rm openmp; gcc-4.9 -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c;
#rm mpi; mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o mpi mpi.c;

# n=1000

echo "código openmp. n=1000 threads=2"
./openmp 1000 2 >> output_omp_1k_2t

echo "código openmp. n=1000 threads=4"
./openmp 1000 4 >> output_omp_1k_4t

echo "código openmp. n=1000 threads=8"
./openmp 1000 8 >> output_omp_1k_8t

echo "código secuencial. n=1000"
./seq 1000 >> output_seq_1k

# n=2500

# echo "código openmp. n=2500 threads=2"
# ./openmp 2500 2 >> output_omp_2.5k_2t

# echo "código openmp. n=2500 threads=4"
# ./openmp 2500 4 >> output_omp_2.5k_4t

# echo "código openmp. n=2500 threads=8"
# ./openmp 2500 8 >> output_omp_2.5k_8t

# echo "código secuencial. n=2500"
# ./seq 2500 >> output_seq_2.5k

# n=5000

echo "código openmp. n=5000 threads=2"
./openmp 5000 2 >> output_omp_5k_2t

echo "código openmp. n=5000 threads=4"
./openmp 5000 4 >> output_omp_5k_4t

echo "código openmp. n=5000 threads=8"
./openmp 5000 8 >> output_omp_5k_8t

echo "código secuencial. n=5000"
./seq 5000 >> output_seq_5k
