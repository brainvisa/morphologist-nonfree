/*  parameters */

typedef struct {

  char fname_epi[200], fname_corr_epi[200], fname_echo[200], fname_fieldmap[200], fname_mask[200]; /* filenames */

  double  noise_threshold; /* percentage of the corrected max value of the 3D map */ 
  
  int ref_x;                        /* reference point for unwrapping */
  int ref_y;

  int extrapol_base_size;
  int unwrap_base_size;

  int actual_map;  /* 1 : actual map ; -1 : use the model only */
  
  int segmentation;/* 1 : segmentation performed ; -1 : mask read from file fname_mask */
  int mask_reverse_z_axis; /* -1 if the z_axis should be reversed ; 1 otherwise */
  
  int extrapol_kernel_size;

  int erode_sx, erode_sy, erode_sz;    /* size of the structuring element (ellipsoid) */
  int dilate_sx, dilate_sy, dilate_sz; /* for erosion and dilation */ /* NOT USED YET */

  int erode_radius;  /* radius for 2D erosion and dilation */
  int dilate_radius;

} param;


/* acquisition parameters */
typedef struct {
  double inter_echo_time, dwell_time;
} param_acq;


/* matrix parameters */
typedef struct {
  int s_x,s_y,s_z,s_t; /* size */
  int vox_sx, vox_sy, vox_sz, vox_st; /* voxel size */
  int reverse_x, reverse_y, reverse_z; /* flipping the axes */
  int transpose; /* = 1 if the read_out gradient direction is x, 0 if it is y */
                 /* not implemented yet */
} param_mat;
