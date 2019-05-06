/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_histo_ss.c       * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/04/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : compute histo scale space 
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

#include <vip/histo.h>
#include <vip/histo_static.h>

/*---------------------------------------------------------------------------*/
static void DiffusionIteration(
double *input,
double *output,
double dt,
int dim
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void SecondDeriv(
double *input,
double *output,
int dim
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void FirstDeriv(
double *input,
double *output,
int dim
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipConvertScaleSpaceToSplotFormat( 
Volume *vol,
int xmin,
int xmax,
int scalemin,
int scalemax,
char *filename)
/*---------------------------------------------------------------------------*/

{
    char *error;
    FILE *f;
    Vip_DOUBLE *ptr;
    VipOffsetStruct *vos;
    int ix, iy;
    int xsize, ysize;

    f = fopen(filename,"w");
    if(!f)
    {
        error = VipMalloc( strlen( filename ) + 100, "VipConvertScaleSpaceToSplotFormat" );
        sprintf(error,"Can not open %s for writing",filename);
        VipPrintfError(error);
        VipPrintfExit("VipConvertScaleSpaceToSplotFormat");
        VipFree(error);
        return(PB);
    }

    vos = VipGetOffsetStructure(vol);

    ptr = VipGetDataPtr_VDOUBLE(vol) + vos->oFirstPoint;

    xsize = mVipVolSizeX(vol);
    ysize = mVipVolSizeY(vol);


    for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
    {
        for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
        {
            if((ix>=xmin)&&(ix<=xmax)&&(iy>=scalemin)&&(iy<=scalemax)&&(iy%10==0)&&(ix%2==0))
                fprintf(f,"%f\n",(float)*ptr);
            ptr++;
        }
        if((iy>=scalemin)&&(iy<=scalemax)&&(iy%10==0))   fprintf(f,"\n");

        ptr += vos->oPointBetweenLine;  /*skip border points*/
    }

    fclose(f);
    return(OK);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
Volume *VipComputeScaleSpaceFirstDeriv(
Volume *vol)
/*---------------------------------------------------------------------------*/
{
  Volume *deriv;
  Vip_DOUBLE *ptrin, *ptrout;
  VipOffsetStruct *vos;
  int ix, iy;
  int xsize, ysize;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,VDOUBLE)==PB)
    {
      VipPrintfExit("(histo)VipComputeScaleSpaceDeriv");
      return(PB);
    }

  deriv = VipDuplicateVolumeStructure(vol,"deriv");
  if(deriv==PB) return(PB);

  if(VipAllocateVolumeData(deriv)==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  ptrin = VipGetDataPtr_VDOUBLE(vol) + vos->oFirstPoint;
  ptrout = VipGetDataPtr_VDOUBLE(deriv) + vos->oFirstPoint;

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);


  for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
    {
      ptrin++;
      *ptrout++=0;
      for ( ix = 2; ix < xsize; ix++ )          /* loop on points */
	{
	  *ptrout++ = 0.5 * (*(ptrin+1) - *(ptrin-1));
	  ptrin++;
	}
      ptrin++;
      *ptrout++=0;
      ptrin += vos->oPointBetweenLine;  /*skip border points*/
      ptrout += vos->oPointBetweenLine;  /*skip border points*/
    }
 
  return(deriv);
}

/*---------------------------------------------------------------------------*/
Volume *VipComputeScaleSpaceSecondDeriv(
Volume *vol)
/*---------------------------------------------------------------------------*/
{
  Volume *deriv;
  Vip_DOUBLE *ptrin, *ptrout;
  VipOffsetStruct *vos;
  int ix, iy;
  int xsize, ysize;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,VDOUBLE)==PB)
    {
      VipPrintfExit("(histo)VipComputeScaleSpaceDeriv");
      return(PB);
    }

  deriv = VipDuplicateVolumeStructure(vol,"deriv2");
  if(deriv==PB) return(PB);

  if(VipAllocateVolumeData(deriv)==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  ptrin = VipGetDataPtr_VDOUBLE(vol) + vos->oFirstPoint;
  ptrout = VipGetDataPtr_VDOUBLE(deriv) + vos->oFirstPoint;

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);


  for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
    {
      ptrin++;
      *ptrout++=0;
      for ( ix = 2; ix < xsize; ix++ )          /* loop on points */
	{
	  *ptrout++ = *(ptrin+1) + *(ptrin-1) - 2 * *ptrin;
	  ptrin++;
	}
      ptrin++;
      *ptrout++=0;
      ptrin += vos->oPointBetweenLine;  /*skip border points*/
      ptrout += vos->oPointBetweenLine;  /*skip border points*/
    }
 
  return(deriv);
}


