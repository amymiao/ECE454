/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "TeamName",              /* Team name */

    "student1",     /* First member full name */
    "student1@ecf.utoronto.ca",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    naive_rotate(dim, src, dst);
}


char rotate_two_descr[] = "second attempt: reversing i and j to save (dim-1-j) computations";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (j = 0; j < dim; j++)
	for (i = 0; i < dim; i++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

}

char rotate_three_descr[] = "third attempt: loop unrolling";
void attempt_three(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (j = 0; j < dim; j++) {
	    for (i = 0; i < dim; i=i+8) {
	        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
            dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];
            dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];
            dst[RIDX(dim-1-j, i+4, dim)] = src[RIDX(i+4, j, dim)];
            dst[RIDX(dim-1-j, i+5, dim)] = src[RIDX(i+5, j, dim)];
            dst[RIDX(dim-1-j, i+6, dim)] = src[RIDX(i+6, j, dim)];
            dst[RIDX(dim-1-j, i+7, dim)] = src[RIDX(i+7, j, dim)];
        }
    }

}

char rotate_four_descr[] = "fourth attempt: loop unblocking";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int ii, jj;
    int block_size = 32;

    for (j = 0; j < dim; j+=block_size) {
    	for (i = 0; i < dim; i+=block_size) {
            for (jj=j; jj < j+block_size; jj++) {
                for (ii=i; ii < i+block_size; ii++) {
	                dst[RIDX(dim - 1 - jj, ii, dim)] = src[RIDX(ii, jj, dim)];
                }
            }
        }
    }

}

char rotate_five_descr[] = "fifth attempt: getting rid of RIDX, simplifying math";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int ii, jj;
    int block_size = 32;
    int dim_n = dim - 1;
    int dim_compute = dim * dim - dim;

    int temp = 0;
    for (j = 0; j < dim; j+=block_size) {
    	for (i = 0; i < dim; i+=block_size) {
            for (jj=j; jj < j+block_size; jj++) {
                int temp = dim_compute-jj*dim;
                for (ii=i; ii < i+block_size; ii++) {
	                dst[temp + ii] = src[ii*dim + jj];
                }
            }
        }
    }

}

char rotate_six_descr[] = "sixth attempt: restricting block, loop unrolling";
void attempt_six(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int ii, jj;
    int block_size = 16;
    int dim_compute = dim * dim - dim;

    for (j = 0; j < dim; j+=block_size) {
    	for (i = 0; i < dim; i+=block_size) {
            for (jj=j; jj < j+block_size; jj++) {
                int temp = dim_compute-jj*dim;
                for (ii=i; ii < i+block_size; ii=ii+8) {
	                dst[temp + ii] = src[ii*dim + jj];
                    dst[temp + ii + 1] = src[(ii+1)*dim + jj];
                    dst[temp + ii + 2] = src[(ii+2)*dim + jj];
                    dst[temp + ii + 3] = src[(ii+3)*dim + jj];
                    dst[temp + ii + 4] = src[(ii+4)*dim + jj];
                    dst[temp + ii + 5] = src[(ii+5)*dim + jj];
                    dst[temp + ii + 6] = src[(ii+6)*dim + jj];
                    dst[temp + ii + 7] = src[(ii+7)*dim + jj];
                }
            }
        }
    }

}

char rotate_seven_descr[] = "seventh attempt: loop unblocking";
void attempt_seven(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int ii, jj;
    int block_size = 8;
    int block_size2 = 32;
    int dim_n = dim - 1;
    int dim_compute = dim * dim - dim;

    int temp = 0;
    for (j = 0; j < dim; j+=block_size2) {
    	for (i = 0; i < dim; i+=block_size) {
            for (jj=j; jj < j+block_size2; jj++) {
                int temp = dim_compute-jj*dim;
                for (ii=i; ii < i+block_size; ii++) {
	                dst[temp + ii] = src[ii*dim + jj];
                }
            }
        }
    }

}

char rotate_eight_descr[] = "eighth attempt: loop unblocking";
void attempt_eight(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int ii, jj;
    int block_size = 8;
    int block_size2 = 32;
    int dim_n = dim - 1;
    int dim_compute = dim * dim - dim;

    int temp = 0;
    for (j = 0; j < dim; j+=block_size2) {
    	for (i = 0; i < dim; i+=block_size) {
            for (jj=j; jj < j+block_size2; jj=jj+2) {
                int temp = dim_compute-jj*dim;
	                dst[temp + i] = src[i*dim + jj];
                    dst[temp + i +1 ] = src[(i+1)*dim + jj];
                    dst[temp + i+2] = src[(i+2)*dim + jj];
                    dst[temp + i+3] = src[(i+3)*dim + jj];
                    dst[temp + i+4] = src[(i+4)*dim + jj];
                    dst[temp + i +5 ] = src[(i+5)*dim + jj];
                    dst[temp + i+6] = src[(i+6)*dim + jj];
                    dst[temp + i+7] = src[(i+7)*dim + jj];
                    temp = dim_compute-(jj+1)*dim;
	                dst[temp + i] = src[i*dim + jj+1];
                    dst[temp + i +1 ] = src[(i+1)*dim + jj+1];
                    dst[temp + i+2] = src[(i+2)*dim + jj+1];
                    dst[temp + i+3] = src[(i+3)*dim + jj+1];
                    dst[temp + i+4] = src[(i+4)*dim + jj+1];
                    dst[temp + i +5 ] = src[(i+5)*dim + jj+1];
                    dst[temp + i+6] = src[(i+6)*dim + jj+1];
                    dst[temp + i+7] = src[(i+7)*dim + jj+1];

            }
        }
    }

}

