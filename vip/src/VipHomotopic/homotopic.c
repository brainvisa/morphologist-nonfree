/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipHomotopic.c      * TYPE     : Command line
 * AUTHOR      : FISCHER C.          * CREATION : 08/03/2011
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
#include <vip/morpho.h>
#include <vip/skeleton.h>
#include <vip/pyramid.h>
#include <vip/histo.h>
#include <vip/deriche.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    
    /*declarations and initializations*/
    
    VIP_DEC_VOLUME(vol);
    VIP_DEC_VOLUME(closing);
    VIP_DEC_VOLUME(mask);
    VIP_DEC_VOLUME(extedge);
    VIP_DEC_VOLUME(copypyr);
    Pyramid *pyr, *pyrlab;
    char *input = NULL;
    char output[VIP_NAME_MAXLEN] = "brain";
    char *cortexname = NULL;
    Volume *cortex = NULL;
    char *squeletonname = NULL;
    Volume *squeleton = NULL;
    int i;
    float fclosingsize = 5.;
    int linside = 0;
    int loutside = 11;
    int readlib, writelib;
    
    readlib = VIDA;
    writelib = TIVOLI;

  /*loop on command line arguments*/

  for (i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-Cortex", 2)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
            cortexname = argv[i];
        }
      else if (!strncmp (argv[i], "-Squeleton", 2)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
            squeletonname = argv[i];
        }
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipHomotopic");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipHomotopic");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return Help();
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL) 
    {
      VipPrintfError("input arg is required by VipHomotopic");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
   if (cortexname==NULL) 
   {
       VipPrintfError("cortex arg is required by VipHomotopic");
       return(Usage());
   }


  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,1);
  else
    vol = VipReadVolumeWithBorder(input,1);
  if (vol==NULL) return (VIP_CL_ERROR);

  printf("Reading cortex...\n");
  cortex = VipReadVolumeWithBorder(cortexname,1);
  if(!cortex) return(VIP_CL_ERROR);
  printf("Reading squeleton...\n");
  squeleton = VipReadVolumeWithBorder(squeletonname,1);
  if(!squeleton) return(VIP_CL_ERROR);
  
  
  printf("-------------------------------\n");
  printf("Masking %s with the sulcus...\n",vol->name);
  printf("-------------------------------\n");
  mask = VipCreateSingleThresholdedVolume( squeleton, GREATER_OR_EQUAL_TO, 19, BINARY_RESULT );
  extedge = VipCopyVolume( mask, "extedge" );
  VipExtedge( extedge, EXTEDGE3D_ALL, SAME_VOLUME );
  VipMerge( mask, extedge, VIP_MERGE_ONE_TO_ONE, 255, 0 );
  VipFreeVolume(extedge);
  extedge = VipCreateSingleThresholdedVolume( squeleton, EQUAL_TO, 80, BINARY_RESULT );

  VipMerge( mask, extedge, VIP_MERGE_ONE_TO_ONE, 255, 255 );
  VipMerge( vol, mask, VIP_MERGE_ONE_TO_ONE, 255, 0 );
  VipFreeVolume(extedge);
  
  
  printf("-------------------------------\n");
  printf("Computing closing of %s...\n",vol->name);
  printf("-------------------------------\n");
  closing = VipCopyVolume(vol,"closing");
  if (closing==PB) return (VIP_CL_ERROR);
  if (VipClosing ( closing, CHAMFER_BALL_3D, fclosingsize )== PB) return (VIP_CL_ERROR);
  VipChangeIntLabel( closing, 255, 1 );
  VipMerge( closing, vol, VIP_MERGE_SAME_VALUES, 0, 0);
  
  
  printf("-------------------------------\n");
  printf("CSF/Gray interface detection using homotopic erosion of %s...\n", cortexname);
  printf("-------------------------------\n");
  VipHomotopicErosionFromInside( cortex, closing, 100, 255, linside, loutside );
  
  
  printf("-------------------------\n");
  printf("Writing %s...\n",output);
  if (writelib == TIVOLI)
  {
      if(VipWriteTivoliVolume(cortex,output)==PB) return(VIP_CL_ERROR);
  }
  else
      if(VipWriteVolume(cortex,output)==PB) return(VIP_CL_ERROR);

    return(0);
    
} 
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
    (void)fprintf(stderr,"Usage: VipHomotopic\n");
    (void)fprintf(stderr,"        -i[nput] {segmented brain image name}\n");
    (void)fprintf(stderr,"        [-o[utput] {image name (default:\"cortex\")}]\n");
    (void)fprintf(stderr,"        [-C[ortex] {cortex image name}]\n");
    (void)fprintf(stderr,"        [-S[keleton] {skeleton image name}]\n");
    (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
    (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default:t)}]\n");
    (void)fprintf(stderr,"        [-h[elp]\n");
    return(VIP_CL_ERROR);
}
/*****************************************************/

static int Help()
{
    VipPrintfInfo("Homotopic segmentation of an object defined by the external hull");
    (void)printf("of the brain (the surface of a brain closing) and by the CSF/gray interface\n");
    (void)printf("Usage: VipHomotopicSnake\n");
    (void)printf("        -i[nput] {brain image name}\n");
    (void)printf(" a brain (or hemisphere) segmentation usually obtained with VipGetBrain\n");
    (void)printf("        [-C[ortex] {cortex image name}]\n");
    (void)printf(" a white matter segmentation obtained with VipHomotopicSnake\n");
    (void)printf("        [-S[keleton] {skeleton image name}]\n");
    (void)printf(" an image of the sulci obtained with VipSkeleton\n");
    (void)printf("        [-o[utput] {image name (default:\"homotopic\")}]\n");
    (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
    (void)printf("        [-w[riteformat] {char: v or t (default:t)}]\n");
    (void)printf("        [-h[elp]\n");
    return(VIP_CL_ERROR);
}
/******************************************************/