/*---------------------------------------------------------------------------*/
Volume *VipComputeHistoScaleSpaceByDiffusion(
VipHisto *histo,
int scalemax,
float dt)
/*---------------------------------------------------------------------------*/
{
  Volume *ss=NULL;
  int xsize, ysize;
  int scale;
  Vip_DOUBLE *input, *inputptr;
  Vip_DOUBLE *output;
  int offsetline;
  int i;

  if(histo==NULL)
    {
      VipPrintfError("NULL pointer in VipComputeHistoScaleSpaceByDiffusion!");
      VipPrintfExit("VipComputeHistoScaleSpaceByDiffusion");
      return(PB);
    }
  if(scalemax<=0)
    {
      VipPrintfError("negative maximum scale!");
      VipPrintfExit("VipComputeHistoScaleSpaceByDiffusion");
      return(PB);
    }
  xsize = mVipHistoRangeMax(histo) - mVipHistoRangeMin(histo) +1;
  ysize = scalemax+1;
  ss = VipCreate3DVolume(xsize,ysize,1,1.,1.,1.,VDOUBLE,"scalespace",0);
  if(ss==PB) return(PB);



  offsetline = VipOffsetLine(ss);
  input = (Vip_DOUBLE *)VipGetDataPtr_VDOUBLE(ss); /* 0 borderwidth*/
  inputptr = input;
  for(i=0;i<xsize;i++) *inputptr++ = (double)(histo->histo_ad[i]);
  output = input + offsetline;  

  for(scale=1;scale<=scalemax;scale++)
    {            
      DiffusionIteration(input,output,dt,xsize);
      input += offsetline;
      output += offsetline;
    }

  return(ss);
}





/*---------------------------------------------------------------------------*/
static void DiffusionIteration(
double *input,
double *output,
double dt,
int dim
)
/*---------------------------------------------------------------------------*/
{
  int i;
  double *inputm1, *inputp1;


  inputm1 = input-1;
  inputp1 = input+1;

  *output++ = *input + dt*(-2 * *input + *inputp1);
  input++;
  inputp1++;
  inputm1++;

  dt /= 2.;
  for(i=2;i<dim;i++)
    {
        *output++ = *input + dt*(-2 * *input + *inputp1++ + *inputm1++);
	input++;
    }

  *output = *input + dt*(-2 * *input + *inputm1);

}

/*---------------------------------------------------------------------------*/
static void FirstDeriv(
double *input,
double *output,
int dim
)
/*---------------------------------------------------------------------------*/
{
  int i;
  double *inputm1, *inputp1;


  inputm1 = input-1;
  inputp1 = input+1;
  *output++ = 0;

  inputp1++;
  inputm1++;

  for(i=2;i<dim;i++)
    {
        *output++ = (*inputp1++ - *inputm1++)*0.5;
    }

  *output = 0;

}

/*---------------------------------------------------------------------------*/
static void SecondDeriv(
double *input,
double *output,
int dim
)
/*---------------------------------------------------------------------------*/
{
  int i;
  double *inputm1, *inputp1;

  inputm1 = input-1;
  inputp1 = input+1;

  *output++ = 0;
  input++;
  inputp1++;
  inputm1++;

  for(i=2;i<dim;i++)
    {
        *output++ = (-2 * *input + *inputp1++ + *inputm1++);
	input++;
    }

  *output = 0;

}


/*---------------------------------------------------------------------------*/
Vip1DScaleSpaceStruct *VipCompute1DScaleSpaceStructFromHisto( 
  VipHisto *shorthisto,
  int scalemax,
  float dscale,
  int offset,
  int nderivative,
  int undersampling_factor)
