#include <stdio.h>
#include <stdlib.h>

#include <vip/epidistorsion/utils.h>
#include "unwrap.h"
#include "lfit.h"
#include <vip/epidistorsion/cpp_defs.h>
#include <vip/epidistorsion/dyn_alloc.h>
#include "models.h"
#include <vip/epidistorsion/volume_utils.h>

void DiffPhase (double **res, double **map, param_mat *par) {
  /* ------------------------------------------------------
     backward difference of the phases
     res[k][j][i]  = phase[k][j][i]-phase[k][j][i-1]
     res[k][j][2*size_x+i]= phase[k][j][i]-phase[k][j-1][i]
     indexes are taken circularly
     ------------------------------------------------------ */
  int i,j;
  double temp[2];
  
  int size_x = par->s_x;
  int size_y = par->s_y;
  
  /* along the x-axis */
  for (j=0;j<size_y;j++) {
      
    cdiv(temp,map[j],map[j]+2*(size_x-1));
    cphase(res[j],temp);
    
    for (i=1;i<size_x;i++) {
      
      cdiv(temp,map[j]+2*i,map[j]+2*(i-1));
      cphase(res[j]+i,temp);
    }
  }
  
  /* along the y-axis */
  for (i=0;i<size_x;i++) {
    
    cdiv(temp,map[0]+2*i,map[size_y-1]+2*i);
    cphase(res[0]+size_x+i,temp);
      
    for (j=1;j<size_y;j++) {
	
      cdiv(temp,map[j]+2*i,map[j-1]+2*i);
      cphase(res[j]+size_x+i,temp);
      
    }
  }
}

void DiffMask(double **res, double **imask, param_mat *par) {
  /* ------------------------------------------------------
     mask for the diffmap
     (x,y), x<size_x has non zero weight if imask[j][i] =1 
     and imask [j][i-1] = 1 (circularly)
     (corresponding formula for x>=size_x, along the y-axis)
     ------------------------------------------------------ */
  int i,j,i2,j2;
  int s_x = par->s_x;
  int s_y = par->s_y;
  
  /* building the binary diffmask */
  for(j=0; j<s_y;j++)
    for(i=0; i<s_x;i++) {
      
      i2=(i>0)? (i-1) : (s_x-1);
      j2=(j>0)? (j-1) : (s_y-1);
      
      if ((imask[j][i]>0)&&(imask[j][i2]>0)) {
	res[j][i] = 1.0;
	} else { 
	  res[j][i] = 0.0;
	}
      
      if ((imask[j][i]>0)&&(imask[j2][i]>0)) {
	res[j][s_x+i] = 1.0;
      } else { 
	res[j][s_x+i] = 0.0;
      }
    }
} 


int CheckNeighbours (double **mask, double **phasemap, int refy, int refx) {
  /* ---------------------------------------------------------------------
     returns YES if the distance between the value of each
     neighbour and the value of the reference point
     is less than pi/10
     --------------------------------------------------------------------- */
  int i,j;
  int flag = YES;
  double reference, upperbound, lowerbound, current_value;
  double threshold = 0.314159;

  reference = phasemap[refy][refx];
  upperbound = reference + threshold;
  lowerbound = reference - threshold;

  for (i=refx-1;i<=refx+1;i++)
    for (j=refy-1;j<=refy+1;j++) {
      current_value = phasemap[j][i];
      if ((mask[j][i]<0.5)||
	  (current_value<lowerbound)||(current_value>upperbound))
	flag = NO;
    }
  return flag;
}

int FindReferencePoint (int *ref_y, int *ref_x, double **mask, double **phasemap, int start_y, int start_x, int s_y, int s_x)
{
  (void)(phasemap);
  (void)(s_x);
  /* ---------------------------------------------------------------
     Finds a reference point lying within the mask
     A candidate point is kept if his
     eight neighbours have a value lying in the range [-pi/10,pi/10[
     WARNING : THE MAXIMUM RANGE TOLERATED SHOULD DEPEND
     ON THE DURATION BETWEEN THE TWO ECHOES AND THE VALUE
     OF THE STATIC MAGNETIC FIELD
     (The proposed values only aim to prevent the algorithm from
     choosing incorrect points)
     ---------------------------------------------------------------- */
  int refx, refy /*, found*/;
  
  refx = start_x;
  refy = start_y;
  
  /*found = NO;*/

  /* Old research */

  while ((mask[refy][refx]<0.2)&&(refy<s_y-1)) 
    refy++;
  if (refy == (s_y -1))
    while ((mask[refy][refx]<0.2)&&(refy>0)) 
      refy--;
  if (mask[refy][refx]<0.5) {
    fprintf(stderr, "pas de point de reference\n");
    exit(EXIT_FAILURE);
  }
  
  *ref_y=refy;
  *ref_x=refx;

  return YES;
  
  /* Checking neighbours consistency (TO BE DEBUGGED...)*/
/*  
  for (refx=start_x;((refx<=s_x-3)&&(found == NO));refx++)
    for (refy=start_y;((refy<=s_y-3)&&(found == NO));refy++) {
      
      found = CheckNeighbours(mask, phasemap,refy,refx);
      if (found == YES) {
	*ref_y = refy;
	*ref_x = refx;
      }
    }
  return found;
*/

} 

