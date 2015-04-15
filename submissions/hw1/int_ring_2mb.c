/* Communication ring for a large array:
 * Pass a message around in circle adding your rank every time
 * 0 -> 1 -> 2 -> 3 -> 0, ....
 * author: Akshay Jain 
 * a 2 MB array is created by using 250,000 floating numbers
 */

#include <stdio.h>
#include<stdlib.h>
#include <mpi.h>

int main( int argc, char *argv[])
{
  int rank, tag, origin, destination, size, i, j;
  int iterations = atoi(argv[1]);
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size); 
  
  int array_size = 250000;
  //double *message = malloc(array_size * sizeof(double));
  //double *message_in = malloc(array_size * sizeof(double));
  
  double message[array_size];
  double message_in[array_size];

  for(j=0; j< array_size; j++){
	message[j] = rank;
  }

  tag = 99; 

  for(i=0 ;i<iterations; i++){
	destination = (rank +1) % size;
	origin = (rank + size -1) % size;

	if(i == 0 && rank ==0){
		MPI_Send(message, array_size, MPI_DOUBLE, destination, tag, MPI_COMM_WORLD);	
	} else {
	  	MPI_Recv(message_in, array_size, MPI_DOUBLE, origin,      tag, MPI_COMM_WORLD, &status);
		if(iterations-1 != i || rank != size-1){		
			MPI_Send(&message, array_size, MPI_DOUBLE, destination, tag, MPI_COMM_WORLD);   
		}
	}
  }
  
  MPI_Finalize();
  return 0;
}
