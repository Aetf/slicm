// test 5:  has conflicts, multiple independent chains to be hoisted with USES.
#include "stdio.h"

int a[100];
int b[100];
int c[100];
int d[100];

int main()
{
    int x = 0,i;
    int *ptra = a;

    for (i=0;i<100;i++){
        a[i] = i;
        b[i] = 1;
    }

    for (i=0;i<100;i++){
        if (a[i] > 95){ 
            a[i+1] = 1;
            b[i+1] = 0;
        }
        c[i] = a[97]*2+ a[98]*3+ a[99]*4 + 10;
        d[i] = b[97]*2+ a[99]*3;
    }
    printf ("%d, %d, %d\n",c[97],c[98],c[99]);
    printf ("%d, %d, %d\n",d[97],d[98],d[99]);
    return 0;
}
