/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
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

#ifndef VIP_CORTEX_REL_H
#define VIP_CORTEX_REL_H

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct cortexbirelation {
    int index1;
    int index2;
    struct surfacesimple *s1;
    struct surfacesimple *s2;
    float size; /*skiz junction point number*/
    int size_filled;
    float surface; /*second voronoi influence zones*/
    int surface_filled;
    float hulljunctiondist;
    int hulljunctiondist_filled;
    Vip3DPoint_S16BIT SS1nearest;
    int SS1nearest_filled;
    Vip3DPoint_VFLOAT refSS1nearest;
    int refSS1nearest_filled;
    Vip3DPoint_S16BIT SS2nearest;
    int SS2nearest_filled;
    Vip3DPoint_VFLOAT refSS2nearest;
    int refSS2nearest_filled;    
    struct cortexbirelation *next;
    Vip3DBucket_S16BIT *skiz;
  } CortexBiRelation;

  typedef struct cortexrelationset {
    int n_bi;
    CortexBiRelation *first_bi;
  } CortexRelationSet;

#ifdef __cplusplus
}
#endif

#endif
