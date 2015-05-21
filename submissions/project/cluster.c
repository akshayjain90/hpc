#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

int compare(const void *a, const void *b){
  return ( *(int*)a - *(int*)b );
}


int main( int argc, char *argv[])
{
/*Input consists of multiple files containing 2D points. (each line has tab separated x and y coordinates)
Multiple MPI processes are started each of which load an equal number of files in memory and calculate the local min and max for x and y. Data-points are locally sorted in place according to gridpoint order
*/
  int th = 10; 
  int rank, size, num_of_files, lines_per_file, N=0, i=0, j=0, k=0;
  int root =0;
  int xMin=INT_MAX, xMax=INT_MIN, yMin=INT_MAX, yMax=INT_MIN;

  int *vecx, *vecy;

  num_of_files = atoi(argv[1]);
  lines_per_file = atoi(argv[2]);
  if(argc >3)
	th = atoi(argv[3]);  

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank ==root)
	  printf("Started with %d MPI processes\n", size);
  if(num_of_files % size !=0){
	printf("N should divide number of files");
	exit(1);
  }

  MPI_Status status;
  
  int vecSize = lines_per_file* num_of_files /size;

  vecx = calloc(vecSize, sizeof(int));
  vecy = calloc(vecSize, sizeof(int));

  FILE *inpF;
  //this loop freaks the system out pretty bad. Parralel i/o problems?
  for (i = (num_of_files/size )*rank ; i < (num_of_files/size) *(rank+1)  ; i++) {
	char buf[30];
	sprintf(buf,"input%07d.txt",i);
	inpF = fopen(buf,"r+");

	for(j=0; j< lines_per_file; j++){
		fscanf(inpF, "%d\t%d\n", &vecx[(i -(num_of_files/size )*rank) *lines_per_file +j], &vecy[(i -(num_of_files/size )*rank)*lines_per_file +j]);
		
	}
	fclose(inpF);
  }

/*
	char buf[30];
	sprintf(buf,"input%07d.txt",rank);
	FILE *inpF = fopen(buf,"r+");

	for(j=0; j< lines_per_file; j++){
		fscanf(inpF, "%d\t%d\n", &vecx[i*lines_per_file +j], &vecy[i*lines_per_file +j]);

	}
	fclose(inpF);
*/

  
/*
  //debug
  if(rank ==root){
   printf("%d\t%d\n",vecx[0],vecy[0]);
   printf("%d\t%d\n",vecx[1],vecy[1]);
  }
*/
  /* sort locally */
  qsort(vecx, vecSize, sizeof(int), compare);
  qsort(vecy, vecSize, sizeof(int), compare);

  xMin = vecx[0];
  xMax = vecx[vecSize-1];
  yMin = vecy[0];
  yMax = vecy[vecSize-1];

  