/*---------------------------------------------------------------------------*/
{
  Vip1DScaleSpaceStruct *monster;
  VipHisto *histo, *historesamp=NULL, *newhisto;
  int resampflag = VFALSE;
  int factor;

  if(shorthisto==NULL)
    {
      VipPrintfError("Null histo in VipCompute1DScaleSpaceStructFromHisto");
      VipPrintfExit("VipCompute1DScaleSpaceStructFromHisto");
      return(PB);
    }
  if((scalemax<=0)||(scalemax>100000))
    {
      VipPrintfError("Strange scalemax in VipCompute1DScaleSpaceStructFromHisto");
      VipPrintfExit("VipCompute1DScaleSpaceStructFromHisto");
      return(PB);
    }
  if((dscale<=0)||(dscale>0.5))
    {
      VipPrintfError("Strange dscale in VipCompute1DScaleSpaceStructFromHisto");
      VipPrintfExit("VipCompute1DScaleSpaceStructFromHisto");
      return(PB);
    }
  if((offset>10000)||(offset<0))
    {
      VipPrintfError("Strange offset in VipCompute1DScaleSpaceStructFromHisto");
      VipPrintfExit("VipCompute1DScaleSpaceStructFromHisto");
      return(PB);
    }
  if(nderivative<1 || nderivative>4)
   {
      VipPrintfError("nderivative should be between 1 and 4");
      VipPrintfExit("VipCompute1DScaleSpaceStructFromHisto");
      return(PB);
    }

  monster = (Vip1DScaleSpaceStruct *)VipCalloc(1,sizeof(Vip1DScaleSpaceStruct),"VipCompute1DScaleSpaceStructFromHisto");
  if(monster==PB) return(PB);

  monster->undersampling_ratio=undersampling_factor;

  resampflag = VTRUE;
  historesamp = VipGetPropUndersampledHisto(shorthisto, 95, &(monster->undersampling_ratio), &factor, 0, 100);
  if (factor>=1) printf("Undersampling histogram... (factor %d )\n",monster->undersampling_ratio);
  if(historesamp==PB) return(PB);
  shorthisto = historesamp;

  if( (mVipHistoRangeMax(shorthisto) - mVipHistoRangeMin(shorthisto) ) > 256 )
    VipCutRightFlatPartOfHisto(shorthisto,100);

  newhisto = VipGetRightExtendedHisto( shorthisto, 20);
  /*VipFreeHisto(shorthisto); this histogram is still outside, it is ugly*/
  shorthisto=newhisto;

  /*
  if((mVipHistoRangeMax(shorthisto)-mVipHistoRangeMin(shorthisto))>200)
    {
      resampflag = VTRUE;
      historesamp = VipGetUndersampledHisto(shorthisto,&(monster->undersampling_ratio));
      printf("Undersampling histogram... (factor %d)\n",monster->undersampling_ratio);
      if(historesamp==PB) return(PB);
      shorthisto = historesamp;
    }
  */

  monster->offset = offset;
  monster->dscale = dscale;

  monster->itermax = scalemax;
  monster->scalemax = (int)(monster->itermax*dscale+0.5);
 
  if(monster->offset!=0)
    {
      printf("Adding interval of length %d on histogram left...\n",monster->offset);
      histo = VipGetLeftExtendedHisto(shorthisto,monster->offset);
      if(histo==PB) return(PB);
    }
  else histo = shorthisto;

  monster->histo = histo;
  monster->hcumul = VipGetCumulHisto( histo);

  printf("Computing histogram scale space...\n");
  monster->ss = VipComputeHistoScaleSpaceByDiffusion(histo,monster->itermax,dscale);
  if (monster->ss==PB) return(PB);

  printf("Computing scale space extrema...\n");
  monster->ss_e = VipDetectExtremaFrom1DSS(monster->ss);
  if (monster->ss_e==PB) return(PB);

  printf("Computing histogram scale space first derivative...\n");
  monster->d1 = VipComputeScaleSpaceFirstDeriv(monster->ss);
  if (monster->d1==PB) return(PB);

  printf("Computing first derivative extrema...\n");
  monster->d1_e = VipDetectExtremaFrom1DSS(monster->d1);
  if (monster->d1_e==PB) return(PB);
  
  if(nderivative>2)
      {
	  printf("Computing histogram scale space third derivative...\n");
	  monster->d3 = VipComputeScaleSpaceSecondDeriv(monster->d1);
	  if (monster->d3==PB) return(PB);
      }

  VipFreeVolume(monster->d1);
  monster->d1 =NULL;

  if(nderivative>2)
      {
	  printf("Computing third derivative extrema...\n");
	  monster->d3_e = VipDetectExtremaFrom1DSS(monster->d3);
	  if (monster->d3_e==PB) return(PB);

	  VipFreeVolume(monster->d3);
	  monster->d3 =NULL;
      }

  if(nderivative>1)
      {
	  printf("Computing histogram scale space second derivative...\n");
	  monster->d2 = VipComputeScaleSpaceSecondDeriv(monster->ss);
	  if (monster->d2==PB) return(PB);
      }

  VipFreeVolume(monster->ss);
  monster->ss =NULL;

  if(nderivative>1)
      {
	  printf("Computing second derivative extrema...\n");
	  monster->d2_e = VipDetectExtremaFrom1DSS(monster->d2);
	  if (monster->d2_e==PB) return(PB);
      }

  if(nderivative>3)
      {
	  printf("Computing histogram scale space fourth derivative...\n");
	  monster->d4 = VipComputeScaleSpaceSecondDeriv(monster->d2);
	  if (monster->d4==PB) return(PB);
      }

  if(nderivative>1)
      {
	  VipFreeVolume(monster->d2);
	  monster->d2 =NULL;
      }

  if(nderivative>3)
      {
	  printf("Computing forth derivative extrema...\n");
	  monster->d4_e = VipDetectExtremaFrom1DSS(monster->d4);
	  if (monster->d4_e==PB) return(PB);

	  VipFreeVolume(monster->d4);
	  monster->d4 =NULL;
      }

  return(monster);
}

