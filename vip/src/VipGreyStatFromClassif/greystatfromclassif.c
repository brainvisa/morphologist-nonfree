/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipGreyWhiteClassif.c      * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 24/09/2002
 * VERSION     : 2.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Laptop
 ****************************************************************************
 *
 * DESCRIPTION : 
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/




/*##############################################################################

	I N C L U S I O N S

##############################################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/morpho.h>
#include <vip/brain.h>
#include <vip/histo.h>
#include <vip/connex.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(classif); 
  char *input = NULL;
  char *classifname = NULL;
  char *hananame =NULL;
  char anyname[1024];
  int i;
  int readlib;
  int glabel = 2;
  int wlabel = 3;
  float mG=-1., sG=-1., mW=-1., sW=-1.;
  int cmG=0.,csG=0., cmW=0., csW=0.;
  VipT1HistoAnalysis *hana;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr, *cptr;
  double gsum, wsum;
  int gn, wn;
 
  readlib = ANY_FORMAT;

  /*loop on command line arguments*/

  for (i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-mwhite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  mW= atof(argv[i]);
	}
     else if (!strncmp (argv[i], "-ana", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  hananame = argv[i];
	}
      else if (!strncmp (argv[i], "-classif", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  classifname = argv[i];
	}
      else if (!strncmp (argv[i], "-mgray", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  mG= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-glabel", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  glabel = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-wlabel", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  wlabel = atoi(argv[i]);
	} 
      else if (!strncmp (argv[i], "-sgray", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  sG= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-swhite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  sW= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else if(argv[i][0]=='a') readlib = ANY_FORMAT;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else
	    {
	      (void)fprintf(stderr,"This format is not implemented for reading\n");
	      Usage();
	    }
	}  
      else if (!strncmp(argv[i], "-help",2)) return Help();
      else return(Usage());
    }

  /*check that all required arguments have been given*/


  if (input==NULL) 
    {
      VipPrintfError("input arg is required by VipGreyStatFromClassif");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
  if (classifname==NULL && mG<0 && mW<0 && sG<0 && sW<0) 
    {
      VipPrintfError("classif volume or 4 input values are required by VipGreyStatFromClassif");
      return(Usage());
    }
  if (classifname!=NULL)
    {
      if (VipTestImageFileExist(classifname)==PB)
	{
	  (void)fprintf(stderr,"Can not open this image: %s\n",classifname);
	  return(VIP_CL_ERROR);
	}
    }
 
  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,0);
  else
    vol = VipReadVolumeWithBorder(input,0);

  if (vol==NULL) return (VIP_CL_ERROR);

  if (classifname!=NULL)
    {
      printf("Reading %s...\n",classifname);
      if (readlib == TIVOLI)
	classif = VipReadTivoliVolumeWithBorder(classifname,0);
      else if (readlib == SPM)
	classif = VipReadSPMVolumeWithBorder(classifname,0);
      else
	classif = VipReadVolumeWithBorder(classifname,0);
    }
  if (classif==NULL) return (VIP_CL_ERROR);

 if (classifname!=NULL)
   {
     vos = VipGetOffsetStructure(vol);
     ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
     cptr = VipGetDataPtr_S16BIT( classif ) + vos->oFirstPoint;
     gsum = 0.;
     wsum = 0.;
     gn = 0;
     wn = 0;
     for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
       {
	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	   {
	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		 if(*cptr == glabel)
		   {
		     gn++;
		     gsum += *ptr;
		   }
		 else if(*cptr == wlabel)
		   {
		     wn++;
		     wsum += *ptr;
		   }
		 cptr++;
		 ptr++;
	       }
	     ptr += vos->oPointBetweenLine;  /*skip border points*/
	     cptr += vos->oPointBetweenLine;  /*skip border points*/
	   }
	 ptr += vos->oLineBetweenSlice; /*skip border lines*/
	 cptr += vos->oLineBetweenSlice; /*skip border lines*/
       }
     cmG = (int)(gsum/gn+0.5);
     cmW = (int)(wsum/wn+0.5);

     ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
     cptr = VipGetDataPtr_S16BIT( classif ) + vos->oFirstPoint;
     gsum = 0.;
     wsum = 0.;
     for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
       {
	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	   {
	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		 if(*cptr == glabel)
		   {
		     gsum += (*ptr-cmG)*(*ptr-cmG);
		   }
		 else if(*cptr == wlabel)
		   {
		     wsum += (*ptr-cmW)*(*ptr-cmW);
		   }
		 cptr++;
		 ptr++;
	       }
	     ptr += vos->oPointBetweenLine;  /*skip border points*/
	     cptr += vos->oPointBetweenLine;  /*skip border points*/
	   }
	 ptr += vos->oLineBetweenSlice; /*skip border lines*/
	 cptr += vos->oLineBetweenSlice; /*skip border lines*/
       }
     csG = (int)(sqrt((double)(gsum/(gn-1)))+0.5);
     csW = (int)(sqrt((double)(wsum/(wn-1)))+0.5);

     printf("Grey matter: %d points; mean: %d; sigma: %d\n",gn, cmG, csG);
     printf("White matter: %d points; mean: %d; sigma: %d\n",wn, cmW, csW);
   }

  hana = VipCalloc(1,sizeof(VipT1HistoAnalysis),"VipAnalyseCascades");
  if(hana==PB) return(VIP_CL_ERROR);
  hana->gray = VipAllocSSObject();
  if(hana->gray==PB) return(VIP_CL_ERROR);
  hana->white = VipAllocSSObject();
  if(hana->white==PB) return(VIP_CL_ERROR);

  hana->gray->mean = (int)(cmG+0.5);
  hana->gray->sigma = (int)(csG+0.5);
  hana->white->mean = (int)(cmW+0.5);
  hana->white->sigma = (int)(csW+0.5);
  hana->sequence=MRI_T1_SPGR;

  if (mG > 0)
    hana->gray->mean = (int)(mG+0.5);
  if (sG > 0)
    hana->gray->sigma = (int)(sG+0.5);
  if (mW > 0)
    hana->white->mean = (int)(mW+0.5);
  if (sW > 0)
    hana->white->sigma = (int)(sW+0.5);
  
  if (hananame==NULL)
      strcpy(anyname,input);
  else
    strcpy(anyname,hananame);

  if(VipWriteT1HistoAnalysis( hana, anyname)==PB) return(VIP_CL_ERROR);


  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipGreyStatFromClassif\n");
  (void)fprintf(stderr,"        -i[nput] {MR image (bias corrected)}\n");
  (void)fprintf(stderr,"        -c[lassif] {label image}\n");
  (void)fprintf(stderr,"        -a[na] {histogram analysis (default input)}]\n");
  (void)fprintf(stderr,"        [-g[label] {int (default:2)}]\n");
  (void)fprintf(stderr,"        [-w[label] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-mg[ray] {float (default:from classif)}]\n");
  (void)fprintf(stderr,"        [-sg[ray] {float (default:from classif)}]\n");
  (void)fprintf(stderr,"        [-mw[hite] {float (default:from classif)}]\n");
  (void)fprintf(stderr,"        [-sw[hite] {float (default:from classif)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:a)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
  VipPrintfInfo("Create grey/white statistics file from classif or input\n");
  printf("Can be used in case of failure of VipHistoAnalysis because of low grey/white contrast\n");
  printf("If you gave a classified or partly classified image, for instance a drawn slice, or the result of\n");
  printf("a classification algorithm based on Talairach grid in one way or another\n");
  (void)printf("Usage: VipGreyStatFromClassif\n");
  (void)printf("        -i[nput] {MR image (bias corrected)}\n");
  (void)printf("        -c[lassif] {label image}\n");
  (void)printf("        -a[na] {histogram analysis (default input)}]\n");
  (void)printf("        [-g[label] {int (default:2)}]\n");
  (void)printf("        [-w[label] {int (default:3)}]\n");
  (void)printf("        [-mg[ray] {float (default:from classif)}]\n");
  (void)printf("        [-sg[ray] {float (default:from classif)}]\n");
  (void)printf("        [-mw[hite] {float (default:from classif)}]\n");
  (void)printf("        [-sw[hite] {float (default:from classif)}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:a)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
