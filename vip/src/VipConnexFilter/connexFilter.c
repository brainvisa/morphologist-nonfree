/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipConnexFilter.c    * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 13/10/1996
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

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(voldual); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "cfiltered";
  int size = 0;
  int biggest =0;
  int how;
  int mode = CONNEX_GREYLEVEL;
  char stringconnectivity[256] = "26";
  int connectivity;
  int dual = VFALSE;

  /* temporary stuff */
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
      else if (!strncmp (argv[i], "-size", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  size = atoi(argv[i]);
	} 
      else if (!strncmp (argv[i], "-dual", 2)) 
	{
	  dual = VTRUE;
	} 
      else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(stringconnectivity,argv[i]);
	} 
      else if (!strncmp (argv[i], "-biggest", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  biggest = atoi(argv[i]);
	} 
     else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='b') mode = CONNEX_BINARY;
	  else if(argv[i][0]=='g') mode = CONNEX_GREYLEVEL;
	  else if(argv[i][0]=='l') mode = CONNEX_LABEL;
	  else
	    {
	      VipPrintfError("This mode is not implemented");
	      VipPrintfExit("(commandline)VipConnexFilter");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else if(argv[i][0]=='a') readlib = ANY_FORMAT;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipConnexFilter");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipConnexFilter");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipConnexFilter");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }

  if(size==0)
    {
      how = -biggest;
    }
  else how = size;

  connectivity = VipConvertStringToConnectivity(stringconnectivity);
  if(connectivity==PB)
    {
      VipPrintfError("This connectivity is unknown...");
      return(Usage());
    }

  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,1);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,1);
  else if (readlib == VIDA)
    vol = VipReadVidaVolumeWithBorder(input,1);
  else
    vol = VipReadVolumeWithBorder(input,1);

  if(vol==NULL) return(VIP_CL_ERROR);

  if(dual==VTRUE)
    {
      if(!VipTestType(vol,ANY_INT))
	{
	  voldual = VipTypeConversionToS16BIT(vol,RAW_TYPE_CONVERSION);
	  VipFreeVolume(vol);
	  VipFree(vol);
	  if (voldual==PB) return(VIP_CL_ERROR);
	  vol = voldual;
	}
      VipInvertBinaryVolume(vol);
    }

  if (VipConnexVolumeFilter (vol, connectivity, how, mode)==PB) return(VIP_CL_ERROR);
  
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
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipConnexFilter\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-s[ize] {int (default:0)}]\n");
  (void)fprintf(stderr,"        [-d[ual]]\n");
  (void)fprintf(stderr,"        [-b[iggest] {int (default:1)}]\n");
  (void)fprintf(stderr,"        [-m[ode] {b, g or l (default:g)}]\n");
  (void)fprintf(stderr,"        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"cfiltered\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, s, v or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: s, v or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Removes connected component according to their size");
  (void)printf("the smallest ones can be removed or one of the biggest kept.\n");
  (void)printf("It is also possible to get a labelling\n");
  (void)printf("\n");
  (void)printf("Usage: VipConnexFilter\n");
  (void)printf("        -i[nput] {image name} : object definition\n");
  (void)printf("        [-o[utput] {image name (default:\"distmap\")}]\n");
  (void)printf("        [-d[ual] (work on background)]\n");
  (void)printf("        [-s[ize] {int (default:0)} : smaller will be removed]\n");
  (void)printf("        [-b[iggest] {int (default:1)} : you can choose first, second...]\n");
  (void)printf("        [-m[ode] {b, g or l (default:g)} : binary, grey result or labelling (1,2,3...)]\n");
  (void)printf("        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)printf("Suffixes \"s\" and \"c\" means sagittal and coronal orientations\n");
  VipPrintfInfo("Read and Write flag are actually configured for debugging...");
  (void)printf("        [-r[eadformat] {char: a, s, v or t (default:any)}]\n");
  (void)printf("        [-w[riteformat] {char: s, v or t (default:t)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
