/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/pvec/gtm_method.h* TYPE     : Header
 * AUTHOR      : Frouin V.            * CREATION : 20/09/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC
 *****************************************************************************
 *
 * DESCRIPTION : header for Partial Volume Effect Correction GTM METHOD
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


#ifndef VIP_PVEC_GTM_METHOD_H
#define VIP_PVEC_GTM_METHOD_H

#ifdef __cplusplus
  extern "C" {
#endif

/* general includes -------------------------------------------------------  */


/* application includes ---------------------------------------------------  */
#include <vip/volume.h>
#include <vip/alloc.h>
#include <vip/util.h>
#include <vip/gaussian.h>
#include <vip/morpho.h>





#define GRAY_LEVEL_ANATOMY 3
#define BINARY_BRAIN       4
#define FORCE_BACK         5

/* extern declaration   ---------------------------------------------------  */




#ifdef __cplusplus
  }
#endif

#endif
