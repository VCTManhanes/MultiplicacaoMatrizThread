#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

//Matrices rows and columns sizes
int amat_row, amat_col, bmat_row, bmat_col;
//declaring files
FILE *fa;
FILE *fb;
FILE *fc ;

//Struct to be passed to threads
struct matricesObj {
	//a is 1st input matrix , b is second & c is the output matrix
    int **a, **b, **c;
    //i & to store the next true row or column to be calculated
    int i, j;
};

//function to read row and column dimensions  from file
//a&b matrix row and columns  global variables to be passed
void readrowcol(FILE *f ,int *row,int *col){
	//get character by character
	char c = fgetc(f);
	int count = 0;
	//loop over file's first line
	while(1){
		//if = found
		if(c == '='){
			//if 2nd time so its "col="
			if(count){
				//fscanf by filtering an integer & put it in row & col
				fscanf(f, "%d",col);
				return ;
			}
			else{
				fscanf(f, "%d",row);
				count++;
			}
		}
		c=fgetc(f);
	}

}
//function to copy file contents to 2d array
//array to be copied in , file to be read ,row ,column
void readmatrix(int **array,FILE *f,int row,int col){
	for(int i=0 ;i<row ;i++){
		for(int j=0 ;j<col;j++){
			//int num ;
			fscanf(f, "%d",&array[i][j]);
			//array[i][j] =  num ;
		}
	}

}
//Function to solve in one thread
void SolveForMatrix(int **a,int **b,int **c){

	for(int i = 0 ;i<amat_row ; i++){
		for(int j=0 ;j<bmat_col;j++){
			c[i][j] = 0;
			for(int r =0 ;r<amat_col;r++){
				c[i][j] += a[i][r] * b[r][j] ;
			}
		}
	}


}
//Function to solve matrix by rows , a thread for each row
void *SolveForRow(void *object){
	//initialize new struct (object)
	struct matricesObj *SystemDataRow   ;
	SystemDataRow = (struct matricesObj *) object ;
	//current i (row) in matrix a to be multiplied by columns in matrix b
	int i = SystemDataRow->i ;
	//get input and output arrays
	int **a = SystemDataRow->a , **b = SystemDataRow->b ,**c = SystemDataRow->c ;

		for(int j=0 ;j<bmat_col;j++){
			c[i][j] = 0;
			for(int r =0 ;r<amat_col;r++){
				c[i][j] += a[i][r] * b[r][j] ;
			}
		}


//exit after finishing
pthread_exit(NULL) ;

}

