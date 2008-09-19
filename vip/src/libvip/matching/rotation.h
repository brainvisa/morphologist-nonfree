/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/rotation.h       * TYPE     : Header
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
 *            09/02/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#ifndef VIP_ROTATION_H
#define VIP_ROTATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/matching/matching.h>
#include <vip/matching/interpol.h>
#include <vip/matching/mask.h>

extern SuperList *VipGetRotatedBucket(Vip3DBucket_VFLOAT *buck,
				      VipDeplacement *d,
				      Echelle *echref,
				      int interpol);

extern SuperList *ApplyRotationWithInterpolTrilin(Vip3DBucket_VFLOAT *buck,
						  VipDeplacement *d,
						  Echelle *echref);

extern SuperList *ApplyRotationWithPPVoisin(Vip3DBucket_VFLOAT *buck,
					    VipDeplacement *d,
					    Echelle *echref);

extern int ConvertRotationTabDouble22(VipRotation *rot,
				      double trot[3][3]);

extern int ConvertTabDouble22Rotation(double trot[3][3],
				      VipRotation *rot);

#ifdef __cplusplus
}
#endif

#endif
