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

#ifndef VIP_JUNCTION_H
#define VIP_JUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif

#define BIJUNCTION 260
#define MULTIJUNCTION3 273
#define MULTIJUNCTION4 274
#define MULTIJUNCTION5 275
#define MULTIJUNCTION6 276
#define MULTIJUNCTION7 277
#define MULTIJUNCTION8 278
#define MULTIJUNCTIONN 279

  typedef struct bijunction {
    Vip3DBucket_S16BIT *points;   	
    int index1;
    int index2;
    struct surfacesimple *s1;
    struct surfacesimple *s2;
    struct bijunction *next;
    int size_filled;
    float size;
      int maxdepth_filled;
      float maxdepth;
      int mindepth_filled;
      float mindepth;
      /* direction de la junction*/
      Vip3DPoint_VFLOAT direction;
      int direction_filled;
      /* direction dans le referentiel commun*/
      Vip3DPoint_VFLOAT refdirection;
      int refdirection_filled;
      /* first extremity de la junction*/
      Vip3DPoint_S16BIT extremity1;
      int extremity1_filled;
      /* first extremity dans le referentiel commun*/
      Vip3DPoint_VFLOAT refextremity1;
      int refextremity1_filled;
      /* Second extremity de la junction*/
      Vip3DPoint_S16BIT extremity2;
      int extremity2_filled;
      /* Second extremity dans le referentiel commun*/
      Vip3DPoint_VFLOAT refextremity2;
      int refextremity2_filled;
  } BiJunction;

  typedef struct junctionset {
    int n_bi;
    BiJunction *first_bi;
  } JunctionSet;



#ifdef __cplusplus
}
#endif

#endif