char rotate_nine_descr[] = "ninth attempt: more loop unrolling";
void attempt_nine(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int ii, jj;
    int block_size = 8;
    int block_size2 = 32;
    int dim_n = dim - 1;
    int dim_compute = dim * dim - dim;

    int temp = 0;
    for (j = 0; j < dim; j+=block_size2) {
    	for (i = 0; i < dim; i+=block_size) {
            for (jj=j; jj < j+block_size2; jj=jj+4) {
                int temp = dim_compute-jj*dim;
	                dst[temp + i] = src[i*dim + jj];
                    dst[temp + i +1 ] = src[(i+1)*dim + jj];
                    dst[temp + i+2] = src[(i+2)*dim + jj];
                    dst[temp + i+3] = src[(i+3)*dim + jj];
                    dst[temp + i+4] = src[(i+4)*dim + jj];
                    dst[temp + i +5 ] = src[(i+5)*dim + jj];
                    dst[temp + i+6] = src[(i+6)*dim + jj];
                    dst[temp + i+7] = src[(i+7)*dim + jj];
                    temp = dim_compute-(jj+1)*dim;
	                dst[temp + i] = src[i*dim + jj+1];
                    dst[temp + i +1 ] = src[(i+1)*dim + jj+1];
                    dst[temp + i+2] = src[(i+2)*dim + jj+1];
                    dst[temp + i+3] = src[(i+3)*dim + jj+1];
                    dst[temp + i+4] = src[(i+4)*dim + jj+1];
                    dst[temp + i +5 ] = src[(i+5)*dim + jj+1];
                    dst[temp + i+6] = src[(i+6)*dim + jj+1];
                    dst[temp + i+7] = src[(i+7)*dim + jj+1];
                    temp = dim_compute-(jj+2)*dim;
	                dst[temp + i] = src[i*dim + jj+2];
                    dst[temp + i +1 ] = src[(i+1)*dim + jj+2];
                    dst[temp + i+2] = src[(i+2)*dim + jj+2];
                    dst[temp + i+3] = src[(i+3)*dim + jj+2];
                    dst[temp + i+4] = src[(i+4)*dim + jj+2];
                    dst[temp + i +5 ] = src[(i+5)*dim + jj+2];
                    dst[temp + i+6] = src[(i+6)*dim + jj+2];
                    dst[temp + i+7] = src[(i+7)*dim + jj+2];
                    temp = dim_compute-(jj+3)*dim;
	                dst[temp + i] = src[i*dim + jj+3];
                    dst[temp + i +1 ] = src[(i+1)*dim + jj+3];
                    dst[temp + i+2] = src[(i+2)*dim + jj+3];
                    dst[temp + i+3] = src[(i+3)*dim + jj+3];
                    dst[temp + i+4] = src[(i+4)*dim + jj+3];
                    dst[temp + i +5 ] = src[(i+5)*dim + jj+3];
                    dst[temp + i+6] = src[(i+6)*dim + jj+3];
                    dst[temp + i+7] = src[(i+7)*dim + jj+3];
            }
        }
    }

}

char rotate_ten_descr[] = "tenth attempt: calvin's solution";
void attempt_ten(int dim, pixel *src, pixel *dst) 
{
	int a,b,i,j;
	int sav=0;	//LICM
	int sav1=0;	//LICM
	int sav2=0;	//LICM
	int adj_dim = dim-1;
	
	for (j=0; j<dim; j+=32)
	{
		sav = adj_dim-j;
		for (i=0; i<dim; i+=8)
		{
			//Using for loop unrolling
			for (b=0; b<32; b++)
			{
				sav1 = sav - b;
				sav2 = j+b;
				//Use for loop unrolling
				for (a=0; a<8; a++)
					dst[RIDX(sav1, i+a, dim)] = src[RIDX(i+a, sav2, dim)];
			}
		}
	}

}
/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    //add_rotate_function(&naive_rotate, naive_rotate_descr);   
    //add_rotate_function(&rotate, rotate_descr);   

    //add_rotate_function(&attempt_two, rotate_two_descr);   
    //add_rotate_function(&attempt_three, rotate_three_descr);   
    add_rotate_function(&attempt_four, rotate_four_descr);   
    add_rotate_function(&attempt_five, rotate_five_descr);   
    add_rotate_function(&attempt_six, rotate_six_descr);   
    add_rotate_function(&attempt_seven, rotate_seven_descr);   
    add_rotate_function(&attempt_eight, rotate_eight_descr);   
    add_rotate_function(&attempt_nine, rotate_nine_descr);   
    add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

