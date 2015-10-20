#include "stdio.h"
#include <sys/types.h>
#include <sys/time.h>

#define INDEX 1000

struct timeval t_start, t_end;
double t_diff;

unsigned int a[3];
unsigned int b[5];
unsigned int c[INDEX][INDEX];
unsigned int d[INDEX][INDEX];

int main(void)
{
		int i, j, k;

		for (i=0;i<3;i++){
			a[i] = i ;          
		}
		
		gettimeofday(&t_start,NULL);

		for (i=0;i<INDEX;i++){

			for (j=0;j<INDEX;j++){
				// create alias of different frequency
			  if ((j & 0xf)== 0){
					a[0] ++;	
				}
				if ((j & 0x1f) == 0){
					a[1] ++;	
				}
				if ((j & 0x3f) == 0){
					a[2] ++;	
				}
				// test hoist
				int temp_1 = ((a[0]*2 + 20 ) & 0xff  -5) << 1;
				int temp_2 = (((a[0]*3+31)&0xff) + ((a[1]*4)+2)&0xfff)   ;
				int temp_3 = (a[0]*5 -1 )+ ((a[1]+3)&0xff0) + a[2]  ;
				c[i][j] = temp_1 + temp_2 + temp_3;
			}
		}

    gettimeofday(&t_end,NULL);
    t_diff = (t_end.tv_sec - t_start.tv_sec) + (double)(t_end.tv_usec - t_start.tv_usec)/1000000;
	  printf(" ---  time spent = %.6f  --- \n", t_diff);

	  return 0;
}
