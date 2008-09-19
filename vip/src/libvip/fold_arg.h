/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg.h       * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 02/03/1999
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : recuperation du format de graphe relationnel
 *               attribue sur les plis corticaux  de la these de JF Mangin
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_FOLD_ARG_H
#define VIP_FOLD_ARG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/bucket.h>
#include <vip/depla.h>
#include <vip/talairach.h>

#define HULL_SURFACE 290
#define HULL_SURFACE_INDEX 0
#define FORBIDEN_JUNCTION 99

#include <vip/fold_arg/junction.h>
#include <vip/fold_arg/cortex_rel.h>
#include <vip/fold_arg/plidepassage.h>
#include <vip/fold_arg/simplesurface.h>
#include <vip/fold_arg/attribut.h>
#define HULL_SURFACE 290
#define HULL_SURFACE_INDEX 0
  typedef struct fold_arg {
    char name[1024];
    SurfaceSimpleList *ss_list;
    JunctionSet *jset;
    PliDePassageSet *ppset;
    CortexRelationSet *cortex_jset;
    VipDeplacement gotoref; 
    Vip3DPoint_S16BIT min_ima;
    Vip3DPoint_S16BIT max_ima;
    Vip3DPoint_VFLOAT min_ref;
    Vip3DPoint_VFLOAT max_ref;
    Vip3DPoint_VFLOAT scaleref;
    Vip3DPoint_VFLOAT vox;
    int gotoref_filled;
    int gonetoref;
    int CAfilled;
    int CPfilled;
    int IHfilled;
    Vip3DPoint_S16BIT CA;
    Vip3DPoint_S16BIT CP;
    Vip3DPoint_S16BIT IH;
  } FoldArg;

  /*
    ss_list: liste des noeuds du graphe relationnel: surfaces simples
    du squelette non externes;
    jset: liste des jonctions topologiques entre ss
    NB: les jonctions avec l'exterieur sont inclues dans cette liste;
    cortex_jset: liste des jonctions corticales entre ss;
    gotoref: transformation rigide 3D permettant de passer au referentiel commun
    du modele (a priori defini a partir du referentiel d'inertie
    du plan inter-hemispherique, autre possibilite moins
    automatique: Talairach)
    refmin, refmax: boite englobant le squelette dans le referentiel
    commun. Elle permet la proportionnalisation ( a priori,
    6 parametres ==> boite (-1,-1,-1,1,1,1)
  */


#include <vip/fold_arg/fold_arg_io.h>

/*-------------------------------------------------------------------------*/
extern int VipUnhookBijunctionFromJset(
					  BiJunction *ToUnhook,
					  JunctionSet *jset);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern JunctionSet *VipComputeFoldArgJunctionSet( Volume *vol,
						  SurfaceSimpleList *sslist,
						  int inside, int outside);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern CortexRelationSet *VipComputeFoldArgCortexRelationSet( Volume *vol,
					   SurfaceSimpleList *sslist );
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern FoldArg *CreateFoldArgFromSquel( char *name, Volume *skeleton, int inside, int outside, int limitsssize,
int compute_triang, char *rootsvoronoi_name);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern FoldArg *CreateEmptyFoldArg(char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern PliDePassageSet *VipComputeFoldArgPliDePassageSet( SurfaceSimpleList *sslist,
						   JunctionSet *jset);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern PliDePassage *CreatePliDePassage(
			     int index1,
			     int index2,
			     Vip3DBucket_S16BIT *buck,
			     SurfaceSimpleList *sslist);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern int FillSSListPliDePassagePtr(
				 SurfaceSimpleList *ss_list,
				 PliDePassageSet *ppset);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern int VipComputeTmtkTriangulation( Volume *skeleton, FoldArg *arg);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipComputeFoldArgSemanticAttributes(Volume *skeleton, FoldArg *arg,
				       int inside, int outside,
					  VipTalairach *tal);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipComputeSSAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipComputeJunctionAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipComputePliDePassageAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipComputeCortexRelationAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal);
/*-------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif
