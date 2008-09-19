/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_util.c           * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 01/04/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Creation de 'Vip_util.c' fortement inspire des 
 *               entrees/sorties de la librairie Tivoli du département image
 *               de Telecom Paris de maniere a pouvoir utiliser le meme format
 *               d'images en memoire. Le but escompte est la possibilite de 
 *               link simultane avec Tivoli (ENST) et Vip (Volumic Image 
 *               Processing SHFJ).
 *               Les principales differences resident dans la gestion du 
 *               format du SHFJ dedie aux images fonctionnelles, dans la
 *               gestion des series dynamiques (images 4D) et la mise en 
 *               oeuvre de procedures generiques (plutot que des switch comme
 *               dans Tivoli) dans le cas ou la gestion du type des images 
 *               est necessaire.
 *               Cette derniere difference se manifeste par un split du
 *               fichier volume.c initial en un fichier de procedure "type
 *               independant" et un fichier generique volume.gen
 *
 *               Tout ce qui concerne le mode trace a ete decalque...
 *               (debugger oblige)
 *
 *               En outre, de maniere a reduire les probabilites de conflits 
 *               entre les deux librairies, les diverses fonctions destinees
 *               a la gestion de cette structure de donnee ont ete dotees du 
 *               prefixe "Vip" (ainsi que generalement d'une majuscule 
 *               supplementaire), les alias des divers types de donnees ont 
 *               ete dotes du prefixe "Vip_" (le suffixe _t a ete supprime)
 *               jfmangin@cea.fr
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/alloc.h>

/*----------------------------------------------------------------------------*/
FILE *VipOpenFile(
char *filename,
char *mode,
char *caller)
/*----------------------------------------------------------------------------*/
{
  FILE *f;
  char error[512];
  
  f = fopen(filename, mode);
  if (!f)
    {
      sprintf(error,"Can not open file: %s for %s", filename, mode);
      VipPrintfError(error);
      VipPrintfExit(caller);
      return(PB);
    }
  else return(f);
}


/*----------------------------------------------------------------------------*/

void			VipPrintfExit( const char *fname)

 /*----------------------------------------------------------------------------*/
{
  char error[2048];
		
  (void) sprintf ( error,"\nExecution aborted in function '%s'.\n", fname );
  VipPrintfError(error);
  if(VIP_EXIT_IN_PRINTFEXIT==VTRUE) exit ( PB );

}


/*----------------------------------------------------------------------------*/

void VipPrintfError ( const char *s )

/*----------------------------------------------------------------------------*/
{
  (void) fflush ( stdout );
  (void) fprintf ( stderr, "\n------------------------------" );
  (void) fprintf ( stderr, "\n!! VIP Error" );
  if (s!=NULL && strlen(s)!=0)
    (void) fprintf ( stderr, ":\n\t %s", s );
  (void) fprintf( stderr, "\n------------------------------\n" );
  (void) fflush ( stderr );
}


/*----------------------------------------------------------------------------*/

void VipPrintfWarning ( const char *s )

/*----------------------------------------------------------------------------*/
{
  (void) fflush ( stdout );
  (void) fprintf ( stderr, "\n------------------------------" );
  (void) fprintf ( stderr, "\n! VIP Warning" );
  if (s!=NULL && strlen(s)!=0)
    (void) fprintf ( stderr, ":\n\t %s", s );
  (void) fprintf( stderr, "\n------------------------------\n" );
  (void) fflush ( stderr );
}


/*----------------------------------------------------------------------------*/

void VipPrintfInfo ( const char *s )

/*----------------------------------------------------------------------------*/
{
  (void) fflush ( stderr );
  (void) fprintf ( stdout, "\n------------------------------" );
  (void) fprintf ( stdout, "\nVIP Info" );
  if (s!=NULL && strlen(s)!=0)
    (void) fprintf ( stdout, ":\n\t %s", s );
  (void) fprintf( stdout, "\n------------------------------\n" );
  (void) fflush ( stdout );
}


/*----------------------------------------------------------------------------*/


/*---------------------------------------------------------------*/
int VipComputeLogVolume(Volume *vol, int logmode)
/*---------------------------------------------------------------*/
{

  int i, j, k;
  Vip_DOUBLE *imptr;
  VipOffsetStruct *vos;
  double temp;
  int sign;

  if((logmode!=LOG_NEP)&&(logmode!=LOG_TEN))
    {
      VipPrintfError("Unknown log mode");
      VipPrintfExit("VipComputeLogVolume");
      return(PB);
    }

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(histo)VipComputeLogVolume");
      return(PB);
    }
   if (VipTestType(vol,VDOUBLE)!=OK )
    {
      VipPrintfError("Sorry,  ComputeVolumeHisto is only implemented for VDOUBLE volume");
      VipPrintfExit("(histo)VipComputeLogVolume");
      return(PB);
    }

  vos = VipGetOffsetStructure(vol);
  imptr = VipGetDataPtr_VDOUBLE(vol)+vos->oFirstPoint;

 
  for (k=mVipVolSizeZ(vol); k--; imptr+=vos->oLineBetweenSlice)
    for (j=mVipVolSizeY(vol); j--; imptr+=vos->oPointBetweenLine)
      for (i=mVipVolSizeX(vol); i--;)
      { 
	temp = *imptr;
	if(temp>0) sign = 1;
	else
	  {
	    sign = -1;
	    temp = -temp;
	  }
	temp += 1;
	if(temp>1)
	  {
	    if(logmode==LOG_NEP) temp = (double)log((double)(temp));
	    else temp = (double)log10((double)(temp));
	  }
	*imptr++ = (temp*sign);
      }

  VipFree(vos);

  return(OK);
}

/*---------------------------------------------------------------*/
int VipComputeReversedSignVolume(Volume *vol)
/*---------------------------------------------------------------*/
{

  int i, j, k;
  Vip_DOUBLE *imptr;
  VipOffsetStruct *vos;

 

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(histo)VipComputeReversedSignVolume");
      return(PB);
    }
   if (VipTestType(vol,VDOUBLE)!=OK )
    {
      VipPrintfError("Sorry,  VipComputeReversedSignVolume is only implemented for VDOUBLE volume");
      VipPrintfExit("(histo)VipComputeReversedSignVolume");
      return(PB);
    }

  vos = VipGetOffsetStructure(vol);
  imptr = VipGetDataPtr_VDOUBLE(vol)+vos->oFirstPoint;

 
  for (k=mVipVolSizeZ(vol); k--; imptr+=vos->oLineBetweenSlice)
    for (j=mVipVolSizeY(vol); j--; imptr+=vos->oPointBetweenLine)
      for (i=mVipVolSizeX(vol); i--;)
      { 
	*imptr = - *imptr;
	imptr++;
      }

  VipFree(vos);

  return(OK);
}

/***************************/
/*                         */
/*    USER I/O- functions  */
/*                         */
/***************************/

