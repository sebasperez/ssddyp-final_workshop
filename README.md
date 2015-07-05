Especificaciones del problema:

	Tamaños de las corridas: 1k, 2,5k, 5k, 10k
	cantidad de threads openmp: 2, 4, 6, 8
	MPI: 4 procesos
	10 corridas promediadas

Como Correr:

	seq:
		rm seq; gcc-4.9 -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c; ./seq 100
		rm seq; gcc-4.9 -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c; ./seq 100 1
	openmp
		rm openmp; gcc-4.9 -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c; ./openmp 100 4
		rm openmp; gcc-4.9 -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c; ./openmp 1000 4 1
	mpi
		rm mpi; mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o mpi mpi.c; mpiexec -np 4 ./mpi 100 4
		rm mpi; mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o mpi mpi.c; mpiexec -np 4 ./mpi 100 4 1

Convenciones

	Cada proceso de MPI debe generar su propia matriz
	Las matrices son de nxn
	Solo medimos el trabajo que implica paralelización




Secuencial

echo "10 ejecuciones del código secuencial. n=100"
for i in {1..10};do rm seq; gcc -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c; ./seq 100 1; done;

echo "10 ejecuciones del código secuencial. n=1000"
for i in {1..10};do rm seq; gcc -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c; ./seq 1000 1; done;




run gcc -std=c99 -O3 -funroll-loops -ffast-math -o seq seq.c

run gcc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o openmp openmp.c

run mpicc -fopenmp -std=c99 -O4 -funroll-loops -ffast-math -o mpi mpi.c