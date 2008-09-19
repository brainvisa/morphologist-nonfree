/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/depla_static.h       * TYPE     : Header
 * AUTHOR      : MANGIN J.F.              * CREATION : 15/01/1997
 * VERSION     : 0.1                      * REVISION :
 * LANGUAGE    : C                        * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Diverses constantes et procedures liees aux rotations 
 *               affines
 *               jfmangin@cea.fr
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



#ifndef VIP_DEPLA_STATIC_H
#define VIP_DEPLA_STATIC_H

#ifdef __cplusplus
  extern "C" {
#endif

/*---------------------------------------------------------------*/
extern int StaticInverse(
   double Pin[3][3],
   double Q[3][3]);
/*---------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif /* VIP_DEPLA_STATIC_H */

