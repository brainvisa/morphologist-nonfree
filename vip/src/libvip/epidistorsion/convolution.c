#include <stdio.h>
#include <stdlib.h>

#include <vip/epidistorsion/convolution.h>
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/dyn_alloc.h>

void conv3D (double ***result, double ***image, int r_sz, int r_sy, int r_sx, 
	     double ***kernel, int k_sz, int k_sy, int k_sx)
/*----------------------------------------------------------
  convolution of the image with the given kernel
  the result is written in the result matrix
  (which has to be of the same size as the image)
  the dimensions of the kernel should be ODD
  WARNING RESULT MUST BE DIFFERENT FROM IMAGE
  ---------------------------------------------------------- */
{
  int
    r_i,r_j,r_k, /* result */
    k_i,k_j,k_k, /* kernel */
    i_i,i_j,i_k, /* image  */
    c_z=(k_sz-1)/2,
    c_y=(k_sy-1)/2,
    c_x=(k_sx-1)/2;

  double image_pt;
  
  for (r_k=0;r_k<r_sz;r_k++)
    for (r_j=0;r_j<r_sy;r_j++)
      for (r_i=0;r_i<r_sx;r_i++)
	result[r_k][r_j][r_i]=0;
  
  for (i_k=0;i_k<r_sz;i_k++)
    for (i_j=0;i_j<r_sy;i_j++)
      for (i_i=0;i_i<r_sx;i_i++) {
	
	image_pt = image[i_k][i_j][i_i];
	
	for (k_k=0;k_k<k_sz;k_k++)
	  for (k_j=0;k_j<k_sy;k_j++)
	    for (k_i=0;k_i<k_sx;k_i++) {

	      r_k = i_k + k_k - c_z;
	      r_j = i_j + k_j - c_y;
	      r_i = i_i + k_i - c_x;
	      if ((r_k >= 0)&&(r_j >= 0)&&(r_i >= 0)
		  &&(r_k < r_sz)&&(r_j < r_sy)&&(r_i < r_sx))
		
		result[r_k][r_j][r_i] += image_pt*kernel[k_k][k_j][k_i];
	    }
      }
}

int Convolution (double ***mask, int s_z, int s_y, int s_x,
	    double ***kernel, int k_sz, int k_sy, int k_sx) {
  /* ----------------------------------------------------------------------
     Convolution of the mask with the kernel
     ---------------------------------------------------------------------- */
  double ***copy;

  if ((Alloc3Dim((void ****)&copy,sizeof(double),s_z,s_y,s_x)) == ERROR) {
    printf ("Dilatation/Erosion : Alloc failed");
    return ERROR;
  } else {
    copy3D (copy,mask,s_z,s_y,s_x,0,0,0);
    conv3D (mask,copy,s_z,s_y,s_x,kernel,k_sz,k_sy,k_sx);

    Free3Dim((void ***)copy,s_z,s_y);
    return OK;
  }
}
