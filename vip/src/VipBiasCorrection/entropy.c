/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_brain.c          * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 29/12/1998
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : correction des non uniformite des images T1
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


#include <vip/volume.h>
#include <vip/util.h>
#include <vip/math.h>								
#include <vip/connex.h>	
#include <vip/histo.h>	
#include <vip/splineresamp.h>

#define ICM 1
#define ANNEALING 2

#define BORDERLEVEL -124

#define F2D_REGULARIZED_FIELD 22
#define F3D_REGULARIZED_FIELD 33

#define MEAN_QUADRATIC_VARIATION 321
#define STANDARD_DEVIATION 432
#define GEOMETRY 543

#define GREY_VAL 128
#define WHITE_VAL 255
#define ANY_VAL -123

/*---------------------------------------------------------------*/
static int ZeroAverageEgalZeroField(Volume *field, Volume *average);
/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/
static int ApplyMirrorToFieldBorder(Volume *vol);
/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/
static float ComputeInitialAverageOffset(Volume *field, Volume *average);
/*---------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
Volume *ResampleInitField(Volume *field,
			  int xfieldsize,
			  int yfieldsize,
			  int zfieldsize,
			  float xvoxsize,
			  float yvoxsize,
			  float zvoxsize
			  );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
Volume *VipComputeBiasFieldOneGrid(int mode, int dumb, Volume *vol, float undersampling, float Kentropy,
                                   float Kregularization, float Koffset, float amplitude,
float Tinit, float geom,int fieldtype, int nb_sample_proba, float increment,
				   Volume *init_field, float *delta_entropy, float RegulZTuning);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------*/
static Volume *ComputeAverageField( Volume *vol, Volume *mask, int masklevek, Volume *field, int xundersampling, int yundersampling, int zundersampling);
/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/
static Volume *ComputeSquareField( Volume *vol, Volume *mask, int mask_level, Volume *field, int xundersampling, int yundersampling, int zundersampling, double *totalaverage, double *totalsquare, double *initsd, int *volsize);
/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/
static int InitializeField(Volume *field, RandomBox *rbox, float factormax); 
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
static int UpdateHistoFromField( Volume *vol, VipHisto *hcurrent,
				 Volume *field, int xundersampling, int yundersampling, int zundersampling);
/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/
static float ComputeDeltaEntropy(VipHisto *hcurrent, VipHisto *hmodif, VipHisto *htouched,
				 VipIntBucket *touchlist, Volume *vol, VipOffsetStruct *vos, Vip_S16BIT *vol_first, int volsize,
				 int x, int y, int z,
				 int xundersampling, int yundersampling, int zundersampling,
				 float oldfield, float newfield,int fieldtype);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
static float ComputeInitialRegularization(Volume *vol, VipConnectivityStruct *vcs,
float * RegulDirTuning);
/*---------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
Volume *VipComputeCompressedVolume(Volume *vol, int compression)
/*---------------------------------------------------------------------------*/
{
    Volume *new;
    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_S16BIT *ptr, *ptrnew;
    int POWER_compm1;
    int i;


    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
	{
	    VipPrintfExit("(VipComputeCompressedVolume");
	    return(PB);
	}
    if(compression<0 || compression>15)
	{
	    VipPrintfError("Bas compression factor (2^n)");
	    VipPrintfExit("(VipComputeCompressedVolume");
	    return(PB);
	}

    new = VipDuplicateVolumeStructure( vol, "compressed");
    if(!new)
	{
	    VipPrintfExit("(VipComputeCompressedVolume");
	    return(PB);
	}
    if(VipAllocateVolumeData(new)==PB)
	{
	    VipPrintfExit("(VipComputeCompressedVolume");
	    return(PB);
	}

   vos = VipGetOffsetStructure(vol);
   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
   ptrnew = VipGetDataPtr_S16BIT( new ) + vos->oFirstPoint;

   POWER_compm1=1;
   for(i=1;i<=(compression-1);i++) POWER_compm1*=2;
   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
	   if(*ptr<=0) *ptrnew = 0;
	   else
	     *ptrnew = (*ptr+POWER_compm1) >> compression;
	   ptr++;
	   ptrnew++;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
   }
   
   return(new);
}


