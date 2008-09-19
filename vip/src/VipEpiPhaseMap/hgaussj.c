/* gaussj.c
   Solution of linear equations using Gauus-Jordan elimination.
   Copied from numerical recipes in c, modified for standard
   c indexing (0..N-1)
   
   F.H. 8/3/00
   --------------------------------------------------------- */
   
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "hgaussj.h"

#define SWAp(a,b) {double ttmp; ttmp=a; a=b; b=ttmp;}
   
   
int gaussj(double **a, int n, double **b, int m)
/*--------------------------------------------------------------------
 solving simultaneously m systems of n equtions: 
         B[i] = A[i][k]*X[k][n] (sum over k=0..n-1)
 for X[k].
     
 i=0..n - number of equation
 n=0..m - number of system
     
 a[i][k] - coeff. of unknown k in equation i (matrix A)
 b[i][n] - at input: constant in equation i of system n (all B[i]'s) 
           at output: solution for unknown i in system n (all X[i]'s)
	       
 Returns 0  - ok;
         -1 - alloc error;
	 -2 - singular matrix a.
 --------------------------------------------------------------------*/
{
  int *indxc, *indxr, *ipiv,
    i, icol, irow, j,k,l,ll;
  double big, dum, pivinv;
    
  icol=irow=-1;

  indxc = calloc(n, sizeof(int));
  indxr = calloc(n, sizeof(int));
  ipiv  = calloc(n, sizeof(int));
  if(indxc==NULL || indxr==NULL || ipiv==NULL)
    return -1;
    
  for(i=0; i<n; i++)
    ipiv[i]=0;
      
  for(i=0; i<n; i++)
  {
    big = 0.;
    for(j=0; j<n; j++)
      if(ipiv[j] != 1)
        for(k=0; k<n; k++)  
	{
	    if(ipiv[k] == 0)
	    {
	      if(fabs(a[j][k]) >= big)
	      {
	        big = fabs(a[j][k]);
		irow = j;
		icol = k;
	      }
	    }
	    else if(ipiv[k]>1)
	    {
	      free(indxc); free(indxr); free(ipiv);
	      return -2;
	    }
	} /* for k.. */ 
    ++(ipiv[icol]);	    
    
    if(irow != icol)
    {
      for(l=0; l<n; l++) 
        SWAp(a[irow][l],a[icol][l]);
      for(l=0; l<m; l++)
        SWAp(b[irow][l],b[icol][l]);
    }	         
    indxr[i] = irow;
    indxc[i] = icol;
    if(a[icol][icol]== 0.0)
    {
      free(indxc); free(indxr); free(ipiv);
      return -2;
    }
    pivinv = 1/a[icol][icol];
    a[icol][icol] = 1.0;
    for(l=0; l<n; l++)
      a[icol][l] *= pivinv;
    for(l=0; l<m; l++)
      b[icol][l] *= pivinv;
      
    for(ll=0; ll<n; ll++)
      if(ll != icol)
      {
        dum=a[ll][icol];
	a[ll][icol]=0.;
	for(l=0;l<n;l++)
	  a[ll][l] -= a[icol][l]*dum;
	for(l=0;l<m;l++)
	  b[ll][l] -= b[icol][l]*dum;
      }
  } /* for i.. */
  
  for(l=n-1; l>=0; l--)
  {
    if(indxr[l] != indxc[l])
      for(k=0; k<n; k++)
        SWAp(a[k][indxr[l]], a[k][indxc[l]]);
  }
  free(indxc); 
  free(indxr);
  free(ipiv);
  return 0;
} /* end of gaussj */    