/*---------------------------------------------------------------------------*/
Vip1DScaleSpaceStruct *VipCompute1DScaleSpaceStructUntilLastCascade( 
  VipHisto *shorthisto,
  float dscale,
  int offset,
  int nderivative,
  int undersampling_factor)
/*---------------------------------------------------------------------------*/
{
  Vip1DScaleSpaceStruct *monster;
  VipHisto *histo, *historesamp=NULL, *newhisto;
  int resampflag = VFALSE;
  int iter, itermaxestim, iterinc;
  double *ss,*ssp1,*d1=NULL,*d2=NULL,*d3=NULL,*d4=NULL,*d5=NULL,*ssptr;
  int i;
  int dim;
  Vip_S8BIT *e_ssptr, *e_d1ptr, *e_d2ptr, *e_d3ptr=NULL, *e_d4ptr=NULL;
  int ne_ssp, ne_ssm, ne_d1p, ne_d1m, ne_d2p, ne_d2m, ne_d3p, ne_d3m, ne_d4p, ne_d4m;
  int rebours;
  int factor;

  /* to prevent warning linked to comments*/
  d3 = d4 = d5 = d1;
  ne_ssm = ne_ssp = 0;
  e_ssptr = NULL;

  if(shorthisto==NULL)
    {
      VipPrintfError("Null histo in VipCompute1DScaleSpaceStructUntilLastCascade");
      VipPrintfExit("VipCompute1DScaleSpaceStructUntilLastCascade");
      return(PB);
    }
  if((dscale<=0)||(dscale>0.5))
    {
      VipPrintfError("Strange dscale in VipCompute1DScaleSpaceStructUntilLastCascade");
      VipPrintfExit("VipCompute1DScaleSpaceStructUntilLastCascade");
      return(PB);
    }
  if((offset>10000)||(offset<0))
    {
      VipPrintfError("Strange offset in VipCompute1DScaleSpaceStructUntilLastCascade");
      VipPrintfExit("VipCompute1DScaleSpaceStructUntilLastCascade");
      return(PB);
    }
  if(nderivative<2 || nderivative>4)
   {
      VipPrintfError("nderivative should be between 2 and 4");
      VipPrintfExit("VipCompute1DScaleSpaceStructUntilLastCascade");
      return(PB);
    }

  monster = (Vip1DScaleSpaceStruct *)VipCalloc(1,sizeof(Vip1DScaleSpaceStruct),"VipCompute1DScaleSpaceStructUntilLastCascade");
  if(monster==PB) return(PB);

  monster->undersampling_ratio = undersampling_factor;

  resampflag = VTRUE;
  
  historesamp = VipGetPropUndersampledHisto(shorthisto, 95, &(monster->undersampling_ratio), &factor, 0, 100);
  if (factor>=1) printf("Undersampling histogram... (factor %d )\n",monster->undersampling_ratio);
  if(historesamp==PB) return(PB);
  shorthisto = historesamp;

  if( (mVipHistoRangeMax(shorthisto) - mVipHistoRangeMin(shorthisto) ) > 256 )
    VipCutRightFlatPartOfHisto(shorthisto,100);

  newhisto = VipGetRightExtendedHisto( shorthisto, 20);
  /*VipFreeHisto(shorthisto); this histogram is still outside, it is ugly*/
  shorthisto=newhisto;
  /*
  if((mVipHistoRangeMax(shorthisto)-mVipHistoRangeMin(shorthisto))>200)
    {
      resampflag = VTRUE;
      historesamp = VipGetUndersampledHisto(shorthisto,&(monster->undersampling_ratio));
      printf("Undersampling histogram... (factor %d)\n",monster->undersampling_ratio);
      if(historesamp==PB) return(PB);
      shorthisto = historesamp;
    }
  */
  monster->offset = offset;
  monster->dscale = dscale;

  if(monster->offset!=0)
    {
      printf("Adding interval of length %d on histogram left...\n",monster->offset);
      histo = VipGetLeftExtendedHisto(shorthisto,monster->offset);
      if(histo==PB) return(PB);
    }
  else histo = shorthisto;

  monster->histo = histo;
  monster->hcumul = VipGetCumulHisto( histo);

  dim = mVipHistoRangeMax(histo) - mVipHistoRangeMin(histo) +1;
  ss = VipCalloc(dim,sizeof(double),"VipCompute1DScaleSpaceStructUntilLastCascade");
  if(ss==PB) return(PB);
  ssp1 = VipCalloc(dim,sizeof(double),"VipCompute1DScaleSpaceStructUntilLastCascade");
  if(ssp1==PB) return(PB);
  d1 = VipCalloc(dim,sizeof(double),"VipCompute1DScaleSpaceStructUntilLastCascade");
  if(d1==PB) return(PB);
  d2 = VipCalloc(dim,sizeof(double),"VipCompute1DScaleSpaceStructUntilLastCascade");
  if(d2==PB) return(PB);
  /*Higher derivatives can be computed to derive extremum speed (bad) or Lindeberg
    gamma derivatives*/
  if(nderivative>2)
      {
	  d3 = VipCalloc(dim,sizeof(double),"VipCompute1DScaleSpaceStructUntilLastCascade");
	  if(d3==PB) return(PB);
      }
  if(nderivative>3)
      {
	  d4 = VipCalloc(dim,sizeof(double),"VipCompute1DScaleSpaceStructUntilLastCascade");
	  if(d4==PB) return(PB);
      }
  /*
  d5 = VipCalloc(dim,sizeof(double),"VipCompute1DScaleSpaceStructUntilLastCascade");
  if(d5==PB) return(PB);
  */
  itermaxestim = 25000;
  iterinc = 5000;

  /* in fact, we do not use 0-order extrema
  monster->ss_e = VipCreate3DVolume(dim,itermaxestim+1,1,1.,1.,1.,S8BIT,"ss_extrema",0);
  if (monster->ss_e==PB) return(PB);
  */
  /*
  monster->ss_val = VipMalloc(100000*sizeof(Singularity),"ss val buffer in VipCompute1DScaleSpaceStructUntilLastCascade");
  if(monster->ss_val==PB) return(PB);
  monster->sizessval = 100000;
  monster->nssval = 0;
  */
  monster->d1_e = VipCreate3DVolume(dim,itermaxestim+1,1,1.,1.,1.,S8BIT,"d1_extrema",0);
  if (monster->d1_e==PB) return(PB);
  /*
   monster->d1_val = VipMalloc(100000*sizeof(Singularity),"d1 val buffer in VipCompute1DScaleSpaceStructUntilLastCascade");
  if(monster->d1_val==PB) return(PB);
  monster->sized1val = 100000;
  monster->nd1val = 0;
  */
  monster->d2_e = VipCreate3DVolume(dim,itermaxestim+1,1,1.,1.,1.,S8BIT,"d2_extrema",0);
  if (monster->d2_e==PB) return(PB);
  /*
  monster->d2_val = VipMalloc(100000*sizeof(Singularity),"d2 val buffer in VipCompute1DScaleSpaceStructUntilLastCascade");
  if(monster->d2_val==PB) return(PB);
  monster->sized2val = 100000;
  monster->nd2val = 0;
  */

  if(nderivative>2)
      {
	  monster->d3_e = VipCreate3DVolume(dim,itermaxestim+1,1,1.,1.,1.,S8BIT,"d3_extrema",0);
	  if (monster->d3_e==PB) return(PB);
      }
  if(nderivative>3)
      {
	  monster->d4_e = VipCreate3DVolume(dim,itermaxestim+1,1,1.,1.,1.,S8BIT,"d4_extrema",0);
	  if (monster->d4_e==PB) return(PB);
      }



  /* e_ssptr = (Vip_S8BIT *)VipGetDataPtr_S8BIT(monster->ss_e); *//* 0 borderwidth*/
  e_d1ptr = (Vip_S8BIT *)VipGetDataPtr_S8BIT(monster->d1_e); /* 0 borderwidth*/
  e_d2ptr = (Vip_S8BIT *)VipGetDataPtr_S8BIT(monster->d2_e); /* 0 borderwidth*/
  if(nderivative>2)
      e_d3ptr = (Vip_S8BIT *)VipGetDataPtr_S8BIT(monster->d3_e); /* 0 borderwidth*/
  if(nderivative>3)
      e_d4ptr = (Vip_S8BIT *)VipGetDataPtr_S8BIT(monster->d4_e); /* 0 borderwidth*/

  ssptr = ssp1;
  for(i=mVipHistoRangeMin(histo);i<=mVipHistoRangeMax(histo);i++)
    *ssptr++ = (double)(mVipHistoVal(histo,i));
  FirstDeriv(ssp1,d1,dim);
  SecondDeriv(ssp1,d2,dim);
  
  if(nderivative>2) FirstDeriv(d2,d3,dim);
  if(nderivative>3) SecondDeriv(d2,d4,dim);
  /*
  FirstDeriv(d4,d5,dim);
  */
  /*
  if(VipDetectSSExtrema(ssp1,e_ssptr,dim,&ne_ssp,&ne_ssm)==PB) return(PB);
  */
  /*
  if(VipPutSSExtremaValInBuffer(ssp1,e_ssptr,dim,
				monster->ss_val,&(monster->nssval),monster->sizessval,0)==PB) return(PB);
				*/
  if(VipDetectSSExtrema(d1,e_d1ptr,dim,&ne_d1p,&ne_d1m)==PB) return(PB);
  /*
  if(VipPutSSExtremaValInBuffer(d1,e_d1ptr,dim,
				monster->d1_val,&(monster->nd1val),monster->sized1val,0)==PB) return(PB);
				
  if(VipPutSSExtremaValAndSpeedInBuffer(d1,d3,d4,e_d1ptr,dim,
				monster->d1_val,&(monster->nd1val),monster->sized1val,0)==PB) return(PB);
				*/
  if(VipDetectSSExtrema(d2,e_d2ptr,dim,&ne_d2p,&ne_d2m)==PB) return(PB);
  /*
  if(VipPutSSExtremaValInBuffer(d2,e_d2ptr,dim,
				monster->d2_val,&(monster->nd2val),monster->sized2val,0)==PB) return(PB);
				
  if(VipPutSSExtremaValAndSpeedInBuffer(d2,d4,d5,e_d2ptr,dim,
				monster->d2_val,&(monster->nd2val),monster->sized2val,0)==PB) return(PB);*/


  if(nderivative>2)  
      {
	  if(VipDetectSSExtrema(d3,e_d3ptr,dim,&ne_d3p,&ne_d3m)==PB) return(PB);
      }
  if(nderivative>3)  
      {
	  if(VipDetectSSExtrema(d4,e_d4ptr,dim,&ne_d4p,&ne_d4m)==PB) return(PB);
      }

  rebours = 100;
  for(iter=1;iter<3*itermaxestim;iter++)
    {
      /*
      e_ssptr += dim;
      */
      e_d1ptr += dim;
      e_d2ptr += dim;
      if(nderivative>2) e_d3ptr += dim;
      if(nderivative>3) e_d4ptr += dim;
  
      ssptr = ss; /*buffer swap*/
      ss = ssp1;
      ssp1 = ssptr;

      DiffusionIteration(ss,ssp1,dscale,dim);
      FirstDeriv(ssp1,d1,dim);
      SecondDeriv(ssp1,d2,dim);
      
      if(nderivative>2) FirstDeriv(d2,d3,dim);
      if(nderivative>3) SecondDeriv(d2,d4,dim);
      /*
      FirstDeriv(d4,d5,dim);
      */
      /*
      if(VipDetectSSExtrema(ssp1,e_ssptr,dim,&ne_ssp,&ne_ssm)==PB) return(PB);
      */
      /*
      if(VipPutSSExtremaValInBuffer(ssp1,e_ssptr,dim,
				monster->ss_val,&(monster->nssval),monster->sizessval,iter)==PB) return(PB);
				*/
      if(VipDetectSSExtrema(d1,e_d1ptr,dim,&ne_d1p,&ne_d1m)==PB) return(PB);
      /*
      if(VipPutSSExtremaValAndSpeedInBuffer(d1,d3,d4,e_d1ptr,dim,
				monster->d1_val,&(monster->nd1val),monster->sized1val,iter)==PB) return(PB);
      
      if(VipPutSSExtremaValInBuffer(d1,e_d1ptr,dim,
				monster->d1_val,&(monster->nd1val),monster->sized1val,iter)==PB) return(PB);
				*/
      if(VipDetectSSExtrema(d2,e_d2ptr,dim,&ne_d2p,&ne_d2m)==PB) return(PB);
      /*
      if(VipPutSSExtremaValInBuffer(d2,e_d2ptr,dim,
				monster->d2_val,&(monster->nd2val),monster->sized2val,iter)==PB) return(PB);
      if(VipPutSSExtremaValAndSpeedInBuffer(d2,d4,d5,e_d2ptr,dim,
				monster->d2_val,&(monster->nd2val),monster->sized2val,iter)==PB) return(PB);
*/

      if(nderivative>2)  
	  {
	      if(VipDetectSSExtrema(d3,e_d3ptr,dim,&ne_d3p,&ne_d3m)==PB) return(PB);
	  }
      if(nderivative>3)  
	  {
	      if(VipDetectSSExtrema(d4,e_d4ptr,dim,&ne_d4p,&ne_d4m)==PB) return(PB);
	  }
      
      if(ne_d2m<=1)
	{
	  rebours--;
	  if(rebours<0) break;
	}
      
      if(iter>=itermaxestim)
	{
	  VipPrintfError("Sorry, unsufficient max scale (correction has to be implemented");
	  VipPrintfExit("VipCompute1DScaleSpaceStructUntilLastCascade");
	  return(PB);
	}
    }


  monster->itermax = iter;
  monster->scalemax = (int)(iter*dscale+0.5);

  return(monster);
}



