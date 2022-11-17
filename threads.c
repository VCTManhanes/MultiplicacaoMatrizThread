#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

int amat_row, amat_col, bmat_row, bmat_col;
FILE *fa;
FILE *fb;
FILE *fc ;

struct matricesObj {
    int **a, **b, **c;
    int i, j;
};

void readrowcol(FILE *f ,int *row,int *col){
	char c = fgetc(f);
	int count = 0;
	while(1){
		if(c == '='){
			if(count){
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

void readmatrix(int **array,FILE *f,int row,int col){
	for(int i=0 ;i<row ;i++){
		for(int j=0 ;j<col;j++){
			fscanf(f, "%d",&array[i][j]);;
		}
	}

}

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

void *SolveForRow(void *object){
	struct matricesObj *SystemDataRow   ;
	SystemDataRow = (struct matricesObj *) object ;
	int i = SystemDataRow->i ;
	int **a = SystemDataRow->a , **b = SystemDataRow->b ,**c = SystemDataRow->c ;

		for(int j=0 ;j<bmat_col;j++){
			c[i][j] = 0;
			for(int r =0 ;r<amat_col;r++){
				c[i][j] += a[i][r] * b[r][j] ;
			}
		}
	pthread_exit(NULL) ;
}

void *SolveForElement(void *object){
	struct matricesObj *SystemDataCol   ;
	SystemDataCol = (struct matricesObj *) object ;
	int i = SystemDataCol->i , j =SystemDataCol->j;
	int **a = SystemDataCol->a , **b = SystemDataCol->b ,**c = SystemDataCol->c ;
	c[i][j] = 0;
		for(int r =0 ;r<amat_col;r++){
			c[i][j] += a[i][r] * b[r][j] ;
		}
	pthread_exit(NULL) ;
}

void RowThreads(int **a,int **b,int **c){
	 pthread_t threads[amat_row];
	 int rc ;
	 for(int i=0;i<amat_row;i++){
		 struct matricesObj *object = malloc(sizeof(struct matricesObj));
		 object->a = a, object->b = b, object->c = c;
		 object->i = i;
		 rc = pthread_create(&threads[i], NULL, SolveForRow, (void *) object);
		 if(rc){
			 fprintf(stderr, "ERROR thread can not be created \n");
			 exit(-1);
		 }
	 }

	 for (int i = 0; i < amat_row; ++i) {

	         pthread_join(threads[i], NULL);

	         if (rc) {   
	             fprintf(stderr, "ERROR");
	             exit(-1);
	         }
	     }
}

void ElementsThreads(int **a,int **b,int **c){
	int threadsnum =  amat_row * bmat_col ;
	 pthread_t threads[threadsnum];
	 int rc ;
	 for(int i=0;i<amat_row;i++){
		 for(int j=0; j<bmat_col;j++){
			 struct matricesObj *object = malloc(sizeof(struct matricesObj));
			 object->a = a, object->b = b, object->c = c;
			 object->i = i;
			 object->j = j;
			 rc = pthread_create(&threads[i*bmat_col + j], NULL, SolveForElement, (void *) object);
			 if(rc){
				 fprintf(stderr, "ERROR thread can not be created \n");
				 exit(-1);
		 	}
	 	}
	}

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

void WriteOutput(int **c, char *filepath) {
    FILE *f = fopen(filepath, "w");
    if (f == NULL) {     
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
	    char *MatAPath = "a.txt", *MatBPath = "b.txt", *MatCPath = "c.out";
	    if (argc == 2) {
	        MatAPath = argv[1];
	    } else if (argc == 3) {
	        MatAPath = argv[1];
	        MatBPath = argv[2];
	    } else if (argc == 4) {
	        MatAPath = argv[1];
	        MatBPath = argv[2];
	        MatCPath = argv[3];
	    } else if (argc > 4) {
	        fprintf(stderr, "Exceeded arguments than allowed \n");
	        exit(1);
	    }
	    fa = fopen(MatAPath,"r");
	    fb = fopen(MatBPath,"r");
	  if (fa== NULL) {
	        fprintf(stderr, "Cannot open one of the files 1\n");
	        exit(1);
	    }
	  if (fb == NULL) {  
	  	        fprintf(stderr, "Cannot open one of the files 2\n");
	  	        exit(1);
	  	    }
	readrowcol(fa, &amat_row, &amat_col);
	readrowcol(fb, &bmat_row, &bmat_col);

	 int **a = (int **) malloc(amat_row * sizeof(int *));
	 for (int i = 0; i < amat_row; ++i) {
	     a[i] = (int *) malloc(amat_col * sizeof(int));
	   }
	 int **b = (int **) malloc(bmat_row * sizeof(int *));
	 	 for (int i = 0; i < bmat_row; ++i) {
	 	     b[i] = (int *) malloc(bmat_col * sizeof(int));
	 	   }

	 readmatrix(a, fa, amat_row, amat_col);
	 readmatrix(b, fb, bmat_row, bmat_col);



	 int **c = (int **) malloc(amat_row * sizeof(int *));
	 	 	 for (int i = 0; i < amat_row; ++i) {
	 	 	     c[i] = (int *) malloc(bmat_col * sizeof(int));
	 	 	   }

	 if(!(amat_col == bmat_row)){
		 printf("Can not multiply matrices (false dimensions) " );
		 return 0 ;
	 }

	 struct timeval stop, start;

	 gettimeofday(&start, NULL); 
	 printf("Method 1 threads : 1 \n");
	 SolveForMatrix(a, b, c) ;
	 gettimeofday(&stop, NULL);
	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
	 printf("---------------------------------------\n");


	 gettimeofday(&start, NULL);
	 printf("Method 2 threads : %d \n", amat_row );
	 RowThreads(a, b, c);
	 gettimeofday(&stop, NULL);

	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
	 printf("---------------------------------------\n");
	 
	 gettimeofday(&start, NULL);
	 printf("Method 3 threads : %d \n", amat_row*bmat_col );
	 ElementsThreads(a, b, c) ;
	 gettimeofday(&stop, NULL);
	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

	 WriteOutput(c, MatCPath);

	 fclose(fa) ;
	 fclose(fb) ;

	 free(a);
	 free(b);
	 free(c);

	return 0;
}

