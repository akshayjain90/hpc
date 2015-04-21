/******************************************************************************
* FILE: mpi_bug5.c
* DESCRIPTION: 
*   This is an "unsafe" program. It's behavior varies depending upon the
*   platform and MPI library
* AUTHOR: Blaise Barney 
* LAST REVISED: 01/24/09
* Hint: If possible, try to run the program on two different machines,
* which are connected through a network. You should see uneven timings;
* try to understand/explain them.
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MSGSIZE 2000

int main (int argc, char *argv[])
{
int        numtasks, rank, i, tag=111, dest=1, source=0, count=0;
char       data[MSGSIZE];
double     start, end, result;
MPI_Status status;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
  printf ("mpi_bug5 has started...\n");
  if (numtasks > 2) 
    printf("INFO: Number of tasks= %d. Only using 2 tasks.\n", numtasks);
  }

/******************************* Send task **********************************/
if (rank == 0) {

  /* Initialize send data */
  for(i=0; i<MSGSIZE; i++)
     data[i] =  'x';

  start = MPI_Wtime();
  while (1) {
    MPI_Send(data, MSGSIZE, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
    count++;
    if (count % 10 == 0) {
      end = MPI_Wtime();
      printf("Count= %d  Time= %f sec.\n", count, end-start);
      start = MPI_Wtime();
      }
    //Using barrier for syncronization
    MPI_Barrier(MPI_COMM_WORLD);
    }
  }

/****************************** Receive task ********************************/

if (rank == 1) {
  while (1) {
    MPI_Recv(data, MSGSIZE, MPI_BYTE, source, tag, MPI_COMM_WORLD, &status);
    /* Do some work  - at least more than the send task */
    result = 0.0;
    for (i=0; i < 1000000; i++) 
      result = result + (double)random();
    /*The above loop does more work than the send task, and since the send task is
    in a while loop, the recieve buffer will overflow at some point. To fix this ,either 
    the above loop can be removed and done outside while but that will change the program logic
    so a barrier can be used to do this. 
    Using the barrier makes the timing consistent and saves overflow, but slows down the
    program considerably. In a real world program barrier can be used once every 10, 100 or 1000
    iterations of while loop depending on the system configuration**/
    MPI_Barrier(MPI_COMM_WORLD); 
    }
  }

//Barrier needs  to be executed by all processes 
if( rank >1){
  while(1){
    MPI_Barrier(MPI_COMM_WORLD);
  }
}

MPI_Finalize();
}

