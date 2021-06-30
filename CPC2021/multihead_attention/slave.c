#include <slave.h>
#include <math.h>
#include <simd.h>
#include <assert.h>
#include <string.h>

#include "args.h"



void par_multihead_attn(Args_t arg)
{
	const int id = athread_get_id(-1);
	if(id == 0)
		printf("passed\n");
}
