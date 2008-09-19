/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_pca.h            * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 28/06/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Analyse en Composantes Principales
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_alloc.h - Vip_eigen - Vip_util.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_PCA_H
#define VIP_PCA_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <vip/alloc.h>
#include <vip/eigen.h>
#include <vip/util.h>

#define PCA_CORRELATION 100
#define PCA_COVARIANCE  101
#define PCA_SSCP        102

#ifdef __cplusplus
  }
#endif

#endif
