#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vip/epidistorsion/param_io.h>
#include <vip/epidistorsion/readwrite.h>

/* funtions reading and writing parameter files */

int ReadParam(FILE *fid,param *par,param_acq *par_a,param_mat *par_echo,param_mat *par_epi) {
  char garbage[200];

  if ((ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_echo->s_x,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_echo->s_y,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_echo->s_z,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_epi->s_x,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_epi->s_y,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_epi->s_z,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_epi->s_t,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_echo->reverse_x,fid) == 1)
   
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_echo->reverse_y,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_echo->reverse_z,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_epi->reverse_x,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_epi->reverse_y,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par_epi->reverse_z,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadDouble (&par_a->inter_echo_time,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadDouble (&par_a->dwell_time,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadDouble (&par->noise_threshold,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->ref_x,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->ref_y,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->unwrap_base_size,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->extrapol_base_size,fid) == 1)
      
      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->extrapol_kernel_size,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->erode_radius,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->dilate_radius,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->actual_map,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->segmentation,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadString (par->fname_mask,fid) == 1)

      ||(ReadString (garbage,fid) == 1)
      ||(ReadInt (&par->mask_reverse_z_axis,fid) == 1)

      ) 
    return 1;
  else return 0;
}

int WriteParam(FILE *fid,param *par,param_acq *par_a,param_mat *par_echo,param_mat *par_epi) {
  
  if ((fWrite ("gradient echo image - size x",fid) == 1)
      ||(fWriteInt (par_echo->s_x,fid) == 1)

      ||(fWrite ( "gradient echo image - size y",fid) == 1)
      ||(fWriteInt (par_echo->s_y,fid) == 1)

      ||(fWrite ( "gradient echo image - size z",fid) == 1)
      ||(fWriteInt (par_echo->s_z,fid) == 1)
      
      ||(fWrite ("epi image - size x",fid) == 1)
      ||(fWriteInt (par_epi->s_x,fid) == 1)

      ||(fWrite ("epi image - size y",fid) == 1)
      ||(fWriteInt (par_epi->s_y,fid) == 1)

      ||(fWrite ( "epi image - size z",fid) == 1)
      ||(fWriteInt (par_epi->s_z,fid) == 1)

      ||(fWrite ( "epi image - size t",fid) == 1)
      ||(fWriteInt (par_epi->s_t,fid) == 1)
      
      ||(fWrite ("gradient echo image - reverse x (-1/1)",fid) == 1)
      ||(fWriteInt (par_echo->reverse_x,fid) == 1)
      
      ||(fWrite ( "gradient echo image - reverse y (-1/1)",fid) == 1)
      ||(fWriteInt (par_echo->reverse_y,fid) == 1)

      ||(fWrite ( "gradient echo image - reverse z (-1/1)",fid) == 1)
      ||(fWriteInt (par_echo->reverse_z,fid) == 1)
      
      ||(fWrite ("epi image - reverse x (-1/1)",fid) == 1)
      ||(fWriteInt (par_epi->reverse_x,fid) == 1)
      
      ||(fWrite ("epi image - reverse y (-1/1)",fid) == 1)
      ||(fWriteInt (par_epi->reverse_y,fid) == 1)

      ||(fWrite ( "epi image - reverse z (-1/1)",fid) == 1)
      ||(fWriteInt (par_epi->reverse_z,fid) == 1)
      
      ||(fWrite ( "inter_echo_time",fid) == 1)
      ||(fWriteDouble (par_a->inter_echo_time,fid) == 1)
      
      ||(fWrite ( "dwell_time",fid) == 1)
      ||(fWriteDouble (par_a->dwell_time,fid) == 1)

      ||(fWrite ( "noise threshold",fid) == 1)
      ||(fWriteDouble (par->noise_threshold,fid) == 1)

      ||(fWrite ( "unwrapping starting point - coord x",fid) == 1)
      ||(fWriteInt (par->ref_x,fid) == 1)

      ||(fWrite ( "unwrapping starting point - coord y",fid) == 1)
      ||(fWriteInt (par->ref_y,fid) == 1)

      ||(fWrite ( "unwrapping base size (should be a square number)",fid) == 1)
      ||(fWriteInt (par->unwrap_base_size,fid) == 1)

      ||(fWrite ( "extrapolating base size (should be a square number)",fid) == 1)
      ||(fWriteInt (par->extrapol_base_size,fid) == 1)

      ||(fWrite ( "extrapolating kernel size",fid) == 1)
      ||(fWriteInt (par->extrapol_kernel_size,fid) == 1)

      ||(fWrite ( "eroding radius",fid) == 1)
      ||(fWriteInt (par->erode_radius,fid) == 1)

      ||(fWrite ( "dilating radius",fid) == 1)
      ||(fWriteInt (par->dilate_radius,fid) == 1)

      ||(fWrite ( "Actual map (1 = yes; -1 = no)",fid) == 1)
      ||(fWriteInt (par->actual_map,fid) == 1)

      ||(fWrite ( "Perform segmentation (1=yes ; -1=mask provided)",fid) == 1)
      ||(fWriteInt (par->segmentation,fid) == 1)

      ||(fWrite ( "Segmented mask filename (not used if segmentation performed)",fid) == 1)
      ||(fWrite(par->fname_mask,fid) == 1)

      ||(fWrite ( "Provided Mask : reverse z axis (1 = no ; -1 = reverse)",fid) == 1)
      ||(fWriteInt (par->mask_reverse_z_axis,fid) == 1)

      )
  return 1;
  else return 0;
}

void DefaultParam (param *par,param_acq *par_a,param_mat *par_echo,param_mat *par_epi) {

      par_echo->s_x = 128;
      par_echo->s_y = 128;
      par_echo->s_z = 22;
      
      par_epi->s_x = 64;
      par_epi->s_y = 64;
      par_epi->s_z = 22;
      par_epi->s_t = 1;


      par_a->inter_echo_time = 2.28;
      par_a->dwell_time = 0.64;

      par->noise_threshold = 0.01;
      par->ref_x = (par_echo->s_x)/2;
      par->ref_y = (par_echo->s_y)/2 + 5;
      
      par->unwrap_base_size = 49;
      par->extrapol_base_size = 49;

      par->extrapol_kernel_size = 5;

      par->erode_radius = 3;
      par->dilate_radius = 4;

      par->actual_map = 1;

      par->segmentation = 1;
      (void) strcpy (par->fname_mask,"NONE");
      par->mask_reverse_z_axis = -1;

      par_echo->reverse_x = 1;
      par_echo->reverse_y = 1;
      par_echo->reverse_z = 1;
      
      par_epi->reverse_x = 1;
      par_epi->reverse_y = 1;
      par_epi->reverse_z = -1;
}

