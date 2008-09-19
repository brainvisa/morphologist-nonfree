/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/recalage.h       * TYPE     : Header
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

#ifndef VIP_RECALAGE_H
#define VIP_RECALAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/matching/matching.h>
#include <vip/matching/list.h>
#include <vip/matching/translation.h>
#include <vip/matching/rotation.h>
#include <vip/matching/distance.h>
#include <vip/matching/interpol.h>

#define NB_ITERATION_MAX 20
#define GAIN_RELATIF     0.001
#define NO_BEST_DILATION -123456789.0

extern double VipGetMinDistTrilinAndFillBestTransForRot(Surface *surf, 
							Volume *ref, 
							VipDeplacement *dep);

extern int VipDumbMainRegistration(Surface *surf, 
				   Volume *ref,
				   VipDeplacement *dep,
				   float incredef,
				   float precidef);

extern int VipDumbMain2DRegistration(Surface *surf, 
				     Volume *ref,
				     VipDeplacement *dep,
				     float incredef,
				     float precidef);

extern int VipDestroyOutliers(Surface *surf,
			      VipDeplacement *dep,
			      Volume *ref);

extern int VipDestroyOutliersParam(Surface *surf,
				   VipDeplacement *dep,
				   Volume *ref,
				   float param);

extern int VipIncrementeRotWithRotAroundG(VipDeplacement *depini,
					  VipDeplacement *newdep,
					  VipRotation *incrrot,
					  Gravity *g,
					  Echelle *ech);

extern double VipFindBestRotAroundGForAnAngle(Surface *surf,
					      Volume *ref,
					      VipDeplacement *olddep,
					      VipDeplacement *bestdep,
					      float increment);

extern double VipFindBest2DRotAroundGForAnAngle(Surface *surf,
						Volume *ref,
						VipDeplacement *olddep,
						VipDeplacement *bestdep,
						float increment);

extern float VipFindBestDilationForDep(Surface *surf,
				       Volume *ref,
				       VipDeplacement *dep,
				       float dilatini,
				       float dilatincrement);

#ifdef __cplusplus
}
#endif

#endif
