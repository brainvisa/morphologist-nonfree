/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/gaussian_static.h     * TYPE     : Header
 * AUTHOR      : MANGIN J.F.              * CREATION : 7/12/1998
 * VERSION     : 1.4                      * REVISION :
 * LANGUAGE    : C                        * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Informations privees du package gaussian
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


#ifndef VIP_GAUSSIAN_STATIC_H
#define VIP_GAUSSIAN_STATIC_H

#define PI 2*acos(0.0)


typedef struct Coefficient
{
  float n00;
  float n11;
  float n22;
  float n33;
  float n11b;
  float n22b;
  float n33b;
  float n44b;
  float d44;
  float d33;
  float d22;
  float d11;
} Coefficient;

/*---------------------------------------------------------------------------*/
int Deriche3DSmoothing(
Volume *vol,
float sigmax, float sigmay, float sigmaz);
/*---------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------*/
#endif /* VIP_GAUSSIAN_STATIC_H */
