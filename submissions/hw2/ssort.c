/* Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>


static int compare2(const void *a, const void *b)
{
  int *da = (int *)a;
  int *db = (int *)b;

  if (*da > *db)
    return 1;
  else if (*da < *db)
    return -1;
  else
    return 0;
}

int compare(const void *a, const void *b){
  return ( *(int*)a - *(int*)b );
}

int main( int argc, char *argv[])
{
  int rank, size;
  int i, N;
  int *vec;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  /* Number of random numbers per processor (this should be increased
   * for actual tests or could be passed in through the command line */
  N = atoi(argv[1]);
  int root =0;
  vec = calloc(N, sizeof(int));
  /* seed random number generator differently on every core */
  srand((unsigned int) (rank + 393919));

  /* fill vector with random integers */
  for (i = 0; i < N; ++i) {
    vec[i] = rand();
  }
  printf("rank: %d, first entry: %d\n", rank, vec[0]);
  

  /* sort locally */
  qsort(vec, N, sizeof(int), compare);
  printf("Initial local sorted numbers for process %d is :",rank);
  for(i=0;i<N;i++){
   printf("%d \n",vec[i]);
  }
 
  int s=2; 
  if(N>1000)
    s = N/100;
 
 
 
  int *sample;
  
  printf("\nGoing to get random sample\n");
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
    printf("\nNumbers Gathered at root :\n");
    for(i=0;i<s*size; i++){
      printf("%d \n",sE[i]);
    }
 
    printf("Splitters found at root: \n");
    for(i=s;(i/s)<size;i=i+s){
      splitters[i/s-1]=sE[i];
      printf("%d \n",splitters[i/s-1]);
    }
   
  }

  MPI_Bcast(splitters, size, MPI_INT, root, MPI_COMM_WORLD);  

  int *sBucketSizes;
  int *rBucketSizes;
  sBucketSizes = calloc(size, sizeof(int));
  rBucketSizes = calloc(size, sizeof(int));
  int j=0,count=0 ;
 
  printf("\nSend bucket sizes for process %d:",rank);
 
  for(i = 0; i<size-1;i++){
   while(j!=N && vec[j]<splitters[i]){
     count++;
     j++;
   }
   sBucketSizes[i]=count;
   printf(" %d \n",sBucketSizes[i]);
   count =0;
  }
  while(j!=N){
   count++;
   j++;
  }
  sBucketSizes[size-1]=count;
  printf(" %d \n", sBucketSizes[i]);

  MPI_Alltoall(sBucketSizes,1,MPI_INT, rBucketSizes, 1, MPI_INT, MPI_COMM_WORLD);

  printf("\nRecieve bucket sizes for process %d:",rank);
  for(i =0;i<size;i++){
    printf(" %d \n", rBucketSizes[i]);
  }

  int tag =1;
  int *recBuf,rbsSum=0,rCount=0;
  for(i=0;i<size;i++){
    rbsSum +=rBucketSizes[i];
  }

  recBuf = calloc(rbsSum,sizeof(int));
  count=0; 
  if(rank % 2 == 0)
  {
   for(i=0;i<size;i++){
    
    MPI_Send(&vec[count], sBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD);
    printf("Trying to send %d integers from process %d to process %d\n",sBucketSizes[i],rank,i);
    count+=sBucketSizes[i];
    MPI_Recv(&recBuf[rCount], rBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD, &status);
    printf("Trying to recieve %d integers from process %d to process %d\n",rBucketSizes[i],rank,i);
    rCount+=rBucketSizes[i];
    }
  }
  else
  {
   for(i=0;i<size;i++){
    if(rank !=i){
     MPI_Recv(&recBuf[rCount], rBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD, &status);
     printf("Trying to recieve %d integers from process %d to process %d\n",rBucketSizes[i],rank,i);
     rCount+=rBucketSizes[i];
     MPI_Send(&vec[count], sBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD);
     printf("Trying to send %d integers from process %d to process %d\n",sBucketSizes[i],rank,i);
     count+=sBucketSizes[i];
    }else{
       
    MPI_Send(&vec[count], sBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD);
    printf("Trying to send %d integers from process %d to process %d\n",sBucketSizes[i],rank,i);
    count+=sBucketSizes[i];
    MPI_Recv(&recBuf[rCount], rBucketSizes[i], MPI_INT, i, tag, MPI_COMM_WORLD, &status);
    printf("Trying to recieve %d integers from process %d to process %d\n",rBucketSizes[i],rank,i);
    rCount+=rBucketSizes[i];
    
    }

   }

  }

  qsort(recBuf, rbsSum, sizeof(int), compare);
  char buf[20];
  sprintf(buf,"output%04d.txt",rank);
  
  FILE *outpF = fopen(buf,"w+");

  printf("\nProcess %d sorted numbers:\n ",rank);
  for(i=0 ;i<rbsSum; i++){
    printf("%d \n",recBuf[i]);
    fprintf(outpF,"%d\n",recBuf[i]);
  }

  
  fclose(outpF);
  free(vec);
  MPI_Finalize();
  return 0;
}
