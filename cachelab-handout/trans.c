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
#include <stdlib.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

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
    int ii, jj, i, j; // ii, jj fix the block of B, i, j scan the block of A and put the value into B's fixed block
    // int bsize = 8; // block size, 8*8*4B = 256B, which is the size of one block in cache
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7; // for 8*8 block, we need 8 temporary variables to store the value of A's block and put them into B's block together, which can reduce the miss of B's block and improve the performance.
    if (M == 61 && N == 67)   
    {
        for (ii = 0; ii < N; ii += 16) {
            for (jj = 0; jj < M; jj += 16) {
                for (i = ii; i < ii + 16 && i < N; i++) {
                    for (j = jj; j < jj + 16 && j < M; j++) {
                        tmp0 = A[i][j];
                        B[j][i] = tmp0; // in the loop of i,j the B's block is used again and again,its good locality for time.
                    }
                }
            }
        }
    } else {
        for (ii = 0; ii < N; ii += 8) {
            for (jj = 0; jj < M; jj += 8) {

                i = ii;

                for (i = ii; i < ii + 4 && i < N; i++) {
                    
                    tmp0 = A[i][jj];
                    tmp1 = A[i][jj+1];
                    tmp2 = A[i][jj+2];
                    tmp3 = A[i][jj+3];
                    
                    
                    B[jj][i]   = tmp0;
                    B[jj+1][i] = tmp1;
                    B[jj+2][i] = tmp2;
                    B[jj+3][i] = tmp3;
                    
                }

                jj = jj + 4; // for the second loop of i, j, we need to fix the block of B, so we need to move jj to the next block of B, which is jj + 4, and then we can put the value of A's block into B's block together, which can reduce the miss of B's block and improve the performance.
                for (i = ii; i < ii + 4 && i < N; i++) {
                    
                    tmp0 = A[i][jj];
                    tmp1 = A[i][jj+1];
                    tmp2 = A[i][jj+2];
                    tmp3 = A[i][jj+3];
                    
                    
                    B[jj - 4][i + 4]   = tmp0;
                    B[jj - 3][i + 4] = tmp1;
                    B[jj - 2][i + 4] = tmp2;
                    B[jj - 1][i + 4] = tmp3;
                    
                }
                
                j = 0;
                for (i = ii + 4; i < ii + 8 && i < N; i++, j++)
                {
                    tmp0 = B[i - 4][jj];
                    tmp1 = B[i - 4][jj + 1];
                    tmp2 = B[i - 4][jj + 2];
                    tmp3 = B[i - 4][jj + 3];

                    tmp4 = A[ii + 4][jj - 4 + j];
                    tmp5 = A[ii + 5][jj - 4 + j];
                    tmp6 = A[ii + 6][jj - 4 + j];
                    tmp7 = A[ii + 7][jj - 4 + j];

                    B[i - 4][jj] = tmp4;
                    B[i - 4][jj + 1] = tmp5;
                    B[i - 4][jj + 2] = tmp6;
                    B[i - 4][jj + 3] = tmp7;

                    tmp4 = A[ii + 4][jj + j];
                    tmp5 = A[ii + 5][jj + j];
                    tmp6 = A[ii + 6][jj + j];
                    tmp7 = A[ii + 7][jj + j];

                    B[i][jj] = tmp4;
                    B[i][jj + 1] = tmp5;
                    B[i][jj + 2] = tmp6;
                    B[i][jj + 3] = tmp7;

                    B[i][jj - 4] = tmp0;
                    B[i][jj - 3] = tmp1;
                    B[i][jj - 2] = tmp2;
                    B[i][jj - 1] = tmp3;
                }
                





                jj = jj - 4; // move jj back to the original position for the next block of B
            }
        }
    }

    // if (is_transpose(M, N, A, B) == 0) {
    //     printf("Transpose is incorrect!\n");
    //     exit(1);
    // }
}





/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
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
    // registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

