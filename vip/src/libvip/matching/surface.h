/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/surface.h        * TYPE     : Header
 * AUTHOR      : MANGIN J.-F.         * CREATION : 
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : 
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            03/02/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#ifndef VIP_SURFACE_H
#define VIP_SURFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/bucket.h>

#define GRAD_2D 2
#define GRAD_3D 3

typedef Vip3DPoint_VFLOAT Gravity;

typedef struct surface 
{
  int xsize;
  int ysize;
  int zsize;
  float xvoxsize;
  float yvoxsize;
  float zvoxsize;
  Gravity g; /* Centre de gravite de la surface dans le reperere */
             /* de la surface */
  int n_buckets;
  VipG3DBucket *bucket;
  Vip3DBucket_VFLOAT *dilated_bucket;
  int dilated_bucket_allocated;
  Vip3DBucket_S16BIT *global;
  Vip2DPoint_VFLOAT *gslice; 
  int gslicefilled;

  int gradDim;
  int gradFilled;
  char grad_file_name[VIP_NAME_MAXLEN];
} Surface;

extern Surface *VipVolumeToLittleSurface(Volume *vol,
					 int ratio,
					 char *gradFile,
					 int gradDim);

extern Surface *VipVolumeToLittleSurfaceGrad2D(Volume *vol,
					       int ratio,
					       VipG2DBucket *gbuck);

extern Surface *VipVolumeToLittleSurfaceGrad3D(Volume *vol,
					       int ratio,
					       VipG3DBucket *gbuck);

extern Surface *VipVolumeToSurfaceDumb(Volume *vol,
				       char *gradFile,
				       int gradDim);

extern Surface *VipVolumeToSurface(Volume *vol,
				   char *gradFile,
				   int gradDim);

extern int VipFreeSurface(Surface *surf);

#ifdef __cplusplus
}
#endif

#endif
