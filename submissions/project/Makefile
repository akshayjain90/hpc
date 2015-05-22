EXECS=create_random_data cluster
MPICC=mpicc
CC=gcc

all: ${EXECS}

create_random_data: create_random_data.c
	${CC} -o create_random_data create_random_data.c -lm

cluster : cluster.c
	${MPICC} -o cluster cluster.c -lm


clean:
	rm ${EXECS}
	rm input*
	rm output*
	rm clusterData* 
