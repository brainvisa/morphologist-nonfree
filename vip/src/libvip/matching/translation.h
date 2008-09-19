/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/translation.h    * TYPE     : Header
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

#ifndef VIP_TRANSLATION_H
#define VIP_TRANSLATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/matching/matching.h>
#include <vip/matching/gravite.h>
#include <vip/matching/list.h>
#include <vip/matching/mask.h>

typedef Vip3DPoint_VFLOAT Translation;

extern Translation *VipGetInitialTranslation(Surface *surf,
					     Volume *vol,
					     Gravity *volgrav);

extern Echelle *GetVolumeEchelle(Volume *vol);

extern Echelle *GetSurfaceEchelle(Surface *surf);

extern Translation *CalculInitialTranslation( Gravity *gref, 
					      Gravity *echref, 
					      Echelle *grecal, 
					      Echelle *echrecal );

extern SuperList *VipGetTranslatedBucket(VipG3DBucket *buck,
					 Translation *t,
					 Echelle *echref,
					 int interpol);

extern SuperList *ApplyTranslationWithPPVoisin(VipG3DBucket *buck,
					       Translation *t,
					       Echelle *echref);

extern SuperList *ApplyTranslationWithInterpolTrilin(VipG3DBucket *buck,
						     Translation *t,
						     Echelle *echref);

#ifdef __cplusplus
}
#endif

#endif
