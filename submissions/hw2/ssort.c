/* Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include "util.h"

int compare(const void *a, const void *b){
  return ( *(int*)a - *(int*)b );
}

int main( int argc, char *argv[])
{
  int rank, size,sCount;
  int i, N;
  int *vec;
/* Number of random numbers per processor (this should be increased
   * for actual tests or could be passed in through the command line */
  N = atoi(argv[1]);
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Status status;
  MPI_Request *statusI;
  statusI = calloc(2*size, sizeof(MPI_Request));
  int root =0;
  vec = calloc(N, sizeof(int));
  /* seed random number generator differently on every core */
  srand((unsigned int) (rank + 393919));

  /* fill vector with random integers */
  for (i = 0; i < N; ++i) {
    vec[i] = rand();
  }
//  printf("rank: %d, first entry: %d\n", rank, vec[0]);
  
  //Start time calculation
  timestamp_type time1, time2;
  get_timestamp(&time1);
  /* sort locally */
  qsort(vec, N, sizeof(int), compare);
  //printf("Initial local sorted numbers for process %d is :",rank);
  //for(i=0;i<N;i++){
  // printf("%d \n",vec[i]);
  //}
 
  int s=2; 
  if(N>1000)
    s = N/100;
 
 
 
  int *sample;
  
  //printf("\nGoing to get random sample\n");
  sample = calloc(s, sizeof(int));
  for(i = 0; i<s; ++i){
    sample[i] = vec[rand()%N];
  }
  
  int *sE; 
  int *splitters; 
  splitters = calloc(size-1, sizeof(int));

  if(rank ==root){
   sE = calloc(s*size, sizeof(int));
  }
  
  MPI_Gather(sample,s,MPI_INT, sE,s,MPI_INT,root, MPI_COMM_WORLD);
 
  if(rank == root){
    qsort(sE, s*size, sizeof(int), compare);
    //printf("\nNumbers Gathered at root :\n");
    //for(i=0;i<s*size; i++){
    //  printf("%d \n",sE[i]);
    //}
 
    //printf("Splitters found at root: \n");
    for(i=s;(i/s)<size;i=i+s){
      splitters[i/s-1]=sE[i];
     // printf("%d \n",splitters[i/s-1]);
    }
   
  }

  MPI_Bcast(splitters, size, MPI_INT, root, MPI_COMM_WORLD);  
  MPI_Barrier(MPI_COMM_WORLD);

  int *sBucketSizes;
  int *rBucketSizes;
  sBucketSizes = calloc(size, sizeof(int));
  rBucketSizes = calloc(size, sizeof(int));
  int j=0,count=0 ;
 
  //printf("\nSend bucket sizes for process %d:",rank);
 
  for(i = 0; i<size-1;i++){
   while(j!=N && vec[j]<splitters[i]){
     count++;
     j++;
   }
   sBucketSizes[i]=count;
  // printf(" %d \n",sBucketSizes[i]);
   count =0;
  }
  while(j!=N){
   count++;
   j++;
  }
  sBucketSizes[size-1]=count;
  //printf(" %d \n", sBucketSizes[i]);

  MPI_Alltoall(sBucketSizes,1,MPI_INT, rBucketSizes, 1, MPI_INT, MPI_COMM_WORLD);

  //printf("\nRecieve bucket sizes for process %d:",rank);
  //for(i =0;i<size;i++){
  //  printf(" %d \n", rBucketSizes[i]);
  //}

  int tag =1;
  int *recBuf,rbsSum=0,rCount=0;
  for(i=0;i<size;i++){
    rbsSum +=rBucketSizes[i];
  }
  
  MPI_Barrier(MPI_COMM_WORLD);

  recBuf = calloc(rbsSum,sizeof(int));
  count=0; 
  if(rank % 2 == 0)
  {
   for(i=0;i<size;i++){
    
    MPI_Isend(&vec[count], sBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD,&statusI[i*2]);
    //printf("Trying to send %d integers from process %d to process %d\n",sBucketSizes[i],rank,i);
    count+=sBucketSizes[i];
    MPI_Irecv(&recBuf[rCount], rBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD, &statusI[i*2+1]);
    //printf("Trying to recieve %d integers from process %d to process %d\n",rBucketSizes[i],rank,i);
    rCount+=rBucketSizes[i];
    }
  }
  else
  {
   for(i=0;i<size;i++){
    if(rank !=i){
     MPI_Irecv(&recBuf[rCount], rBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD, &statusI[i*2+1]);
     //printf("Trying to recieve %d integers from process %d to process %d\n",rBucketSizes[i],rank,i);
     rCount+=rBucketSizes[i];
     MPI_Isend(&vec[count], sBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD,&statusI[i*2]);
     //printf("Trying to send %d integers from process %d to process %d\n",sBucketSizes[i],rank,i);
     count+=sBucketSizes[i];
    }else{
       
    MPI_Isend(&vec[count], sBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD,&statusI[i*2]);
    //printf("Trying to send %d integers from process %d to process %d\n",sBucketSizes[i],rank,i);
    count+=sBucketSizes[i];
    MPI_Irecv(&recBuf[rCount], rBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD, &statusI[i*2+1]);
    //printf("Trying to recieve %d integers from process %d to process %d\n",rBucketSizes[i],rank,i);
    rCount+=rBucketSizes[i];
    
    }

   }

  }

  MPI_Waitall(2*size, statusI, MPI_STATUSES_IGNORE);
  MPI_Finalize();
  qsort(recBuf, rbsSum, sizeof(int), compare);
  
  get_timestamp(&time2);
  double elapsed = timestamp_diff_in_seconds(time1,time2);
  if (0 == rank) {
    printf("Time elapsed for N=%d is %f seconds.\n",N, elapsed);
  }
  char buf[30];
  sprintf(buf,"output%7d_%04d.txt",N,rank);
  
  FILE *outpF = fopen(buf,"w+");

  //printf("\nProcess %d sorted numbers:\n ",rank);
  for(i=0 ;i<rbsSum; i++){
    //printf("%d \n",recBuf[i]);
    fprintf(outpF,"%d\n",recBuf[i]);
  }

  if(rank ==0){
   free(sE);
  }
  fclose(outpF);
  free(rBucketSizes);
  free(sBucketSizes);
  free(splitters);
  free(sample);
  free(vec);
  free(recBuf);
  
  return 0;
}
