/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/random.h         * TYPE     : Header
 * AUTHOR      : MANGIN J.-F.         * CREATION : 22/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Generation de nombre aleatoires
 *
 ****************************************************************************
 *
 * USED MODULES : stdio.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_ALEATOIRE_H
#define VIP_ALEATOIRE_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdio.h>

#include <vip/volume.h>

typedef struct random_box
{ unsigned char label;
  float         proba;
  struct random_box *next;
} RandomBox;

/*----------------------------------------------------------------------------*/
extern RandomBox *VipCreateRandomBoxCircularList(
  int size,
  int nb_label
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern RandomBox *VipCreateRandomBox(
  unsigned char label,
  float         proba
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipFreeRandomBox(
  RandomBox *rb_list
);
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
