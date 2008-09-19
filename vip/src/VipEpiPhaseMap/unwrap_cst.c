#include "unwrap_cst.h"
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/volume_utils.h>
#include <vip/epidistorsion/dyn_alloc.h>
#include <vip/epidistorsion/cpp_defs.h>
#include <vip/epidistorsion/gravity.h>
#include <vip/epidistorsion/readwrite.h>
#include <vip/epidistorsion/volume_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void CorrectPhaseMap (double ***shift_map, double ***epi, double ***epi_gmap, double ***gmap,param_mat *par_epi,
		      double cst, double ***phasemap, double ***mask, param_mat *par_echo, param *par) {
/* ---------------------------------------------------------------
   add 0, -1 or +1 times cst to the shift_map in order
   to minimize the distance between gravity centers of the columns
   of corrected image and reference undistorted image
   (this should correct a 2*pi error during the phase unwrapping
   procedure ; cst is a 2*pi shift)
   --------------------------------------------------------------- */

  double vect[3];
  double *dist = vect+1;
  int k,i,j,l,refx,refy;
  int shift;
  double temp, temp2, absvalue;
  double **refarray;
  double twopi = 2*M_PI;
  
  int s_y = par_epi->s_x;
  int s_z = par_epi->s_z;
  
#if UNWRAPCST_DEBUG
  Write ("CorrectPhaseMap");
  NewLine();
#endif
  
  for (k=0;k<s_z;k++) {
    dist[0]=dist[1]=dist[-1]=0;
    
    /* gravity centers of the corrected epis */
    for (l=-1;l<=1;l++) {
      
      shifted_gravity_center (epi_gmap, epi, par_epi, k, shift_map, l, cst);
      temp = 0;
      
      for (i=0;i<s_y;i++) {
	
	temp += (epi_gmap[k][i][1]);
	dist[l] +=  (epi_gmap[k][i][0])*(epi_gmap[k][i][1]); 
      }
      dist[l] /= temp;
    }
    
    /* gravity center of the anatomical map */
    temp = temp2 =0 ;
    for (i=0;i<s_y;i++) {
      temp += (gmap[k][i][0])*(gmap[k][i][1]);
      temp2 += gmap[k][i][1];
    }
    temp /= temp2;
    
    /* calculation of the distances between the g-centers */
    for (l=-1;l<=1;l++) {
      temp2 = dist[l]-temp;
      dist[l]=ABS(temp2);
    }
    
#if UNWRAPCST_DEBUG
    printf ("%f %f %f \n",dist[-1],dist[0],dist[1]);
#endif
    
    if ((dist[-1] < dist[0])&&(dist[-1]<dist[1])) {shift = -1;} 
    else {
      if ((dist[1] < dist[0])&&(dist[1]<dist[-1])) {shift = 1;} 
      else {shift = 0;}
    }
    
#if UNWRAPCST_DEBUG
    printf ("slice %d : shift %d\n",k,shift);
#endif

    for (i=0;i<par_echo->s_x;i++)
      for (j=0;j<par_echo->s_y;j++)
	if (mask[k][j][i]>0.01) {phasemap[k][j][i] += (twopi*shift);}
  }

  /* checking the correctness of the shift along the z - axis */

  if (Alloc2Dim ((void ***)&refarray, sizeof(double),s_z,2)==ERROR) {
    Write ("CorrectPhaseMap : alloc error. No consistency check along the z axis");
    NewLine();
  } else {
    refx = par->ref_x;
    refy = par->ref_y;
   
    /* looking for a common reference point for all maps */
    k=s_z-1;
    while ((k>=0)&&(refy<par_echo->s_y)) {
#if UNWRAPCST_DEBUG
    printf ("k : %d ; refy : %d\n",k,refy);
#endif
      if (mask[k][refy][refx] > 0.2) k--;
      else {k=s_z-1; refy++;}
    }
    if (k != -1) {Write ("CorrectPhaseMap : no reference point found. No consistency check along the z axis");NewLine();}
    else {
      refarray[s_z-1][0] = phasemap[s_z-1][refy][refx]; refarray[s_z-1][1]=0;
      for (k=s_z-2;k>=0;k--) {
	refarray[k][0] = phasemap[k][refy][refx];
	refarray[k][1] = 0;
	while (refarray[k+1][0] - refarray[k][0] > M_PI) {refarray[k][0] += twopi;; refarray[k][1] += 1;}
      	while (refarray[k][0] - refarray[k+1][0] > M_PI) {refarray[k][0] -= twopi; refarray[k][1] -= 1;}
      }
      temp = 0;
      for (k=0;k<s_z;k++) temp += refarray[k][1];
      temp /= s_z;
      temp = roundvalue (temp);
#if UNWRAPCST_DEBUG
      printf ("rounded mean %f\n",temp);
#endif
      for (k=0;k<s_z;k++) {
	temp2 = roundvalue(refarray[k][1] - temp);
#if UNWRAPCST_DEBUG
	printf ("slice : %d difference : %f\n",k,temp2);
#endif
	absvalue = ABS(temp2);
	if (absvalue > 0.5) {
#if UNWRAPCST_DEBUG
	  printf ("modifying slice %d\n",k);
#endif
	  for (i=0;i<par_echo->s_x;i++)
	    for (j=0;j<par_echo->s_y;j++)
	      if (mask[k][j][i]>0.01) {phasemap[k][j][i] += temp2*twopi;} 
	}
      }
    }
  }
}

