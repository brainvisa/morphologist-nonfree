#include <stdio.h>
#include <stdlib.h>

#include "extrapolation.h"
#include "models.h"
#include "lfit.h"
#include <vip/epidistorsion/readwrite.h>
#include <vip/epidistorsion/dyn_alloc.h>
#include <vip/epidistorsion/cpp_defs.h>
#include <vip/epidistorsion/morpho.h>
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/volume_utils.h>
#include <vip/epidistorsion/convolution.h>


int MakeApplyMask (double ***mask, int kernel_size, param_mat *par_echo) {
/* ----------------------------------------------
   widens the mask by convolving it with a kernel
   and setting all values !=0 to 1,
   then convolves the mask with the same kernel
   once again to make it decrease smoothly to zero
   ---------------------------------------------- */
#if EXTRAPOL_DEBUG
  int i;
#endif
  int k_sz,k_sy,k_sx;
  double ***kernel;
  double radius = kernel_size;
  double kernel_surface;
  int flag = 0;
  int
    s_x = par_echo->s_x,
    s_y = par_echo->s_y,
    s_z = par_echo->s_z;

  k_sz = 1;
  k_sy = k_sx = 2*kernel_size+1;
  
  if (Alloc3Dim ((void ****)&kernel,sizeof(double),k_sz,k_sy,k_sx) == ERROR) {
    printf ("MakeApplyMask error : couldn't allocate memory for the kernel\n");
    return ERROR;
  } else {
    
    BuildKernel (kernel, k_sz,k_sy,k_sx, XY, 0.0, radius, radius);
    kernel_surface = sum (kernel,k_sz,k_sy,k_sx);
 
    /* convolutions */
    if ((Dilate (mask, s_z, s_y, s_x, kernel, k_sz, k_sy, k_sx) == OK)
	&&(Convolution(mask,s_z,s_y,s_x,kernel,k_sz,k_sy,k_sx) == OK)) {
      flag = OK;
    } else {
      flag = ERROR;
    }
    
    cstMultiply (mask,mask,1/kernel_surface,par_echo);

#if EXTRAPOL_DEBUG
    printf ("%s\n", "ligne mediane");
    for (i=0;i<s_x;i++) printf ("%d %f\n",i,mask[0][s_y/2][i]);
#endif
    Free3Dim((void ***)kernel,k_sz,k_sy);
    return flag;
  } 
}

int Extrapolation (double ***map, double ***mask, param *par, param_mat *par_echo) {
  /* -------------------------------------------------
     2D extrapolation of the map using a least-squares
     fit of a cosinusoidal model on the data
     inside the mask
     ------------------------------------------------- */
  int k,j,i,l;
  double *a, *b,  **covar, chisq;

  int esize = par->extrapol_base_size;
  
  if (((Alloc2Dim((void ***)&covar,sizeof(double),esize,esize))==ERROR)  
      ||((Alloc1Dim((void **)&a,sizeof(double),esize))==ERROR)
      ||((Alloc1Dim((void **)&b,sizeof(double),esize))==ERROR)  ) { 
    Write ("Extrapolation : alloc failed\n");
    return ERROR;
  }
  
  for (k=0;k<par_echo->s_z;k++) {
    
    lfit (map[k],mask[k], par_echo->s_y,par_echo->s_x, a, par->extrapol_base_size,
	  covar, &chisq, &EBASE);
       
    printf ("Extrapolation : slice %d done\n",k);
    
    for (i=0;i<par_echo->s_x;i++) 
	 for (j=0;j<par_echo->s_y;j++) {
	   
	   
	   if ((par->actual_map != 1)||(mask[k][j][i]< 0.01)) { /* WRITING THE MODEL */
	     EBASE (j,i,b,esize) ; 
	     map[k][j][i]=0;
	     for (l=0;l<esize;l++) {
	       map[k][j][i] += b[l]*a[l];  /* constructing the model */
	     }
	   }
	 }
  }


  Free2Dim((void **)covar, esize);
  Free1Dim((void *)b);
  Free1Dim((void *)a);

  /* making the model decrease slowly to zero */
  
  MakeApplyMask (mask, par->extrapol_kernel_size, par_echo);
  
  ApplyMask (map,mask,par_echo); 
  
  return OK;
}
