/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/distance.h       * TYPE     : Header
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

#ifndef VIP_DISTANCE_H
#define VIP_DISTANCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/bucket.h>
#include <vip/distmap.h>
#include <vip/matching/list.h>

#define POURCENTAGE_MIN_DANS_CARTE                0.3
#define POURCENTAGE_MIN_HORS_CARTE_POUR_AFFICHAGE 0.8
#define SORT_DE_LA_CARTE                          123456789
#define INFINI                                    1.0e10

typedef struct dist6connexe
{
  double central;
  double xmoins;
  double xplus;
  double ymoins;
  double yplus;
  double zmoins;
  double zplus;
} Dist6Connexe;

extern double VipGetMoyenneSomSquareDistance(
  SuperList *list,
  Volume *dmap,
  Vip3DPoint_S16BIT *offset_3d
);

extern double GetMoyenneSomSquareDistancePPVoisin(
  SuperList *list,
  Volume *dmap,
  Vip3DPoint_S16BIT *offset_3d
);

extern int VipFillSuperPointErreurInterpolTrilin(
  SuperList *list,
  Volume *dmap
);

extern double GetMoyenneSomSquareDistanceInterpolTrilin(
  SuperList *list,
  Volume *dmap,
  Vip3DPoint_S16BIT *offset3d
);

extern double VipFindMinDistInVoisinage(
  SuperList *list,
  Volume *dmap,
  Vip3DPoint_S16BIT *initoffset,
  Vip3DPoint_S16BIT *maxoffset,
  Vip3DPoint_S16BIT *bestoffset,
  Dist6Connexe *distvoisins
);

extern double VipFindBestPositionInDistanceMap(
  SuperList *list,
  Volume *dmap,
  Vip3DPoint_S16BIT *maxoffset,
  Vip3DPoint_S16BIT *bestoffset,
  Dist6Connexe *distvoisins
);

extern int FillSuperPointExternalFlag(
  SuperList *list,
  Volume *dmap,
  Vip3DPoint_S16BIT *initoffset,
  Vip3DPoint_S16BIT *maxoffset
);

extern int FillSuperPointOffset(
  SuperList *list,
  Volume *dmap
);

#ifdef __cplusplus
}
#endif

#endif
