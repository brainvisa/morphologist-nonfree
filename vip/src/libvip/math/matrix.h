/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/matrix.h         * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 14/03/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Declarations du package de calcul matriciel
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

#ifndef VIP_MATRIX_H
#define VIP_MATRIX_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <vip/volume.h>
#include <vip/alloc.h>

#include <vip/math/matrix_gen.h>

#define C_NORMAL   100
#define C_INVERSE  101
#define C_VARIANCE 102

#ifdef __cplusplus
  }
#endif

#endif
