/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 02/06/1999
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

#ifndef VIP_FOLD_ARG_IO_H
#define VIP_FOLD_ARG_IO_H

#ifdef __cplusplus
  extern "C" {
#endif

/*-------------------------------------------------------------------------*/
extern int VipReadOldARGtoRef(
FoldArg *arg,
char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/ 
extern int VipWriteGenericFoldArg(FoldArg *arg ,char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteOldPliDePassageSet(
		      PliDePassageSet *ppset,
		      char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteGenericPliDePassageSet(
		      PliDePassageSet *ppset,
		      char *name,
		      char *shortname,
		      Vip3DPoint_VFLOAT voxsize);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern PliDePassageSet *VipReadOldPliDePassageSet( 
char *name,
SurfaceSimpleList *sslist);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteGenericCortexRelationSet(
		      CortexRelationSet *jset,
		      char *name,
		      char *shortname,
		      Vip3DPoint_VFLOAT voxsize);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteGenericJunctionSet(
		      JunctionSet *jset,
		      char *name,
		      char *shortname,
		      Vip3DPoint_VFLOAT voxsize);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteGenericSSfile(
		    SurfaceSimpleList *sslist,
		    char *name,
		      char *shortname,
		    Vip3DPoint_VFLOAT voxsize);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern int VipWriteGenericARGtoRef(
	      FoldArg *arg,
	      char *name,
	      char *shortname);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern SurfaceSimpleList *VipReadOldSSfile(
char *name);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern FoldArg *VipReadOldFoldArg(
char *name);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
JunctionSet *VipReadOldJunctionSet( 
char *name,
SurfaceSimpleList *sslist);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
CortexRelationSet *VipReadOldCortexRelationSet( 
char *name,
SurfaceSimpleList *sslist);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern int VipWriteOldFoldArg(FoldArg *arg ,char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteOldSSfile(
		    SurfaceSimpleList *sslist,
		    char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteOldJunctionSet(
		      JunctionSet *jset,
		      char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteOldCortexRelationSet(
		      CortexRelationSet *jset,
		      char *name);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipWriteOldARGtoRef(
	      FoldArg *arg,
	      char *name);
/*-------------------------------------------------------------------------*/
#ifdef __cplusplus
  }
#endif

#endif
