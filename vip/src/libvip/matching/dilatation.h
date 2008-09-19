/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/dilatation.h     * TYPE     : Header
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

#ifndef VIP_DILATATION_H
#define VIP_DILATATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/matching/matching.h>
#include <vip/matching/translation.h>

extern int VipFillDilatedBucket(Surface *surf,
				float dilation);

extern int FillZeroDilatedBucket(Surface *surf);

extern int FillGrad2DDilatedBucket(Surface *surf,
				   float dilation);

extern int FillGrad3DDilatedBucket(Surface *surf,
				   float dilation);

extern int FillGRayDilatedBucket(Surface *surf,
				 float dilation);

extern int FillG3DRayDilatedBucket(Surface *surf,
				   float dilation);

extern int VipFillSurfaceGSlice(Surface *surf);

#ifdef __cplusplus
}
#endif

#endif
