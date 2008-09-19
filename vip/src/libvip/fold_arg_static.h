/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Include
 * AUTHOR      : MANGIN J.F.          * CREATION : 2/06/99
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : private defines
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *****************************************************************************/

#ifndef VIP_FOLD_ARG_STATIC_H
#define VIP_FOLD_ARG_STATIC_H

#ifdef __cplusplus
  extern "C" {
#endif

#define MIN_SIZE_BASSIN_SPLITTED 20

  typedef struct junctionpoint {
    short x;
    short y;
    short z;
    unsigned char nb_ss;
    Vip_S16BIT *label_ss;
  } JunctionPoint;

  typedef struct junctionpointlist {
    int n_points;
    int size;
    JunctionPoint *first;
  } JunctionPointList;

  typedef struct touchbassin {
    int n_bassins;
    int *lab;
    int *size;
  } TouchBassin;

/*-------------------------------------------------------------------------*/
extern JunctionPointList *GetListOfJunctionPoint(Volume *v,
						 int inside, int outside);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipFreeJunctionPointList( JunctionPointList *jpl);
/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
