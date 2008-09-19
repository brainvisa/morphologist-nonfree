#include <stdio.h>
#include <stdlib.h>

#include "reconstruct.h"
#include <vip/epidistorsion/fourier.h>

void dc_off(double ***data, param_mat *par) {
/*--------------------------------------------------------------------
  dc offset correction. Takes average of right 20% of data matrix
  --------------------------------------------------------------------*/
  int i,j,k,n=0;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  
  double offset_re = 0.0,
         offset_im = 0.0;
  
  for(k=0; k<size_z; k++)
   for(j=0; j<size_y; j++)
    for(i=0.8*size_x; i<size_x; i++)
    {
      n++;
      offset_re += data[k][j][2*i];
      offset_im += data[k][j][2*i+1];
    }
    
  offset_re /= n;
  offset_im /= n;
  
  for(k=0; k<size_z ; k++)
   for(j=0; j<size_y; j++)
    for(i=0; i<size_x; i++)
    {   
      data[k][j][2*i] -= offset_re;
      data[k][j][2*i+1] -= offset_im;
    }
}

void reconstruct (double ***data, param_mat *par_data) {
/*-----------------------------------------------------
  dc correction
  2D Fourier transform of each slice of data
  -----------------------------------------------------*/
  dc_off (data,par_data);
  ft_1st_dim (data, par_data->s_z,par_data->s_y,par_data->s_x);
  ft_2nd_dim (data, par_data->s_z,par_data->s_y,par_data->s_x);
}