/*-----------------------------------------------------------------------*/
int VipPutSSExtremaValInBuffer(double *data,Vip_S8BIT *eptr,int dim,Singularity *val,int *nval,int sizeval,int iter)
/*-----------------------------------------------------------------------*/
{
  int i;
  
  if(data==NULL || eptr==NULL || val==NULL || nval==NULL)
    {
	  VipPrintfError("NULL argument");
	  VipPrintfExit("VipPutSSExtremaValInBuffer");
	  return(PB);
    }

  for(i=0;i<dim;i++)
    {
      if(*eptr++)
	{
	  if(*nval<sizeval)
	    {
	      val[*nval].val = *data;
	      val[*nval].loc = i;
	      val[*nval].iter = iter;
	      (*nval)++;
	    }
	  else
	    {
	      VipPrintfError("Unsufficient val buffer size");
	      VipPrintfExit("VipPutSSExtremaValInBuffer");
	      return(PB);
	    }
	}
      data++;
    }
  return(OK);

}

/*-----------------------------------------------------------------------*/
int VipPutSSExtremaValAndSpeedInBuffer(double *data,double *denom, double *num,Vip_S8BIT *eptr,int dim,Singularity *val,int *nval,int sizeval,int iter)
/*-----------------------------------------------------------------------*/
{
  int i;
  double VIP_INFINITY = 1000000000000.;
  double EPSILON = 0.0000001;

  if(data==NULL || denom==NULL || num==NULL || eptr==NULL || val==NULL || nval==NULL)
    {
	  VipPrintfError("NULL argument");
	  VipPrintfExit("VipPutSSExtremaValInBuffer");
	  return(PB);
    }

  for(i=0;i<dim;i++)
    {
      if(*eptr++)
	{
	  if(*nval<sizeval)
	    {
	      val[*nval].val = *data;
	      val[*nval].loc = i;
	      val[*nval].iter = iter;
	      if(fabs(*denom)<EPSILON) val[*nval].speed = VIP_INFINITY;
	      else val[*nval].speed = -0.5 * *num / *denom; 
	      (*nval)++;
	    }
	  else
	    {
	      VipPrintfError("Insufficient val buffer size");
	      VipPrintfExit("VipPutSSExtremaValInBuffer");
	      return(PB);
	    }
	}
      data++;
      denom++;
      num++;
    }
  return(OK);

}














