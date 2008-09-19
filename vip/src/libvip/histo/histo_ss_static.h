/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/histo_ss_static.h       * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/08/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : sparc5
 *****************************************************************************
 *
 * DESCRIPTION : Scalespace1D des histogrammes
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
#ifndef VIP_HISTO_SS_STATIC_H
#define VIP_HISTO_SS_STATIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/histo.h>

#define D0m 0
#define D0M 1
#define D1m 2
#define D1M 3 
#define D2m 4
#define D2M 5
#define D3m 6
#define D3M 7
#define D4m 8
#define D4M 9

  /*for convenience, TRACKED HAS to 2*the old define, CASCADE 3...**/
#define MAX1D 1
#define MAX1DTRACKED 2
#define MAX1DCASCADE 3
#define MAX1DSPEEDMINIMUM 4
#define MIN1D -1
#define MIN1DTRACKED -2
#define MIN1DCASCADE -3
#define MIN1DSPEEDMINIMUM -4

#define LOST 123


/*-----------------------------------------------------------------------*/
extern int VipPutSSExtremaValInBuffer(double *data,Vip_S8BIT *eptr,int dim,Singularity *val,int *nval,int sizeval,int iter);
/*-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipComputeCascadeVolume( SSCascade *c,  VipHisto *hcumul );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipMarkSSExtremum( SSSingularity *s, Vip1DScaleSpaceStruct *ssstruct);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipMarkSSCascadeExtrema( SSCascade *c, Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipTrackSSSingularity(
Vip_S8BIT *event,
int loc,
int scale_event,
int nature,
int xsize,
SSSingularity **sarg) ;
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipAnalyseCascadesHistology(VipT1HistoAnalysis *ana,
                                SSCascade *clist, Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipScaleInsertSSCascadeInList(
SSCascade *inserted,
SSCascade **list);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipInsertSSCascadeInList(
SSCascade *inserted,
SSCascade **list);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSSingularity * VipAllocSSSingularity( int scale_event, int nature );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipInsertSSSingularity(
SSSingularity *inserted,
SSSingularity **list);
/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* VIP_HISTO_SS_H */
