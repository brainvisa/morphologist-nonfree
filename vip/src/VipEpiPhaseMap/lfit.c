/* -----------------------------------------------------------------
   General Linear Weighted Least-Squares
   Adapted from Numerical Recipes

   ndatx, ndaty : size of the data matrix 
   (0...ndatx-1,0...ndaty-1) points
   y[][] : data at the given points
   sig[][] : standard deviations 
   weight[][] : weight of the points (0 or 1)

   ma : number of parameters
   a[0...ma-1] : parameters

   (*funcs) (i,j,afunc,ma) sets afunc[k] with the value of the 
             k-th base function at point (i,j)
   
   covar[1...ma-1][1...ma-1] : covariance matrix (result)
   *chisq : value of the chi-square estimator

   return 0-succes 1-error
   ----------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>

#include "lfit.h"
#include <vip/epidistorsion/dyn_alloc.h>
#include "error.h"
#include "hgaussj.h"

#define LFIT_DEBUG 0

static float sqrarg;
#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)

int lfit(double **y, double **weight, int ndaty, int ndatx, double a[],
	 int ma, double **covar, double *chisq, void (*funcs)(int, int, double [], int))
{

  int ix,iy,j,k;
  double ym,wt,sum,sig2i,*beta,*afunc;
  
  if (Alloc1Dim((void **) &beta,sizeof(double),ma) != 0) {
    error ("lfit : alloc failed");
  }
  
  if (Alloc1Dim((void **) &afunc,sizeof(double),ma) !=0) {
    error ("lfit : alloc failed");  
  }
  
  for (j=0;j<ma;j++) {
    /* initialisation of the matrix */
    for (k=0;k<ma;k++) covar[j][k]=0.0;
    beta[j]=0.0;
  }
  
  for (ix=0;ix<ndatx;ix++) {
    for (iy=0;iy<ndaty;iy++) {
      
      /* filling the matrix under the diagonal */
    
      (*funcs) (iy,ix,afunc,ma);
      ym=y[iy][ix];
      sig2i=weight[iy][ix];
      
      for (j=0;j<ma;j++) {
	wt=afunc[j]*sig2i;
	
	for (k=0;k<=j;k++)
	  covar[j][k] += wt*afunc[k];
      
	beta[j] += ym*wt;		  
      }
      
    }
  }

  for (j=1;j<ma;j++)
    for (k=0;k<j;k++)
      /* filling the matrix above the diagonal by symmetry */
      covar[k][j]=covar[j][k];

#if LFIT_DEBUG

  printf ("%s\n", "input of gauss-jordan");
  
  for (j=0;j<ma;j++) {
    for (k=0;k<ma;k++) {
      printf ("%f ",covar[k][j]);
    }
    printf ("%s","\n");
  }

  for (k=0;k<ma;k++) {
      printf ("%f ",beta[k]);
      
  }
  printf ("%s","now calling gaussj1\n");

#endif
  
  
  gaussj1(covar,ma,beta);
  
#if LFIT_DEBUG
  printf ("%s","excited gaussj1\n");


  for (j=0;j<ma;j++) {
    for (k=0;k<ma;k++) {
      printf ("%f ",covar[k][j]);
    }
    printf ("%s","\n");
  }
#endif
  
  /* writing the results */
  for (j=0;j<ma;j++) {a[j]=beta[j];}
  
  *chisq=0.0;
  
  for (ix=0;ix<ndatx;ix++) {
      for (iy=0;iy<ndaty;iy++) {

	(*funcs)(iy,ix,afunc,ma);
	for (sum=0.0,j=0;j<ma;j++) sum += a[j]*afunc[j];
	*chisq += (weight[iy][ix])*SQR(y[iy][ix]-sum);
      }
  }
  Free1Dim(afunc);
  Free1Dim(beta);
  
  return 0;
}

