EXECS=int_ring jacobi-mpi int_ring_2mb
MPICC=mpicc

all: ${EXECS}

int_ring: int_ring.c
	${MPICC} -o int_ring int_ring.c -lm

jacobi-mpi: jacobi-mpi.c
	${MPICC} -o jacobi-mpi jacobi-mpi.c -lm

int_ring_2mb: int_ring_2mb.c
	${MPICC} -o int_ring_2mb int_ring_2mb.c -lm

clean:
	rm ${EXECS}