int Unwrap (double ***result, double ***cimage, double ***mask,param *par,  param_mat *par_echo) {
  /* -------------------------------------------------------------------------------
     2D unwrapping of the phase of each slice of the complex-valued image cimage. 
     A mask of the area of interest is provided.
     A model-based unwrapping algorithm is used : the model is fitted to the x and y
     derivatives of the map in the area of interest, in the least-squares sense
     ------------------------------------------------------------------------------- */
  double
    wrapped_phase,
    temp,
    delta,
    **diffmap,
    **diffmask,
    **covar,
    *a, *b,
    chisq;
  int
    i,j,k,l, refy, refx,
    s_y = par_echo->s_y,
    s_x = par_echo->s_x,
    s_z = par_echo->s_z;
  
  if (((Alloc2Dim((void ***)&diffmap,sizeof(double),s_y,2*s_x))==ERROR) 
      ||((Alloc2Dim((void ***)&diffmask,sizeof(double),s_y,2*s_x))==ERROR)  
      ||((Alloc2Dim((void ***)&covar,sizeof(double),par->unwrap_base_size,par->unwrap_base_size))==ERROR)  
      ||((Alloc1Dim((void **)&a,sizeof(double),par->unwrap_base_size))==ERROR)
      ||((Alloc1Dim((void **)&b,sizeof(double),par->unwrap_base_size))==ERROR)  ) {
    printf ("Unwrap : allocation error. Exiting...\n");
    return ERROR;
  }

  GetPhase (result,cimage,par_echo);

  for (k=0;k<s_z;k++) {    
    
    DiffPhase (diffmap,cimage[k],par_echo);
    
    DiffMask (diffmask, mask[k], par_echo);
    
    /* getting the wrapped phase map */
        
    lfit (diffmap,diffmask, s_y,2*s_x, a, par->unwrap_base_size-1,
	  covar, &chisq, &DIFFWBASE);
    
    if (FindReferencePoint(&refy,&refx,mask[k],result[k],par->ref_y,par->ref_x,par_echo->s_y,par_echo->s_x) == NO) {
      printf ("Unwrap : could not find any suitable unwrapping starting point.\n");
      printf ("Proceeding with default values ; the unwrapping of slice %d might be incorrect\n",k);
      
      refy = par->ref_y;
      refx = par->ref_x;
    }
    
    /* calculating the constant shift between the model and the real map */
    WBASE(refy,refx,b,par->unwrap_base_size);

    delta = result[k][refy][refx];   
    for (l=0;l<par->unwrap_base_size-1;l++)
      delta -= b[l+1]*a[l];
    
    /* unwrapping the map */

    for (i=0;i<s_x;i++) 
      for (j=0;j<s_y;j++) {
	
	if (mask[k][j][i]>0.01) {
	  
	  WBASE (j,i,b,par->unwrap_base_size) ;
	  wrapped_phase = result[k][j][i];
	  result[k][j][i] = delta;
	  
	  for (l=0;l<par->unwrap_base_size-1;l++)
	    result[k][j][i] += b[l+1]*a[l]; /* constructing the model */
	  
	  temp = (pvalue (wrapped_phase-result[k][j][i])); 
 	  result[k][j][i]+=(temp);
	  
	} else {
	  result[k][j][i]=0;
	}
	
      }
    
    printf ("Unwrap : slice %d unwrapped\n",k);
  }
  Free2Dim((void **)diffmap,s_y);
  Free2Dim((void **)diffmask,s_y);
  Free2Dim((void **)covar,par->unwrap_base_size);
  Free1Dim((void *)a);
  Free1Dim((void *)b);


  return OK;
}
