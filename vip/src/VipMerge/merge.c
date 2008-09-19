/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipMerge.c           * TYPE     : Command line
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
  VIP_DEC_VOLUME(volmask); 
  VIP_DEC_VOLUME(volconverted); 

  int usefloat = VFALSE;
  char *input = NULL;
  char *mask = NULL;
  char output[VIP_NAME_MAXLEN] = "merge";
  int level = 0;
  int level_activ = VFALSE;
  int value = 0;
  int value_activ = VFALSE;
  char mode='a';
  int mode_value;
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
      else if (!strncmp (argv[i], "-merged", 2)) 
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
      else if (!strncmp (argv[i], "-value", 2)) 
	{
	  value = atoi(argv[++i]);
	  value_activ = VTRUE;
	} 
      else if (!strncmp (argv[i], "-customize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  mode = argv[i][0];
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
      (void)fprintf(stderr,"input and merged args are required by VipMerge\n");
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
  switch (mode)
    {
    case 'a':
      if(level_activ==VTRUE)
	{
	  VipPrintfWarning("You use level flag: The 'l' mode is triggered");
	  if(value_activ==VTRUE) mode_value = VIP_MERGE_ONE_TO_ONE;
	  else mode_value = VIP_MERGE_ONE_TO_MAXP1;
	}
      else
	{
	  if(value_activ==VTRUE) mode_value = VIP_MERGE_ALL_TO_ONE;
	  else mode_value = VIP_MERGE_ALL_TO_MAXP1;
	}
      break;
    case 'l':
      if(value_activ==VTRUE) mode_value = VIP_MERGE_ONE_TO_ONE;
      else mode_value = VIP_MERGE_ONE_TO_MAXP1;
      break;
    case 's':
      mode_value = VIP_MERGE_SAME_VALUES;
      break;
    default:  
      VipPrintfError("Unknown mode in VipMerge");
      return(Help());
    }

  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,0);
  else if(usefloat==VFALSE)
    vol = VipReadVolumeWithBorder(input,0);
  else
    vol = VipReadVidaFloatVolumeWithBorder(input,0);

  if ((vol==PB))
    {
      VipPrintfError("Reading Problem...");
      return(VIP_CL_ERROR);
    }


  if (readlib == TIVOLI)
    volmask = VipReadTivoliVolumeWithBorder(mask,0);
  else if (readlib == SPM)
    volmask = VipReadSPMVolumeWithBorder(mask,0);
  else 
    volmask = VipReadVolumeWithBorder(mask,0);
 
  if ((vol==PB)||(volmask==PB))
    {
      VipPrintfError("Reading Problem...");
      return(VIP_CL_ERROR);
    }
  if((mVipVolType(volmask)!=U8BIT)&&(mVipVolType(volmask)!=S16BIT))
    {
      VipPrintfWarning("Converting merged to S16BIT...");
      volconverted = VipTypeConversionToS16BIT(volmask,RAW_TYPE_CONVERSION);
      VipFreeVolume(volmask);
      volmask = volconverted;
    }
 
 
  if(VipMerge (vol, volmask, mode_value, level, value)==PB) return(VIP_CL_ERROR);
 
  
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
  (void)fprintf(stderr,"Usage: VipMerge\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -m[erged] {image name}\n");
  (void)fprintf(stderr,"        [-f[loat]]\n");
  (void)fprintf(stderr,"        [-c[ustomize] {a/l/s (default:a)}]\n");
  (void)fprintf(stderr,"        [-l[evel]] {int (level to be merged) (default: not used)}\n");
  (void)fprintf(stderr,"        [-v[alue]] {int (merged value) (default: input \"max+1\")}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"merge\")}]\n");
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
  (void)printf("        -m[erged] {image name}\n");
  (void)printf("        [-f[loat]]\n");
  (void)printf("The command works with float input volume (reading and writing do the same)\n");
  (void)printf("        [-c[ustomize] {a/l/s (default:a)}]\n");
  (void)printf("a: all to value, l: level to value, s: all to same value\n");
  (void)printf("        [-l[evel]] {int (level to be merged) (default: not used)}\n");
  (void)printf("        [-v[alue]] {int (merged value) (default: input \"max+1\")}\n");
  (void)printf("        [-o[utput] {image name (default:\"merge\")}]\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
