/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/list.h           * TYPE     : Header
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

#ifndef VIP_LIST_H
#define VIP_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/bucket.h>

#define EXIST 123456

typedef struct superpoint
{
  Vip3DPoint_S16BIT p;
  int offset;
  float *coef;
  float dist;
  int out;
} SuperPoint;

typedef struct superlist
{
  SuperPoint *data;
  int interpol;
  int n_points;
  int size;
  int offsetfilled;
  int ping;
} SuperList;

extern SuperList *CreateSuperList(int size);

extern int FreeSuperList(SuperList *dead);

extern int KillSuperList(SuperList *sup);

extern Volume *SuperListToVolume(SuperList *list,
				 Volume *volref);

extern int AddSuperListToVolume(SuperList *list,
				Volume *vol,
				int value);

extern int MixteErreurVolumeRef(SuperList *list,
				SuperList *listpp,
				Volume *ref,
				double moy);

extern int AddErreurToVolumeRecal(SuperList *list,
				  VipG3DBucket *buck,
				  Volume *recal,
				  double moy);

#ifdef __cplusplus
}
#endif

#endif
