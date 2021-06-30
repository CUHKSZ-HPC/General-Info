/*************************************************************************
	> File Name: args.h
	> Author: 
	> Mail: 
	> Created Time: 
 ************************************************************************/

#ifndef _ARGS_H
#define _ARGS_H

typedef struct Args
{
    int B; // batch
    int S; // sequence length
    int D; // vector size
    int N; // number of heads
    float* x; // input x
    float* w; // Wq, Wk, Wv
    float* Q; 
    float* K;
    float* V;
    float* QK;
    float* y; // output
}Args, *Args_t;

#define TEST_DATA_NUM 3



#endif
