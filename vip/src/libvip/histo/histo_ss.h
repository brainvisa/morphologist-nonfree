/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/histo_ss.h       * TYPE     : Header
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
#ifndef VIP_HISTO_SS_H
#define VIP_HISTO_SS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/histo.h>


  typedef struct singularity { /*for historic reasons SSSingularity do not use Singularity*/
    double val; /*finally, this structure is not used but I let it there for potential*/ 
    double speed; /* further experiments with extremum speed and Lindeberg Gamma-derivatives*/
    int iter;
    short loc;
  } Singularity;

 typedef struct vip_ss_struct {
   int scalemax;    
   int itermax;
   int offset;
   int undersampling_ratio;
   float dscale;
   VipHisto *histo;
   VipHisto *hcumul;
   Volume *ss;
   Volume *ss_e;
   Singularity *ss_val;
   int nssval;
   int sizessval;
   Volume *d1;
   Volume *d1_e;
   Singularity *d1_val;
   int nd1val;
   int sized1val;
   Volume *d2;
   Volume *d2_e;
   Singularity *d2_val;
   int sized2val;
   int nd2val;
   Volume *d3;
   Volume *d3_e;
   Volume *d4;
   Volume *d4_e;
 } Vip1DScaleSpaceStruct;

  typedef struct sssingularity {
    int scale_event;
    short *loc;
    int analysed_loc;
    int nature;
    struct sssingularity *right;
    struct sssingularity *left;
    struct sssingularity *mate;
    struct SScascade *cascade;
    int n_scale_extrema;
    int *scale_extrema;
  } SSSingularity;

  typedef struct SScascade {
    int complet;
    int volume;
    int scale_event;
    int D1_scale_event;
    SSSingularity *D1Ms;
    SSSingularity *D1ms;
    int D2_scale_event;
    SSSingularity *D2Ms;
    SSSingularity *D2ms;
    struct SScascade *next;
  } SSCascade;

  typedef struct SSobject {
    int scale_event;
    SSSingularity *D2Mls; /* desallocated after VipGetT1HistoAnalysisCustomized */
    SSSingularity *D1Ms;
    SSSingularity *D2ms;
    SSSingularity *D1ms;
    SSSingularity *D2Mrs;
    int good_scale;
    int mean;
    int sigma;
    int left_sigma;
    int right_sigma;
  } SSObject;

    typedef struct vip_T1histogram_analysis {
      int sequence;
      int partial_volume_effect;
      SSObject *background;
      SSObject *brain;
      SSObject *white;
      SSObject *gray;
      SSObject *csf;
      SSObject *skull;
      SSObject *mode1;
      SSObject *mode2;
      SSObject *mode3;
      SSObject *mode4;
      SSObject *candidate[6];


    } VipT1HistoAnalysis;

#define SPGR_IR_RATIO 0.20 /*for 3T unbiased image 0.25->0.20, JFM 8-2001*/

#define MRI_T1_SPGR 20
#define MRI_T1_IR 21
#define MRI_T1_SEGMENTED_BRAIN 22
#define MODES_2 23
#define MODES_3 24
#define MODES_4 25
#define MODE_TISSUE 26
#define MODE_HISTO 27
#define MODE_RIDGE 28

#define SS_SIMPLE_EXTREMUM 11
#define SS_TRACKED_EXTREMUM 12
#define SS_CASCADE_EXTREMUM 13

/*---------------------------------------------------------------------------*/
extern VipT1HistoAnalysis *VipReadT1HistoAnalysis(char *name);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipWriteT1HistoAnalysis(VipT1HistoAnalysis *ana, char *name);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipComputeSSSingularityGammaExtrema( SSSingularity *s, float gamma, Singularity *sval, int nval, float dscale );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSCascade *VipCreateHighestCascade(SSSingularity *slist, int nbiter, int ninfinite );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSCascade *VipComputeScaleOrderedCascadeList( SSSingularity *list,int itermax, VipHisto *hcumul );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSCascade *VipComputeOrderedCascadeList( SSSingularity *list,int itermax, VipHisto *hcumul );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSCascade *VipDetectCascadeFromD2( SSSingularity *list,SSSingularity *s);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSCascade *VipDetectCascadeFromD1( SSSingularity *list,SSSingularity *s);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipCountSingularitiesStillAlive( SSSingularity *slist, int *n, int scalemax );
/*---------------------------------------------------------------------------*/
 /*---------------------------------------------------------------------------*/
  extern Volume *VipComputeHistoScaleSpaceByDiffusion(
						      VipHisto *histo,
						      int scalemax,
						      float dt);
  /*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
  extern Volume *VipComputeScaleSpaceFirstDeriv(
						Volume *vol);
  /*---------------------------------------------------------------------------*/
  /*---------------------------------------------------------------------------*/
  extern Volume *VipComputeScaleSpaceSecondDeriv(
						 Volume *vol);
  /*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern SSSingularity *VipComputeSSSingularityList(
Vip1DScaleSpaceStruct *ssstruct,
int scalemin,
int trackD0,
int trackD1,
int trackD2,
int trackD3,
int trackD4
);
/*---------------------------------------------------------------------------*/
  /*NB in fact borderwidth should be zero for a sure tracking*/

