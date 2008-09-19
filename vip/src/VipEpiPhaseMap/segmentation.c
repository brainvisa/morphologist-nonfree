#include "segmentation.h"
#include <vip/epidistorsion/dyn_alloc.h>
#include <vip/epidistorsion/readwrite.h>
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/morpho.h>
#include <vip/epidistorsion/volume_utils.h>
#include <vip/epidistorsion/cpp_defs.h>
#include <vip/epidistorsion/volume_io.h>

#if SEGMENTATION_MAPS
#include <vip/epidistorsion/debug.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double corrmax3D (double ***map, int throw, int keep, int s_z, int s_y, int s_x) {
/* -----------------------------------------------
   mean of the KEEP max values, except the
   THROW max ones (KEEP > THROW)
   ----------------------------------------------- */

  double th[5];  /* thresholds : max , 95%, 90%, 85%, 80% (first attempt) */
  int vect2[4];
  double **vect3;

  int *index;
  double **storage;
  int i,j,k,l,ind;
  int flag, fst_not_full, good_index, main_loop;
  double pct = 0.05;  /* percentage step */
  double fst_pct; /* constant of the affine relationship between the pcts used (cf formula) */
  double temp;
  double result =0;

#if SEGMENTATION_DEBUG
  Write ("corrmax3D\n");
#endif

  if (Alloc2Dim((void ***)&vect3,sizeof(double),4, 10*keep) == ERROR) {
    Write ("corrmax3D : alloc failed");
    exit(EXIT_FAILURE);
  }
  
  /* indexes between 1 and 4 */
  storage = vect3 -1;
  index = vect2 -1; 

  th[0] = max3D(map,s_z,s_y,s_x);
  
  fst_pct = 1;
  main_loop = 1;


  while (main_loop == 1) {
        
#if SEGMENTATION_DEBUG
  Write ("corrmax3D : starting loop\n");
#endif
  
    index[1]=index[2]=index[3]=index[4]=0;
    
    /* setting the thresholds */
    for (l=1;l<5;l++)
      th[l] = (fst_pct - pct*l)*th[0];

#if SEGMENTATION_DEBUG
    printf ("max : %f ; fst_pct : %f ; pct : %f\n",th[0],fst_pct,pct);
#endif
    
    if (pct < 0.001) {return th[0];}

    fst_not_full = 4;
    
    for (k=0;k<s_z;k++) 
      for (j=0;j<s_y;j++) 
	for (i=0;i<s_x;i++) {
	  flag =1;
	  temp = map[k][j][i];
	  
	  for (l=fst_not_full;((flag==1)&&(l>0));l --) {
	    if (index[l] < 10*keep) {
	      if (temp > th[l]) {
		ind = index[l];
		storage[l][ind] = temp;
		index[l]++;
	      } else {
		flag =0;
	      }
	    } else {
	      fst_not_full--;
	    }
	  }
	}
    
    /* looking for a storage vect with more than KEEP values and not FULL */

    if (fst_not_full == 0) {  /* all the storage vects are full */
      pct /=4;
    } else {
      good_index = 1;
      while ((index[good_index]<keep)&&(good_index<5)) good_index++;
      
      if (good_index == 5) {  /* all the storage vects have less than KEEP values */
	fst_pct -= (4*pct);
      } else {
	if (index[good_index] == (10*keep)) { /* the pct step is too large */
	  fst_pct -= (good_index-1)*pct;
	  pct /=4;
	} else { /* OK */
	  hsort (storage[good_index],10*keep);  /* sort from min to max ! */
	  result = mean(storage[good_index],9*keep,10*keep-throw);
	  main_loop = 0;
	}
      } 
    }
  }

#if SEGMENTATION_DEBUG
  printf ("corrected maximum (pct of max) : %f\n", result/th[0]);
  Write ("end corrmax3D\n");
  NewLine();
#endif

  return result;
}

void GetMask (double ***res, double ***m, double noise_threshold, int contrast, param_mat *par) {
  /* ------------------------------------------------------------
     contrast = 1 or 0;
     res[j][i] = contrast if m[j][i] is more than the
                             calculated threshold
	         1-contrast otherwise
     ------------------------------------------------------------ */
  double max, min, ave, var, sigma, threshold;
  int i,j,k;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

#if SEGMENTATION_DEBUG
  printf ("GetMask : start\n");
#endif


  max = corrmax3D (m,THROW,KEEP,size_z,size_y,size_x);
  avevar (&ave, &var, m, size_z, size_y, size_x);
  sigma = (var > 0) ? sqrt(var) : 0;
  min = ave + 4.0*sigma;

  threshold = min +noise_threshold*(max - min);

#if SEGMENTATION_DEBUG
  printf ("GetMask : corrmax %f ; ave %f ; sigma %f ; threshold %f\n",max,ave,sigma,threshold);
#endif
    
  for (k=0; k<size_z; k++)
    for(j=0; j<size_y; j++)
      for(i=0; i<size_x; i++) {
	
	if (m[k][j][i]>threshold) {
	  res[k][j][i] = contrast;
	} else { 
	  res[k][j][i] = (1 - contrast);
	}
      }
}

