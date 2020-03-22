/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_1(int M, int N, int A[N][M], int B[M][N]);
void transpose_2(int M, int N, int A[N][M], int B[M][N]);
void transpose_3(int M, int N, int A[N][M], int B[M][N]);
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch (M)
    {
    case 32:
        transpose_1(M, N, A, B);
        break;
    case 64:
        transpose_2(M, N, A, B);
        break;
    case 61:
        transpose_3(M, N, A, B);
        break;
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */
char trans_1[] = "transpose_1";
void transpose_1(int M, int N, int A[N][M], int B[M][N])
{
    int bsize = 8;
    int kk = 0, jj = 0, j = 0, k = 0;
    int temp = 0;

    for (; jj < N; jj += bsize)
        for (kk = 0; kk < M; kk += bsize)
            for (j = jj; j < jj + bsize; j++)
            {
                temp = 0;
                for (k = kk; k < kk + bsize; k++)
                    if (j == k)
                        temp = A[j][k];
                    else
                        B[k][j] = A[j][k];
                if (temp != 0)
                    B[j][j] = temp;
            }
}

char trans_2[] = "transpose_2";
void transpose_2(int M, int N, int A[N][M], int B[M][N])
{
    int kk = 0, jj = 0, j = 0, k = 0;
    int a1, a2, a3, a4, a5, a6, a7, a8;

    for (jj = 0; jj < 64; jj += 8)
        for (kk = 0; kk < 64; kk += 8)
        {
            for (j = jj; j < jj + 4; j++)
            {
                // up half of A block
                k = kk;
                a1 = A[j][k];
                a2 = A[j][k + 1];
                a3 = A[j][k + 2];
                a4 = A[j][k + 3];
                a5 = A[j][k + 4];
                a6 = A[j][k + 5];
                a7 = A[j][k + 6];
                a8 = A[j][k + 7];
                // store in up half of B block(in tranverse)
                B[k][j] = a1;
                B[k + 1][j] = a2;
                B[k + 2][j] = a3;
                B[k + 3][j] = a4;
                B[k][j + 4] = a5;
                B[k + 1][j + 4] = a6;
                B[k + 2][j + 4] = a7;
                B[k + 3][j + 4] = a8;
            }

            for(k = kk; k < kk + 4; k++){
                j = jj + 4;
                a1 = B[k][j];
                a2 = B[k][j+1];
                a3 = B[k][j+2];
                a4 = B[k][j+3];
                a5 = A[j][k];
                a6 = A[j+1][k];
                a7 = A[j+2][k];
                a8 = A[j+3][k];

                B[k][j] = a5;
                B[k][j+1] = a6;
                B[k][j+2] = a7;
                B[k][j+3] = a8;
                B[k+4][j-4] = a1;
                B[k+4][j-3] = a2;
                B[k+4][j-2] = a3;
                B[k+4][j-1] = a4;
            }

            for(j = jj + 4; j < jj + 8; j++) {
                k = kk + 4;
                a1 = A[j][k];
                a2 = A[j][k+1];
                a3 = A[j][k+2];
                a4 = A[j][k+3];
                B[k][j] = a1;
                B[k+1][j] = a2;
                B[k+2][j] = a3;
                B[k+3][j] = a4;
            }
        }
}

char trans_3[] = "transpose_3";
void transpose_3(int M, int N, int A[N][M], int B[M][N])
{
    int bsize = 16, temp = 0;
    int en = bsize * (N / bsize), em = bsize * (M / bsize);
    int kk = 0, jj = 0, j = 0, k = 0, jjstore = 0, kkstore = 0;

    for (; jj < en; jj += bsize)
    {
        for (kk = 0; kk < em; kk += bsize)
        {
            jjstore = jj + bsize;
            for (j = jj; j < jjstore; j++)
            {
                temp = 0;
                kkstore = kk + bsize;
                for (k = kk; k < kkstore; k++)
                    if (j == k)
                        temp = A[j][k];
                    else
                        B[k][j] = A[j][k];
                if (temp != 0)
                    B[j][j] = temp;
            }
        }
        if (em < M)
        {
            for (j = jj; j < jjstore; j++)
                for (k = em; k < M; k++)
                    B[k][j] = A[j][k];
        }
    }
    if (en < N)
    {
        for (kk = 0; kk < em; kk += bsize)
        {
            kkstore = kk + bsize;
            for (j = en; j < N; j++)
                for (k = kk; k < kkstore; k++)
                    B[k][j] = A[j][k];
        }
        if (em < M)
            for (j = en; j < N; j++)
                for (k = em; k < M; k++)
                    B[k][j] = A[j][k];
    }
}
/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(transpose_2, trans_2);

    // registerTransFunction(transpose_3, trans_3);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
