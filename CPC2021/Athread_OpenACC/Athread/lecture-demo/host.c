#include <stdlib.h>
#include <stdio.h>
#include <athread.h>


#define J 64
#define I 1024

extern SLAVE_FUN(func)();
double a[J][I],b[J][I],c[J][I];

int main(int argc, char**argv)
{
  	int i,j; 



	for(j=0;j<J;j++)
 	for(i=0;i<I;i++){
  		a[j][i]=(i+j+0.5);
	  	b[j][i]=(i+j+1.0);
	}

	athread_init();
        athread_spawn(func,0);
        athread_join();
        athread_halt();      

	return 0;  

}    