int Segmentation (double ***mask, double ***image, param *par, param_mat *par_echo) {
  /* ----------------------------------------------------------------
     returns the 3D mask of the brain, given the complex
     data of the 2nd echo image (anatomy)
     ---------------------------------------------------------------- */
  int
    s_z = par_echo->s_z,
    s_y = par_echo->s_y,
    s_x = par_echo->s_x;

  double ***temp_mask;

#if SEGMENTATION_MAPS
  param_mat par_debug;
  double ***debugmap;
  unsigned char ***eightbitmap;

  par_debug.s_z=s_z;
  par_debug.s_y=s_y;
  par_debug.s_x=s_x;
#endif

#if SEGMENTATION_DEBUG
  printf ("Segmentation : start\n");
#endif

  if( 
     (Alloc3Dim((void ****)&temp_mask,sizeof(double),s_z,s_y,s_x)==ERROR) 
#if SEGMENTATION_MAPS
     ||(Alloc3Dim((void ****)&debugmap,sizeof(double),s_z,s_y,s_x)==ERROR) 
     ||(Alloc3Dim((void ****)&eightbitmap,sizeof(unsigned char),s_z,s_y,s_x)==ERROR) 
#endif
     )
    {
      Write("Segmentation : alloc failed\n");
      return ERROR;
    }
   
  GetMask (mask,image,par->noise_threshold,1,par_echo);
 
  copy3D (temp_mask,mask,s_z,s_y,s_x,0,0,0);

#if SEGMENTATION_MAPS
  copy3D (debugmap,temp_mask,s_z,s_y,s_x,0,0,0);
  ConvertMod (eightbitmap,debugmap,&par_debug);
  if ((WriteDebug (eightbitmap,"threshold",&par_debug))==ERROR) {
    Write("Segmentation : WriteDebug error\n");
    return ERROR;
  };    
#endif
#if SEGMENTATION_DEBUG
  Write("Erode\n");
#endif
  Erosion (temp_mask,s_z,s_y,s_x,0.9,par->erode_radius,par->erode_radius);

#if SEGMENTATION_MAPS
  copy3D (debugmap,temp_mask,s_z,s_y,s_x,0,0,0);
  ConvertMod (eightbitmap,debugmap,&par_debug);
  if ((WriteDebug (eightbitmap,"erosion",&par_debug))==ERROR) {
    Write("Segmentation : WriteDebug error\n");
    return ERROR;
  };    
#endif
#if SEGMENTATION_DEBUG
  Write("Connect3D\n");
#endif
  Connect3D (temp_mask, 1000, s_z, s_y,s_x);
  
#if SEGMENTATION_MAPS
  copy3D (debugmap,temp_mask,s_z,s_y,s_x,0,0,0);
  ConvertMod (eightbitmap,debugmap,&par_debug);
  if ((WriteDebug (eightbitmap,"connectcomp",&par_debug))==ERROR) {
    Write("Segmentation : WriteDebug error\n");
    return ERROR;
  };    
#endif
#if SEGMENTATION_DEBUG
  Write("Dilate\n");
#endif
  Dilatation (temp_mask,s_z,s_y,s_x,1.0,par->dilate_radius,par->dilate_radius);
#if SEGMENTATION_MAPS
  copy3D (debugmap,temp_mask,s_z,s_y,s_x,0,0,0);
  ConvertMod (eightbitmap,debugmap,&par_debug);
  if ((WriteDebug (eightbitmap,"dilatation",&par_debug))==ERROR) {
    Write("Segmentation : WriteDebug error\n");
    return ERROR;
  };    
#endif
  ApplyMask (mask, temp_mask,par_echo);

#if SEGMENTATION_MAPS
  copy3D (debugmap,mask,s_z,s_y,s_x,0,0,0);
  ConvertMod (eightbitmap,debugmap,&par_debug);
  if ((WriteDebug (eightbitmap,"finalmask",&par_debug))==ERROR) {
    Write("Segmentation : WriteDebug error\n");
    return ERROR;
  };     
  Free3Dim ((void ***)debugmap,s_z,s_y);
  Free3Dim ((void ***)eightbitmap,s_z,s_y);
#endif

  Free3Dim((void ***)temp_mask,s_z,s_y);
#if SEGMENTATION_DEBUG 
  Write("end of Segmentation\n");
#endif
  return OK;
}

int GetSegmentedMask (double ***res, param *par, param_mat *par_echo) {
  
  FILE *fmask;
  param_mat par_mask;
  
  if ((fmask = fopen (par->fname_mask,"r")) == NULL) {
    printf ("Phasemap : could not open segmented mask file %s. Exiting...\n",par->fname_mask);
    return ERROR;
  }
  
  /* getting the given mask (this is dirty hacking...) */
  par_mask.s_z=par_echo->s_z;
  par_mask.s_y=par_echo->s_y;
  par_mask.s_x=par_echo->s_x;
  par_mask.reverse_z=par->mask_reverse_z_axis;
  par_mask.reverse_y=par_echo->reverse_y;
  par_mask.reverse_x=par_echo->reverse_x;

    
  if (ReadEpi(res, fmask, &par_mask) == ERROR) {
    Write ("GetSegmentedMask : could not read segmented mask\n");
    return ERROR;
  }
  boxcar (res,1,0.05,par_echo->s_z,par_echo->s_y,par_echo->s_x);
  return OK;
}
