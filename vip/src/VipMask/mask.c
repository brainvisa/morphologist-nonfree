/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipMask.c            * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 06/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : 
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




/*##############################################################################

	I N C L U S I O N S

##############################################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/util.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(volconverted); 
  VIP_DEC_VOLUME(volmask); 

  int usefloat = VFALSE;
  char *input = NULL;
  char *mask = NULL;
  char output[VIP_NAME_MAXLEN] = "masked";
  int level = 0;
  int level_activ = VFALSE;
  int readlib, writelib;
 
  int i;

  readlib = ANY_FORMAT;
  writelib = TIVOLI;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-mask", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  mask = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-float", 2)) 
	{
	  usefloat = VTRUE;
	} 
      else if (!strncmp (argv[i], "-level", 2)) 
	{
	  level = atoi(argv[++i]);
	  level_activ = VTRUE;
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
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else
	    {
	      (void)fprintf(stderr,"This format is not implemented for writing\n");
	      return(Usage());
	    }    
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL || mask==NULL)
    {
      (void)fprintf(stderr,"input and mask args are required by VipMask\n");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }
  if (VipTestImageFileExist(mask)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",mask);
      return(VIP_CL_ERROR);
    }

  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,0);
  else if (readlib == VIDA)
    {
      if(usefloat==VFALSE)
	vol = VipReadVidaVolumeWithBorder(input,0);
      else
	vol = VipReadVidaFloatVolumeWithBorder(input,0);
    }
  else
    vol = VipReadVolumeWithBorder(input,0);

  if ((vol==PB))
    {
      VipPrintfError("Reading Problem...");
      return(VIP_CL_ERROR);
    }
  if (readlib == TIVOLI)
    volmask = VipReadTivoliVolumeWithBorder(mask,0);
  else if (readlib == SPM)
    volmask = VipReadSPMVolumeWithBorder(mask,0);
  else if (readlib == VIDA)
    volmask = VipReadVidaVolumeWithBorder(mask,0);
  else 
    volmask = VipReadVolumeWithBorder(mask,0);
 
  if ((vol==PB)||(volmask==PB))
    {
      VipPrintfError("Reading Problem...");
      return(VIP_CL_ERROR);
    }
  if((VipTestType(volmask,U8BIT)==PB)&&(VipTestType(volmask,S16BIT)==PB))
    {
      VipPrintfWarning("Converting mask to S16BIT...");
      volconverted = VipTypeConversionToS16BIT(volmask,RAW_TYPE_CONVERSION);
      VipFreeVolume(volmask);
      volmask = volconverted;
    } 
  if(level_activ==VFALSE)
    {
      if(VipMaskVolume (vol, volmask)==PB) return(VIP_CL_ERROR);
    }
  else 
    {
      if(VipMaskVolumeLevel (vol, volmask, level)==PB) return(VIP_CL_ERROR);
    }
  
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if(usefloat==VFALSE)
    {
      if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else
    {
      if(VipWriteVidaScaledVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipMask\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -m[ask] {mask name}\n");
  (void)fprintf(stderr,"        [-f[loat]]\n");
  (void)fprintf(stderr,"        [-l[evel]] {int (defines real mask)}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"masked\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Simply mask an image");


  (void)printf("\n");
  (void)printf("Usage: VipMask\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        -m[ask] {mask name}\n");
  (void)printf("        [-f[loat]]\n");
  (void)printf("The command works with float input volume (reading and writing do the same)\n");
  (void)printf("        [-l[evel]] {int (defines real mask)}\n");
  (void)printf("        [-o[utput] {image name (default:\"masked\")}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
