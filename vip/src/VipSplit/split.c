/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipSplit.c           * TYPE     : Command line
 * AUTHOR      :                      * CREATION : 18/04/1997
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

  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "split";
  char outfile[128];
  int readlib, writelib;
  int status;
  int flo=0;

  int nbx, nby, nbz, nbf;
  int i, l;

  readlib = VIDA;
  writelib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-i", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-o", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp(argv[i], "-float",2)) flo=1;
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      (void)fprintf(stderr,"input args are required by VipSplit\n");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }

  printf("Lecture de la taille de l'image en entree %s\n",input);
  if (readlib == VIDA)
    vol = VipReadVidaHeader(input);


  if (vol==PB)
    {
      VipPrintfError("Reading Problem...");
      return(VIP_CL_ERROR);
    }
 
/* Lecture du nombre de frames						*/

  nbx = mVipVolSizeX(vol);
  nby = mVipVolSizeY(vol);
  nbz = mVipVolSizeZ(vol);
  nbf = mVipVolSizeT(vol);

  printf("nbx = %d nby = %d nbz = %d nbf = %d\n",nbx,nby,nbz,nbf);

  if( nbf < 1 ) {
    VipPrintfError("Nombre de frames inferieur a 1");
    return(VIP_CL_ERROR);
  }

/* Boucle sur chacune des frames					*/

  for(l=1;l<=nbf;l++){

/* Creation d'une structure Vip avec une seule frame			*/
/* Transfert des data du volume en entree vers le volume cree		*/

     printf("Extraction frame %d\n",l);

     if( flo == 1 )
       vol = VipReadVidaFloatFrame(input,l-1,l-1);
     else
       vol = VipReadVidaFrame(input,l-1,l-1);

     if (vol==PB)
     {
       VipPrintfError("VipReadVidaFrame: erreur");
       return(VIP_CL_ERROR);
     }

/* Ecriture sur disque du volume cree mono-frame			*/

     sprintf(outfile,"%s%03d",output,l);

     if( flo == 1 )
       status = VipWriteVidaScaledVolume(vol,outfile);
     else 
       status = VipWriteVolume(vol,outfile);

     if( status == PB ){
       VipPrintfError("VipWriteVolume: erreur");
       return(VIP_CL_ERROR);
     }

/* Desallocation du volume en memoire					*/

     VipFreeVolume(vol);

  }

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipSplit\n");
  (void)fprintf(stderr,"        -i {image name}\n");
  (void)fprintf(stderr,"        [-f[loat]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"split\")}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Split multi-frame volume");


  (void)printf("\n");
  (void)printf("Usage: VipSplit\n");
  (void)printf("        -i {image name}\n");
  (void)printf("        [-f[loat] : the volume is splited with Perfectly scale factor\n");
  (void)printf("        [-o[utput] {image name (default:\"split\")}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
