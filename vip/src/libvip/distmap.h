/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/distmap.h        * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 05/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Diverses constantes et procedures liees aux cartes de 
 *               distance
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


#ifndef VIP_DISTMAP_H
#define VIP_DISTMAP_H

#ifdef __cplusplus
  extern "C" {
#endif

#define VIP_USUAL_DISTMAP_MULTFACT 50.


#define VIP_NO_LIMIT_IN_PROPAGATION 60000
#define VIP_PUT_LIMIT_TO_OUTSIDE -123456
#define VIP_PUT_LIMIT_TO_LIMIT -123457
#define VIP_PUT_LIMIT_TO_INFINITE -123458
#define VIP_DO_NOTHING -123459


/*----------------------------------------------------------------------------*/
extern int VipComputeChamferDistanceMap (
   Volume *vol
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipComputeVoronoi (
   Volume *vol
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeCustomizedChamferDistanceMap (
  Volume *vol,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipComputeCustomizedVoronoi (
  Volume *vol,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeConnectivityChamferDistanceMap (
  Volume *vol,
  int connectivity
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipComputeConnectivityVoronoi (
  Volume *vol,
  int connectivity
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeGeodesicChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipComputeIteratedGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipComputeConnectivityGeodesicChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax,
  int connectivity
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeCustomizedGeodesicChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipComputeCustomizedIteratedGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipComputeConnectivityIteratedGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax,
  int connectivity
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipComputeFrontPropagationChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int limit_mode,
  float limit_value
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipComputeCustomizedFrontPropagationChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int limit,
  float limit_value,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
Volume *VipComputeCustomizedFrontPropagationGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
);
/*---------------------------------------------------------------------------*/

Volume *VipComputeFrontPropagationGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipComputeFrontPropagationConnectivityDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int limit_mode,
  int limit_value,
  int connectivity
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
Volume *VipComputeConnectivityFrontPropagationGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int connectivity
);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif /* VIP_DISTMAP_H */
