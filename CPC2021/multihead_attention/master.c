/*************************************************************************
	> File Name: convolution_forward.c
	> Author: 
	> Mail: 
	> Created Time: 
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <athread.h>

#include "args.h"
#include "util.h"

extern void SLAVE_FUN(par_multihead_attn)(); //declare slave parallel method


int multihead_attention(Args_t arg)
{
    athread_spawn(par_multihead_attn, arg); // spawn
    athread_join(); // wait for all slave threads finished
    return 0;
}

