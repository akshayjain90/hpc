EXECS=ssort mpi_solved1 mpi_solved2 mpi_solved3 mpi_solved4 mpi_solved5 mpi_solved6 mpi_solved7

MPICC=mpicc

all: ${EXECS}

ssort: ssort.c
	${MPICC} -o ssort ssort.c -lm -lrt

mpi_solved1 : mpi_solved1.c
	${MPICC} -o mpi_solved1 mpi_solved1.c -lm

mpi_solved2 : mpi_solved2.c
	${MPICC} -o mpi_solved2 mpi_solved2.c -lm

mpi_solved3 : mpi_solved3.c
	${MPICC} -o mpi_solved3 mpi_solved3.c -lm

mpi_solved4 : mpi_solved4.c
	${MPICC} -o mpi_solved4 mpi_solved4.c -lm

mpi_solved5 : mpi_solved5.c
	${MPICC} -o mpi_solved5 mpi_solved5.c -lm

mpi_solved6 : mpi_solved6.c
	${MPICC} -o mpi_solved6 mpi_solved6.c -lm

mpi_solved7 : mpi_solved7.c
	${MPICC} -o mpi_solved7 mpi_solved7.c -lm

clean:
	rm ${EXECS}