int UnwrapCst (double ***map, double ***modmap, double ***mask, param *par, param_acq *par_a, 
	       param_mat *par_echo, param_mat *par_epi) {
  /* -----------------------------------------------------------------------------------------------
     Shifts each slice of the phasemap by a multiple of 2PI, within the mask, to get the correct
     value.
     Comparison of the the gravity_centers of the anatomy and of the prospective corrected image
     is used, as well as verification of the z-axis consistency of the slice unwrapping
     ----------------------------------------------------------------------------------------------- */
  double factor, factor2pi;
  double
    ***epi,
    ***echo2_gmap,
    ***epi_gmap,
    ***shift_map;
  param_mat
    par_g,
    par_g_epi;
  FILE *fepi;

  if ((fepi = fopen (par->fname_epi,"r")) == NULL) {
    printf ("UnwrapCst : cannot  open epi_image %s\n",par->fname_epi);
    return ERROR;
  }

  par_g.s_z=par_echo->s_z;
  par_g.s_y=par_echo->s_x;
  par_g.s_x=2;

  par_g_epi.s_z = par_epi->s_z;
  par_g_epi.s_y = par_epi->s_x;
  par_g_epi.s_x = 2;


  if (
      ((Alloc3Dim((void ****)&epi_gmap,sizeof(double),par_g_epi.s_z,par_g_epi.s_y,par_g_epi.s_x))==ERROR)
      ||((Alloc3Dim((void ****)&echo2_gmap,sizeof(double),par_g.s_z,par_g.s_y,par_g.s_x))==ERROR)
      ||((Alloc3Dim((void ****)&shift_map,sizeof(double),par_epi->s_z,par_epi->s_y,par_epi->s_x))==ERROR)
      ||((Alloc3Dim((void ****)&epi,sizeof(double),par_epi->s_z,par_epi->s_y,par_epi->s_x))==ERROR)
      ) {
    Write ("UnwrapCst : allocation error\n");
    return ERROR;
  }
  
#if UNWRAPCST_DEBUG
  Write ("ReadEpi\n");
#endif 
  /* Reading one volume */
  if ((ReadEpi (epi,fepi,par_epi)) == ERROR) {
    Write("error reading epi data");
    NewLine();
    return ERROR;
  }
  
  factor2pi = (par_epi->s_y)*(par_a->dwell_time)/(par_a->inter_echo_time);
  factor = factor2pi*M_1_PI/2;
  
  /* getting the gravity center map */
  gravity_center (echo2_gmap,modmap, par_echo);
  Resize (echo2_gmap, echo2_gmap, &par_g_epi, &par_g);
  
  Resize (shift_map, map, par_epi, par_echo);
  cstMultiply (shift_map, shift_map,factor, par_epi);
  
  CorrectPhaseMap (shift_map, epi, epi_gmap, echo2_gmap, par_epi, factor2pi, map, mask, par_echo, par);

  Free3Dim((void ***)epi_gmap,par_g_epi.s_z,par_g_epi.s_y);
  Free3Dim((void ***)echo2_gmap,par_g.s_z,par_g.s_y);
  Free3Dim((void ***)epi,par_epi->s_z,par_epi->s_y);
  (void) fclose (fepi);
  return OK;
}
  
  
