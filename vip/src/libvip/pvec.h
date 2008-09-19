/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/pvec.h           * TYPE     : Header
 * AUTHOR      : Frouin V.            * CREATION : 20/09/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC
 *****************************************************************************
 *
 * DESCRIPTION : header for Partial Volume Effect Correction
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


#ifndef VIP_PVEC_H
#define VIP_PVEC_H

#ifdef __cplusplus
  extern "C" {
#endif

/* general includes -------------------------------------------------------  */


/* application includes ---------------------------------------------------  */
#include <vip/pvec/gtm_method.h>



/* extern declaration   ---------------------------------------------------  */
extern VipMatrix_VDOUBLE *VipStriataGtm (
                                 Volume *label_vol, int *label, int nb_label,
			         float sigmaTrans, float sigmaAxi,
				 Volume *anatomy, int mode, int dump
                                );




#ifdef __cplusplus
  }
#endif

#endif
