/*************************************************************************
	> File Name: function.h
	> Author: 
	> Mail: 
	> Created Time: 
 ************************************************************************/

#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "args.h"

#define VALIDATE_PASSED 1
#define VALIDATE_FAILED 0



/*****************************
 * create an empty Args struct
 * return : pointer of the created empty Args
 * implemented in function.c
 ****************************/
Args_t create_empty_args();

/*****************************
 * free arg and destroy the Args, free x, w, y if required
 * implemented in function.c
 *****************************/
void destroy_args(Args_t arg);

/******************************
 * read parameters and data to args, and the original y to ori_y
 *****************************/
int read_data(Args_t arg, const char* filename, void** ori_y);

/*******************************
 * out: the output of your program
 * ori: true results
 * return VALIDATE_PASSED if passed, otherwise VALIDATE_FAILED
 *******************************/
int validate_results(const float* out, const float* ori, int len);

/******************************
 * interface of convolution
 ******************************/
int run_multihead_attention(Args_t arg);

int naive_multihead_attention(Args_t arg);


#endif
