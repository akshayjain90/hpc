#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main( int argc, char *argv[])
{
  int num_files=100, lines_per_file=1000;
  int *vec1, *vec2, i =0, j=0;
  num_files = atoi(argv[1]);
  lines_per_file = atoi(argv[2]);
 
  for(i=0;i<num_files;i++){
	  vec1 = calloc(lines_per_file, sizeof(int));
	  vec2 = calloc(lines_per_file, sizeof(int));
	  /* seed random number generator differently on every core */
	  srand((unsigned int) (i + 393919));

	  /* fill vector with random integers */
	  for (j = 0; j < lines_per_file; ++j) {
	    vec1[j] = rand();
	    vec2[j] = rand();
	  }
	  char buf[30];
  	  sprintf(buf,"input%07d.txt",i);
	  FILE *outpF = fopen(buf,"w+");
	
          for(j=0 ;j< lines_per_file; j++){
    
    		fprintf(outpF,"%d\t%d\n",vec1[j],vec2[j]);
  	  }
 
	  fclose(outpF);

  }

return 0;
}
