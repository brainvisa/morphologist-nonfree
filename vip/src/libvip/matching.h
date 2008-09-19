/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/matching.h       * TYPE     : Header
 * AUTHOR      : MANGIN J.-F.         * CREATION : 
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : 
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            03/01/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#ifndef VIP_MATCHING_TOP_H
#define VIP_MATCHING_TOP_H

#ifdef __cplusplus
  extern "C" {
#endif

#define FMRI_ALPHA 1.0
#define FMRI_DERICHE_THRESHOLD 10
 
#define FMRI_2DCONNEXE_FILTER_HYSTERESIS 10
#define FMRI_LOW_THRESHOLD_HYSTERESIS 700
#define FMRI_HIGH_THRESHOLD_HYSTERESIS 1200
#define FMRI_2DCONNEXE_FILTER_SCALP 10

#define FMRI_FINAL_ACCURACY 0.05

#define SCALP_LUT_DISTANCE_CORRECTION 10.0 /*mm.*/
#define BRAIN_LUT_DISTANCE_CORRECTION 10.0 /*mm.*/

#define POURCENTAGE_LOW    0.15
#define POURCENTAGE_HIGH   0.3
#define POURCENTAGE_LOW3D  0.1
#define POURCENTAGE_HIGH3D 0.2

#include <vip/volume.h>

#include <vip/matching/matching.h>
#include <vip/matching/surface.h>
#include <vip/matching/mask.h>
#include <vip/matching/translation.h>
#include <vip/matching/gravite.h>
#include <vip/matching/dilatation.h>
#include <vip/matching/distance.h>
#include <vip/matching/interpol.h>
#include <vip/matching/list.h>
#include <vip/matching/recalage.h>
#include <vip/matching/rotation.h>

#ifdef __cplusplus
  }
#endif

#endif /* VIP_MATCHING_H */
