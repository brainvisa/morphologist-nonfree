/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/topology.h       * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 22/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Calcul de composantes sur des 18 et 26 voisinages
 * Transfert des codes de la these de jeff par F. Poupon
 *
 ****************************************************************************
 *
 * USED MODULES : vip/volume.h - vip/alloc.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *         10/à4/97   |  J-F Mangin  |  extensions -> skeleton
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_TOPOLOGY_STATIC_H
#define VIP_TOPOLOGY_STATIC_H

#ifdef __cplusplus
  extern "C" {
#endif


#define VIP_IN_LIST          123
#define VIP_IL_EXIST         101
#define VIP_IL_EST_TROP_LOIN 111 /*computing for 18-neighborhood using 26-neighborhood stuff*/

#define TOPO_PB -1 /*specific test for int returning functions...bad stuff but for development only*/

extern char TopologyTable[4][4][40];

extern int TopologicalClassification[10][7];




#ifdef __cplusplus
  }
#endif

#endif
