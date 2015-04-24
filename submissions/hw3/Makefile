EXECS=gs-omp jacobi-omp omp_solved2 omp_solved3 omp_solved4 omp_solved5 omp_solved6

GCC=gcc

all: ${EXECS}

gs-omp: gs-omp.c
	${GCC} -fopenmp -o gs-omp gs-omp.c -lm

jacobi-omp : jacobi-omp.c
	${GCC} -fopenmp -o jacobi-omp jacobi-omp.c -lm

omp_solved2 : omp_solved2.c
	${GCC} -fopenmp -o omp_solved2 omp_solved2.c -lm

omp_solved3 : omp_solved3.c
	${GCC} -fopenmp -o omp_solved3 omp_solved3.c -lm

omp_solved4 : omp_solved4.c
	${GCC} -fopenmp -o omp_solved4 omp_solved4.c -lm

omp_solved5 : omp_solved5.c
	${GCC} -fopenmp -o omp_solved5 omp_solved5.c -lm

omp_solved6 : omp_solved6.c
	${GCC} -fopenmp -o omp_solved6 omp_solved6.c -lm


clean:
	rm ${EXECS}
