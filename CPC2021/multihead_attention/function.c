/*************************************************************************
	> File Name: function.c
	> Author: 
	> Mail: 
	> Created Time: 
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "function.h"
#include "util.h"

/***************************
 * master function in master.c
 ***************************/
extern int multihead_attention(Args_t arg);

Args_t create_empty_args()
{
    Args_t arg = (Args_t)malloc(sizeof(Args));
    arg->B = arg->S = arg->D = arg->N = 0;
    arg->x = NULL;
    arg->w = NULL;
    arg->Q = NULL;
    arg->K = NULL;
    arg->V = NULL;
    arg->QK = NULL;
    arg->y = NULL;
    return arg;
}

void print_arg(Args_t arg)
{
    LOG("B: %d, S: %d, D: %d, N: %d",
            arg->B, arg->S, arg->D, arg->N);
}

void destroy_args(Args_t arg)
{
    if(arg)
    {
        if(arg->x)
            aligned_free((void*)arg->x);
        if(arg->w)
            aligned_free((void*)arg->w);
        if(arg->y)
            aligned_free(arg->y);
        if(arg->Q)
            aligned_free(arg->Q);
        if(arg->K)
            aligned_free(arg->K);
        if(arg->V)
            aligned_free(arg->V);
        if(arg->QK)
            aligned_free(arg->QK);
        free(arg);
    }
}

int read_data(Args_t arg, const char* filename, void** ori_y)
{
    char arg_name[128] = {0};
    char data_name[128] = {0};
    strcpy(arg_name, filename);
    strcpy(data_name, filename);
    strcat(arg_name, "_arg");
    strcat(data_name, "_data");

    LOG("reading args from file %s", arg_name);
    FILE* fp = fopen(arg_name, "rb");
    CHECK(fp, "open file failed: %s", arg_name);
    // read args    
    fread(arg, sizeof(Args), 1, fp);
    LOG("multihead attention args");
    print_arg(arg);

    fclose(fp);

    LOG("reading data from file %s", data_name);
    fp = fopen(data_name, "rb");
    size_t size_x = arg->B*arg->S*arg->D;
    size_t size_w = 3*arg->D*arg->D;
    size_t size_y = arg->B*arg->S*arg->D;
    size_t size_qk = arg->B*arg->N*arg->S*arg->S;
    arg->x = aligned_malloc(size_x * sizeof(float), 128);
    arg->w = aligned_malloc(size_w * sizeof(float), 128);
    arg->Q = aligned_malloc(size_y * sizeof(float), 128);
    arg->K = aligned_malloc(size_y * sizeof(float), 128);
    arg->V = aligned_malloc(size_y * sizeof(float), 128);
    arg->QK = aligned_malloc(size_qk * sizeof(float), 128);
    arg->y = aligned_malloc(size_y * sizeof(float), 128);
    *ori_y = aligned_malloc(size_y * sizeof(float), 128);

	LOG("total size of W: %d", size_x+size_w+size_y);

    memset(arg->Q, 0, size_y * sizeof(float));
    memset(arg->K, 0, size_y * sizeof(float));
    memset(arg->V, 0, size_y * sizeof(float));
    memset(arg->QK, 0, size_qk * sizeof(float));
    memset(arg->y, 0, size_y * sizeof(float));

    int cnt = fread(arg->x, sizeof(float), size_x, fp);
    cnt = fread(arg->w, sizeof(float), size_w, fp);
    cnt = fread(*ori_y, sizeof(float), size_y, fp);

    fclose(fp);

    return 0;
}

int validate_results(const float* out, const float* ori, int len)
{
    for(int i = 0;i < len; i ++)
        if(NEQUAL_F(out[i], ori[i]))
        {
            LOG("check result failed at out[%d]=%f ori[%d]=%f", i, out[i], i, ori[i]);
            return VALIDATE_FAILED;
        }
    return VALIDATE_PASSED;
}

static void _local_gemm_rcr(const float* A, const int LDA, const float* B, const int LDB, float* C, const int LDC, int M, int N, int K)
{
    for(int i = 0;i < M; i ++)
        for(int j = 0; j < N; j ++)
            for(int k = 0; k < K; k ++)
                C[i*LDC+j] += A[i*LDA+k]*B[k+j*LDB];
}

static void _local_gemm_rrr(const float* A, const int LDA, const float* B, const int LDB, float* C, const int LDC, int M, int N, int K)
{
    for(int i = 0;i < M; i ++)
        for(int j = 0; j < N; j ++)
            for(int k = 0; k < K; k ++)
                C[i*LDC+j] += A[i*LDA+k]*B[k*LDB+j];
}



