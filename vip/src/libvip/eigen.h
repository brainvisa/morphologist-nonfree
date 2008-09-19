/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/eigen.h          * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 20/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Declarations liees au calcul des vecteurs et valeurs
 *               propres ( adapted from Numerical Recipes in C )
 *               cpoupon@cea.fr
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



#ifndef VIP_EIGEN_H
#define VIP_EIGEN_H

#ifdef __cplusplus
  extern "C" {
#endif 

#include <math.h>

#include <vip/util.h>
#include <vip/alloc.h>

#include <vip/eigen/eigen_gen.h>

#define VipEigenSign(a, b)  (((b) >= 0.0) ? fabs(a) : -fabs(a))

#ifdef __cplusplus
  }
#endif

#endif
