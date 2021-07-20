#include "slave.h"

#define J 64
#define I 1024

extern double a[J][I],b[J][I],c[J][I];
      
void func()
{
	volatile int i,j, get_reply,put_reply;

	double a_slave[I],b_slave[I],c_slave[I];
	int my_id;
        my_id = athread_get_id(-1);
        
	get_reply = 0;
	athread_get(PE_MODE,&a[my_id][0],&a_slave[0],I*8,&get_reply,0,0,0);
	athread_get(PE_MODE,&b[my_id][0],&b_slave[0],I*8,&get_reply,0,0,0);
	while(get_reply!=2);
      
       
        for(i=0;i<I;i++){
                c_slave[i]=a_slave[i]+b_slave[i];
        }

	put_reply=0;
	athread_put(PE_MODE,&c_slave[0],&c[my_id][0],I*8,&put_reply,0,0);
        while(put_reply!=1);	

}
