/* Communication adder ring:
 * Pass a message around in circle adding your rank every time
 * 0 -> 1 -> 2 -> 3 -> 0, ....
 * author: Akshay Jain 
 */

#include <stdio.h>
#include <mpi.h>

int main( int argc, char *argv[])
{
  int rank, tag, origin, destination, size, i;
  int iterations = atoi(argv[1]);
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size); 
  
  int message_out = rank;
  int message_in = -1;
  tag = 99; 

  for(i=0 ;i<iterations; i++){
	destination = (rank +1) % size;
	origin = (rank + size -1) % size;

	if(i == 0 && rank ==0){
		MPI_Send(&message_out, 1, MPI_INT, destination, tag, MPI_COMM_WORLD);	
	} else {
		
	  	MPI_Recv(&message_in,  1, MPI_INT, origin,      tag, MPI_COMM_WORLD, &status);
		message_out = message_in + rank;
		if(iterations-1 != i || rank != size-1){		
			MPI_Send(&message_out, 1, MPI_INT, destination, tag, MPI_COMM_WORLD);   
		} else {
			printf("the msg is %d after %d iterations with a ring of size %d\n\n", message_out, iterations, size);
		}
	}
  }
  
  MPI_Finalize();
  return 0;
}