/*---------------------------------------------------------------------------*/
extern Vip1DScaleSpaceStruct *VipCompute1DScaleSpaceStructFromHisto( 
  VipHisto *shorthisto,
  int scalemax,
  float dscale,
  int offset,
  int nderivative,
  int undersampling_factor);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipFindSSSingularityMates( SSSingularity *slist );
/*---------------------------------------------------------------------------*/  

/*---------------------------------------------------------------------------*/
Volume *VipDetectExtremaFrom1DSS( Volume *ss);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipConvertScaleSpaceToSplotFormat( 
Volume *vol,
int xmin,
int xmax,
int scalemin,
int scalemax,
char *filename);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipAnalyseCascadesIRSequence(VipT1HistoAnalysis *ana,
SSCascade *clist, Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipExpandSSObjectSingularities (SSObject *o, Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipMarkNHighestCascades ( SSCascade *clist, int n,  Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipMarkNBiggestCascades ( SSCascade *clist, int n,  Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
  int VipMarkAnalysedObjectsHisto( VipT1HistoAnalysis *ana, Vip1DScaleSpaceStruct *volstruct );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipMarkAnalysedObjects( VipT1HistoAnalysis *ana, Vip1DScaleSpaceStruct *volstruct );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipCreateGnuplotFileFromExtrema(
  Vip1DScaleSpaceStruct *volstruct,
  char *name,
  int tracked,
  int psfile,
  int title,
  int writeD0,
  int writeD1,
  int writeD2,
  int writeD3,
  int writeD4
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipFree1DScaleSpaceStruct( Vip1DScaleSpaceStruct *ssstruct);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipFreeSSSingularityList( SSSingularity *ss);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipFreeCascadeList( SSCascade *c);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern VipT1HistoAnalysis *VipAnalyseCascades(SSCascade *clist, Vip1DScaleSpaceStruct *volstruct, int sequence);
/*---------------------------------------------------------------------------*/

extern int  VipAnalyseCascadesModes(VipT1HistoAnalysis *ana,
                                    SSCascade *clist,
                                    Vip1DScaleSpaceStruct *volstructana, 
                                    int sequence);

/*---------------------------------------------------------------------------*/
extern int VipAnalyseCascadesSegmentedSequence(VipT1HistoAnalysis *ana,
                                        SSCascade *clist, Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipAnalyseCascadesSPGRSequence(VipT1HistoAnalysis *ana,
SSCascade *clist, SSCascade *cbrain, Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSCascade *VipGetInsideRangeCascade( SSCascade *clist, int left, int right, int scalemax);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern SSCascade *VipGetInsideRangeBiggestCascade( SSCascade *clist, int left, int right, int scalemax);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern SSSingularity *VipGetNextSingularityOnRight( SSSingularity *s, int nature );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern SSObject *VipAllocSSObject();
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipDetectSSExtrema(
double *input,
Vip_S8BIT *extrema,
int dim,
int *numberplus,
int *numbermoins
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Vip1DScaleSpaceStruct *VipCompute1DScaleSpaceStructUntilLastCascade( 
  VipHisto *shorthisto,
  float dscale,
  int offset,
  int nderivative,
  int undersampling_factor);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipMarkSSObjectExtrema( SSObject *o, Vip1DScaleSpaceStruct *volstruct);
/*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
extern int VipPutSSExtremaValAndSpeedInBuffer(double *data,double *denom, double *num,Vip_S8BIT *eptr,int dim,Singularity *val,int *nval,int sizeval,int iter);
/*-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipComputeSSSingularitySpeedMinima( SSSingularity *s, Singularity *sval, int nval, float dscale );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipComputeSSSingularitySimpleSpeedMinima( SSSingularity *s );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipComputeSSSingularityGaussianLikeSpeed( SSSingularity *s, Singularity *sval, int nval, float dscale, float sigmamin, int nature);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipComputeAnalysedLoc (SSObject *o, int scale);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern VipT1HistoAnalysis *VipGetT1HistoAnalysis(
Volume *vol);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
VipT1HistoAnalysis *VipGetT1HistoAnalysisCustomized(
Volume *vol,
int track,
float dscale);
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* VIP_HISTO_SS_H */
