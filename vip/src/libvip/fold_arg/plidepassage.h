/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 09/08/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : creation de la relation de type plidepassage
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_PLIDEPASSAGE_H
#define VIP_PLIDEPASSAGE_H

#ifdef __cplusplus
extern "C" {
#endif



  typedef struct plidepassage {
    Vip3DBucket_S16BIT *points;   	
    int index1;
    int index2;
    struct surfacesimple *s1;
    struct surfacesimple *s2;
    struct plidepassage *next;
    int size_filled;
    float size;
    int depth_filled;
    float depth;
    Vip3DPoint_S16BIT location;
    int location_filled;
    Vip3DPoint_VFLOAT reflocation;
    int reflocation_filled;
  } PliDePassage;

  typedef struct plidepassageset {
      int n_pp;
      PliDePassage *first_pp;
  } PliDePassageSet;





#ifdef __cplusplus
}
#endif

#endif
