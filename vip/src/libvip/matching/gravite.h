/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/gravite.h        * TYPE     : Header
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

#ifndef VIP_GRAVITE_H
#define VIP_GRAVITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/matching/surface.h>

extern int VipFillSurfG(Surface *surf);

extern int VipFillVolumeG(Volume *vol,
			   Gravity *volgrav);

extern Gravity *GetGBucketGxy(VipG3DBucket *buck);

extern Gravity *GetGBucketGxyz(VipG3DBucket *buck);

#ifdef __cplusplus
}
#endif

#endif
