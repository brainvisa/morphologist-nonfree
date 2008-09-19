/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/interpol.h       * TYPE     : Header
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

#ifndef VIP_INTERPOL_H
#define VIP_INTERPOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/bucket.h>
#include <vip/matching/list.h>
#include <vip/matching/distance.h>

#define PLUS_PROCHE_VOISIN 1000
#define TRILINEAIRE        1100

extern Vip3DPoint_VFLOAT *VipGetPolynomInterpolBestPosition(
  SuperList *list,
  Volume *dmap,
  Vip3DPoint_S16BIT *best_offset,
  Dist6Connexe *distvoisins
);

#ifdef __cplusplus
}
#endif

#endif