int gaussj1(double **a, int n, double *b)
/*--------------------------------------------------------------------
 solving one system of n equtions: 
         B[i] = A[i][k]*X[k] (sum over k=0..n-1)
 for X[k].
     
 i=0..n-1
     
 a[i][k] - coeff. of unknown k in equation i (matrix A)
 b[i] - at input: constant in equation i of system n (all B[i]'s) 
        at output: solution for unknown i in system n (all X[i]'s)
	       
 Returns 0  - ok;
         -1 - alloc error;
	 -2 - singular matrix a.
 --------------------------------------------------------------------*/
{
  int *indxc, *indxr, *ipiv,   
    i, icol, irow, j,k,l,ll;
  double big, dum, pivinv;
    
  icol = irow = -1;

  indxc = calloc(n, sizeof(int));
  indxr = calloc(n, sizeof(int));
  ipiv  = calloc(n, sizeof(int));
  if(indxc==NULL || indxr==NULL || ipiv==NULL)
    return -1;
    
  for(i=0; i<n; i++)
    ipiv[i]=0;
      
  for(i=0; i<n; i++)
  {
    big = 0.;
    for(j=0; j<n; j++)
      if(ipiv[j] != 1)
        for(k=0; k<n; k++)
	{
	    if(ipiv[k] == 0)
	    {
	      if(fabs(a[j][k]) >= big)
	      {
	        big = fabs(a[j][k]);
		irow = j;
		icol = k;
	      }
	    }
	    else if(ipiv[k]>1)
	    {
	      free(indxc); free(indxr); free(ipiv);
	      return -2;
	    }
	} /* for k.. */ 
    ++(ipiv[icol]);	    
    
    if(irow != icol)
    {
      for(l=0; l<n; l++) 
        SWAp(a[irow][l],a[icol][l]);
      SWAp(b[irow],b[icol]);
    }	         
    indxr[i] = irow;
    indxc[i] = icol;
    if(a[icol][icol]== 0.0)
    {
      free(indxc); free(indxr); free(ipiv);
      return -2;
    }
    pivinv = 1/a[icol][icol];
    a[icol][icol] = 1.0;
    for(l=0; l<n; l++)
      a[icol][l] *= pivinv;
    b[icol] *= pivinv;
      
    for(ll=0; ll<n; ll++)
      if(ll != icol)
      {
        dum=a[ll][icol];
	a[ll][icol]=0.;
	for(l=0;l<n;l++)
	  a[ll][l] -= a[icol][l]*dum;
	
	b[ll] -= b[icol]*dum;
      }
  } /* for i.. */
  
  for(l=n-1; l>=0; l--)
  {
    if(indxr[l] != indxc[l])
      for(k=0; k<n; k++)
        SWAp(a[k][indxr[l]], a[k][indxc[l]]);
  }
  free(indxc); 
  free(indxr);
  free(ipiv);
  return 0;
} /* end of gaussj */    


/*
int main()
{
#define N 4
   double *a[N], *b[N], *c[N], y[N];
   int i, k, l;
   
  
   for(i=0; i<N; i++) 
   {
     a[i]=calloc(N, sizeof(double));
     b[i]=calloc(N, sizeof(double));
     c[i]=calloc(N, sizeof(double));
     y[i]=i;
   }
   
   for(i=0; i<N; i++)
   for(k=0; k<N; k++)
   {
     a[i][k] = b[i][k] = rand();
   }
   
   i=gaussj1(a, N, y);
   (void) printf("return value %d\n", i);
   
   for(i=0; i<N; i++)
   {
     for(k=0; k<N; k++)
     {
       c[i][k] = 0;
       for(l=0; l<N; l++)
         c[i][k] += a[i][l]*b[l][k];
       
       
     }
   }
  
 
   
   for(i=0; i<N; i++)
   {
     for(k=0; k<N; k++)
       printf("%12g ", a[i][k]);
     printf("\n");
   }   
   printf("\n");
   for(i=0; i<N; i++)
   {
     for(k=0; k<N; k++)
       printf("%12g ", b[i][k]);
     printf("\n");
   }   
   printf("\n");
   for(i=0; i<N; i++)
   {
     for(k=0; k<N; k++)
       printf("%12g ", c[i][k]);
     printf("\n");
   }   
  
   
   for(k=0; k<N; k++)
       printf("%12g \n ", c[N-1][k]);
       
   return 0;
}  
  
*/
