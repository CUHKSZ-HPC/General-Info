/*************************************************************************
	> File Name: util.h
	> Author: 
	> Mail: 
	> Created Time: Fri Jun  4 14:25:14 2021
 ************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>

#define LOG(format, ...) do{ \
    printf("INFO [%s %d]: %s => " format " \n", \
           __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    }while(0) 

#define CHECK(cond, format, ...) do{ \
    if(!(cond)) \
        printf("ERROR [%s %d]: %s => " format " \n", \
               __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);  \
    }while(0) 

#define _F_PRECISION 1e-5
#define EQUAL_F(a, b) (fabs((a)-(b)) < (fabs(a)+fabs(b))*_F_PRECISION)
#define NEQUAL_F(a, b) (fabs((a)-(b)) >= (fabs(a)+fabs(b))*_F_PRECISION)

typedef struct timeval TIME_T;
#define MARK_TIME(t) gettimeofday(&t, NULL)
#define DIFF_TIME(start, end) ((end.tv_sec-start.tv_sec)*1e3+(end.tv_usec-start.tv_usec)*1e-3) // ms

static inline void* aligned_malloc(size_t size, int align)
{
    CHECK((align&(align-1)) == 0, "not supported align : %d", align);
    void* tmp = malloc(size + align + sizeof(unsigned long));
    CHECK(tmp != NULL, "malloc failed, size %lu", size);
    unsigned long* ret = (unsigned long*)(((unsigned long)tmp + align)& ~((unsigned long)(align-1)));
    *(ret-1) = (unsigned long)tmp;
    return (void*)ret;
}

static inline void aligned_free(void* ptr)
{
    unsigned long* pptr = (unsigned long*)ptr;
    void* real_ptr = (void*)(*(pptr-1));
    free(real_ptr);
}

#endif
