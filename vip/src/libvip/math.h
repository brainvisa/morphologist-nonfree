/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/math.h           * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 31/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Declarations du package de calcul mathematique
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_MATH_H
#define VIP_MATH_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef PI
#define PI M_PI
#endif

#include <vip/volume.h>
#include <vip/alloc.h>
#include <vip/util.h>

#include <vip/math/random.h>
#include <vip/math/trigo.h>
#include <vip/math/matrix.h>
#include <vip/math/vector_gen.h>

#ifdef __cplusplus
  }
#endif

#endif
