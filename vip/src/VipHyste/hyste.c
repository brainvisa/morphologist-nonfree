/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipHyste.c           * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 13/10/1996
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
#include <vip/connex.h>
#include <vip/util.h>
#include <vip/bucket.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "hysteresis";
  float tlow=0.;
  float thigh=0.;
  int plow = -1;
  int phigh = -1;
  int quotient = -1;
  int ncut = 1;
  int size = 0;
  int biggest =0;
  int how;
  int mode = CONNEX_GREYLEVEL;
  int hystemode = HYSTE_NUMBER;
  char stringconnectivity[256] = "26";
  int connectivity;
  float lemax;
  FILE *fg2d;
  char filename[512];
  int gmode = VFALSE;
  VipG2DBucket *g2dbuck;
  VipG3DBucket *g3dbuck;

  /* temporary stuff */
  int readlib, writelib;
 
  int i;

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
      else if (!strncmp (argv[i], "-plow", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  plow = atoi(argv[i]);
	} 
     else if (!strncmp (argv[i], "-gradient", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  gmode = atoi(argv[i]);
	} 
      else if (!strncmp (argv[i], "-phigh", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  phigh = atoi(argv[i]);
	} 
     else if (!strncmp (argv[i], "-quotient", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  quotient = atoi(argv[i]);
	} 
     else if (!strncmp (argv[i], "-ncut", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ncut = atoi(argv[i]);
	} 
      else if (!strncmp (argv[i], "-tlow", 3)) 
	{
	  tlow = atof(argv[++i]);
	} 
      else if (!strncmp (argv[i], "-thigh", 3)) 
	{
	  thigh = atof(argv[++i]);
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
	      VipPrintfExit("(commandline)VipHyste");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipDistanceMap");
	      return(VIP_CL_ERROR);
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
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipHyste");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipHyste");
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

  if((gmode!=VFALSE)&&(gmode!=2)&&(gmode!=3))
    {
      VipPrintfError("Unknown gradient mode (2/3)");
      return(Help());
    }
  
 
  connectivity = VipConvertStringToConnectivity(stringconnectivity);
  if(connectivity==PB)
    {
      VipPrintfError("This connectivity is unknown...");
      return(Usage());
    }

  if (quotient!=-1) 
  {
     hystemode = HYSTE_PERCENT;
     ncut = quotient;
  }
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,1);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,1);
  else
    vol = VipReadVolumeWithBorder(input,1);

  if(vol==NULL) return(VIP_CL_ERROR);	

  if(((plow==-1)&&(phigh!=-1))||((phigh==-1)&&(plow!=-1)))
   {
      lemax = VipGetVolumeMax(vol);
      if((lemax<0))
      {
         VipPrintfError("Negative maximum value? (not implemented...)");
         return(VIP_CL_ERROR);
     }
     if(plow!=-1)
     {
       tlow = (float)plow/(float)(100.)*lemax;
       plow=-1;
     }
     if(phigh!=-1)
     {
       thigh = (float)phigh/(float)(100.)*lemax;
       phigh = -1;   
     }
   (void)printf("Hysteresis values: %f %f\n",tlow,thigh);
   }

  if(plow!=-1)
   {
      if(VipHysteresisPercentThresholding(vol,connectivity,how,mode,
                                    plow,phigh,hystemode,ncut)==PB) return(VIP_CL_ERROR);
   }
  else
   {
      if(VipHysteresisThresholding(vol,connectivity,how,mode,
                                    tlow,thigh,hystemode,ncut)==PB) return(VIP_CL_ERROR);
   }
  
  if(gmode==2)
    {
      strcpy(filename,input);
      strcat(filename,".g2D");
      fg2d = fopen(filename,"rb");
      if(fg2d!=NULL)
	{
	  fclose(fg2d);
	  g2dbuck = VipReadG2DFile(input);
	  VipDwindleG2DBucketFromVolume(g2dbuck,vol);
	  VipSaveG2DBucketList(g2dbuck,output);
	}
      else
	{
	  VipPrintfWarning("No g2D file to used (triggered by -g flag)");
	}
    }
  else if (gmode==3)
    {
      strcpy(filename,input);
      strcat(filename,".g3D");
      fg2d = fopen(filename,"rb");
      if(fg2d!=NULL)
	{
	  fclose(fg2d);
	  g3dbuck = VipReadG3DFile(input);
	  VipDwindleG3DBucketFromVolume(g3dbuck,vol);
	  VipSaveG3DBucketList(g3dbuck,output);
	}
      else
	{
	  VipPrintfWarning("No g3D file to used (triggered by -g flag)");
	}
    }
      
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else
    if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipHyste\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -tl {float (low threshold)}\n");
  (void)fprintf(stderr,"        -th {float (high threshold)}\n");
  (void)fprintf(stderr,"        [-pl {int in [1..100] (low threshold: percent of max)}]\n");
  (void)fprintf(stderr,"        [-ph {int in [1..100] (high threshold: percent of max)}]\n");
  (void)fprintf(stderr,"        [-n[points] {int (default:1)}]\n");
  (void)fprintf(stderr,"        [-q[uotient] {int (quotient=[1..100] def:not used)}]\n");
  (void)fprintf(stderr,"        [-s[ize] {int (default:0)}]\n");
  (void)fprintf(stderr,"        [-b[iggest] {int (default:1)}]\n");
  (void)fprintf(stderr,"        [-m[ode] {b, g or l (default:b)}]\n");
  (void)fprintf(stderr,"        [-g[radient] (2/3 (default:not used)]\n");
  (void)fprintf(stderr,"        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"hysteresis\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:v)}]\n");
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
  (void)printf("Usage: VipHyste\n");
  (void)printf("        -i[nput] {image name} : object definition\n");
  (void)printf("        -tl {float (low threshold)}\n");
  (void)printf("        -th {float (high threshold)}\n");
  (void)printf("        [-pl {int in [1..100] (low threshold: percent of max)}]\n");
  (void)printf("        [-ph {int in [1..100] (high threshold: percent of max)}]\n");
  (void)printf("        [-n[points] {int (default:1) over high threshold}]\n");
  (void)printf("        [-q[uotient] {int (quotient=[1..100] points over high threshold)}]\n");
  (void)printf("        [-o[utput] {image name (default:\"hysteresis\")}]\n");
  (void)printf("        [-s[ize] {int (default:0)} : smaller will be removed]\n");
  (void)printf("        [-g[radient] (2/3 (default:not used)]\n");
  (void)printf("read, dwindle ans save .gXD file, where X=2/3\n");
  (void)printf("        [-b[iggest] {int (default:1)} : you can choose first, second...]\n");
  (void)printf("        [-m[ode] {b, g or l (default:b)} : binary result or labelling (1,2,3...)]\n");
  (void)printf("        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)printf("Suffixes \"s\" and \"c\" means sagittal and coronal orientations\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
