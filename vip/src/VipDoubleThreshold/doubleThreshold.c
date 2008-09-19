/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipDoubleThreshold.c * TYPE     : Command line
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
#include <vip/alloc.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "thresholded";
  float flevel1 = -123456.;
  int level1 = -123456;
  float flevel2 = -123456.;
  int level2 = -123456;
  int mode = VIP_BETWEEN_OR_EQUAL_TO;
  int type = GREYLEVEL_RESULT;
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
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-tl", 3)) 
	{
	  level1 = atoi(argv[++i]);
	}
      else if (!strncmp (argv[i], "-th", 3)) 
	{
	  level2 = atoi(argv[++i]);
	}
      else if (!strncmp (argv[i], "-fl", 3)) 
	{
	  flevel1 = atof(argv[++i]);
	}
      else if (!strncmp (argv[i], "-fh", 3)) 
	{
	  flevel2 = atof(argv[++i]);
	}
      else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(!strcmp(argv[i],"b")) mode = VIP_BETWEEN;
	  else if(!strcmp(argv[i],"be")) mode = VIP_BETWEEN_OR_EQUAL_TO;
	  else if(!strcmp(argv[i],"o")) mode = VIP_OUTSIDE;
	  else if(!strcmp(argv[i],"oe")) mode = VIP_OUTSIDE_OR_EQUAL_TO;
	  else
	    {
	      (void)fprintf(stderr,"Unknown mode: %s\n",argv[i]);
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-color", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(!strcmp(argv[i],"b")) type = BINARY_RESULT;
	  else if(!strcmp(argv[i],"g")) type = GREYLEVEL_RESULT;
	  else
	    {
	      (void)fprintf(stderr,"Unknown type: %s\n",argv[i]);
	      return(Usage());
	    }
	}     
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='a') readlib = ANY_FORMAT;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else
	    {
	      (void)fprintf(stderr,"This format is not implemented for reading\n");
	      return(Usage());
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

  if (input==NULL)
    {
      (void)fprintf(stderr,"input arg is required by VipDoubleThreshold\n");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }

  if(((level1==-123456)||(level2==123456))&&((flevel1==-123456.)||(flevel2==123456.)))
    {
      (void)fprintf(stderr,"(tl,th) (or fl,fh) args are required by VipDoubleThreshold\n");
      return(Usage());
    }

  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,0);
  else if(level1!=-123456)
    vol = VipReadVolumeWithBorder(input,0);
  else 
    {
      vol = VipReadVidaFloatVolume(input);
      if(vol==NULL)
	{
	  vol = VipReadVolume(input);
	}
    }

  if(vol==NULL) return(VIP_CL_ERROR);

  if(level1!=-123456)
    {
      if (VipDoubleThreshold (vol, mode, level1, level2, type)==PB) return(VIP_CL_ERROR);
    }
  else
    {
      if (VipDoubleFloatThreshold (vol, mode, flevel1, flevel2, type)==PB) return(VIP_CL_ERROR);
    }
  
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if(level1!=-123456)
    {
      if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else VipWriteVidaScaledVolume(vol,output);

  VipFreeVolume(vol);
  VipFree(vol);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipDoubleThreshold\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -tl {int (low threshold)\n");
  (void)fprintf(stderr,"        -th {int (high threshold)\n");
  (void)fprintf(stderr,"        [-fl] {float (low threshold)}\n");
  (void)fprintf(stderr,"        [-fh] {float (high threshold)}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"thresholded\")}]\n");
  (void)fprintf(stderr,"        [-m[ode] {b | be | o | oe (default:be)}]\n");
  (void)fprintf(stderr,"        [-c[olor] {b | g (default:g)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Applies a double threshold to input image according to << mode >> and << type >>");


  (void)printf("\n");
  (void)printf("Usage: VipDoubleThreshold\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        -tl {int (low threshold)\n");
  (void)printf("        -th {int (high threshold)\n");
  (void)printf("        [-fl] {float (low threshold)}\n");
  (void)printf("        [-fh] {float (high threshold)}\n");
  (void)printf("If fl and fh are given, tl and th are no more required...\n");
  (void)printf("        [-o[utput] {image name (default:\"thresholded\")}]\n");
  (void)printf("        [-m[ode] {b | be | o | oe (default:be)}]\n");
  (void)printf("b=BETWEEN, be=BETWEEN_OR_EQUAL_TO, o=OUTSIDE, oe=OUTSIDE_OR_EQUAL_TO\n");
  (void)printf("        [-c[olor] {b | g (default:g)}]\n");
  (void)printf("Output image nature: b=BINARY_RESULT: ( 255 / 0 ), g=GREYLEVEL_RESULT\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