//Function to solve matrix by each elemnt , a thread for each element
void *SolveForElement(void *object){
	//initialize new struct (object)
	struct matricesObj *SystemDataCol   ;
	SystemDataCol = (struct matricesObj *) object ;

	//current i (row) & j (column) in matrix a & b respectively
	int i = SystemDataCol->i , j =SystemDataCol->j;
	//get input and output arrays
	int **a = SystemDataCol->a , **b = SystemDataCol->b ,**c = SystemDataCol->c ;

		//calculate element c[i][j]
		c[i][j] = 0;
		for(int r =0 ;r<amat_col;r++){
			c[i][j] += a[i][r] * b[r][j] ;
		}

//exit after finishing
pthread_exit(NULL) ;

}
//creating Threads for solving by rows
void RowThreads(int **a,int **b,int **c){
	//Array of threads has #row threads
	 pthread_t threads[amat_row];
	 int rc ;
	 //generate threads
	 for(int i=0;i<amat_row;i++){
		 //set struct to be passed to the function
		 struct matricesObj *object = malloc(sizeof(struct matricesObj));
		 object->a = a, object->b = b, object->c = c;
		 object->i = i;
		 //create pthread
		 rc = pthread_create(&threads[i], NULL, SolveForRow, (void *) object);
		 //Check if thread is not created
		 if(rc){
			 fprintf(stderr, "ERROR thread can not be created \n");
			 exit(-1);
		 }
	 }

	 //join threads
	 for (int i = 0; i < amat_row; ++i) {

	         pthread_join(threads[i], NULL);

	         if (rc) {   // Check if thread is not joined
	             fprintf(stderr, "ERROR");
	             exit(-1);
	         }
	     }
}
//Creating Threads for solving by each element
void ElementsThreads(int **a,int **b,int **c){
	//number of threads
	int threadsnum =  amat_row * bmat_col ;
	//Array of threads has #element threads
	 pthread_t threads[threadsnum];
	 int rc ;
	 //generate threads
	 for(int i=0;i<amat_row;i++){
		 for(int j=0; j<bmat_col;j++){
			 //set struct to be passed to the function
			 struct matricesObj *object = malloc(sizeof(struct matricesObj));
			 object->a = a, object->b = b, object->c = c;
			 object->i = i;
			 object->j = j;
			 //create pthread
			 rc = pthread_create(&threads[i*bmat_col + j], NULL, SolveForElement, (void *) object);
			 //Check if thread is not created
			 if(rc){
				 fprintf(stderr, "ERROR thread can not be created \n");
				 exit(-1);
		 }
	 }
}

	 //join threads
	 for (int i = 0; i < amat_row; ++i) {
		 for(int j =0 ; j< bmat_col;j++){

	         pthread_join(threads[i*bmat_col+j], NULL);

	         if (rc) {   // Check if thread is not joined
	             fprintf(stderr, "ERROR");
	             exit(-1);
	         }
	     }
}

}
// Writes output array to a distinct file
void WriteOutput(int **c, char *filepath) {
    FILE *f = fopen(filepath, "w");
    if (f == NULL) {     // Check if file doesn't exist
          fprintf(stderr, "Cannot open one of the files 3\n");
          exit(1);
      }
    fprintf(f, "row=%d col=%d\n", amat_row,bmat_col);

    for (int i = 0; i < amat_row; ++i) {
        for (int j = 0; j < bmat_col; ++j) {
            if (j != bmat_col - 1) {
                fprintf(f, "%d\t", c[i][j]);
            } else {
                fprintf(f, "%d\n", c[i][j]);
            }
        }

}
    fclose(f);
}
int main(int argc, char *argv[]) {

	// Default files' names , if there are not arguments
	    char *MatAPath = "a.txt", *MatBPath = "b.txt", *MatCPath = "c.out";

	    // Taking arguments if exist
	    if (argc == 2) {
	        MatAPath = argv[1];
	    } else if (argc == 3) {
	        MatAPath = argv[1];
	        MatBPath = argv[2];
	    } else if (argc == 4) {
	        MatAPath = argv[1];
	        MatBPath = argv[2];
	        MatCPath = argv[3];
	    } else if (argc > 4) {      // Print error if too many arguments
	        fprintf(stderr, "Exceeded arguments than allowed \n");
	        exit(1);
	    }

	    fa = fopen(MatAPath,"r");
	    fb = fopen(MatBPath,"r");

	//read row and column dimensions from a and b files

	  if (fa== NULL) {
	        fprintf(stderr, "Cannot open one of the files 1\n");
	        exit(1);
	    }
	  if (fb == NULL) {     // Check if file doesn't exist
	  	        fprintf(stderr, "Cannot open one of the files 2\n");
	  	        exit(1);
	  	    }
	readrowcol(fa, &amat_row, &amat_col);



	readrowcol(fb, &bmat_row, &bmat_col);

	//allocate 2d arrays to store matrices from file
	 int **a = (int **) malloc(amat_row * sizeof(int *));
	 for (int i = 0; i < amat_row; ++i) {
	     a[i] = (int *) malloc(amat_col * sizeof(int));
	   }
	 int **b = (int **) malloc(bmat_row * sizeof(int *));
	 	 for (int i = 0; i < bmat_row; ++i) {
	 	     b[i] = (int *) malloc(bmat_col * sizeof(int));
	 	   }

	 //store input matrices in arrays
	 readmatrix(a, fa, amat_row, amat_col);
	 readmatrix(b, fb, bmat_row, bmat_col);



	 //allocate output memory
	 int **c = (int **) malloc(amat_row * sizeof(int *));
	 	 	 for (int i = 0; i < amat_row; ++i) {
	 	 	     c[i] = (int *) malloc(bmat_col * sizeof(int));
	 	 	   }

	 //if matrices dimensions not suitable return
	 if(!(amat_col == bmat_row)){
		 printf("Can not multiply matrices (false dimensions) " );
		 return 0 ;
	 }

	 struct timeval stop, start;

	 //measure time for first method ( one thread )
	 gettimeofday(&start, NULL); //start checking time
	 printf("Method 1 threads : 1 \n");
	 SolveForMatrix(a, b, c) ;

	 gettimeofday(&stop, NULL); //end checking time

	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
	 printf("---------------------------------------\n");

	 //measure time for 2nd method ( thread per row )
	 gettimeofday(&start, NULL); //start checking time

	 printf("Method 2 threads : %d \n", amat_row );
	 RowThreads(a, b, c);

	 gettimeofday(&stop, NULL); //end checking time

	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
	 printf("---------------------------------------\n");
	 //measure time for 3rd method ( thread per element )
	 gettimeofday(&start, NULL); //start checking time
	 printf("Method 3 threads : %d \n", amat_row*bmat_col );
	 ElementsThreads(a, b, c) ;

	 gettimeofday(&stop, NULL); //end checking time

	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

	 //write output array in the output file
	 WriteOutput(c, MatCPath);

	 fclose(fa) ;
	 fclose(fb) ;

	 //free allocated memory of I/O arrays
	 free(a);
	 free(b);
	 free(c);

	return 0;
}

