/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipDilatation.c      * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 03/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
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
#include <vip/connex.h>
#include <vip/morpho.h>
#include <vip/skeleton.h>
#include "vip/histo.h"

/*---------------------------------------------------------------------------*/
static int VipPutOneSliceToVal(Volume *vol, int z, int val);
/*---------------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(dual);
  VIP_DEC_VOLUME(horn);  
  char *histofilename=NULL;
  char *input = NULL;
  char *hornname = NULL;
  char output[VIP_NAME_MAXLEN] = "head";
  int borderwidth = 1;
  int readlib, writelib;
  int threshold = -1;
  int nguillotine = 3;
  int i;
  float closingsize = 4.;
  VipT1HistoAnalysis *ana;
  
  readlib = VIDA;
  writelib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-hname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  histofilename = argv[i];
	} 
      else if (!strncmp (argv[i], "-horn", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  hornname = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-closing", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  closingsize = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-threshold", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  threshold = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-nguillotine", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  nguillotine = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipGetHead");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipGetHead");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipGetHead");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
  if(hornname!=NULL)
    if (VipTestImageFileExist(hornname)==PB)
      {
	(void)fprintf(stderr,"Can not open this image: %s\n",input);
	return(VIP_CL_ERROR);
      }

  printf("Reading %s...\n",input);fflush(stdout);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,borderwidth);
  else
    vol = VipReadVolumeWithBorder(input,borderwidth);

  if(vol==NULL) return(VIP_CL_ERROR);

  /*compute seed*/

  if(threshold==-1)
    {
      if (histofilename!=NULL)
      {
        printf("Reading histogram analysis: %s\n", histofilename);fflush(stdout);
        ana = VipReadT1HistoAnalysis( histofilename);
        if(ana==PB || !(ana->gray) || !(ana->white))
	   {
	       printf("Histogram analysis read problem (file %s), sorry...\n",
		      histofilename);
	       return(VIP_CL_ERROR);	   
	 }
        if(ana->gray)
	{
	  ana->gray->left_sigma = ana->gray->sigma;
	  ana->gray->right_sigma = ana->gray->sigma;
	}
        if(ana->white)
	{
	  ana->white->left_sigma = ana->white->sigma;
	  ana->white->right_sigma = ana->white->sigma;
	}                 
      }
      else
      {
         ana = VipGetT1HistoAnalysisCustomized(vol,5,0.5);
         if(ana==PB)
	 {
	   printf("Histogram analysis problem, sorry...\n");
	   return(VIP_CL_ERROR);
	 }
       }
      threshold = ana->gray->mean - 3*ana->gray->left_sigma;
    
    }
  printf("Used threshold: %d\n", threshold);

  if(VipSingleThreshold(vol, GREATER_THAN, threshold, BINARY_RESULT)==PB) return(VIP_CL_ERROR);

  if(hornname!=NULL)
    {
      printf("Reading horn: %s...\n",hornname);
      if (readlib == TIVOLI)
	horn = VipReadTivoliVolumeWithBorder(hornname,borderwidth);
      else
	horn = VipReadVolumeWithBorder(hornname,borderwidth);
    
      if(horn==NULL) return(VIP_CL_ERROR);

      if( VipMerge(vol, horn, VIP_MERGE_ALL_TO_ONE, 0, 0 )== PB) return(VIP_CL_ERROR);
      VipFreeVolume(horn);

    }



  if( VipConnexVolumeFilter( vol, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(VIP_CL_ERROR);

  for(i = 0; i < nguillotine; i++)
    {
      VipPutOneSliceToVal(vol,mVipVolSizeZ(vol)-1-i,255);
    }
  if (VipClosing(vol,CHAMFER_BALL_3D,closingsize)==PB) return(VIP_CL_ERROR);

  dual = VipCopyVolume(vol,"dual");
  VipInvertBinaryVolume(dual);

  if (VipConnexVolumeFilter (dual, CONNECTIVITY_6, 0, CONNEX_LABEL)==PB) return(VIP_CL_ERROR);

  VipChangeIntLabel(dual, 1, 0);

  if( VipMerge( vol, dual, VIP_MERGE_ALL_TO_ONE, 0, 255 )== PB) return(VIP_CL_ERROR);

  VipFreeVolume(dual);

  for(i = 0; i < (nguillotine+1); i++)
    {
      VipPutOneSliceToVal(vol,mVipVolSizeZ(vol)-1-i,0);
    }

  printf("Writing %s...\n",output);
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else
    if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);

  VipFreeVolume(vol);
  VipFree(vol);

  return(0);

}

/*---------------------------------------------------------------------------*/
int VipPutOneSliceToVal(Volume *vol, int z, int val)
/*---------------------------------------------------------------------------*/
{
    VipOffsetStruct *vos;
    int ix, iy;
    Vip_S16BIT *ptr;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
	VipPrintfError("S16BIT type");
	VipPrintfExit("(VipPutOneSliceToVal");
	return(PB);
    }
  if(z<0 || z>=mVipVolSizeZ(vol))
      {
	  VipPrintfWarning("This slice is out of volume (VipPutOneSliceToVal)");
	  return(OK);
      }

   vos = VipGetOffsetStructure(vol);
   if(!vos) return(PB);

   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint + z*vos->oSlice;

   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
       {
	   for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		   *ptr++ = val;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
      }
 
   return(OK);
}

/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipGetHead\n");
  (void)fprintf(stderr,"        -i[nput] {MRI image name}\n");
  (void)fprintf(stderr,"        [-h[orn]] {image to be removed}\n");
  (void)fprintf(stderr,"        [-t[hreshold] {algo t: int (default : not used)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"head\")}]\n");
  (void)fprintf(stderr,"        [-c[losing] {float (mm) closing size for f mode (default: 4)}]\n");
  (void)fprintf(stderr,"        [-n[guillotine] {int (default : 3)}]\n");
  (void)fprintf(stderr,"        [-hn[ame] {histo analysis, default:input )}]\n");
  (void)printf("read when -a r is set, this file has the .han extension (VipHistoAnalysis)\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);
 
}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Get a binary mask of the head");
  (void)printf("Usage: VipGetHead\n");
  (void)printf("        -i[nput] {MRI image name}\n");
  (void)printf("        [-h[orn]] {image to be removed}\n");
  (void)printf("        [-hn[ame] {histo analysis, default:input )}]\n");
  (void)printf("read when -a r is set, this file has the .han extension (VipHistoAnalysis)\n");
  (void)printf("        [-t[hreshold] {algo t: int (default : not used)}]\n");
  (void)printf("        [-o[utput] {image name (default:\"head\")}]\n");
  (void)printf("        [-c[losing] {float (mm) closing size for f mode (default: 4)}]\n");
  (void)printf("        [-n[guillotine] {int (default : 3)}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