static void _local_trans_head(float* src, float* dst, int B, int S, int D, int N)
{
    int pD = D/N;
#define SRC(b, s, d) src[b*S*D+s*D+d]
#define DST(b, n, s, pd) dst[b*N*S*pD + n*S*pD + s*pD + pd]
    for(int b = 0; b < B; b ++)
        for(int n = 0; n < N; n ++)
            for(int s = 0; s < S; s ++)
                for(int pd = 0; pd < pD; pd ++)
                    DST(b,n,s,pd) = SRC(b,s,n*pD+pd);
}

static void _local_trans_head_back(float* src, float* dst, int B, int S, int D, int N)
{
    int pD = D/N;
#define D3(b, s, d) dst[b*S*D+s*D+d]
#define D4(b, n, s, pd) src[b*N*S*pD + n*S*pD + s*pD + pd]
    for(int b = 0; b < B; b ++)
        for(int n = 0; n < N; n ++)
            for(int s = 0; s < S; s ++)
                for(int pd = 0; pd < pD; pd ++)
					D3(b,s,n*pD+pd) = D4(b,n,s,pd);
}


static void _local_norm(float* buf, int len)
{
	double sum = 0.0f;
	for(int i = 0;i < len; i ++)
		sum += buf[i];
	for(int i = 0;i < len;i ++)
		buf[i] /= sum;
}

static void _print_buf(float* buf, int len, const char* name)
{
	printf("====%s\n", name);
	for(int i = 0; i < 10 && i < len; i ++)
		printf("%f ", buf[i]);
	printf("\n");
}

int naive_multihead_attention(Args_t arg)
{
    const int B = arg->B;
    const int S = arg->S;
    const int D = arg->D;
    const int N = arg->N;
    const float* x = arg->x;
    const float* w = arg->w;
    float* Q = arg->Q;
    float* K = arg->K;
    float* V = arg->V;
    float* QK = arg->QK;
    float* y = arg->y;
	const int PD = D/N;
    memset(Q, 0, sizeof(float)*B*S*D);
    memset(K, 0, sizeof(float)*B*S*D);
    memset(V, 0, sizeof(float)*B*S*D);
    memset(QK, 0, sizeof(float)*B*N*S*S);
    memset(y, 0, sizeof(float)*B*S*D);
	float* QN = (float*)aligned_malloc(sizeof(float)*B*N*S*PD, 128);
	float* KN = (float*)aligned_malloc(sizeof(float)*B*N*S*PD, 128);
	float* VN = (float*)aligned_malloc(sizeof(float)*B*N*S*PD, 128);
    //calculate Q, K, V
    for(int b = 0; b < B; b ++)
    {
        _local_gemm_rcr(x+b*S*D, D, w, D, Q+b*S*D, D, S, D, D);
        _local_gemm_rcr(x+b*S*D, D, w+D*D, D, K+b*S*D, D, S, D, D);
        _local_gemm_rcr(x+b*S*D, D, w+2*D*D, D, V+b*S*D, D, S, D, D);
    }
    _local_trans_head(Q, QN, B, S, D, N);
    _local_trans_head(K, KN, B, S, D, N);
    _local_trans_head(V, VN, B, S, D, N);
#define NI(b,n,s,pd) ((((b)*N+n)*S+s)*PD+pd)
#define QKI(b,n,sh,sl) ((((b)*N+n)*S+sh)*S+sl)
	// QK = Q*KT
	for(int b = 0; b < B; b ++)
		for(int n = 0; n < N; n ++)
			_local_gemm_rcr(QN+NI(b,n,0,0), PD, KN+NI(b,n,0,0), PD, QK+QKI(b,n,0,0), S, S, S, PD);

	double norm = sqrt(PD*1.0);
	for(int i = 0; i < B*N*S*S; i ++)
		QK[i] /= norm;
	for(int b = 0; b < B; b ++)
		for(int n = 0; n < N; n ++)
			for(int s = 0; s < S; s ++)
				_local_norm(QK+QKI(b,n,s,0), S);

	// reuse Q
	memset(QN, 0, sizeof(float)*B*S*D);
	for(int b = 0; b < B; b ++)
		for(int n = 0; n < N; n ++)
			_local_gemm_rrr(QK+QKI(b,n,0,0), S, VN+NI(b,n,0,0), PD, QN+NI(b,n,0,0), PD, S, PD, S);
    //trans back
	_local_trans_head_back(QN, y, B, S, D, N);
    
	aligned_free(QN);
	aligned_free(KN);
	aligned_free(VN);
    return 0;
}
int run_multihead_attention(Args_t arg)
{
    //multihead_attention(arg);
	naive_multihead_attention(arg);
    return 0;
}

