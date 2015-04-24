#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

double lres=0.0;

double compute_residual(double *lu, int lN, double invhsq){

	int i;
	double tmp;
	lres=0.0;
	#pragma omp parallel			
	{
		#pragma omp for reduction(+:lres)
		for (i = 1; i < lN; i++){
			tmp = ((2.0*lu[i] - lu[i-1] - lu[i+1]) * invhsq - 1);
			lres += tmp * tmp;
		}	
	}
	return sqrt(lres);
}

int main(int argc, char** argv){
	
	if(argc <3 ){
		printf("Need a value for N and number of iterations\n");
		exit(1);
	}
	

	int N = atoi(argv[1]), i,it, iter = atoi(argv[2]), tid;
	double *u1, *u2;

	u1 = (double *) calloc(N+2,sizeof(double));
	u2 = (double *) calloc(N+2,sizeof(double));
	
	double ihsq =(double)(N+1)*(N+1);
	double aiiInv = 1/(2*ihsq),res ;
	
	for(it=0; it<iter; it++){			

		if(it %2 ==0){
			#pragma omp parallel			
			{			
				#pragma omp for schedule(dynamic,100)
				for(i=1; i< N+1; i=i+2){				
					u2[i] = aiiInv + u1[i-1]/2 + u1[i+1]/2;		
				}
			
				#pragma omp for schedule(dynamic,100)
				for(i=2; i< N+1; i=i+2){				
					u2[i] = aiiInv + u2[i-1]/2 + u2[i+1]/2;		
				}
			}
		} else {
			#pragma omp parallel			
			{
				#pragma omp for schedule(dynamic,100)
				for(i=1; i< N+1; i=i+2){				
					u1[i] = aiiInv + u2[i-1]/2 + u2[i+1]/2;		
				}
				#pragma omp for schedule(dynamic,100)
				for(i=2; i< N+1; i=i+2){				
					u1[i] = aiiInv + u1[i-1]/2 + u1[i+1]/2;		
				}	
			}
		}
		
		if(it %100 ==0){		
			res= compute_residual(u2,N+1,ihsq);	
			if(omp_get_thread_num()==0)
				printf("Iter %d:Residual: %g \n", it, res);
		}
	}

        res= compute_residual(u2,N+1,ihsq);
        printf("Iter %d:Residual: %g \n", iter, res);

	free(u1);
	free(u2);

	return 0;
}