/*---------------------------------------------------------------------------*/
int VipComputeUnbiasedVolume(Volume *field, Volume *ima)
/*---------------------------------------------------------------------------*/
{
    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_S16BIT *ptr;
    Vip_FLOAT *fptr;

    if (VipVerifyAll(field)==PB || VipTestType(field,VFLOAT)==PB)
	{
	    VipPrintfExit("(VipComputeUnbiasedVolume");
	    return(PB);
	}
    if (VipVerifyAll(ima)==PB || VipTestType(ima,S16BIT)==PB)
	{
	    VipPrintfExit("(VipComputeUnbiasedVolume");
	    return(PB);
	}
    if(VipTestEqual3DSize(field,ima)==PB || VipTestEqualBorderWidth(field,ima)==PB)
	{
	    VipPrintfExit("(VipComputeUnbiasedVolume");
	    return(PB);
	}

   vos = VipGetOffsetStructure(ima);
   ptr = VipGetDataPtr_S16BIT( ima ) + vos->oFirstPoint;
   fptr = VipGetDataPtr_VFLOAT( field ) + vos->oFirstPoint;

   (void)printf("Slice:   ");

   for ( iz = mVipVolSizeZ(ima); iz-- ; )               /* loop on slices */
   {
       (void)printf("\b\b\b%3d",mVipVolSizeZ(ima)-iz);
       fflush(stdout);
      for ( iy = mVipVolSizeY(ima); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(ima); ix-- ; )/* loop on points */
         {
	     if(*ptr!=0)
		 *ptr = (int)(mVipMin(*ptr * *fptr, 32500.));
	     ptr++;
	     fptr++;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
         fptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      fptr += vos->oPointBetweenLine;  /*skip border points*/
   }
   printf("\n");

   return(OK);
}

/*---------------------------------------------------------------------------*/
Volume *VipResampleField(Volume *field, Volume *ima)
/*---------------------------------------------------------------------------*/
{
    Volume *rfield=NULL;
    VipDeplacement *dep;
    Vip3DPoint_VFLOAT t;
    int b;

    if (VipVerifyAll(field)==PB || VipTestType(field,VFLOAT)==PB)
	{
	    VipPrintfExit("(ResampleField");
	    return(PB);
	}
    if (VipVerifyAll(ima)==PB || VipTestType(ima,S16BIT)==PB)
	{
	    VipPrintfExit("(ResampleField");
	    return(PB);
	}

    t.x = -(mVipVolVoxSizeX(field)+mVipVolVoxSizeX(ima))/2.;
    t.y = -(mVipVolVoxSizeY(field)+mVipVolVoxSizeY(ima))/2.;
    t.z = -(mVipVolVoxSizeZ(field)+mVipVolVoxSizeZ(ima))/2.;

    dep = VipInitialiseDeplacement(&t);

    VipSetBorderLevel(field,1);
    if(ApplyMirrorToFieldBorder(field)==PB) return(PB);

    b = mVipVolBorderWidth(field);
    mVipVolSizeX(field)=mVipVolSizeX(field)+2*b;
    mVipVolSizeY(field)=mVipVolSizeY(field)+2*b;
    mVipVolSizeZ(field)=mVipVolSizeZ(field)+2*b;
    mVipVolBorderWidth(field)=0;

    printf("Resampling field using cubic spline\n");
      rfield = VipSplineResamp(field,ima,dep,3); 

    return(rfield);
}

    

/*---------------------------------------------------------------------------*/
Volume *ResampleInitField(Volume *field,
			  int xfieldsize,
			  int yfieldsize,
			  int zfieldsize,
			  float xvoxsize,
			  float yvoxsize,
			  float zvoxsize
				)
/*---------------------------------------------------------------------------*/
{
    Volume *rfield=NULL;
    VipDeplacement *dep;
    Vip3DPoint_VFLOAT t;
    int b;
    Volume *newVolume;

    /*compute a fild with new image spatial resolution,
      for final minimization with more degrees fo freedom*/

    if (VipVerifyAll(field)==PB || VipTestType(field,VFLOAT)==PB)
	{
	    VipPrintfExit("(VipFirstResampleField");
	    return(PB);
	}
  
    t.x = -(mVipVolVoxSizeX(field)+xvoxsize)/2.;
    t.y = -(mVipVolVoxSizeY(field)+yvoxsize)/2.;
    t.z = -(mVipVolVoxSizeZ(field)+zvoxsize)/2.;
    rfield = VipDeclare3DVolumeStructure(xfieldsize,
					yfieldsize,
					zfieldsize,
					xvoxsize,
					yvoxsize,
					zvoxsize,
					VFLOAT,
					"oversampled bias field",
					0);

    dep = VipInitialiseDeplacement(&t);

    VipSetBorderLevel(field,1);
    if(ApplyMirrorToFieldBorder(field)==PB) return(PB); /*spline border*/

    b = mVipVolBorderWidth(field);
    mVipVolSizeX(field)=mVipVolSizeX(field)+2*b;
    mVipVolSizeY(field)=mVipVolSizeY(field)+2*b;
    mVipVolSizeZ(field)=mVipVolSizeZ(field)+2*b;
    mVipVolBorderWidth(field)=0;

    printf("Down one level of grid...\n");
    rfield = VipSplineResamp(field,rfield,dep,1);


    newVolume = VipDuplicateVolumeStructure ( rfield,
					      "Down" );
    newVolume->borderWidth = 1;
    VipTransferVolumeData ( rfield, newVolume );
    VipFreeVolume(rfield);
    rfield = newVolume;
 
    VipSetBorderLevel(rfield,0);

    return(rfield);
}

/*---------------------------------------------------------------------------*/
Volume *VipComputeBiasFieldMultiGrid(int mode, int dumb, Volume *vol, float undersampling, 
				     float Kentropy,
				     float Kregularization, float Koffset,
				     float amplitude,
				     float Tinit, float geom,int fieldtype,
				     int nb_sample_proba, float increment,
				     int ngrid, float RegulZTuning)
/*---------------------------------------------------------------------------*/
{
  Volume *field=NULL;
  float delta_entropy;
  static int first_level = VTRUE;

  if((RegulZTuning>3)||(RegulZTuning<0.1))
    {
      VipPrintfError("Regul tuning inside Z direction out of legal range [0.1,3]");
    }

  while(ngrid--)
    {
      field = VipComputeBiasFieldOneGrid(mode, dumb, vol, undersampling, Kentropy,
					  Kregularization, Koffset, amplitude, 
					  Tinit, geom, fieldtype, nb_sample_proba, 
					  increment,
					  field, &delta_entropy, RegulZTuning);
      if(field==NULL)
	{
	  VipPrintfExit("VipComputeBiasFieldMultiGrid");
	  return(PB);
	}
      printf("Entropy decrease: %f percents\n",-delta_entropy*100);
      /*just relax regularization to get more, the bias seems high or absent*/
      /*      if (first_level == VTRUE && -delta_entropy <0.001)
	{
	  Kregularization /=2.;
	  VipPrintfInfo("Less than 0.1% of entropy gain, I divide regularization by 2 (high bias?)");
	  printf("New Kregularization: %f\n",Kregularization);
	  VipFreeVolume(field);
	  field = NULL;
	  ngrid++;
	  }
	  else
      */
      if (first_level == VTRUE)
	{
	  if(-delta_entropy >0.1)
	    {
	      /*just more regularization to calm down, in case of low contrast I am losing info*/
	      /*Kregularization *=2.;*/
	      VipPrintfWarning("More than 10% of entropy gain, maybe you should increase regularization (low contrast?)");
	      /*
		printf("New Kregularization: %f\n",Kregularization);
		VipFreeVolume(field);
		field = NULL;
		ngrid++;
	      */
	    }     
	}
      undersampling /= 2.;
      increment = 1.02;
      nb_sample_proba = 1;
      Kregularization /= 4; /*x4 facets but 2 step to get the same square variation*/
      if(undersampling < 2*mVipVolVoxSizeX(vol))
	undersampling=2*mVipVolVoxSizeX(vol)+0.00001;
      if(undersampling < 2*mVipVolVoxSizeY(vol))
	undersampling=2*mVipVolVoxSizeY(vol)+0.00001;
      if(undersampling < 2*mVipVolVoxSizeZ(vol))
	undersampling=2*mVipVolVoxSizeZ(vol)+0.00001;
      first_level = VFALSE;
    }
  return(field);

}
/*---------------------------------------------------------------------------*/
Volume *VipComputeBiasFieldOneGrid(int mode, int dumb, Volume *vol, float undersampling, float Kentropy,
 float Kregularization, float Koffset, float amplitude,
float Tinit, float geom,int fieldtype, int nb_sample_proba, float increment,
Volume *init_field, float *delta_entropy, float RegulZTuning)
/*---------------------------------------------------------------------------*/
{
  static float entropy_weight;
  static int level=1;
  Volume *field=NULL, *average=NULL, *square=NULL;
  Volume *averageG=NULL, *squareG=NULL;
  Volume *averageW=NULL, *squareW=NULL;
  Volume *both=NULL;
  RandomBox *rbox, *rboxptr;
  VipHisto *histo, *hcurrent, **hmodif, **htouched;
  VipIntBucket **touchlist=NULL;
  int i, j, ix, iy, iz;
  Vip_S16BIT *vol_first=NULL;
  Vip_FLOAT *field_first=NULL, *field_ptr=NULL, *field_neighbor=NULL;
  Vip_FLOAT *average_first=NULL, *average_ptr=NULL;
  Vip_FLOAT *square_first=NULL, *square_ptr=NULL;
  Vip_FLOAT *averageG_first=NULL, *averageG_ptr=NULL;
  Vip_FLOAT *squareG_first=NULL, *squareG_ptr=NULL;
  Vip_FLOAT *averageW_first=NULL, *averageW_ptr=NULL;
  Vip_FLOAT *squareW_first=NULL, *squareW_ptr=NULL;
  float m2;
  double total_square, total_average;
  double new_total_square;
  double total_squareG, total_averageG, init_diff_average=0. , new_diff_average=0.;
  int diff_sign;
  double new_total_squareG, new_total_averageG;
  double total_squareW, total_averageW;
  double vW, vG, sW, sG, newsW, newsG; 
  double new_total_squareW, new_total_averageW;
  double init_sd, init_sdG, init_sdW;
  float newfield;
  float entropy, *deltaentropy;
  double *deltaU=NULL, U=0., *deltaR=NULL, R=0., *deltaO=NULL, O=0., *deltaO2=NULL, O2=0., signed_SQRTO=0.;
  double OG = 0., *deltaOG=NULL;
  int n, nmodif, nb_iterations;
  VipOffsetStruct *vos, *field_vos;
  int volsize, volsizeG, volsizeW;
  VipConnectivityStruct *vcs=NULL;
  int icon;
  double diff;
  int fieldsize;
  double T;
  int algo=ANNEALING;
  int compteur;
  int xfieldsize=0;
  int yfieldsize=0;
  int zfieldsize=0;
  int xundersampling=0;
  int yundersampling=0;
  int zundersampling=0;
  /*FILE *f=NULL;*/
  int besti;
  double *proba_tab,locZ,probasum;
  float *inc;
  float inc_stop=0.01;
  float modif_stop=0.01;
  int nfield_nonzero = 1;
  float RegulDirTuning[6];

  /*	
  f=fopen("entropy.txt","w");
  if(!f)
    {
      VipPrintfError("Can not open entropy.txt");
      VipPrintfExit("VipComputeBiasField");
    }
  */
  
  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  if(undersampling<1 || undersampling>1000)
    {
      VipPrintfError("Unconsistent undersampling ratio");
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  if(nb_sample_proba<1 || nb_sample_proba>100)
    {
      VipPrintfError("Choose the number of increment between 1 and 100 please");
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  if(increment<1.001 || increment>100)
    {
      VipPrintfError("Choose the multiplicative increment between 1.001 and 100 please");
      printf("%f!!!\n",increment);
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  if(Tinit<0 || Tinit>100000)
    {
      VipPrintfError("Unconsistent Initial temperature");
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  if(geom<0.01 || geom>=1)
    {
      VipPrintfError("Unconsistent geometric reason for annealing");
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
   

  if ( fieldtype==F2D_REGULARIZED_FIELD ) 
    { 
      if(mVipVolSizeX(vol)<=undersampling/mVipVolVoxSizeX(vol))
	{
	  xfieldsize = 1;
	  xundersampling = mVipVolSizeX(vol);
	}
      else
	{
	  xundersampling = (int)(undersampling/mVipVolVoxSizeX(vol));
	  xfieldsize = mVipVolSizeX(vol)/xundersampling+1;
	}
      if(mVipVolSizeY(vol)<=undersampling/mVipVolVoxSizeY(vol))  
	{
	  yfieldsize = 1;
	  yundersampling = mVipVolSizeY(vol);
	}
      else    
	{
	  yundersampling = (int)(undersampling/mVipVolVoxSizeY(vol));
	  yfieldsize = mVipVolSizeY(vol)/yundersampling+1;
	}
      zfieldsize = mVipVolSizeZ(vol);
      zundersampling = 1; /*!!!do not remove*/
    }
	
  if ( fieldtype==F3D_REGULARIZED_FIELD ) 
    {
     if(mVipVolSizeX(vol)<=undersampling/mVipVolVoxSizeX(vol))
	{
	  xfieldsize = 1;
	  xundersampling = mVipVolSizeX(vol);
	}
      else
	{	  
	  xundersampling = (int)(undersampling/mVipVolVoxSizeX(vol));
	  xfieldsize = mVipVolSizeX(vol)/xundersampling+1;
	}
      if(mVipVolSizeY(vol)<=undersampling/mVipVolVoxSizeY(vol))  
	{
	  yfieldsize = 1;
	  yundersampling = mVipVolSizeY(vol);
	}
      else    
	{
	  yundersampling = (int)(undersampling/mVipVolVoxSizeY(vol));
	  yfieldsize = mVipVolSizeY(vol)/yundersampling+1;
	}
      if(mVipVolSizeZ(vol)<=undersampling/mVipVolVoxSizeZ(vol))  
	{
	  zfieldsize = 1;
	  zundersampling = mVipVolSizeZ(vol);
	}
      else    
	{
	  
	  
	  zundersampling = (int)(undersampling/mVipVolVoxSizeZ(vol));
	  zfieldsize = mVipVolSizeZ(vol)/zundersampling+1;
	}
    }
  if (init_field==NULL)
    {
      field = VipCreate3DVolume(xfieldsize,
				yfieldsize,
				zfieldsize,
				mVipVolVoxSizeX(vol)*xundersampling,
				mVipVolVoxSizeY(vol)*yundersampling,
				mVipVolVoxSizeZ(vol)*zundersampling,
				VFLOAT,
				"bias field",
				1);
       if(field==PB) return(PB);
   }
  printf("Field sampling: %dx%dx%d blocks of %dx%dx%d voxels\n",
	 xfieldsize,yfieldsize,zfieldsize,xundersampling,yundersampling,zundersampling);
	      
			      

  rbox = VipCreateRandomBoxCircularList( 10101, 2);
  if(rbox==PB)	
    {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }

  if (mode==GEOMETRY) 
    {
      both = VipReadVolume("both");
      if (!both) return(PB);
    }
 
  histo = VipComputeVolumeHisto(vol); 
  volsize = mVipVolSizeX(vol)*mVipVolSizeY(vol)*mVipVolSizeZ(vol)-mVipHistoVal(histo,0);
  printf("Number of points for computation: %d/%d\n",volsize,mVipVolSizeX(vol)*mVipVolSizeY(vol)*mVipVolSizeZ(vol));
  VipSetHistoVal(histo,0,0);
  VipGetHistoEntropy(histo,&entropy);
  *delta_entropy = -entropy;
  if (init_field==NULL)
    printf("Entropy (in the range) before field correction: %f\n",entropy);
  /*else
    printf("Recall: Entropy (in the range) before any field correction: %f\n",entropy);*/

  if(histo==PB) 
    {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  hcurrent = VipCreateHisto(0,
			    mVipHistoRangeMax(histo)*10);
  if(hcurrent==PB) 
    {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }

  for(i=0;i<=mVipHistoRangeMax(hcurrent);i++)
    {
      VipSetHistoVal(hcurrent,i,0);
    }
  for(i=0;i<=mVipHistoRangeMax(histo);i++)
    {
      VipSetHistoVal(hcurrent,i,mVipHistoVal(histo,i));
    }

  hmodif = (VipHisto **)VipCalloc(2* nb_sample_proba+1,sizeof(VipHisto *),"VipComputeBiasField");
  if(!hmodif)
    {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  htouched = (VipHisto **)VipCalloc(2* nb_sample_proba+1,sizeof(VipHisto *),"VipComputeBiasField");
  if(!htouched)
   {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    } 
  touchlist = (VipIntBucket **)VipCalloc(2* nb_sample_proba+1,sizeof(VipIntBucket *),"VipComputeBiasField");
  if(!touchlist)
   {
      VipPrintfExit("(VipComputeBiasField"); 
      return(PB);
   } 

  for(i=1;i<2* nb_sample_proba+1;i++)
    {
      hmodif[i] = VipCreateHisto(0,
				 mVipHistoRangeMax(histo)*20);
      if(hmodif[i]==PB)	
	{
	  VipPrintfExit("(VipComputeBiasField");
	  return(PB);
	}
      htouched[i] = VipCreateHisto(0,
				   mVipHistoRangeMax(histo)*20);
      if(htouched[i]==PB)
	{
	  VipPrintfExit("(VipComputeBiasField");
	  return(PB);
	}

      touchlist[i] = VipAllocIntBucket(mVipHistoRangeMax(histo)*20);
      if(touchlist[i]==PB)
	{
	  VipPrintfExit("(VipComputeBiasField");
	  return(PB);
	}
      touchlist[i]->n_points = 0; 

      for(j=0;j<=mVipHistoRangeMax(hcurrent);j++)
	{
	  VipSetHistoVal(hmodif[i],j,0);
	  VipSetHistoVal(htouched[i],j,VFALSE);
	}
    }

  VipSetHistoVal(hcurrent,0,0);
  VipFreeHisto(histo);

  if (init_field==NULL) InitializeField(field,rbox,amplitude);
  else
    {
      field = ResampleInitField(init_field,xfieldsize,
				yfieldsize,
				zfieldsize,
				mVipVolVoxSizeX(vol)*xundersampling,
				mVipVolVoxSizeY(vol)*yundersampling,
				mVipVolVoxSizeZ(vol)*zundersampling);
      if(field==PB) return(PB);
    
      VipFreeVolume(init_field);
    }
  field_vos = VipGetOffsetStructure(field);
  field_first = VipGetDataPtr_VFLOAT( field ) + field_vos->oFirstPoint;

  if(mVipVolBorderWidth(field)==0)
    {
      VipPrintfError("This oversampled field should have non zero border width");
      VipPrintfExit("VipComputeBiasField");
    }
  VipSetBorderLevel(field, BORDERLEVEL);

  average = ComputeAverageField(vol,vol,ANY_VAL,field,xundersampling,yundersampling,zundersampling);
  if(average==PB) return(PB);
  average_first = VipGetDataPtr_VFLOAT( average ) + field_vos->oFirstPoint;

  ZeroAverageEgalZeroField(field, average);

  square = ComputeSquareField(vol,vol,ANY_VAL,field,xundersampling,yundersampling,zundersampling, &total_average, &total_square, &init_sd, &volsize);  
  if(square==PB) return(PB);

  square_first = VipGetDataPtr_VFLOAT( square ) + field_vos->oFirstPoint;
    


  average_ptr = average_first;
  nfield_nonzero = 0;

  for ( iz = 0; iz < mVipVolSizeZ(field); iz++ )                /* loop on slices */
    {
      for ( iy = 0; iy < mVipVolSizeY(field); iy++ )             /* loop on lines */
	{
	  for ( ix = 0; ix < mVipVolSizeX(field); ix++ )          /* loop on points */
	    {
	      if(*average_ptr>0.1) nfield_nonzero++;
	      average_ptr++;
	    }
	  average_ptr += field_vos->oPointBetweenLine; 
	}
      average_ptr += field_vos->oLineBetweenSlice;
    }
  
  if (init_field==NULL) entropy_weight=nfield_nonzero; /*keep the same at all level (static)*/

  UpdateHistoFromField(vol,hcurrent,field,xundersampling,yundersampling,zundersampling);

  VipGetHistoEntropy(hcurrent,&entropy);
  fieldsize = mVipVolSizeX(field)*mVipVolSizeY(field)*mVipVolSizeZ(field);
  Kentropy *= entropy_weight;
  U = Kentropy*entropy;


  if ( fieldtype == F2D_REGULARIZED_FIELD ) { vcs = VipGetConnectivityStruct( field, CONNECTIVITY_4 );}
  if ( fieldtype == F3D_REGULARIZED_FIELD ) { vcs = VipGetConnectivityStruct( field, CONNECTIVITY_6 );}
    
  for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
    {
      if (abs(vcs->offset[icon])<(mVipVolSizeX(field)+3)) RegulDirTuning[icon] = 1.;
      else  RegulDirTuning[icon] = RegulZTuning;
    }

  R = Kregularization*ComputeInitialRegularization(field,vcs,RegulDirTuning);
  U += R;
    
  /*Where the begining of the anti compression occurs...
 For some stupid historical reasons, the name of the variables in this area
do not correspond exactly to what they are:
average is a volume of same size as field, embedding the sum of intensities for each cube,
square is a volume of same size as field, embedding the sum of squared intensities for each cube,
m2 is the arithmetic average of a cube for the new configuration,
signed_SQRT denotes the sum of differences between new and old intensities,
total_square denotes the sum of square differences with average, including field (without final averaging, sorry for the mess)
total_average denotes the average including field (with averaging) */

  if(mode==GEOMETRY)
    {
      
      averageG = ComputeAverageField(vol,both,GREY_VAL,field,xundersampling,yundersampling,zundersampling);
      if(averageG==PB) return(PB);
      averageG_first = VipGetDataPtr_VFLOAT( averageG ) + field_vos->oFirstPoint;

      squareG = ComputeSquareField(vol,both,GREY_VAL,field,xundersampling,yundersampling,zundersampling, &total_averageG, &total_squareG, &init_sdG, &volsizeG);  
      if(squareG==PB) return(PB);

      squareG_first = VipGetDataPtr_VFLOAT( squareG ) + field_vos->oFirstPoint;
      
      averageW = ComputeAverageField(vol,both,WHITE_VAL,field,xundersampling,yundersampling,zundersampling);
      if(averageW==PB) return(PB);
      averageW_first = VipGetDataPtr_VFLOAT( averageW ) + field_vos->oFirstPoint;

      squareW = ComputeSquareField(vol,both,WHITE_VAL,field,xundersampling,yundersampling,zundersampling, &total_averageW, &total_squareW, &init_sdW, &volsizeW);  
      if(squareW==PB) return(PB);

      squareW_first = VipGetDataPtr_VFLOAT( squareW ) + field_vos->oFirstPoint;
      if (volsizeG<=1 || volsizeW<=1)
        {
          VipPrintfError("Empty geometric class!");
          return(PB);
        }
    }

  O=0.;
  O2=0.;
  OG=0.;
  Koffset *= entropy_weight;

  if (mode==MEAN_QUADRATIC_VARIATION || mode==STANDARD_DEVIATION || mode==GEOMETRY)
    {
      signed_SQRTO = ComputeInitialAverageOffset(field,average);
      O = signed_SQRTO / volsize ;
      O *= O;
      O *= Koffset;
      U += O;
    }
  if (mode==STANDARD_DEVIATION)
    {
      init_sd += 0.;
      O2 = init_sd - sqrt(total_square / (volsize-1));
      O2 *= O2;
      O2 *= Koffset;
      U += O2;
    }
  if (mode==GEOMETRY)
    {
      init_diff_average = total_averageW - total_averageG;
      vW = total_squareW / (volsizeW-1);
      vG = total_squareG / (volsizeG-1);
      sW = sqrt(vW);
      sG = sqrt(vG);
      OG = sW*sW*0.01;
      OG *= Koffset;
      U += OG;
    }
	
  vos = VipGetOffsetStructure(vol);
  vol_first = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  rboxptr = rbox;


  T = Tinit;
  nmodif = -1;
  nb_iterations = 500000;
  rboxptr = rbox;
  n=0;
  compteur = 5;
     
  if(dumb==VFALSE)
    {
      printf("iteration %d/%d, modif: %d/%d, entropy: %f, \n", n,nb_iterations,nmodif,nfield_nonzero,entropy);
      printf("Uentropy: %f, Uregul: %f, Uoffset: %f, Energy: %f, inc:%f\n",entropy*Kentropy,R,O+O2+OG,U,increment);
    }
  else
    {
      printf("field increment: about %f\n",increment);
      printf("field modif: %6d/%6d",nmodif,nfield_nonzero);
    } 

  proba_tab = (double *)calloc(2*nb_sample_proba+2 , sizeof(double));
  deltaR = (double *)calloc(2*nb_sample_proba+2 , sizeof(double));
  deltaU = (double *)calloc(2*nb_sample_proba+2 , sizeof(double));
  deltaO = (double *)calloc(2*nb_sample_proba+2 , sizeof(double));
  deltaO2 = (double *)calloc(2*nb_sample_proba+2 , sizeof(double));
  deltaOG = (double *)calloc(2*nb_sample_proba+2 , sizeof(double));
  deltaentropy = (float *)calloc(2*nb_sample_proba+2 , sizeof(float));
  inc = (float *)calloc(2*nb_sample_proba+2 , sizeof(float));

  if(!proba_tab || !deltaR || !deltaU || !deltaO || !deltaO2 || !deltaOG || !deltaentropy || !inc )
    {
      VipPrintfError("out of memory");
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }

  if(init_field==NULL)
    {
      algo=ANNEALING;
      inc_stop = 0.02;
      modif_stop = 0.02;
    }
  else 
    {
      T=0.0000001;
      algo=ICM;
      inc_stop = 0.003;
      modif_stop = 0.01;
      compteur=1;
    }
  increment -= 1; /*Just for clearer interface*/
  nmodif = -1;
  for(n=1;(n<=nb_iterations)&&(increment>=inc_stop);n++)
    {
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b%6d/%6d",nmodif,nfield_nonzero);
      fflush(stdout);
      if ((((float)nmodif/(float)nfield_nonzero)<modif_stop ) && nmodif!=-1)
	{
	  compteur--;
	  if (compteur==0)
	    {
	      increment /= 2;
	      compteur = 1;
	      if((n<=nb_iterations)&&(increment>=inc_stop))
		{
		  printf("\nfield increment: about %f\n",1+increment);
		  printf("field modif: %6d/%6d",nmodif,nfield_nonzero);
		}
	    }
	}
      if(algo==ANNEALING)
	{
	  T *= geom;
	  if(dumb==VFALSE) printf("Temperature: %f\n",T);
	}
      if(T<0.000001)
	{
	  algo=ICM;
	  inc_stop = 0.005;
	  modif_stop = 0.01;
	  nb_sample_proba = 1;
	  compteur=1;
	}
      nmodif = 0;
      field_ptr = field_first;
      average_ptr = average_first;

      if (mode==STANDARD_DEVIATION) square_ptr = square_first;
      if (mode==GEOMETRY)
        {
          averageG_ptr = averageG_first;
          averageW_ptr = averageW_first;
          squareG_ptr = squareG_first;
          squareW_ptr = squareW_first;
        }
      for ( iz = 0; iz < mVipVolSizeZ(field); iz++ )                /* loop on slices */
	{
	  for ( iy = 0; iy < mVipVolSizeY(field); iy++ )             /* loop on lines */
	    {
	      for ( ix = 0; ix < mVipVolSizeX(field); ix++ )          /* loop on points */
		{
		  if(((rboxptr->label==1)&&((*average_ptr>0.1 && rboxptr->proba>0.25)||rboxptr->proba>0.9)))
		    {
		      for (i = 0; i < nb_sample_proba ; i++) /* 1=inc 2=1/inc 3=inc^2 5=1/inc^2 ...*/
			{
			  inc[2*i+1] = 1.;
			  for(j=0;j<=i;j++)
			    inc[2*i+1] *=  (1+increment*(rboxptr->proba+0.5));
			  inc[2*i+2] = 1.;
			  for(j=0;j<=i;j++)
			    inc[2*i+2] /=  (1+increment*(rboxptr->proba+0.5));
			}
		      rboxptr = rboxptr->next;

		      deltaU[0] = 0.;
		      for (i = 1; i <= 2* nb_sample_proba; i++)
			{ 
			  newfield = *field_ptr * inc[i];
							     							
			  deltaU[i] = 0.;
			  deltaR[i] = 0.;
			  deltaO[i] = 0.;
			  deltaO2[i] = 0.;
			  deltaOG[i] = 0.;

			  deltaentropy[i] = ComputeDeltaEntropy(hcurrent,hmodif[i],htouched[i],touchlist[i],
							     vol, vos, vol_first, volsize, 
								ix,iy,iz,xundersampling,yundersampling,zundersampling,
							     *field_ptr,newfield,fieldtype);
			 
			  deltaU[i] += Kentropy*deltaentropy[i];

			  for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
			    {
			      field_neighbor = field_ptr + vcs->offset[icon];
			      if(*field_neighbor>0)
				{
				  diff = log((double)(*field_neighbor/(*field_ptr)));
				  deltaR[i] -= diff * diff * RegulDirTuning[icon];
				  diff = log((double)(*field_neighbor/newfield));
				  deltaR[i] += diff * diff* RegulDirTuning[icon];
				}
			    }
			  deltaR[i] *= Kregularization;
			  deltaU[i] += deltaR[i];
                          
                          if (mode==MEAN_QUADRATIC_VARIATION || mode==STANDARD_DEVIATION || mode==GEOMETRY)
                            {
                              diff = signed_SQRTO - *field_ptr * *average_ptr
                                + newfield * *average_ptr;
                              diff /= volsize;
                              diff *= diff;
                              deltaO[i] = Koffset*diff - O;
                              deltaU[i] += deltaO[i] ;
                            }
                          if (mode==STANDARD_DEVIATION)
                            {                             
                              diff =  (newfield - *field_ptr)* *average_ptr;
                              m2 = total_average + diff / volsize;
                              diff = diff*diff/volsize - 2*m2*diff;
                              diff += (newfield*newfield - *field_ptr * *field_ptr) * *square_ptr;
                              new_total_square = total_square + diff;
                              diff = init_sd - sqrt(new_total_square/(volsize-1));
                              diff *= diff;
                              deltaO2[i] = (Koffset*diff - O2);
                              deltaU[i] += deltaO2[i];
                            }
                          if (mode==GEOMETRY)
                          {
                            diff =  (newfield - *field_ptr)* *averageG_ptr;
                            new_total_averageG = total_averageG + diff / volsizeG;
                            diff = diff*diff/volsizeG - 2*new_total_averageG*diff;
                            diff += (newfield*newfield - *field_ptr * *field_ptr) * *squareG_ptr;
                            new_total_squareG = total_squareG + diff;
                            newsG = sqrt(new_total_squareG/(volsizeG-1));
                            
                            diff =  (newfield - *field_ptr)* *averageW_ptr;
                            new_total_averageW  = total_averageW + diff / volsizeW;
                            diff = diff*diff/volsizeW - 2*new_total_averageW*diff;
                            diff += (newfield*newfield - *field_ptr * *field_ptr) * *squareW_ptr;
                            new_total_squareW = total_squareW + diff;
                            newsW = sqrt(new_total_squareW/(volsizeW-1));
                            
                            new_diff_average = new_total_averageW-new_total_averageG;
                            if (new_diff_average>init_diff_average) diff_sign = -1;
                            else diff_sign = 1;

                            deltaOG[i] = Koffset*newsW*newsW*0.01 - OG;
                            deltaU[i] += deltaOG[i];
                          }


			}
		      if(algo==ICM)
			{
			  besti=0;
			  for (i = 1; i<= 2*nb_sample_proba; i++)
			    {
			      if (deltaU[i]<deltaU[besti]) besti=i;			    
			    }	
			}
		      else /*annealing*/
			{
			  locZ=1.; /*no modif*/
			  proba_tab[0] = 1.;
			  for (i = 1; i<= 2*nb_sample_proba; i++)
			    {
			      if((-deltaU[i]/T)<-70) proba_tab[i] = exp(-70.);
			      else if ((-deltaU[i]/T)>70) proba_tab[i] = exp(70.);
			      else proba_tab[i]=  exp(-deltaU[i]/T);
			      locZ += proba_tab[i];
			    }
			  for (i = 0; i<= 2*nb_sample_proba; i++)
			    proba_tab[i] /= locZ;

			  probasum = proba_tab[0];
			  besti = 0;
			  while(rboxptr->proba>=probasum && (besti<=2*nb_sample_proba))
			    {
			      besti++;
			      probasum += proba_tab[besti];
			    }
	      
			}
		      if(besti!=0)
			{
                          if (mode==MEAN_QUADRATIC_VARIATION || mode==STANDARD_DEVIATION || mode==GEOMETRY)
                            {
                              signed_SQRTO += *average_ptr * *field_ptr * (inc[besti]-1);
                             
                            }
                          if (mode==STANDARD_DEVIATION)
                            {
                              diff =  *field_ptr * (inc[besti]-1)* *average_ptr;
                              total_average +=  diff / volsize;
                              diff = diff*diff/volsize - 2*total_average*diff;
                              diff += (*field_ptr * *field_ptr) * (inc[besti]*inc[besti]-1) * *square_ptr;
                              total_square = total_square + diff;                              
                            }
                          if (mode==GEOMETRY)
                            {
                              diff =  *field_ptr * (inc[besti]-1)* *averageG_ptr;
                              total_averageG +=  diff / volsizeG;
                              diff = diff*diff/volsizeG - 2*total_averageG*diff;
                              diff += (*field_ptr * *field_ptr) * (inc[besti]*inc[besti]-1) * *squareG_ptr;
                              total_squareG = total_squareG + diff;       
                              diff =  *field_ptr * (inc[besti]-1)* *averageW_ptr;
                              total_averageW +=  diff / volsizeW;
                              diff = diff*diff/volsizeW - 2*total_averageW*diff;
                              diff += (*field_ptr * *field_ptr) * (inc[besti]*inc[besti]-1) * *squareW_ptr;
                              total_squareW = total_squareW + diff;       
                            }
			  *field_ptr = *field_ptr * inc[besti];
			  entropy += deltaentropy[besti];
			  U += deltaU[besti];
			  R += deltaR[besti];
			  O += deltaO[besti];
			  O2 += deltaO2[besti];
			  OG += deltaOG[besti];

			  for(j=0;j<touchlist[besti]->n_points;j++)
			    {
			      mVipHistoVal(hcurrent,touchlist[besti]->data[j]) +=
				mVipHistoVal(hmodif[besti],touchlist[besti]->data[j]);
			    }
			  if(*average_ptr>0.1) nmodif++; /*to discard points outside head*/
			}
		    
		      for (i = 1; i<= 2*nb_sample_proba; i++)
			for(j=0;j<touchlist[i]->n_points;j++)
			  {
			    mVipHistoVal(hmodif[i],touchlist[i]->data[j]) = 0;
			    mVipHistoVal(htouched[i],touchlist[i]->data[j]) = VFALSE;
			  }
		      
		    }
		  field_ptr++;
		  average_ptr++;
		  if (mode==STANDARD_DEVIATION) square_ptr++;
                  if (mode==GEOMETRY)
                    {
                      averageG_ptr++;
                      averageW_ptr++;
                      squareG_ptr++;
                      squareW_ptr++;
                    }
		  rboxptr = rboxptr->next;
		}
	      field_ptr += field_vos->oPointBetweenLine;  /*skip border points*/
	      average_ptr += field_vos->oPointBetweenLine; 
	      if (mode==STANDARD_DEVIATION) square_ptr += field_vos->oPointBetweenLine;
              if (mode==GEOMETRY)
                {
                  averageG_ptr += field_vos->oPointBetweenLine;
                  averageW_ptr += field_vos->oPointBetweenLine;
                  squareG_ptr += field_vos->oPointBetweenLine;
                  squareW_ptr += field_vos->oPointBetweenLine;
                } 
	    }
	  field_ptr += field_vos->oLineBetweenSlice; /*skip border lines*/
	  average_ptr += field_vos->oLineBetweenSlice;
	  if (mode==STANDARD_DEVIATION) square_ptr += field_vos->oLineBetweenSlice;
          if (mode==GEOMETRY)
            {
              averageG_ptr += field_vos->oLineBetweenSlice;
              averageW_ptr += field_vos->oLineBetweenSlice;
              squareG_ptr += field_vos->oLineBetweenSlice;
              squareW_ptr += field_vos->oLineBetweenSlice;
            } 
	}
      if(dumb==VFALSE)   
	{
	  printf("\nMultiplicative increments:\n");
	  for (i = 1; i <= 2* nb_sample_proba ; i++) printf("%f ",inc[i]);
	  printf("\n");
	}
      if(dumb==VFALSE)
	{
	  printf("--------------------------------\n");
	  printf("iteration %d.%d, modif: %d/%d, entropy: %f\n", level,n,nmodif,nfield_nonzero,entropy);
	  printf("Uentropy: %f, Uregul: %f, Uoffset: %f, Energy: %f, inc:%f\n",entropy*Kentropy,R,O+O2+OG,U,increment);
          if (mode==STANDARD_DEVIATION) printf("average: %f, standard deviation: %f\n", (float)total_average, (float)sqrt(total_square/(volsize-1)));
          if (mode==GEOMETRY)
            {
              printf("Grey average: %f, standard deviation: %f\n", (float)total_averageG, (float)sqrt(total_squareG/(volsizeG-1)));
              printf("White average: %f, standard deviation: %f\n", (float)total_averageW, (float)sqrt(total_squareW/(volsizeW-1)));

            }
	}
            
    }
  printf("\n");
  /*
  fclose(f);
  */
  /*
    VipWriteHisto( hcurrent,"histoc",WRITE_HISTO_ASCII);
  */

  if (square)
    {
      VipFreeVolume(square);
      square = ComputeSquareField(vol,vol,ANY_VAL,field,xundersampling,yundersampling,zundersampling, &total_average, &total_square, &init_sd, &volsize);
    }
  if (square) VipFreeVolume(square);
  if (squareG)
    {
      VipFreeVolume(squareG);
      squareG = ComputeSquareField(vol,both,GREY_VAL,field,xundersampling,yundersampling,zundersampling, &total_averageG, &total_squareG, &init_sdG, &volsizeG); 
    }
  if (squareG) VipFreeVolume(squareG);
  
  if (squareW)
    {
      VipFreeVolume(squareW);
      squareW = ComputeSquareField(vol,both,WHITE_VAL,field,xundersampling,yundersampling,zundersampling, &total_averageW, &total_squareW, &init_sdW, &volsizeW); 
    }
  if (squareW) VipFreeVolume(squareW);

  VipGetHistoEntropy(hcurrent,&entropy);
  printf("Final entropy: %f\n",entropy);
  *delta_entropy += entropy;
  *delta_entropy /= entropy;

  VipFreeHisto(hcurrent);
  for (i = 1; i<= 2*nb_sample_proba; i++)
    {
      VipFreeHisto(hmodif[i]);
      VipFreeHisto(htouched[i]);
      VipFreeIntBucket(touchlist[i]);
    }
  VipFreeRandomBox(rbox);
  if (average) VipFreeVolume(average);

  free(proba_tab);
  free(deltaR);
  free(deltaU);
  free(deltaO);
  free(deltaO2);
  free(deltaOG);
  free(deltaentropy);
  free(inc);

  /*
  fclose(f);
  */
  level++;
  return(field);
}



/*---------------------------------------------------------------*/
static int ZeroAverageEgalZeroField(Volume *field, Volume *average)
/*---------------------------------------------------------------*/
{
    Vip_FLOAT *ptr, *aptr;
    int ix, iy, iz;
    int xsize, ysize, zsize;
    VipOffsetStruct *vos;

    vos = VipGetOffsetStructure(field);
    ptr = VipGetDataPtr_VFLOAT( field ) + vos->oFirstPoint;
    aptr = VipGetDataPtr_VFLOAT( average ) + vos->oFirstPoint;
    xsize = mVipVolSizeX(field);
    ysize = mVipVolSizeY(field);
    zsize = mVipVolSizeZ(field);

    for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
	{
	    for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
		{
		    for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
			{
                          if (*aptr<0.1) *ptr = 1.;
                          ptr++;
                          aptr++;
			}
		    ptr += vos->oPointBetweenLine;  
		    aptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	    ptr += vos->oLineBetweenSlice; /*skip border lines*/
	    aptr += vos->oLineBetweenSlice;
	}

    return(OK);
}


/*---------------------------------------------------------------*/
static float ComputeInitialAverageOffset(Volume *field, Volume *average)
/*---------------------------------------------------------------*/
{
    double diff;
    Vip_FLOAT *ptr, *aptr;
    int ix, iy, iz;
    int xsize, ysize, zsize;
    VipOffsetStruct *vos;

    vos = VipGetOffsetStructure(field);
    ptr = VipGetDataPtr_VFLOAT( field ) + vos->oFirstPoint;
    aptr = VipGetDataPtr_VFLOAT( average ) + vos->oFirstPoint;
    xsize = mVipVolSizeX(field);
    ysize = mVipVolSizeY(field);
    zsize = mVipVolSizeZ(field);

    diff=0.;
    for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
	{
	    for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
		{
		    for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
			{
			    diff += (*ptr - 1) * *aptr;
			    ptr++;
			    aptr++;
			}
		    ptr += vos->oPointBetweenLine;  
		    aptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	    ptr += vos->oLineBetweenSlice; /*skip border lines*/
	    aptr += vos->oLineBetweenSlice;
	}

    return((float)diff);
}

/*---------------------------------------------------------------*/
static float ComputeInitialRegularization(Volume *vol, VipConnectivityStruct *vcs,float * RegulDirTuning)
/*---------------------------------------------------------------*/
{
    int icon;
    double diff;
    Vip_FLOAT *field_neighbor, *ptr;
    double R;
    int ix, iy, iz;
    int xsize, ysize, zsize;
    VipOffsetStruct *vos;

    R = 0.;

    vos = VipGetOffsetStructure(vol);
    ptr = VipGetDataPtr_VFLOAT( vol ) + vos->oFirstPoint;
    xsize = mVipVolSizeX(vol);
    ysize = mVipVolSizeY(vol);
    zsize = mVipVolSizeZ(vol);

    for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
	{
	    for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
		{
		    for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
			{
			    for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
				{
				    field_neighbor = ptr + vcs->offset[icon];
				    if(*field_neighbor>0)
					{
					    diff = log((double)(*field_neighbor/(*ptr)));
					    R += diff * diff * RegulDirTuning[icon]/2;
					    /* /2 because each interface counts two times*/
					}
				}
			    ptr++;
			}
		    ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	    ptr += vos->oLineBetweenSlice; /*skip border lines*/
	}

    return((float)R);
    
}

/*---------------------------------------------------------------*/
static float ComputeDeltaEntropy(VipHisto *hcurrent, VipHisto *hmodif, VipHisto *htouched,
				 VipIntBucket *touchlist, Volume *vol, VipOffsetStruct *vos, Vip_S16BIT *vol_first, int volsize,
				 int x, int y, int z,
				 int xundersampling, int yundersampling, int zundersampling,
				 float oldfield, float newfield,int fieldtype)
/*---------------------------------------------------------------*/
{
  int ix, iy, iz;
  int Ix=0, Iy=0, Iz=0;
  int Mx=0, My=0, Mz=0;
  Vip_S16BIT *ptr;
  int oldval, newval;
  int i, g;
  double deltaE;
  int newni, oldni;
  double oldpi, newpi;

  if ( fieldtype==F2D_REGULARIZED_FIELD ) 
    {
      Ix = x * xundersampling;
      Iy = y * yundersampling;
      Iz = z;
      Mx = Ix + xundersampling;
      My = Iy + yundersampling;
      Mz = Iz + 1;
      Mx = mVipMin(Mx,mVipVolSizeX(vol));
      My = mVipMin(My,mVipVolSizeY(vol));
      Mz = mVipMin(Mz,mVipVolSizeZ(vol));
    }
	
  else if ( fieldtype==F3D_REGULARIZED_FIELD )
    {
      Ix = x * xundersampling;
      Iy = y * yundersampling;
      Iz = z * zundersampling;
      Mx = Ix + xundersampling;
      My = Iy + yundersampling;
      Mz = Iz + zundersampling;
      Mx = mVipMin(Mx,mVipVolSizeX(vol));
      My = mVipMin(My,mVipVolSizeY(vol));
      Mz = mVipMin(Mz,mVipVolSizeZ(vol));
    }
  else
    VipPrintfError("Unknown field type");

  touchlist->n_points = 0; 
  for ( iz = Iz; iz < Mz; iz++ )                /* loop on slices */
    {
      for ( iy = Iy; iy < My; iy++ )             /* loop on lines */
	{
	  for ( ix = Ix; ix < Mx; ix++ )          /* loop on points */
	    {
	      ptr = vol_first + ix + iy * vos->oLine + iz*vos->oSlice;
	      if(!(*ptr)) continue;
	      oldval = (int)(*ptr * oldfield);
	      newval = (int)(*ptr * newfield);
	      if(oldval!=newval)
		{
		  if(mVipHistoVal(htouched,oldval)==VFALSE)
		    {
		      touchlist->data[touchlist->n_points++]=oldval;
		      mVipHistoVal(htouched,oldval)=VTRUE;
		    }
		  mVipHistoVal(hmodif,oldval)-=1;
		  if(mVipHistoVal(htouched,newval)==VFALSE)
		    {
		      touchlist->data[touchlist->n_points++]=newval;
		      mVipHistoVal(htouched,newval)=VTRUE;
		    }
		  mVipHistoVal(hmodif,newval)+=1;
		}
			    
	    } 
	}
    }
  deltaE = 0.;
  for(i=0;i<touchlist->n_points;i++)
    {
      g = touchlist->data[i];
      oldni = mVipHistoVal(hcurrent,g);
      newni = mVipHistoVal(hcurrent,g) + mVipHistoVal(hmodif,g);
      if(oldni!=newni) 
	{

	  oldpi = (double)oldni/(double)volsize;
	  newpi = (double)newni/(double)volsize;
	  if(oldni==0) oldpi=0.00000000001;
	  if(newni==0) newpi=0.00000000001;
	  deltaE += (float)( oldpi * log(oldpi) - newpi * log(newpi));
		     
	  if(deltaE<-100)
	    printf("g: %d, %d->%d %f\n",g,oldni,newni,(float)( oldpi * log(oldpi) - newpi * log(newpi)));
		    
		    
	}
    }
  return((float)deltaE/log(2.));
}

/*---------------------------------------------------------------*/
static int UpdateHistoFromField( Volume *vol, VipHisto *hcurrent,
Volume *field, int xundersampling, int yundersampling, int zundersampling)
/*---------------------------------------------------------------*/
{
  VipOffsetStruct *vos, *field_vos;
  int i, ix, iy, iz;
  int xsize, ysize, zsize;
  Vip_S16BIT *ptr;
  Vip_FLOAT *field_first, *field_ptr=NULL;
  int val;

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  field_vos = VipGetOffsetStructure(field);
  field_first = VipGetDataPtr_VFLOAT( field ) + field_vos->oFirstPoint;
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);

  for (i=mVipHistoRangeMin(hcurrent);i<mVipHistoRangeMax(hcurrent);i++)
    {
      mVipHistoVal(hcurrent,i)=0;
    }

  for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
    {
      for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
	{
	  for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
	    {
	      if(*ptr)
		{
		  field_ptr = field_first +  (int)(ix/xundersampling)
		    + (int)(iy/yundersampling)*field_vos->oLine
		    + (int)(iz/zundersampling)*field_vos->oSlice;
		  val = (int)(*ptr * *field_ptr);
		  mVipHistoVal(hcurrent,val)++;
		}
	      ptr++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
  return(OK);
}


/*---------------------------------------------------------------*/
static Volume *ComputeAverageField( Volume *vol, Volume *mask, int mask_level, Volume *field, int xundersampling, int yundersampling, int zundersampling)
/*---------------------------------------------------------------*/
{

  Volume *average=NULL;
  VipOffsetStruct *vos, *field_vos;
  int ix, iy, iz;
  int xsize, ysize, zsize;
  Vip_S16BIT *ptr, *maskptr;
  Vip_FLOAT *average_first, *average_ptr=NULL;
  int offset;


  average = VipCopyVolume(field,"average");
  if(average==PB)
    {
      VipPrintfExit("ComputeAverageField");
      return(PB);
    }

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  maskptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;
  field_vos = VipGetOffsetStructure(average);
  average_first = VipGetDataPtr_VFLOAT( average ) + field_vos->oFirstPoint;
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);

  VipSetVolumeLevel_VFLOAT(average,0.);

  for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
    {
      for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
	{
	  for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
	    {
              if (mask_level==ANY_VAL || *maskptr==mask_level)
                {
                  offset= (int)(ix/xundersampling)
                    + (int)(iy/yundersampling)*field_vos->oLine
                    + (int)(iz/zundersampling)*field_vos->oSlice;
                  average_ptr = average_first + offset;
                  *average_ptr+=*ptr;
                }
              maskptr++;
	      ptr++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	  maskptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      maskptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

  return(average);
}


/*---------------------------------------------------------------*/
static Volume *ComputeSquareField( Volume *vol, Volume *mask, int mask_level, Volume *field, int xundersampling, int yundersampling, int zundersampling, double *totalaverage, double *totalsquare, double *initsd, int *volsize)
/*---------------------------------------------------------------*/
{

  Volume *square=NULL;
  VipOffsetStruct *vos, *field_vos;
  int ix, iy, iz;
  int xsize, ysize, zsize;
  Vip_S16BIT *ptr, *maskptr;
  Vip_FLOAT *square_first, *square_ptr=NULL;
  int n;
  double average, variance;
  double newaverage, newvariance, temp;
  Vip_FLOAT *field_first, *field_ptr=NULL;
  int offset;


  square = VipCopyVolume(field,"square");
  if(square==PB)
    {
      VipPrintfExit("ComputeSquareField");
      return(PB);
    }

  vos = VipGetOffsetStructure(vol);
  field_vos = VipGetOffsetStructure(square);
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);

  VipSetVolumeLevel_VFLOAT(square,0.);

  average = 0.;
  newaverage = 0.;
  n = 0;
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  maskptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;
  field_first = VipGetDataPtr_VFLOAT( field ) + field_vos->oFirstPoint;

  for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
    {
      for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
	{
	  for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
	    {
	      if(*ptr && (mask_level==ANY_VAL || *maskptr==mask_level))
		{
                  offset = (int)(ix/xundersampling)
		    + (int)(iy/yundersampling)*field_vos->oLine
		    + (int)(iz/zundersampling)*field_vos->oSlice;
		  average +=*ptr;
                  field_ptr = field_first + offset;
                  newaverage += *field_ptr * *ptr;
                  n++;
		}
	      ptr++;
	      maskptr++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	  maskptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      maskptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

  if(n==0 || n==1)
    {
      VipPrintfError("Empty image\n");
      return(PB);
    }
  average /= n;
  newaverage /= n;

  variance = 0.;
  newvariance = 0.;
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  maskptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;

  square_first = VipGetDataPtr_VFLOAT( square ) + field_vos->oFirstPoint;
  field_first = VipGetDataPtr_VFLOAT( field ) + field_vos->oFirstPoint;

  for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
    {
      for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
	{
	  for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
	    {
	      if(*ptr && (mask_level==ANY_VAL || *maskptr==mask_level))
		{
                  variance += (*ptr-average)*(*ptr-average);

                  offset = (int)(ix/xundersampling)
		    + (int)(iy/yundersampling)*field_vos->oLine
		    + (int)(iz/zundersampling)*field_vos->oSlice;
		  square_ptr = square_first + offset;
		  *square_ptr += *ptr * *ptr;

                  field_ptr = field_first + offset;
                  temp = (*field_ptr * *ptr)-newaverage;
                  newvariance += temp*temp;
		}
	      ptr++;
	      maskptr++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	  maskptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      maskptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

  printf("Range initial average: %f, Initial standard deviation: %f (%d points)\n", average, (float)sqrt(variance/(n-1)),n);
  printf("With         field   : %f,                           : %f\n", newaverage, (float)sqrt(newvariance/(n-1)));
  fflush(stdout);

  *totalsquare = newvariance; 
  *totalaverage = newaverage;
  *initsd = (float)sqrt(variance/(n-1));
  *volsize = n;
  return(square);
}


/*---------------------------------------------------------------*/
static int InitializeField(Volume *vol, RandomBox *rbox, float factormax)
{
    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_FLOAT *ptr;

    if(factormax<0.1 || factormax>10)
	{
	    VipPrintfError("Too big factor max in InitializeField");
	    VipPrintfExit("InitializeField");
	    return(PB);
	}

    vos = VipGetOffsetStructure(vol);
    ptr = VipGetDataPtr_VFLOAT( vol ) + vos->oFirstPoint;

    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	{
	    for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
		{
		    for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
			{	

                          if(rbox->label==0)
                            *ptr = 1. + rbox->proba * (factormax-1);
                          else
                            *ptr = 1 - rbox->proba * (1 - 1. / factormax);	
                            
                          ptr++;
                          rbox = rbox->next;
			}
		    ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	    ptr += vos->oLineBetweenSlice; /*skip border lines*/
	}
    return(OK);

}
 
/*---------------------------------------------------------------*/



/*---------------------------------------------------------------*/
static int ApplyMirrorToFieldBorder(Volume *vol)
{
    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_FLOAT *ptr;
    int i;

    if(!vol || mVipVolBorderWidth(vol)!=1)
	{
	    VipPrintfError("No or Strange bias field");
	    VipPrintfExit("ApplyMirrorToFieldBorder");
	    return(PB);
	}

    vos = VipGetOffsetStructure(vol);
    ptr = VipGetDataPtr_VFLOAT( vol );

    /* first slice*/
    for(i=0;i<vos->oSlice;i++)
      {
	*ptr = *(ptr+vos->oSlice);
	ptr++;
      }
    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	{  
	  /*first line*/
	  for(i=0;i<vos->oLine;i++)
	    {
	      *ptr = *(ptr+vos->oLine);
	      ptr++;
	    }
	    for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
		{
		  *ptr = *(ptr+1);
		  ptr++;
		  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		    {			    
		      ptr++;
		    }
		  *ptr = *(ptr-1);
		  ptr++;
		}
	  /*last line*/
	  for(i=0;i<vos->oLine;i++)
	    {
	      *ptr = *(ptr-vos->oLine);
	      ptr++;
	    }
	}  
    /*last slice*/
    for(i=0;i<vos->oSlice;i++)
      {
	*ptr = *(ptr-vos->oSlice);
	ptr++;
      }

    return(OK);

}
 
/*---------------------------------------------------------------*/


