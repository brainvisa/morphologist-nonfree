/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/topology         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 22/01/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Calcul de composantes sur des 18 et 26 voisinages
 * Transfert initial des codes de la these de jeff par F. Poupon
 *
 ****************************************************************************
 *
 * USED MODULES : vip/volume.h - vip/alloc.h -vip/connex.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *         10/04/97   |  J-F Mangin  |  extensions -> skeleton
 *         18/06/2015 |  Y Leprince  |  introduce well-composed topology
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_TOPOLOGY_H
#define VIP_TOPOLOGY_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <math.h>

#include <vip/volume.h>
#include <vip/alloc.h>
#include <vip/connex.h>


/*The following structure could be completely hiden, but we prefer to let it
public to make users aware of the Volume dependence of underlying offsets*/
/*Moreover, it could be used to speed-up other computation related
to local connectivity*/

/* the Topology26Neighborhood structure embeds the precomputation of 3D connectivity
in a 26 neighborhood -> speed-up the connectivity numbers CBAR or CSTAR computations*/
/* relations are based on an ordering of 1+26 neighbors values corresponding
to offset tab (namely 6/18/26 for genericity) in an internal buffer ordered_values[27]*/
typedef struct topology26
{
  int *relation[27];  /*relation[a][b] means connection a-b (ordered according to 6/18/26 connectivity)*/
  int relation_store[316]; /* stores all items of relation consecutively in memory */
  int nb6neighbors[27];  /* nb6neighbors[a] = nb points of X 6-connected to a */
  int nb18neighbors[27]; /* nb18neighbors[a] = nb points of X 18-connected to a (and not 6-con...)*/
  int nb26neighbors[27]; /* nb26neighbors[a] = nb points of X 26-connected to a (and not 18-con...)*/
  int offset[27];
  /* offset to 26 neighbors, WARNING: volume and borderwidth dependent,
     similar to vip/connectivity structures but not necessarily the same ordering*/
  int ordered_values[27];
} Topology26Neighborhood;

#include <vip/topology/topology_gen.h>

/*topolofical classification types according to CBAR and CSTAR*/

#define TOPO_A 10
#define TOPO_VOLUME_POINT 10

#define TOPO_B 20
#define TOPO_ISOLATED_POINT 20

#define TOPO_C 30
#define TOPO_BORDER_POINT 30

#define TOPO_D 40
#define TOPO_CURVE_POINT 40

#define TOPO_E 50
#define TOPO_JUNCTION_CURVES_POINT 50

#define TOPO_F 60
#define TOPO_SURFACE_POINT 60

#define TOPO_G 70
#define TOPO_JUNCTION_SURFACE_CURVES_POINT 70

#define TOPO_H 80
#define TOPO_JUNCTION_SURFACES_POINT 80

#define TOPO_I 90
#define TOPO_JUNCTION_SURFACES_CURVES_POINT 90

/* chose cc adjacency-mode to central point in local cc number computation*/

#define CC_6_ADJACENT 77
#define CC_18_ADJACENT 78
#define CC_26_ADJACENT 79


/* ======================== Well-composed topology ========================  */

/* Well-composed images are binary or multi-label images in which no
 * topological ambiguity exists. In other words, the objects are defined
 * non-ambiguously with regard to the connectivity used (6, 18, or
 * 26-connectivity). This property is achieved by excluding certain ambiguous
 * cases, called critical configurations (see below).
 *
 * For a thorough introduction to 3D well-composed images, see L. J. Latecki.
 * 3D Well-Composed Pictures. Graphical Models and Image Processing 59, 164–172
 * (1997). doi:10.1006/gmip.1997.0422
 */

/*
 * A binary 8-neighbourhood (cube of 2x2x2 voxels) may be in one of these three
 * states regarding well-composedness:
 */
enum {
  /* Non-critical configuration: neither of the two cases below happens, hence
   * no topological ambiguity exists. */
  WELL_COMPOSED_CRITICAL_NONE = 0,

  /* Type 1 critical configuration: two voxels of an object are 18-adjacent
   * (edge-adjacent), but not 6-adjacent (face-adjacent). */
  WELL_COMPOSED_CRITICAL_C1 = 1,

 /* Type 2 critical configuration: two voxels of an object are 26-adjacent
  * (corner-adjacent), but not 6-adjacent (face-adjacent). */
  WELL_COMPOSED_CRITICAL_C2 = 2
};

/* A binary 8-neighbourhood (cube of 2x2x2 voxels) has 2^8=256 possible
 * configurations, which are listed in this table. This is a look-up table with
 * an 8-bit index, whose bits represent a 2x2x2 binary neighbourhood, ordered
 * as in VipIsWellComposedForCentralVoxel (topology/wellcomposed.gen). For
 * each configuration the value is one of WELL_COMPOSED_CRITICAL_NONE,
 * WELL_COMPOSED_CRITICAL_C1, or WELL_COMPOSED_CRITICAL_C2.
 */
extern const char vip_well_composed_critical_configuration_n8lookup[256];

#include <vip/topology/wellcomposed_gen.h>

/*this test discard non simple point which do not really split the background*/
/*-----------------------------------------------------------------------*/
extern int IsRealSurfacePoint(
  Topology26Neighborhood *topo26);
/*-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Topology26Neighborhood *VipCreateTopology26Neighborhood(
  Volume *vol
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

extern int VipGetNumberOfComponentsIn26Neighborhood(Topology26Neighborhood *topo, int connectivity, int adjacency_to_central);
/*----------------------------------------------------------------------------*/

extern int VipGetNumberOfComponentsIn18Neighborhood(Topology26Neighborhood *topo, int connectivity, int adjacency_to_central);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipSwitchTopology26NeighborhoodValueTable(
  Topology26Neighborhood *topo
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipFreeTopology26Neighborhood(
  Topology26Neighborhood *topo
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipDisplayTopologySingularityType(
  int C18,
  int C26
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipCreateTopologyTestVolume(
  int type
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipDisplayTopologyTestVolume(
  Volume *t
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern long *VipGetTopologyTypeTable(
  Topology26Neighborhood *topo
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipDisplayTopologyTypeTable(
  char *filename,
  long *typeTable
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipDisplayTopologyTypeTableInFile(
  char *filename,
  long *typeTable
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipTopologicalClassificationForLabel(
Volume *vol,
int label);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipTopologicalClassificationForLabelComplement(
Volume *vol,
int label);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipTopologicalClassificationForTwoLabelComplement(
Volume *vol,
int inside,
int outside);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern char *VipTopologicalStringFromDefine(int def);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipIsCurvesPoint(
  int Cbar,
  int Cstar
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipIsSimplePoint(
  int Cbar,
  int Cstar
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipFlipTopology26NeighborhoodValueTable(
  Topology26Neighborhood *topo
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipFillSSComponent(
VipS16BITPtrBucket *comp1,
VipS16BITPtrBucket *comp2,
Topology26Neighborhood *top,
Vip_S16BIT *adresse);

/*----------------------------------------------------------------------------*/
int VipSurfaceSimpleEquivalence(
VipS16BITPtrBucket *comp1,
VipS16BITPtrBucket *comp2,
VipS16BITPtrBucket *comp1bis,
VipS16BITPtrBucket *comp2bis);
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
  }
#endif

#endif
