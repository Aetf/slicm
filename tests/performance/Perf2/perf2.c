#include "stdio.h"
#include <sys/types.h>
#include <sys/time.h>

#define INDEX 500

#define INDEX2 100

struct timeval t_start, t_end;
double t_diff;

unsigned int a[3];
unsigned int b[5];
unsigned int c[INDEX][INDEX];
unsigned int d[INDEX][INDEX];
unsigned int f[INDEX2][INDEX2][INDEX2];

int main(void)
{
		int i, j, k;

		for (i=0;i<3;i++){
			a[i] = i ;          
		}
		
		gettimeofday(&t_start,NULL);

		for (i=0;i<INDEX;i++){

			// (1) test for different confict rate
			for (j=0;j<INDEX;j++){
				// create infrequent alias
				if (j == (INDEX/2)){
					a[0] = 0;	
				}
				// create frequent alias
				if ((j & 0x1) == 0){
					a[1] ++;	
					a[2] =10;	
				}
				// test hoist
				c[i][j] = (a[0]*3)+ (a[1]*2+5) + a[2]*2;
			}

			// (2) test for different checking overhead
			for (j=0;j<INDEX;j++){
				// create infrequent alias
				if ((j & 0x3f) == 0){
					b[0]++;	
					b[1]++;	
				}
				d[i][j] = (b[0]*3)+ (b[1]*2+5); 

			}
			for (j=0;j<INDEX;j++){
				// create infrequent alias
				if ((j & 0x3f) == 0){
					b[2]++;	
					b[3]++;	
				}
				int temp = (b[2]+5 + b[3]*2) & 0x63;
				for (k =0; k < INDEX2-1; k++){
					f[k+1][i&0x63][j&0x63] = f[k][i&0x63][j&0x63] +(temp & 0x1) ;      // avoid insert checking here?
				}
			}
		}

    gettimeofday(&t_end,NULL);
    t_diff = (t_end.tv_sec - t_start.tv_sec) + (double)(t_end.tv_usec - t_start.tv_usec)/1000000;
	  printf(" ---  time spent = %.6f  --- \n", t_diff);

	  return 0;
}
