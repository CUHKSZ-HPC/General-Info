#include <stdlib.h>
#include <stdio.h>
#include <athread.h>

extern SLAVE_FUN(func)();


//统计拍数
static inline unsigned long rpcc()
{
        unsigned long time;
        asm("rtc %0": "=r" (time) : );
        return time;
}

#define J 64
#define I 1024


double a[J][I],b[J][I],c[J][I],cc[J][I];
double check[J];
unsigned long counter[J];

int main(int argc, char**argv)
{
  	int i,j; 
        double checksum;
        double checksum2;
        unsigned long st,ed;



	for(j=0;j<J;j++)
 	for(i=0;i<I;i++){
  		a[j][i]=(i+j+0.5);
	  	b[j][i]=(i+j+1.0);
	}

        st=rpcc();
	for(j=0;j<J;j++)
 	for(i=0;i<I;i++){
  		cc[j][i]=(a[j][i])+(b[j][i]);
	}
        ed=rpcc();
        printf("the host counter=%ld\n",ed-st);

        checksum=0.0;
        checksum2=0.0;

	athread_init();
        st=rpcc();
        athread_spawn(func,0);
        athread_join();
        ed=rpcc();
        athread_halt();      
        printf("the manycore counter=%ld\n",ed-st);


	for(j=0;j<J;j++)
 	for(i=0;i<I;i++){
  		checksum=checksum+c[j][i];
  		checksum2=checksum2+cc[j][i];
	}
      
	printf("the master value is %f!\n",checksum2);
	printf("the manycore value is %f!\n",checksum);

	return 0;  

}    
