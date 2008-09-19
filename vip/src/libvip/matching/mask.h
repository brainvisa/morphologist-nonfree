/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/mask.h           * TYPE     : Header
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
 *            08/02/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#ifndef VIP_MASK_H
#define VIP_MASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/volume.h>
#include <vip/distmap.h>

#define EPSILON 0.000001

#define X_MASK_SIZE 5
#define Y_MASK_SIZE 5
#define Z_MASK_SIZE 3

#define VIP_MASK_AUTO -1.23456

extern int VipCreateDistanceMapWithLut(Volume *vol,
				       int xmask,
				       int ymask,
				       int zmask,
				       float float_to_int,
				       float dist_voisinage);

extern Vip_S16BIT *VipCreateLutCorrection(Volume *vol,
					  int xmask,
					  int ymask,
					  int zmask,
					  float distmax);

extern int VipApplyLutCorrection(Volume *vol,
				 Vip_S16BIT *lut,
				 float dist);

#ifdef __cplusplus
}
#endif

#endif
