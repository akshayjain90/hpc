#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

double compute_residual(double *lu, int lN, double invhsq)
{
  int i;
  double tmp, gres = 0.0, lres = 0.0;

  for (i = 1; i <= lN; i++){
    tmp = ((2.0*lu[i] - lu[i-1] - lu[i+1]) * invhsq - 1);
    lres += tmp * tmp;
  }
  /* use allreduce for convenience; a reduce would also be sufficient */
  MPI_Allreduce(&lres, &gres, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  return sqrt(gres);
}

int main(int argc, char** argv){
	
	if(argc <2 ){
		printf("Need a value for N \n");
	}

	int N = atoi(argv[1]), i,it, iter = 10 , p , *u, dest1, dest2, rank;

  	MPI_Status status;
  	MPI_Request request_out, request_in;

  	MPI_Init(&argc, &argv);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);	

  	if(N % p != 0 || p<2){
		printf("N needs to be a multiple of p and p should be more than 1\n");
	}

	*u = calloc(N/p +2, sizeof(int));
	memset(u, 0.0, sizeof(u));

	dest1 = rank-1;
	dest2 = rank+1;
	
	double h =(double)(N+1)*(N+1);
	double u_1=0.0, u_2 = 0.0, aiiInv = 1/(2*h) ;


	for (it=0; it< iter ; iter++) {
		if(rank %2 == 0){
			if(rank !=0){
				MPI_Send(&u[1], 1, MPI_DOUBLE, dest1, it, MPI_COMM_WORLD);
				MPI_Recv(&u[0], 1, MPI_DOUBLE, dest1, it, MPI_COMM_WORLD, &status);
			}
			if(rank != p-1){
				MPI_Send(&u[N/p],1, MPI_DOUBLE, dest2, it, MPI_COMM_WORLD);
				MPI_Recv(&u[N/p+1], 1, MPI_DOUBLE, dest2, it, MPI_COMM_WORLD, &status);			
			}		
				
			
		} else {
			MPI_Recv(&u[0], 1, MPI_DOUBLE, dest1, it, MPI_COMM_WORLD, &status);
			MPI_Send(&u[1], 1, MPI_DOUBLE, dest1, it, MPI_COMM_WORLD);
					
			if(rank != p-1){
				MPI_Recv(&u[N/p+1], 1, MPI_DOUBLE, dest2, it, MPI_COMM_WORLD, &status);	
				MPI_Send(&u[N/p],1, MPI_DOUBLE, dest2, it, MPI_COMM_WORLD);			
			}	

		}
		
		if(rank == 0){
			u_1 = u[1];
			u[1] = aiiInv + u[2]/2;
		
			for(i=2; i< N/p+1; i++){
				u_2 = u[i];
				u[i] = aiiInv + u_1/2 + u[i+1]/2;
				u_1 = u_2;
			} 	
		} else {
			if(rank == p-1){
				u_1 = u[0];
				for(i=1; i< N/p; i++){
					u_2 = u[i];
					u[i] = aiiInv + u_1/2 + u[i+1]/2;
					u_1 = u_2;
				}
				u[N/p+1] = aiiInv + u_1;

			} else {
				u_1 = u[0];
				for(i=1; i< N/p +1; i++){
					u_2 = u[i];
					u[i] = aiiInv + u_1/2 + u[i+1]/2;
					u_1 = u_2;
				}
				
			} 		
			
		}	
		
		int res= compute_residual(u,N/p,h);			
		if (0 == mpirank) {
			printf("Iter %d: Residual: %g\n", it, res);
      		}
	}
	free(u);

	MPI_Finalize();

	return 0;
}

