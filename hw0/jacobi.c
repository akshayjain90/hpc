#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


double residual(double* u, int N){
	double h= pow((double)N+1, 2);
	//Only the first and the last row need a special case so we add them in the starting
	double r= pow(h*(2*u[0]­u[1])­1, 2) + pow(h*(2*u[N­1]­u[N­2])­1, 2);
	int i;
	for (i=1; i<N­1; i++) {
		//Every row of A except the first and last have only ­1,2,­1 in columns incrementing by 1,
		//So we don't need a double loop here
		r += pow(h*(­u[i­1]+2*u[i]­u[i+1])­1, 2);
	}
	return sqrt(r);
}

int main(int argc, char** argv){
	int N = atoi(argv[1]), i, max_iter = 100000, iter;
	double u[N]; //Don't need heap memory as max N is 100,000 which should fit on stack
	memset(u, 0.0, sizeof(u));
	double init_res = residual(u, N);
	printf("Initial residual: %e \n", init_res);
	double res=init_res, u_1=0.0, u_2=0.0, a = 0.5/pow((double)N+1, 2), h = pow((double)N+1, 2);

	for (iter=0; iter<max_iter && (init_res/res)<1.0E6; iter++) {
		u_1 = u[0];
		u[0] = a * (1+h*u[1]);
		for (i=1; i<N­1; i++) {
			u_2 = a * (1+ h*(u_1+u[i+1]));
			u_1 = u[i];
			u[i] = u_2;
		}
		u[N­1] = a * (1+h*u_1);
		if(iter %10 ==0){
			res = residual(u, N);
			printf("Residual at iteration %d: %e \n", iter, res);
		}
	}
}
