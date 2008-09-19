/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipExtEdge.c         * TYPE     : Command line
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
 ************************************************************************/




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
#include <vip/distmap.h>
#include <vip/deriche.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "extedge";
  int size = 0;
  int biggest =0;
  int how;
  int mode = EXTEDGE2D_ALL;
  char stringmode[256] = "2Da";
  char stringconnectivity[256] = "26";
  int string_activ = VFALSE;
  int connectivity;
   FILE *fg2d;
  char filename[512];
  int gmode = VFALSE;
  VipG2DBucket *g2dbuck;
  VipG3DBucket *g3dbuck;
  int fill=VFALSE;

  /* temporary stuff */
  int readlib, writelib;
 
  int i;

  readlib = TIVOLI;
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
      else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(stringconnectivity,argv[i]);
	  string_activ = VTRUE;
	} 
      else if (!strncmp (argv[i], "-biggest", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  biggest = atoi(argv[i]);
	} 
     else if (!strncmp (argv[i], "-gradient", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  gmode = atoi(argv[i]);
	} 
     else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(stringmode,argv[i]); 	    
	}
      else if (!strncmp (argv[i], "-fill", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') fill = VTRUE;
	  else if(argv[i][0]=='n') fill = VFALSE;
	  else
	    {
	      VipPrintfError("y/n");
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
	      VipPrintfExit("(commandline)VipDistanceMap");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipDistanceMap");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }

  mode = VipConvertStringToExtedgeMode(stringmode);
  if(mode==PB)
    {
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

  if(string_activ!=VTRUE)
    {
      if(stringmode[0]=='3') strcpy(stringconnectivity,"26");
      else strcpy(stringconnectivity,"8");
    }
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
  else
    vol = VipReadVolumeWithBorder(input,1);

  if(vol==NULL) return(VIP_CL_ERROR);

  if (fill==VTRUE)
    if (VipExtRay(vol, mode, SAME_VOLUME)==PB) return(VIP_CL_ERROR);
  if (fill==VFALSE)
    if (VipExtedge(vol, mode, SAME_VOLUME)==PB) return(VIP_CL_ERROR);

  if(how!=0)
    if (VipConnexVolumeFilter (vol, connectivity, how, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);

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

  VipFreeVolume(vol);
  VipFree(vol);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipExtedge\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-m[ode] {3Da/3D-ztop/3D-zbot/2Da/2D-ytop/2D-ybot (default:2Da)}]\n");
  (void)fprintf(stderr,"        [-f[ill] {y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-s[ize] {int (default:0)}]\n");
  (void)fprintf(stderr,"        [-b[iggest] {int (default:not activ)}]\n");
  (void)fprintf(stderr,"        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:8 (2D) or 26 (3D))}]\n");
  (void)fprintf(stderr,"        [-g[radient] (2/3 (default:not used)]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"extedge\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
  VipPrintfInfo("Returns edges touched by rays casted from image borders");
  (void)printf("Then removes connected component according to their size\n");
  (void)printf("the smallest ones can be removed or one of the biggest kept.\n");
  (void)printf("\n");
  (void)printf("Usage: VipExtEdge\n");
  (void)printf("        -i[nput] {image name} : object definition\n");
  (void)printf("        [-o[utput] {image name (default:\"extedge\")}]\n");
  (void)printf("        [-m[ode] {3Da/3D-ztop/3D-zbot/2Da/2D-ytop/2D-ybot (default:2Da)}]\n");
  (void)printf("Just a way of specifying borders...\n");
  (void)printf("        [-f[ill] {y/n (default:n)}]\n");
  (void)printf("        [-g[radient] (2/3 (default:not used)]\n");
  (void)printf("        [-s[ize] {int (default:0)} : smaller will be removed]\n");
  (void)printf("        [-b[iggest] {int (default:1)} : you can choose first, second...]\n");
  (void)printf("        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)printf("Suffixes \"s\" and \"c\" means sagittal and coronal orientations\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
