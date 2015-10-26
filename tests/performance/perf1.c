#include "stdio.h"
#include <sys/types.h>
#include <sys/time.h>

#define INDEX 1000
#define MEM_1 (INDEX-3)
#define MEM_2 (INDEX-2)
#define MEM_3 (INDEX-1)
#define MEM_4 10
#define MEM_5 (INDEX-1)

struct timeval t_start, t_end;
double t_diff;

int a[INDEX];
int b[INDEX];
int c[INDEX][INDEX];
int d[INDEX];

int main(void)
{
		int i, j, k;

		for (i=0;i<INDEX;i++){
			a[i] = 1 ;          
		}
		
		gettimeofday(&t_start,NULL);
		// uttermost loop test
		for (i=0;i<INDEX;i++){
			// create infrequent alias
			if (i > (INDEX - 5) ){
				a[i+1] = a[i]+1;					 // modify MEM 1 ,2 ,3 at the end
			}
			// create frequent alias
			if (i%2 == 0){
				a[MEM_4] ++; 
			} 

			// test hoist
			if (i < INDEX -10){
				b[i] = (a[MEM_1] * 2)+ (a[MEM_2]) + 3;  // hoist both
			}else {
				b[i] = a[i] + 1;                        // no hoist here
			}
			d[i] = (a[MEM_4] /100) + a[MEM_3]+ 20;    // not hoist MEM_4

			// nested loop test
			for (j=0;j<INDEX;j++){
				// create infrequent alias
				if (j == (INDEX/2)){
					a[MEM_3] = 0;	
					b[MEM_5] = 0;	
				}
				
				// test hoist
				c[i][j] = (b[MEM_5]*2) + a[MEM_3]+1;  // diff level hoist
			} 
		}

    gettimeofday(&t_end,NULL);
    t_diff = (t_end.tv_sec - t_start.tv_sec) + (double)(t_end.tv_usec - t_start.tv_usec)/1000000;
	  printf(" ---  time spent = %.6f  --- \n", t_diff);

	  return 0;
}