/*
A allreduce is used to calculate global min and max and let all processes know about these values.
Each process maps these ranges to [0-100] and partitions the grid using rank( eg. 0-20,0-20 for rank 0, 0-20,20-40 for rank 1) to fill a count array that determines number of points closest to grid points belonging to different processes.
AllGather is used to communicate this count array to all processes. */


  int gxMin= INT_MAX,gxMax= INT_MIN,gyMax= INT_MIN,gyMin= INT_MAX, one=1; 
  
  //MPI_Barrier(MPI_COMM_WORLD);

  MPI_Allreduce( &xMin,&gxMin, one,MPI_INT, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce( &xMax,&gxMax, one,MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce( &yMin,&gyMin, one,MPI_INT, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce( &yMax,&gyMax, one,MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  
  int xRange = gxMax-gxMin;
  int yRange = gyMax-gyMin;

  int *gridCount;
  gridCount = calloc(10000, sizeof(int));
  
  for(i=0;i<vecSize;i++){
	int xGrid = round(100*(float)(vecx[i]-gxMin)/(float)(gxMax - gxMin)); 
	int yGrid = round(100*(float)(vecy[i]-gyMin)/(float)(gyMax - gyMin));
	if(xGrid==100 &&yGrid==100)	
		gridCount[xGrid *100 + yGrid-1]++; 	
	gridCount[xGrid *100 + yGrid]++; 
  }

  int *allGridCount;
  allGridCount = calloc(10000 *size, sizeof(int));
  
  MPI_Allgather(gridCount, 10000,MPI_INT, allGridCount, 10000,MPI_INT,  MPI_COMM_WORLD);
   
  /*
  //debug
  if(rank == root){
	for(i=0;i<size*10000;i++){
		printf("%d\n",allGridCount[1]);
	}
  }
  */

/*

Each processes calculates the density around each of its own gridpoints using the count array.
If the density exceeds a threshold for 1 or more grid points allocated to a process, it sends those gridpoints to a root(process 0). */

  int *hDPoints, tempSum=0;
  hDPoints = calloc(10000/size, sizeof(int));	
  for(i= rank* (10000/size); i< (rank+1)*10000/size;i++){
	tempSum=0;
	for(j= 0; j< size; j++){
		tempSum +=allGridCount[10000*j+ i];
	}	

	if(tempSum > th){
		hDPoints[i-rank* (10000/size)]= 1;	
	} 
  }

  int *allHD, hdCount=0;
  allHD = calloc(10000,sizeof(int));	
  int tem= 10000/size;
  if(rank ==root){
	
 	for(i =0; i<10000/size;i++){
		if(hDPoints[i] ==1){
			allHD[hdCount] =i;
			hdCount++;
		}
	}
	for(i= 1; i<size;i++){
		MPI_Recv(hDPoints, tem, MPI_INT, i, 42, MPI_COMM_WORLD, &status);
		for(j = i*10000/size; j< (i+1)*10000/size; j++){
			if(hDPoints[j-i* (10000/size)] ==1){
				allHD[hdCount] =j;
				hdCount++;
			}
		} 
	}

  } else {
	MPI_Send(hDPoints, tem, MPI_INT, root, 42, MPI_COMM_WORLD);
  }

 
  
/*
Root allocates equal number(last process might have less or a few might get just 1 point each) of these high-density grid points to each process and broadcasts the distribution.
Each process sends over the data-points close to one of these gridpoints to their respective allocated processes using sends and receives (from the earlier count array each process already knows how much data will it receive from other processes) */
  MPI_Bcast(&hdCount, 1,MPI_INT, root, MPI_COMM_WORLD);
  MPI_Bcast(allHD, hdCount, MPI_INT, root, MPI_COMM_WORLD);
    

//debug
if(rank ==0){
printf("The gridPoints to do clustering on are:\n");	 
 for(i=0; i<hdCount; i++){
	printf("%d\n", allHD[i]);
  }
}

  int *marked;
  marked = calloc(vecSize, sizeof(int));
  int *pointCountsWithGridToSend, **xToSend, **yToSend;

  pointCountsWithGridToSend = calloc(hdCount, sizeof(int));
  xToSend = calloc(hdCount, sizeof( int*));
  yToSend = calloc(hdCount, sizeof( int*));

   for(i =0 ;i<hdCount;i++){
  	pointCountsWithGridToSend[i] = gridCount[allHD[i]];
	xToSend[i] = calloc(gridCount[allHD[i]], sizeof(int));
	yToSend[i] = calloc(gridCount[allHD[i]], sizeof(int));
   } 



  int *myHDIds;
  int myIdsLen = (int)(hdCount/size);

  if(rank < hdCount % size ){
	myIdsLen++;
  }
	  
  myHDIds= calloc(myIdsLen, sizeof(int));  

 
  for(i=rank ;i<hdCount; i=i+size){
	myHDIds[(i-rank)/size] = allHD[i];
	
  }

  int *countFlags = calloc(hdCount, sizeof(int));

  //fill stuff to send 
  for(i=0;i<vecSize;i++){
	int xGrid = round(100*(float)(vecx[i]-gxMin)/(float)(gxMax - gxMin)); 
	int yGrid = round(100*(float)(vecy[i]-gyMin)/(float)(gyMax - gyMin));
	if(xGrid==100 && yGrid==100)	
		yGrid--;	
	int gridId = xGrid*100+yGrid;

	for(j=0;j<hdCount;j++){
		if(allHD[j] == gridId){
			marked[i] =1;
			xToSend[j][countFlags[j]] = vecx[i];
			yToSend[j][countFlags[j]] = vecy[i];
			countFlags[j]++;
			break;
		}	
	}
  }

/* 
  //debug
 for(i=0;i<hdCount;i++)
	printf("%d\t%d\n", pointCountsWithGridToSend[i], countFlags[i]);
*/


  //send it
  int nonZeroCount =0;
  for(i=0 ;i<hdCount;i++){
	if(pointCountsWithGridToSend[i] >0)
		nonZeroCount++;
  }

  MPI_Request *statusSI;
  statusSI = calloc(2* nonZeroCount, sizeof(MPI_Request));
  int tempCount =0;
  for( i=0 ; i<hdCount; i++){
	if(pointCountsWithGridToSend[i] >0){
		MPI_Isend(xToSend[i], pointCountsWithGridToSend[i], MPI_INT, i%size, 1, MPI_COMM_WORLD,&statusSI[tempCount++]);
		
		MPI_Isend(yToSend[i], pointCountsWithGridToSend[i], MPI_INT, i%size, 2, MPI_COMM_WORLD,&statusSI[tempCount++]);

		printf("Sending %d numbers from process %d to process %d for gridId %d \n", pointCountsWithGridToSend[i], rank, i%size,allHD[i]);
	}
  }

  int **recievedClustersX = calloc(myIdsLen, sizeof(int));
  int **recievedClustersY = calloc(myIdsLen, sizeof(int));
  int tempCount2 =0;

  int *myIdsSizes = calloc(myIdsLen, sizeof(int));

  for( i=0;i<myIdsLen; i++){
	tempCount =0;
	for(j=0;j<size;j++){
		if(allGridCount[10000*j +myHDIds[i]] >0){
			myIdsSizes[i] += allGridCount[10000*j +myHDIds[i]];	
			tempCount2++;	
		}	
	}
	recievedClustersX[i] = calloc(myIdsSizes[i] , sizeof(int));
	recievedClustersY[i] = calloc(myIdsSizes[i] , sizeof(int));
  }

  MPI_Request *statusRI;
  statusRI = calloc(2* tempCount2, sizeof(MPI_Request));

  int *countFlags2 = calloc(myIdsLen, sizeof(int));
  tempCount =0;
  for( i=0; i<size; i++){
	for( j =0;j<myIdsLen; j++){
		if(allGridCount[10000*i +myHDIds[j]] >0){
			MPI_Irecv(&recievedClustersX[j][countFlags2[j]], allGridCount[10000*i +myHDIds[j]], MPI_INT, i, 1, MPI_COMM_WORLD,&statusRI[tempCount++]);
			MPI_Irecv(&recievedClustersY[j][countFlags2[j]],allGridCount[10000*i +myHDIds[j]], MPI_INT, i, 2, MPI_COMM_WORLD,&statusRI[tempCount++]);
			countFlags2[j] += allGridCount[10000*i +myHDIds[j]];

			printf("Recieving %d numbers from process %d on process %d for gridId %d \n", allGridCount[10000*i +myHDIds[j]], i, rank,myHDIds[j]);
		}
	}
  }



/*

The unclustered(unmarked) points and the centroids with their sizes are then written out to output files.
Each cluster is written out into separate files.
*/

  char buf[30];
  sprintf(buf,"outputPoints%07d.txt",rank);
  FILE *outpF = fopen(buf,"w+");
  
  for(i=0; i< vecSize; i++){
	  	
          if(marked[i] == 0){
    		fprintf(outpF,"%d\t%d\n",vecx[i],vecy[i]);
  	  }	
  }

 fclose(outpF);

MPI_Waitall(2*nonZeroCount, statusSI, MPI_STATUSES_IGNORE);
MPI_Waitall(2*tempCount2, statusRI, MPI_STATUSES_IGNORE);
MPI_Finalize();

if(myIdsLen >0){
  sprintf(buf,"outputClusters%07d.txt",rank);
  outpF = fopen(buf,"w+");


  FILE *outpF2;
  sprintf(buf,"clusterData%05d.txt",rank);
  outpF2 = fopen(buf,"w+");
  
  for(i=0; i< myIdsLen; i++){
	  	
	fprintf(outpF,"%d\t%d\t%d\t%d\n",size*i+rank,(int)((myHDIds[i]/100)*(gxMax-gxMin)/100+gxMin),(int)((myHDIds[i]%100)*(gyMax-gyMin)/100+gyMin),myIdsSizes[i]);

        fprintf(outpF2,"\n\n%d:\n\n",size*i+rank);
	for(j=0 ; j<countFlags2[i] ; j++){
		fprintf(outpF2,"%d\t%d\n",recievedClustersX[i][j],recievedClustersY[i][j]);
	}
 	
  }

  fclose(outpF2);
  fclose(outpF);
}

  
  free(vecx);
  free(vecy);
  free(gridCount);
  free(allGridCount);
free(hDPoints);
free(allHD);
free(marked);
free(pointCountsWithGridToSend);
free(xToSend);
free(yToSend);
free(myHDIds);
free(countFlags);
free(statusSI);
free(recievedClustersX);
free(recievedClustersY);
free(myIdsSizes);
free(statusRI);
free(countFlags2);


  return 0;
}
