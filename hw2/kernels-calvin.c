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
    "Anonymous",              /* Team name */

    "Calvin",     /* First member full name */
    "calvin@ecf.utoronto",  /* First member email address */

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

//Iterating through j in the outer loop and keeping i in the inner loop results in a CPE speedup to 1.4
void rotate_1_4_iterator_change(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX((dim-1)-(j), (i), dim)] = src[RIDX( (i), (j), dim )];
}

//using manual labour to unroll
void rotate_1_4_unrolled_once(int dim, pixel *src, pixel *dst)
{
	int adj_dim = dim-1; 	//LICM modification
	int i,j;

	for (j=0; j<dim; j++)
		for (i=0; i<dim; i++)
		{
			//dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
			dst[RIDX(adj_dim-j-0, i+0, dim)] = src[RIDX(i+0, j+0, dim)];
		}
}

//using for loop to unroll
void rotate_1_4(int dim, pixel *src, pixel *dst) 
{
	int a,b,i,j;
	int adj_dim = dim-1; //LICM
	//LOOP UNROLLING
	for (j=0; j<dim; j++)
		for (i=0; i<dim; i++)
			for (b=0; b<1; b++)
				for (a=0; a<1; a++)
					dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
}

//using manual labour to unroll
void rotate_1_7_unrolled(int dim, pixel *src, pixel *dst)
{
	int adj_dim = dim-1; 	//LICM modification
	int i,j;

	for (j=0; j<dim; j+=2)
		for (i=0; i<dim; i+=2)
		{
		  //dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
			dst[RIDX(adj_dim-j-0, i+0, dim)] = src[RIDX(i+0, j+0, dim)]; //b=0, a=0
			dst[RIDX(adj_dim-j-0, i+1, dim)] = src[RIDX(i+1, j+0, dim)]; //b=0, a=1
			dst[RIDX(adj_dim-j-1, i+0, dim)] = src[RIDX(i+0, j+1, dim)]; //b=1, a=0
			dst[RIDX(adj_dim-j-1, i+1, dim)] = src[RIDX(i+1, j+1, dim)]; //b=1, a=1
		}
}


//using for loop to unroll - this does unrolling using for loop
void rotate_1_6(int dim, pixel *src, pixel *dst) 
{
	int a,b,i,j;
	int adj_dim = dim-1; //LICM
	//LOOP UNROLLING
	for (j=0; j<dim; j+=2)
		for (i=0; i<dim; i+=2)
			for (b=0; b<2; b++)
				for (a=0; a<2; a++)
					dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
}

//using manual labour to unroll
void rotate_1_8(int dim, pixel *src, pixel *dst)
{
	int adj_dim = dim-1; 	//LICM modification
	int i,j;

	for (j=0; j<dim; j+=4)
		for (i=0; i<dim; i+=4)
		{
		  //dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
			dst[RIDX(adj_dim-j-0, i+0, dim)] = src[RIDX(i+0, j+0, dim)]; //b=0, a=0
			dst[RIDX(adj_dim-j-0, i+1, dim)] = src[RIDX(i+1, j+0, dim)]; //b=0, a=1
			dst[RIDX(adj_dim-j-0, i+2, dim)] = src[RIDX(i+2, j+0, dim)]; //b=0, a=2
			dst[RIDX(adj_dim-j-0, i+3, dim)] = src[RIDX(i+3, j+0, dim)]; //b=0, a=3
			
			dst[RIDX(adj_dim-j-1, i+0, dim)] = src[RIDX(i+0, j+1, dim)]; //b=1, a=0
			dst[RIDX(adj_dim-j-1, i+1, dim)] = src[RIDX(i+1, j+1, dim)]; //b=1, a=1
			dst[RIDX(adj_dim-j-1, i+2, dim)] = src[RIDX(i+2, j+1, dim)]; //b=1, a=2
			dst[RIDX(adj_dim-j-1, i+3, dim)] = src[RIDX(i+3, j+1, dim)]; //b=1, a=3
			
			dst[RIDX(adj_dim-j-2, i+0, dim)] = src[RIDX(i+0, j+2, dim)]; //b=2, a=0
			dst[RIDX(adj_dim-j-2, i+1, dim)] = src[RIDX(i+1, j+2, dim)]; //b=2, a=1
			dst[RIDX(adj_dim-j-2, i+2, dim)] = src[RIDX(i+2, j+2, dim)]; //b=2, a=2
			dst[RIDX(adj_dim-j-2, i+3, dim)] = src[RIDX(i+3, j+2, dim)]; //b=2, a=3
			
			dst[RIDX(adj_dim-j-3, i+0, dim)] = src[RIDX(i+0, j+3, dim)]; //b=3, a=0
			dst[RIDX(adj_dim-j-3, i+1, dim)] = src[RIDX(i+1, j+3, dim)]; //b=3, a=1
			dst[RIDX(adj_dim-j-3, i+2, dim)] = src[RIDX(i+2, j+3, dim)]; //b=3, a=2
			dst[RIDX(adj_dim-j-3, i+3, dim)] = src[RIDX(i+3, j+3, dim)]; //b=3, a=3
		}
}

void rotate_1_9(int dim, pixel *src, pixel *dst) 
{
	int a,b,i,j;
	int adj_dim = dim-1;
	
	for (j=0; j<dim; j+=4)
		for (i=0; i<dim; i+=4)
			for (b=0; b<4; b++)
				for (a=0; a<4; a++)
					dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
}

void rotate_2_7(int dim, pixel *src, pixel *dst) 
{
	int a,b,i,j;
	int adj_dim = dim-1;
	
	for (j=0; j<dim; j+=16)
		for (i=0; i<dim; i+=16)
			for (b=0; b<16; b++)
				for (a=0; a<16; a++)
					dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
}

void rotate_2_9_refined(int dim, pixel *src, pixel *dst) 
{
	int a,b,i,j;
	int adj_dim = dim-1;
	
	for (j=0; j<dim; j+=32)
		for (i=0; i<dim; i+=8)
			for (b=0; b<32; b++)
				for (a=0; a<8; a++)
					dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
}

/* 
 * second attempt (commented out for now)
char rotate_two_descr[] = "second attempt";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    naive_rotate(dim, src, dst);
}
*/

//This shows how much of a difference iterating through i first can be
//Iterating through j is far more efficient because of what's happening in the superscalar pipeline - look at notes
void rotate_inefficient(int dim, pixel *src, pixel *dst) 
{
	int a,b,i,j;
	int adj_dim = dim-1;
	
	for (i=0; i<dim; i+=32)
		for (j=0; j<dim; j+=8)
			for (a=0; a<32; a++)
				for (b=0; b<8; b++)
					dst[RIDX(adj_dim-j-b, i+a, dim)] = src[RIDX(i+a, j+b, dim)];
}

void rotate3(int dim, pixel *src, pixel *dst) 
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

void rotate(int dim, pixel *src, pixel *dst) 
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
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   

    //add_rotate_function(&attempt_two, rotate_two_descr);   
    //add_rotate_function(&attempt_three, rotate_three_descr);   
    //add_rotate_function(&attempt_four, rotate_four_descr);   
    //add_rotate_function(&attempt_five, rotate_five_descr);   
    //add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

