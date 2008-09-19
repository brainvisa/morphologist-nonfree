/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/matching.h       * TYPE     : Header
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

#ifndef VIP_MATCHING_H
#define VIP_MATCHING_H

#ifdef __cplusplus
  extern "C" {
#endif

#define SCALP_LUT_DISTANCE_CORRECTION 10.0 /*mm.*/

#include <stdio.h>

#include <vip/bucket.h>
#include <vip/depla.h>
#include <vip/matching/surface.h>

typedef Vip3DPoint_VFLOAT Echelle;

extern char *VipGetTrans2MriFileName(char *mri, 
				     char *pet);

extern char *VipGetMri2TransFileName(char *mri, 
				     char *pet);

extern int VipDumbMain2DOr3DRegistration(Surface *monsurf, 
					 Volume *ref, 
					 VipDeplacement *dep, 
					 float increment, 
					 float precision,
					 int rotation_2D);

extern VipDeplacement *VipMatching(Volume *volref, Volume *volmob, 
                                   Volume *voldist, char *gradFile,
                                   int gradDim, int undersampling,
                                   VipDeplacement *initDep, int x_mask_size,
                                   int y_mask_size, int z_mask_size,
                                   int dodilation, int rot2D, float threshold,
                                   float M1_init_angle, float M1_final_angle,
                                   float M2_init_angle, float M2_final_angle,
                                   float M3_init_angle, float M3_final_angle);

#ifdef __cplusplus
  }
#endif

#endif
