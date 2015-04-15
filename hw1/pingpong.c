/* Communication ping-pong:
 * Exchange between messages between mpirank
 * 0 <-> 1, 2 <-> 3, ....
 */

#include <stdio.h>
#include <mpi.h>

int main( int argc, char *argv[])
{
  int rank, tag, origin, destination;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int message_out = rank;
  int message_in = -1;
  tag = 99;

  if(rank % 2 == 0)
  {
    destination = rank + 1;
    origin = rank + 1;

    MPI_Send(&message_out, 1, MPI_INT, destination, tag, MPI_COMM_WORLD);
    MPI_Recv(&message_in,  1, MPI_INT, origin,      tag, MPI_COMM_WORLD, &status);
  }
  else
  {
    destination = rank - 1;
    origin = rank - 1;

    MPI_Recv(&message_in,  1, MPI_INT, origin,      tag, MPI_COMM_WORLD, &status);
    MPI_Send(&message_out, 1, MPI_INT, destination, tag, MPI_COMM_WORLD);
  }

  printf("rank %d received from %d the message %d\n", rank, origin, message_in);

  MPI_Finalize();
  return 0;
}
