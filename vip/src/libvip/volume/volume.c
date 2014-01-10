/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_volume.c         * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 16/01/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Creation de 'Vip_volume.c' fortement inspire des 
 *               entrees/sorties de la librairie Tivoli du d�partement image
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
 *          02/03/2000|V Frouin      | Reactivation du memory map
 *                    |              | initialement introduit par D Papadopo.
 *                    |              |
 *                    |              | Lecture d'un fichier de config
 *                    |              | Par primitives simples pour qu'elles
 *                    |              | puissent rester locales et statiques.
 *****************************************************************************/


#include <assert.h>

int VIP_EXIT_IN_PRINTFEXIT = 0;


 #ifndef _XOPEN_SOURCE /* necessaire pour utiliser 'tempnam()', qui n'est */
 #define _XOPEN_SOURCE /* pas C ISO, mais est necessaire au memory mapping */
 #include <stdio.h>
 #undef _XOPEN_SOURCE
 #endif

/* #include <stdio.h> Si C ISO requis il faut reactiver cela et perdre tempna*/


#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include <vip/volume.h>
#include <vip/volume_static.h>
#include <vip/volume/vol_carto.h>


#include <vip/alloc.h>
#include <vip/spm.h>
#include <vip/vida.h>
#include <vip/minc.h>

/*note the package notion violation here*/
#ifdef TIVOLI_LIB	
#include <vip/TivoliIO.h>
#endif


/*----------------------------------------------------------------------------*/
	static
	int			VipVerifyVolumeName
/*check if name is not null*/
/*----------------------------------------------------------------------------*/
(	const char			*name
);


/*----------------------------------------------------------------------------*/
	static
	int			VipVerify4DSize
/*check if 4D dimensions are consistent with Vip*/

/*----------------------------------------------------------------------------*/
(
	int			nx,
	int			ny,
	int			nz,
	int			nt
);


/*----------------------------------------------------------------------------*/
	static
	int			VipVerify4DVoxelSize
/*check if voxel dimensions are consistent with Vip*/
/*----------------------------------------------------------------------------*/
(
	float		dx,
	float		dy,
	float		dz,
	float		dt
);

/*----------------------------------------------------------------------------*/
	static
	int			VipVerifyBorderWidth
/*check border width consistence*/
/*----------------------------------------------------------------------------*/
(
	int			b
);


/*----------------------------------------------------------------------------*/
static
	int			VipVerifyType
/*check if type is consistent with Vip*/
/*----------------------------------------------------------------------------*/
(
	int			type
);


/*----------------------------------------------------------------------------*/
/*static	char		    *VipStateName*/
/* provide a string with state name */
/*----------------------------------------------------------------------------*/
/*(
	int			state
);*/

/*----------------------------------------------------------------------------*/
static	int			VipVerifyState
/*check state consistence*/
/*----------------------------------------------------------------------------*/
(
	int			state
);


/*----------------------------------------------------------------------------*/
	
	static int			VipTestFilled
/*simply check if volume is in state STATE_FILLED
which means it contains significative data*/
/*Warning, this state is up to date if your
own procedure control it...*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	static
	int			VipSizeOf

/*----------------------------------------------------------------------------*/
(
	int			type
);

/*==========================*
 |                          |
 |  input-output functions  |
 |                          |
 *==========================*/
/*stuff of Dimitri Papadopoulos*/
/*----------------------------------------------------------------------------*/
#ifndef VIP_CARTO_VOLUME_WRAPPING
#ifndef _WIN32
static 
void swapfileAtSignal
/*----------------------------------------------------------------------------*/
(
	int a
);
#endif	/* _WIN32 */

/*----------------------------------------------------------------------------*/

	static void swapfileAtExit
	
/*----------------------------------------------------------------------------*/
(
	void
);
#endif


/* memory mapping stuff (cf. Dimitri Papadopoulos) */
/*flag which decide if memory mapping take place...*/
int VipMemoryMapping = VFALSE;
#ifndef VIP_CARTO_VOLUME_WRAPPING
static swapfileNode *swapfileList = NULL;
static int swapfileFlag = 0;
#endif


/*Vip Testing functions which return PB (0) in case of problem*/


/*----------------------------------------------------------------------------*/

int			VipTestExist

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( volume == NULL )
    {
      VipPrintfError("access to volume denied (pointer NULL)\n" );
      return ( PB );
    }
  if ( volume->state == STATE_FREED )
    {
      VipPrintfError("access to volume denied (just freed)\n" );
      return ( PB );
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/

int			VipTestExistN

/*----------------------------------------------------------------------------*/
(
	int			n, ...
)
{
  va_list		ap;
  int			i = 0;
  Volume	    *volume;

  va_start ( ap, n );
  for ( i = 0; i < n; i++ )
    {
      volume = va_arg ( ap, Volume* );
      if ( volume == NULL )
	{
	  VipPrintfError ("access to volume denied (NULL pointer)\n" );
	  return ( PB );
	}
      if ( volume->state == STATE_FREED )
	{
	  VipPrintfError("access to volume denied (just freed)\n" );
	  return ( PB );
	}
    }
  va_end ( ap );
  return ( OK );
}




/*----------------------------------------------------------------------------*/

int			VipTestType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			type
)
{

  if ( type== ANY_TYPE )
    return(VipTestTypeN(volume,8,U8BIT,S8BIT,U16BIT,S16BIT,U32BIT,S32BIT,VFLOAT,VDOUBLE));

  if ( type== ANY_INT )
    return(VipTestTypeN(volume,6,U8BIT,S8BIT,U16BIT,S16BIT,U32BIT,S32BIT));

  if ( type== ANY_FLOAT )
    return(VipTestTypeN(volume,2,VFLOAT,VDOUBLE));

  if ( !VipTestExist ( volume ) ||
       !VipVerifyType ( type ) )
    return ( PB );
  if ( volume->type != type )
    {
      /*
      (void) sprintf ( error,
		       "volume '%s' (%s) has not the expected type (%s)\n",
		       volume->name, VipTypeName ( volume->type ), VipTypeName ( type ) );
      VipPrintfError(error);*/
      return ( PB );
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/

int			VipTestTypeN

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			n, ...
)
{
  va_list		ap;
  int			i,
    type,
    found = VFALSE;
	
  if ( !VipTestExist ( volume ) )
    return ( PB );
  va_start ( ap, n );
  for ( i = 0; i < n; i++ )
    {
      type = va_arg ( ap, int );
      if ( !VipVerifyType ( type ) )
	{
	  VipPrintfError ("bad arguments in 'VipTestTypeN'\n" );
	  return ( PB );
	}
      if ( type == volume->type )
	found = VTRUE;
    }
  va_end ( ap );
  if ( ! found )
    {
      /*
      (void) sprintf ( error,
		       "volume '%s' (%s) has not one of expected types\n",
		       volume->name, VipTypeName ( volume->type ) );
      VipPrintfError(error);
      */
      return ( PB );
    }
  return ( OK );
}


/*----------------------------------------------------------------------------*/

int			VipTestEqual4DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
  char error[VIP_NAME_MAXLEN];

  if ( !VipTestExistN ( 2, volume1, volume2 ) )
    return ( PB );
  if ( volume1->size.x != volume2->size.x ||
       volume1->size.y != volume2->size.y ||
       volume1->size.z != volume2->size.z ||
       volume1->size.t != volume2->size.t)
    {
      (void) sprintf ( error,
		       "volumes '%s' and '%s' have not the same 4D dimensions:\n",
		       volume1->name, volume2->name );
      VipPrintfError(error);
      (void) fprintf ( stderr,
		       "%d x %d x %d x %d  v.  %d x %d x %d x %d  !\n",
                       volume1->size.x, volume1->size.y,
                       volume1->size.z, volume1->size.t, 
                       volume2->size.x, volume2->size.y,
                       volume2->size.z, volume2->size.t );

      return ( PB );
    }
  return ( OK );
}


/*----------------------------------------------------------------------------*/

int			VipTestEqual3DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
  char error[VIP_NAME_MAXLEN];

  if ( !VipTestExistN ( 2, volume1, volume2 ) )
    return ( PB );
  if ( volume1->size.x != volume2->size.x ||
       volume1->size.y != volume2->size.y ||
       volume1->size.z != volume2->size.z )
    {
      (void) sprintf ( error,
		       "volumes '%s' and '%s' have not the same 3D dimensions:\n",
		       volume1->name, volume2->name );
      VipPrintfError(error);
      (void) fprintf ( stderr,
		       "%d x %d x %d   v.  %d x %d x %d !\n",
                       volume1->size.x, volume1->size.y,
                       volume1->size.z, 
                       volume2->size.x, volume2->size.y,
                       volume2->size.z);
      return ( PB );
    }
  return ( OK );
}





/*----------------------------------------------------------------------------*/

int			VipTestEqual2DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
  char error[VIP_NAME_MAXLEN];

  if ( !VipTestExistN ( 2, volume1, volume2 ) )
    return ( PB );
  if ( volume1->size.x != volume2->size.x ||
       volume1->size.y != volume2->size.y )
    {
      (void) sprintf ( error,
		       "volumes '%s' and '%s' have not the same\
dimensions along x and y\n", volume1->name, volume2->name );
      VipPrintfError(error);
      (void) fprintf ( stderr,
		       "%d x %d  v.  %d x %d!\n",
                       volume1->size.x, volume1->size.y,
                       volume2->size.x, volume2->size.y);
      return ( PB );
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/

int			VipTestEqualType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
  char error[VIP_NAME_MAXLEN];

  if ( !VipTestExistN ( 2, volume1, volume2 ) )
    return ( PB );
  if ( volume1->type != volume2->type )
    {
      (void) sprintf ( error,
		       "volumes '%s' (%s) and '%s' (%s) have not the same type\n",
		       volume1->name, VipTypeName ( volume1->type ),
		       volume2->name, VipTypeName ( volume2->type ) );
      VipPrintfError(error);
      return ( PB );
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/

int			VipTestEqualBorderWidth

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
  char error[VIP_NAME_MAXLEN];

  if ( !VipTestExistN ( 2, volume1, volume2 ) )
    return ( PB );
  if ( volume1->borderWidth != volume2->borderWidth )
    {
      (void) sprintf ( error,
		       "volumes '%s' (%d) and '%s' (%d) have not the same border width\n",
		       volume1->name, volume1->borderWidth,
		       volume2->name, volume2->borderWidth );
      VipPrintfError(error);
      return ( PB );
    }
  return ( OK );
}

/*----------------------------------------------------------------------------*/

static int			VipVerifyVolumeName

/*----------------------------------------------------------------------------*/
(
	const char	*name
)
{ 
  if ( name == NULL ) 
  {
    VipPrintfWarning("Improper volume name");
    (void) fprintf ( stderr,
                     "volume name does not exist\n");
    return(PB);
  }
  return(OK);
}


/*----------------------------------------------------------------------------*/

static int			VipVerify4DSize

/*----------------------------------------------------------------------------*/
(
	int			nx,
	int			ny,
	int			nz,
	int			nt
)
{
  if ( nx < 1 ||
       ny < 1 ||
       nz < 1 ||
       nt < 1 ||
       nx > VIP_IMAGE_MAXSIZE ||
       ny > VIP_IMAGE_MAXSIZE ||
       nz > VIP_IMAGE_MAXSIZE ||
       nt > VIP_IMAGE_MAXSIZE)
    {
        VipPrintfWarning ( "Improper volume dimension" );
      (void) fprintf ( stderr,
		       "improper 4D volume dimension (not in [1,%d])\n", VIP_IMAGE_MAXSIZE );
      (void) fprintf ( stderr, "You tried to use [%d, %d, %d, %d].\n",
		       nx, ny, nz, nt );
      
      return ( PB );
    }
  return ( OK );
}


/*----------------------------------------------------------------------------*/
static int			VipVerify4DVoxelSize

/*----------------------------------------------------------------------------*/
(
	float		dx,
	float		dy,
	float		dz,
	float		dt
)
{
  if ( dx <= 0. ||
       dy <= 0. ||
       dz <  0. ||
       dt < 0. /*||
       dx >VIP_VOXEL_MAXSIZE_MM||
       dy > VIP_VOXEL_MAXSIZE_MM||
       dz > VIP_VOXEL_MAXSIZE_MM la c'est jeff*/ )
/*||
       dt > VIP_VOXEL_MAXSIZE_S) modifie temprairement Cyril 11/01/99 */
    {
      (void) VipPrintfWarning ("impropre 4D volume voxel dimension (not positive or to large for being consistent)\n" );
      (void) fprintf ( stderr, "You tried to use [%f, %f, %f, %f].\n",
		       dx, dy, dz, dt );
      return ( OK );
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/
static int			VipVerifyBorderWidth

/*----------------------------------------------------------------------------*/
(
	int			b
)
{
  char message[VIP_NAME_MAXLEN];

  if ( b < 0 || b > VIP_BORDER_MAXWIDTH )
    {
      (void)sprintf(message,"improper volume border width (not in [0,%d])\n", VIP_BORDER_MAXWIDTH );
      (void) VipPrintfError ( message);
      return ( PB );
    }
  return ( OK );
}





/*----------------------------------------------------------------------------*/
char		    *VipTypeName

/*----------------------------------------------------------------------------*/
(
	int			type
)
{
  char *name;
  name = (char *)VipMalloc ( 64, "(volume.c)VipTypeName_name" );
  if (!name) return((char *)NULL);

  switch ( type )
    {
    case U8BIT  :
      strcpy ( name, "unsigned 8 bits" );
      break;
    case S8BIT  :
      strcpy ( name, "signed 8 bits" );
      break;
    case U16BIT :
      strcpy ( name, "unsigned 16 bits" );
      break;
    case S16BIT :
      strcpy ( name, "signed 16 bits" );
      break;
    case U32BIT :
      strcpy ( name, "unsigned 32 bits" );
      break;
    case S32BIT :
      strcpy ( name, "signed 32 bits" );
      break;
    case VFLOAT  :
      strcpy ( name, "float" );
      break;
    case VDOUBLE :
      strcpy ( name, "double" );
      break;
    }

  return ( name );
}





/*----------------------------------------------------------------------------*/
static int			VipVerifyType
/*----------------------------------------------------------------------------*/
(
	int			type
)
{
  char error[VIP_NAME_MAXLEN];

  if ( type != U8BIT  &&
       type != S8BIT  &&
       type != U16BIT &&
       type != S16BIT &&
       type != U32BIT &&
       type != S32BIT &&
       type != VFLOAT  &&
       type != VDOUBLE )
    {
      sprintf ( error,
		"improper volume type #%d !\n", type );
      VipPrintfError(error);
      return ( PB );
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/
/*static char		    *VipStateName*/
/*----------------------------------------------------------------------------*/
/*(
	int			state
)
{
  char		    *name;
  name = VipMalloc ( 64, "(volume.c)stateName_name" );
	
  switch ( state )
    {
    case STATE_DECLARED	 :
      strcpy ( name, "volume declared" );
      break;
    case STATE_ALLOCATED :
      strcpy ( name, "volume built up" );
      break;
    case STATE_FILLED	 :
      strcpy ( name, "volume filled" );
      break;
    case STATE_FREED	 :
      strcpy ( name, "volume washed up" );
      break;
    }
  return ( name );
}
*/



/*----------------------------------------------------------------------------*/
static int			VipVerifyState
/*----------------------------------------------------------------------------*/
(
	int			state
)
{
  char error[VIP_NAME_MAXLEN];

  if ( state != STATE_DECLARED  &&
       state != STATE_ALLOCATED &&
       state != STATE_FILLED    &&
       state != STATE_FREED )
    {
      (void) sprintf ( error,
		       "improper volume state #%d !\n", state );
      VipPrintfError(error);
      return ( PB );
    }
  return ( OK );
}


/*----------------------------------------------------------------------------*/
int VipGetRTConfigLong
(
	const char *token,
	long *value
)
/*----------------------------------------------------------------------------*/
{
  char         confFile[VIP_NAME_MAXLEN], buffer[VIP_NAME_MAXLEN], *pt;
  FILE        *fp;
  int          i;

  sprintf(confFile, "%s/.vip", getenv("HOME"));
  if ( (fp = fopen(confFile, "r") ) == NULL) return( PB );
  while ( fgets(buffer, VIP_NAME_MAXLEN, fp) != NULL )
    {
      for(i=0; (buffer[i] == ' ') || (buffer[i] == '\t'); i++)
      if (buffer[i] == '#') continue;
      pt = &buffer[i];
      if ( strncmp(pt, token, strlen(token) ) != 0 )
	{
	  continue;
	}
      else
	{
	  for (pt = &buffer[i] + strlen(token) + 1;
	       (*pt == ' ') || (*pt == '\t'); pt++);
	  *value = atol( pt );
	  fclose(fp);
	  return( OK );
	}
    }

  fclose(fp);
  return( PB );
}

/*----------------------------------------------------------------------------*/
char *VipGetRTConfigString
(
const char *token
)
/*----------------------------------------------------------------------------*/
{
  char         confFile[VIP_NAME_MAXLEN], buffer[VIP_NAME_MAXLEN], *pt;
  char        *retstring;
  FILE        *fp;
  int          i;


  sprintf(confFile, "%s/.vip", getenv("HOME"));
  if ( (fp = fopen(confFile, "r") ) == NULL) return( PB );
  while ( fgets(buffer, VIP_NAME_MAXLEN, fp) != NULL )
    {
      for(i=0; (buffer[i] == ' ') || (buffer[i] == '\t'); i++)
      if (buffer[i] == '#') continue;
      pt = &buffer[i];
      if ( strncmp(pt, token, strlen(token) ) != 0 )
	{
	  continue;
	}
      else
	{
	  for (pt = &buffer[i] + strlen(token) + 1;
	       (*pt == ' ') || (*pt == '\t'); pt++);
	  retstring = VipCalloc( strlen( pt),  sizeof(char),
				 "VipGetRTConfigChar");
	  if (retstring == NULL) return( PB);
	  strncpy(retstring,  pt, strlen( pt ) -1 );
	  fclose(fp);
	  return( retstring );
	}
    }

  fclose(fp);
  return( PB );
}


/*----------------------------------------------------------------------------*/
int			VipVerify2Dformat
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{

  if ( volume->voxelSize.z == 0 )
    {
      if ( volume->size.z != 1 )
	{
	  VipPrintfError (
			  "bad volume format (several slices but without thickness) !\n" );
	  return ( PB );
	}
      if ( volume->type != U8BIT )
	{
	  VipPrintfError ("bad volume format (no thickness while not unsigned 8 bit data) !" );
	  return ( PB );
	}
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/
	
int			VipVerifyAll

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist		( volume ) ||
       !VipVerifyType		( volume->type ) ||
       !VipVerifyState		( volume->state ) ||
       !VipVerifyBorderWidth	( volume->borderWidth ) ||
       !VipVerify4DSize		( volume->size.x,
				  volume->size.y,
				  volume->size.z,
				  volume->size.t ) ||
       !VipVerify4DVoxelSize	( volume->voxelSize.x,
				  volume->voxelSize.y,
				  volume->voxelSize.z,
				  volume->voxelSize.t )
       )
    return ( PB );
  return ( OK );
}








/*----------------------------------------------------------------------------*/
	
static int			VipTestFilled

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  char error[VIP_NAME_MAXLEN];

  if ( !VipTestExist ( volume ) )
    return ( PB );
  if ( volume->state != STATE_FILLED )
    {
      (void) sprintf ( error,
		       "volume '%s' does not contain significant data\n", volume->name );
      VipPrintfError(error);
      return ( PB );
    }
  return ( OK );
}





/*----------------------------------------------------------------------------*/
int			VipTestState

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			state
)
{

  if ( !VipTestExist ( volume ) ||
       !VipVerifyState ( state ) )
    return ( PB );
  if ( volume->state != state )
    {
      /*This message should be trigger higher
	 (void) sprintf ( error,
	 "volume '%s' (%s) has not the expected state (%s)\n",
	 volume->name, VipStateName ( volume->state ), VipStateName ( state ) );
	 VipPrintfError(error);
	 *** 
	 *** F. Poupon : j'ai mis ce message en commentaire, car
	 sinon il apparait tout le temps. Pb avec le test de l'etat,
	 a savoir que les etats 'filled' ou 'volume built up' ne sont
	 pas compatible avec l'etat 'declared', ce qui devrait pourtant
	 etre le cas !!!...
	 ***/
	 
      return ( PB );
    }
  return ( OK );
}




/*----------------------------------------------------------------------------*/
int			VipTestStateN
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			n, ...
)
{
  va_list		ap;
  int			i,
    state,
    found = VFALSE;
	
  if ( !VipTestExist ( volume ) )
    return ( PB );
  va_start ( ap, n );
  for ( i = 0; i < n; i++ )
    {
      state = va_arg ( ap, int );
      if ( !VipVerifyState ( state ) )
	{
	  (void) fprintf ( stderr,
			   "bad arguments in 'testStateN'\n" );
	  return ( PB );
	}
      if ( state == volume->state )
	found = VTRUE;
    }
  va_end ( ap );
  if ( ! found )
    {
      /* This message should be trigger higher
	 (void) sprintf ( error,
	 "volume '%s' (%s) has not an expected state\n",
	 volume->name, VipStateName ( volume->state ) );
	 VipPrintfError(error);
	 */
      return ( PB );
    }
  return ( OK );
}







/*Procedure which get some info from the volume structure*/


/*----------------------------------------------------------------------------*/
	
char			*VipGetVolumeName

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  char *the_name;

  if ( !VipTestExist ( volume ) ) 
    {
      VipPrintfExit ( "(volume.c)VipGetVolumeName" );
      return(NULL);
    }

  the_name = (char *)VipCalloc(strlen(volume->name),sizeof(char),"VipGetVolumeName");
  if (the_name==NULL) return((char *)NULL);

  strcpy(the_name,volume->name);
  return(the_name);
  /*This way of doing results in a non freed allocation,
    but this allows us to protect volume name*/
}


/*----------------------------------------------------------------------------*/
	
int			VipGet3DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int		    *sizeX,
	int		    *sizeY,
	int		    *sizeZ
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit ( "(volume.c)VipGet3DSize" );
      return(PB);
    }
  *sizeX = volume->size.x;
  *sizeY = volume->size.y;
  *sizeZ = volume->size.z;
  return(OK);
}

/*----------------------------------------------------------------------------*/
	
int			VipGet4DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int		    *sizeX,
	int		    *sizeY,
	int		    *sizeZ,
	int		    *sizeT
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit ( "(volume.c)VipGet4DSize" );
      return(PB);
    }
  *sizeX = volume->size.x;
  *sizeY = volume->size.y;
  *sizeZ = volume->size.z;
  *sizeT = volume->size.t;
  return(OK);
}

/*----------------------------------------------------------------------------*/

long        VipGet3DSizeInVoxels

/*----------------------------------------------------------------------------*/
(
		                 Volume * volume
)
{
  if (!VipTestExist(volume)) 
    {
      VipPrintfExit("(volume.c)VipGet3DSizeInVoxels");
      return(PB);
    }
  return ( volume->size.x*volume->size.y*volume->size.z );
}



/*----------------------------------------------------------------------------*/

long        VipGet3DSizeInBytes

/*----------------------------------------------------------------------------*/
(
		                 Volume * volume
)
{
  if (!VipTestExist(volume))
    {
      VipPrintfExit("(volume.c)VipGet3DSizeInBytes");
      return(PB);
    }
  return (VipGet3DSizeInVoxels(volume) * VipSizeofType(volume));

}

/*----------------------------------------------------------------------------*/


int                     VipShfjStartTime

/*----------------------------------------------------------------------------*/
(
        Volume      *volume,
        int         frameNumb
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipShfjUnite");
      return(PB);
    }
  return ( volume->shfj->start_time[frameNumb] );
}



/*----------------------------------------------------------------------------*/


int                     VipShfjDurationTime

/*----------------------------------------------------------------------------*/
(
        Volume      *volume,
        int         frameNumb
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipShfjUnite");
      return(PB);
    }
  return ( volume->shfj->dur_time[frameNumb] );
}




/*----------------------------------------------------------------------------*/


int			VipShfjUnite

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipShfjUnite");
      return(PB);
    }
  return ( volume->shfj->unite );
}




/*----------------------------------------------------------------------------*/


int			VipShfjVmdType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipShfjVmdType");
      return(PB);
    }
  return ( volume->shfj->VmdType );
}




/*----------------------------------------------------------------------------*/


float			VipShfjScale

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipShfjScale");
      return(PB);
    }
  return ( volume->shfj->scale );
}




/*----------------------------------------------------------------------------*/


float			VipShfjOffset

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipShfjOffset");
      return(PB);
    }
  return ( volume->shfj->offset );
}




/*----------------------------------------------------------------------------*/


	
int			VipSizexOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipSizexOf");
      return(PB);
    }
  return ( volume->size.x );
}



/*----------------------------------------------------------------------------*/
	
int			VipSizeyOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipSizeyOf");
      return(PB);
    }
  return ( volume->size.y );
}




/*----------------------------------------------------------------------------*/
	
int			VipSizezOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipSizezOf");
      return(PB);
    }
  return ( volume->size.z );
}


/*----------------------------------------------------------------------------*/
	
int			VipSizetOf		

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipSizetOf");
      return(PB);
    }
  return ( volume->size.t );
}


/*----------------------------------------------------------------------------*/
	
int			VipTypeOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipTypeOf");
      return(PB);
    }
  return ( volume->type );
}




/*----------------------------------------------------------------------------*/
	
int			VipGet3DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float	    *voxelSizeX,
	float	    *voxelSizeY,
	float	    *voxelSizeZ
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipGet3DVoxelSize");
     return(PB);
    }

  *voxelSizeX = volume->voxelSize.x;
  *voxelSizeY = volume->voxelSize.y;
  *voxelSizeZ = volume->voxelSize.z;

  return(OK);
}

/*----------------------------------------------------------------------------*/
	
int			VipGet4DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float	    *voxelSizeX,
	float	    *voxelSizeY,
	float	    *voxelSizeZ,
	float	    *voxelSizeT
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipGet4DVoxelSize");
      return(PB);
    }

  *voxelSizeX = volume->voxelSize.x;
  *voxelSizeY = volume->voxelSize.y;
  *voxelSizeZ = volume->voxelSize.z;
  *voxelSizeT = volume->voxelSize.t;

  return(OK);
}



/*----------------------------------------------------------------------------*/
	
int			VipBorderWidthOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( !VipTestExist ( volume ) )
    {
      VipPrintfExit("(volume.c)VipBorderWidthOf");
      return(PB);
    }
  return ( volume->borderWidth );
}



/*Function to set some info in volume structure*/

/*----------------------------------------------------------------------------*/
	int			VipSetState

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			state
)
{
  if ( !VipTestExist ( volume ) ||
       !VipVerifyState ( state ) ) 
    {
      VipPrintfExit("(volume.c)VipSetState");
      return(PB);
    }
  if ( volume->data != NULL && state == STATE_DECLARED )
    {
      VipPrintfError ( "volume can not be set to 'STATE_DECLARED' while data is not 'NULL'\n" );
      VipPrintfExit("(volume.c)VipSetState");
      return(PB);
    }
  if ( volume->data == NULL && state == STATE_ALLOCATED )
    {
      VipPrintfError  ( 
		       "volume can not be set to 'STATE_DECLARED' while data is still 'NULL'\n" );
      VipPrintfExit("(volume.c)VipSetState");
      return(PB);
    }
  if ( volume->data == NULL && state == STATE_FILLED )
    {
      VipPrintfError ( 
		      "volume can not be set to 'STATE_DECLARED' while data is still 'NULL'\n" );
      VipPrintfExit("(volume.c)VipSetState");
      return(PB);
    }
  volume->state = state;
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipSetVolumeName

/*----------------------------------------------------------------------------*/
(
	Volume 	*volume,
	char	*name
)
{
  if (!VipTestExist(volume) ||
      !VipVerifyVolumeName(name))
    {
      VipPrintfExit("(volume.c)VipSetVolumeName");
      return(PB);
    }
  strcpy(volume->name, name);
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipSet3DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx,
	int			ny,
	int			nz
)
{

  if ( !VipTestState ( volume, STATE_DECLARED ) ||
       !VipVerify4DSize ( nx, ny, nz, 1 ) ) 
    {
      VipPrintfExit("(volume.c)VipSet3DSize");
      return(PB);
    }
  volume->size.x = nx;
  volume->size.y = ny;
  volume->size.z = nz;
  volume->size.t = 1;
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipSet4DSize

/*----------------------------------------------------------------------------*/
(
    Volume	    *volume,
    int			nx,
    int			ny,
    int			nz,
    int			nt
 )
{

  if ( !VipTestState ( volume, STATE_DECLARED ) ||
       !VipVerify4DSize ( nx, ny, nz, nt ) )
    {
      VipPrintfExit("(volume.c)VipSet4DSize");
      return(PB);
    }
  volume->size.x = nx;
  volume->size.y = ny;
  volume->size.z = nz;
  volume->size.t = nt;
  return(OK);
}

/*----------------------------------------------------------------------------*/

int			VipSetShfjUnite

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			unite
)
{

  if ( !volume )
    {
      VipPrintfExit("(volume)VipSetShfjUnite");
      return(PB);
    }
  volume->shfj->unite = unite;
  return(OK);
}

/*----------------------------------------------------------------------------*/

int			VipSetShfjVmdType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			vmdtype
)
{

  if ( !volume )
    {
      VipPrintfExit("(volume)VipSetShfjVmdType");
      return(PB);
    }
  volume->shfj->VmdType = vmdtype;
  return(OK);
}

/*----------------------------------------------------------------------------*/

int			VipSetShfjScale

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float			scale
)
{

  if ( !volume )
    {
      VipPrintfExit("(volume)VipSetShfjScale");
      return(PB);
    }
  volume->shfj->scale = scale;
  return(OK);
}

/*----------------------------------------------------------------------------*/

int			VipSetShfjOffset

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float			offset
)
{

  if ( !volume )
    {
      VipPrintfExit("(volume)VipSetShfjOffset");
      return(PB);
    }
  volume->shfj->offset = offset;
  return(OK);
}

/*----------------------------------------------------------------------------*/


int			VipSetSizeX

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx
)
{

  if ( !VipTestState ( volume, STATE_DECLARED ) ||
       !VipVerify4DSize ( nx, 1, 1, 1 ) )
    {
      VipPrintfExit("(volume.c)VipSetSizeX");
      return(PB);
    }
  volume->size.x = nx;
  return(OK);
}




/*----------------------------------------------------------------------------*/

int			VipSetSizeY

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			ny
)
{

  if ( !VipTestState ( volume, STATE_DECLARED ) ||
       !VipVerify4DSize ( 1, ny, 1, 1 ) )
    {
      VipPrintfExit("(volume.c)VipSetSizeY");
      return(PB);
    }
  volume->size.y = ny;
  return(OK);
}




/*----------------------------------------------------------------------------*/

int			VipSetSizeZ

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nz
)
{

  if ( !VipTestState ( volume, STATE_DECLARED ) ||
       !VipVerify4DSize ( 1, 1, nz, 1 ) )
    {
      VipPrintfExit("(volume.c)VipSetSizeZ");
      return(PB);
    }
  volume->size.z = nz;
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipSetSizeT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nt
)
{

  if ( !VipTestState ( volume, STATE_DECLARED ) ||
       !VipVerify4DSize ( 1, 1, 1, nt ) )
    {
      VipPrintfExit("(volume.c)VipSetSizeT");
      return(PB);
    }
  volume->size.t = nt;
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipSet3DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float		dx,
	float		dy,
	float		dz
)
{

  if ( !VipTestExist ( volume ) ||
       !VipVerify4DVoxelSize ( dx, dy, dz, 1. ) )
    {
      VipPrintfExit("(volume.c)VipSet3DVoxelSize");
      return(PB);
    }
  volume->voxelSize.x = dx;
  volume->voxelSize.y = dy;
  volume->voxelSize.z = dz;
  volume->voxelSize.t = 1.;
  return(OK);
}

/*----------------------------------------------------------------------------*/

int			VipSet4DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float		dx,
	float		dy,
	float		dz,
	float		dt
)
{

  if ( !VipTestExist ( volume ) ||
       !VipVerify4DVoxelSize ( dx, dy, dz, dt ) )
    {
      VipPrintfExit("(volume.c)VipSet4DVoxelSize");
      return(PB);
    }
  volume->voxelSize.x = dx;
  volume->voxelSize.y = dy;
  volume->voxelSize.z = dz;
  volume->voxelSize.t = dt;
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipSetType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			type
)
{
  char error[VIP_NAME_MAXLEN];

  if ( !VipTestExist ( volume ) || !VipVerifyType ( type ) )
    {
      VipPrintfExit ( "(volume.c)VipSetType" );
      return(PB);
    }
        
  if ( volume->state == STATE_ALLOCATED )
    {
      VipPrintfWarning ( "The volume was already allocated" );
      (void) sprintf ( error, "Volume name: '%s'\n", volume->name );
      VipPrintfError(error);
      (void)fprintf ( stderr, "I'll free it before changing its type.\n");
      VipSetState ( volume, STATE_FILLED );
      VipFreeVolumeData( volume );
    }
        
  if ( volume->state == STATE_FILLED )
    {
      VipPrintfError ( "The volume must not be filled" );
      (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
      VipPrintfExit ( "(volume.c)VipSetType" );
      return(PB);
    }
        
  /* state can only be STATE_DECLARED */

  volume->type = type;
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipSetBorderWidth

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			borderWidth
)
{
  /*Volume	    *newVolume;*/
  char warning[VIP_NAME_MAXLEN];
	
  if ( !VipTestExist ( volume ) ) 
    {
      VipPrintfExit ( "(volume.c)VipSetBorderWidth" );
      return(PB);
    }
  if (!VipVerifyBorderWidth(borderWidth))
    {
      VipPrintfExit ( "(volume.c)VipSetBorderWidth" );
      return(PB);
    }
  if ( (volume->borderWidth == borderWidth)&&(borderWidth!=0) )
    {
      (void) sprintf ( warning, "Volume '%s' has already the good borderwidth: %d\n", volume->name,  borderWidth);	
      VipPrintfWarning(warning);
    }
  else
    {
      switch ( volume->state )
	{ 
	case STATE_FILLED :
	    VipPrintfInfo ( "Changing borderWidth of a filled volume" );
	    (void) fprintf ( stdout, "\tVolume named '%s'",
			   volume->name );
	    (void) fprintf ( stdout, 
			   "\n\tBorder from %d to %d\n",
			   volume->borderWidth, borderWidth );
	    VipPrintfExit("VipSetBorderWidth");
	    return(PB);
	  break;
	case STATE_ALLOCATED :
	  VipPrintfInfo ( "Changing borderWidth of an allocated volume" );
	  (void) fprintf ( stdout, "\tVolume named '%s'",
			   volume->name );
	  (void) fprintf ( stdout, 
			   "\n\tBorder from %d to %d\n",
			   volume->borderWidth, borderWidth );
	  VipFreeVolumeData ( volume );
	  volume->borderWidth = borderWidth;
	  VipAllocateVolumeData ( volume );
	  break;
	case STATE_DECLARED :
	  volume->borderWidth = borderWidth;
	  break;
	default :
	  (void)VipVerifyState ( volume->state );
	  VipPrintfExit ( "(volume.c)VipSetBorderWidth" );
	  return(PB);
	}
    }
  return(OK);
}


/*----------------------------------------------------------------------------*/

int			VipResizeBorder

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			borderWidth
)
{
  int oldBorder, diff;
	
  if ( !VipTestExist ( volume ) ) 
    {
      VipPrintfExit ( "(volume.c)VipResizeBorder" );
      return(PB);
    }
  oldBorder =  volume->borderWidth;
  diff = borderWidth - oldBorder;
  if ( oldBorder == borderWidth )
    {
      return( OK );
    }
  switch ( volume->state )
	{ 
	case STATE_FILLED :
    volume->size.x = mVipVolSizeX(volume)-2*diff;
    volume->size.y = mVipVolSizeY(volume)-2*diff;
    volume->size.z = mVipVolSizeZ(volume)-2*diff;
    volume->borderWidth = borderWidth;
#ifdef VIP_CARTO_VOLUME_WRAPPING
    VipVolumeCartoResizeBorder( volume, borderWidth );
#endif
	  break;
	case STATE_ALLOCATED :
	case STATE_DECLARED :
    return VipSetBorderWidth( volume, borderWidth );
	default :
	  (void)VipVerifyState ( volume->state );
	  VipPrintfExit ( "(volume.c)VipResizeBorder" );
	  return(PB);
	}
  return(OK);
}


/* A lot of function giving useful offset to navigate throughout volumes*/

/*----------------------------------------------------------------------------*/
	VipOffsetStruct *VipGetOffsetStructure

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  VipOffsetStruct *offsetStructure;

  if(!VipVerifyAll ( volume ))
    {
      VipPrintfExit ( "(volume.c)VipGetOffsetStructure" );
      return(NULL);
    }

  offsetStructure = (VipOffsetStruct *)VipCalloc( 1, sizeof(VipOffsetStruct),
			       "VipGetOffsetStructure");

  offsetStructure->oFirstPoint = 
    ( ( ( (long)(volume->size.x) + 2 * volume->borderWidth ) *
	( volume->size.y + 2 * volume->borderWidth + 1 ) + 1 ) *
      volume->borderWidth );

  offsetStructure->oLine = (long)(volume->size.x) + 2 * volume->borderWidth;
  offsetStructure->oPointBetweenLine = ( 2 * volume->borderWidth );
  offsetStructure->oSlice = ( ( (long)(volume->size.y) + 2 * 	
				volume->borderWidth ) *
			      ( volume->size.x + 2 * volume->borderWidth ) );
  offsetStructure->oLineBetweenSlice = ( 2 * (long)(volume->borderWidth) *
					 ( volume->size.x + 2 * volume->borderWidth ) );
  offsetStructure->oVolume = ( ( (long)(volume->size.z) + 2 * 
				 volume->borderWidth ) *
			       ( volume->size.y + 2 * volume->borderWidth ) *
			       ( volume->size.x + 2 * volume->borderWidth ) );

  offsetStructure->oLastPoint = offsetStructure->oVolume - offsetStructure->oFirstPoint
    -1;

  offsetStructure->oSliceBetweenVolume = 2 * (long)(volume->borderWidth)
    * offsetStructure->oSlice;

  return(offsetStructure);
}


/*----------------------------------------------------------------------------*/

long			VipOffsetFirstPoint

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if(!VipVerifyAll ( volume )) 
    {
      VipPrintfExit ( "(volume.c)VipOffsetFirstPoint" );
      return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  return ( ( ( (long)(volume->size.x) + 2 * volume->borderWidth ) *
	     ( volume->size.y + 2 * volume->borderWidth + 1 ) + 1 ) *
	   volume->borderWidth );
}


/*----------------------------------------------------------------------------*/

long			VipOffsetLastPoint

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  long offset;

  if(!VipVerifyAll ( volume ))
    {
      VipPrintfExit ( "(volume.c)VipOffsetLastPoint" );
      return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  offset = VipOffsetVolume( volume ) - VipOffsetFirstPoint( volume) - 1;

  return( offset);

}


/*----------------------------------------------------------------------------*/

long			VipOffsetLine

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if(!VipVerifyAll ( volume )) 
    {
      VipPrintfExit ( "(volume.c)VipOffsetLine" );
      return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  return ( (long)(volume->size.x) + 2 * volume->borderWidth );
}





/*----------------------------------------------------------------------------*/

long			VipOffsetPointBetweenLine

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if(!VipVerifyAll ( volume ))
    {
      VipPrintfExit ( "(volume.c)VipOffsetPointBetweenLine" );
      return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  return ( 2 * volume->borderWidth );
}





/*----------------------------------------------------------------------------*/

long			VipOffsetSlice

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if(!VipVerifyAll ( volume )) 
    {
      VipPrintfExit ( "(volume.c)VipOffsetSlice" );
     return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  return ( ( (long)(volume->size.y) + 2 * volume->borderWidth ) *
	   ( volume->size.x + 2 * volume->borderWidth ) );
}





/*----------------------------------------------------------------------------*/

long			VipOffsetLineBetweenSlice

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if(!VipVerifyAll ( volume ))
    {
      VipPrintfExit ( "(volume.c)VipOffsetLineBetweenSlice" );
     return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  return ( 2 * (long)(volume->borderWidth) *
	   ( volume->size.x + 2 * volume->borderWidth ) );
}





/*----------------------------------------------------------------------------*/

long			VipOffsetVolume

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if(!VipVerifyAll ( volume ))
    {
      VipPrintfExit ( "(volume.c)VipOffsetVolume" );
     return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  return ( ( (long)(volume->size.z) + 2 * volume->borderWidth ) *
	   ( volume->size.y + 2 * volume->borderWidth ) *
	   ( volume->size.x + 2 * volume->borderWidth ) );
}

/*----------------------------------------------------------------------------*/

long			VipOffsetSliceBetweenVolume

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if(!VipVerifyAll ( volume ))
    {
      VipPrintfExit ( "(volume.c)VipOffsetSliceBetweenVolume" );
     return(PB);
    } /* Warning, this implies PB should be set to non zero... */

  return ( (long)(2 * volume->borderWidth ) *
	   ( volume->size.y + 2 * volume->borderWidth ) *
	   ( volume->size.x + 2 * volume->borderWidth ) );
}






/*----------------------------------------------------------------------------*/
static 	int			VipSizeOf

/*----------------------------------------------------------------------------*/
(
	int			type
)
{
  switch ( type )
    {
    case U8BIT :
    case S8BIT :
      return ( 1 );
    case U16BIT :
    case S16BIT :
      return ( 2 );
    case U32BIT :
    case S32BIT :
    case VFLOAT :
      return ( 4 );
    case VDOUBLE :
      return ( 8 );
    default :
      return ( 0 );
    }
}





/*----------------------------------------------------------------------------*/

int			VipSizeofType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  int			size;
	
  if ( !VipVerifyType ( volume->type ) ) 
    {
      VipPrintfExit ( "(volume)VipSizeofType" );
     return(PB);
    }

  size = VipSizeOf ( volume->type );
  return ( size); 
}


/**********************************/
/*Procedures for volume management*/
/**********************************/

/*----------------------------------------------------------------------------*/
Volume *VipAllocVolumeStructure()
  {
    Volume *newb;

    newb = (Volume *) VipCalloc ( 1, sizeof(Volume), "VipAllocVolumeStructure" );
    if(newb==PB) return(PB);

#ifdef VIP_CARTO_VOLUME_WRAPPING
    newb->carto = NULL;
#endif
    newb->shfj = (SHFJ_private_struct *)VipCalloc ( 1, sizeof(SHFJ_private_struct), "VipAllocVolumeStructure" );
    if(newb->shfj==PB) return(PB);

    newb->enst = (ENST_private_struct *)VipCalloc ( 1, sizeof(ENST_private_struct), "VipAllocVolumeStructure" );
    if(newb->enst==PB) return(PB);

#ifdef VIP_CARTO_VOLUME_WRAPPING
    VipVolumeCartoAllocStruct( newb );
#endif

    return(newb);

  }
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

Volume	    *VipDeclare3DVolumeStructure
	
/*----------------------------------------------------------------------------*/
(
    int			xSize,
    int			ySize,
    int			zSize,
    float		xVoxelSize,
    float		yVoxelSize,
    float		zVoxelSize,
    int			type,
    char    *name,
    int			borderWidth
 )
{
  static long	id = 0;
  Volume	    *newVolume;
  char message[VIP_NAME_MAXLEN];
	
  if ( name == NULL )
    {
      VipPrintfError ( "Volume name is a NULL pointer" );
      return ( NULL );
    }
/*  if ( strlen ( name ) == 0 ) 
    {
      VipPrintfError ( "Volume name is empty" );
      return ( NULL );
    } */ /* supprime par F. Poupon, car on peut vouloir passer "" et remplir apres */
  if ( strlen ( name ) > VIP_NAME_MAXLEN ) 
    {
      sprintf(message,"Volume name is too long: %s", name);
      VipPrintfError (message );
      (void) fprintf ( stderr, 
		       "Volume names must be no longer than %d characters.\n", (int)VIP_NAME_MAXLEN );
      return ( NULL );
    }

  newVolume = (Volume *) VipAllocVolumeStructure();
  if(newVolume == PB) return(PB);

  /* necessary to verify consistancy with the future state : */
  newVolume->data = NULL;
  newVolume->tab = NULL;
  newVolume->shfj->start_time = NULL;
  newVolume->shfj->dur_time   = NULL;


#ifndef VIP_CARTO_VOLUME_WRAPPING
  newVolume->swapfile = NULL;
#endif

  newVolume->size3Db = 0;

  /* necessary to set caracteristics : */
  newVolume->state = STATE_DECLARED;
  VipSetType ( newVolume, type );
  VipSet3DSize ( newVolume, xSize, ySize, zSize);
  VipSet3DVoxelSize ( newVolume, xVoxelSize, yVoxelSize, zVoxelSize);
  strcpy ( newVolume->name, name);
  VipSetBorderWidth ( newVolume, borderWidth );

  /* reference (Tivoli specific)*/
  (void) strcpy ( newVolume->ref.name, "" );
  newVolume->ref.x = 0;
  newVolume->ref.y = 0;
  newVolume->ref.z = 0;
  newVolume->ref.t = 0;

  /* if all is right : */
  newVolume->id = ++id;


  return ( newVolume );
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

Volume	    *VipDeclare4DVolumeStructure
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	int			tSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	float		tVoxelSize,
	int			type,
	char    *name,
	int			borderWidth
)
{
  static long	id = 0;
  Volume	    *newVolume;
  char message[VIP_NAME_MAXLEN];
	
  if ( name == NULL )
    {
      VipPrintfError ( "Volume name is a NULL pointer" );
      return ( NULL );
    }
  /*if ( strlen ( name ) == 0 ) 
    {
      VipPrintfError ( "Volume name is empty" );
      return ( NULL );
    } */ /* supprime par F. Poupon, car on peut vouloir passer "" et remplir apres */
  if ( strlen ( name ) > VIP_NAME_MAXLEN ) 
    {
      sprintf(message,"Volume name is too long: %s", name);
      VipPrintfError (message );
      (void) fprintf ( stderr, 
		       "Volume names must be no longer than %d characters.\n", (int)VIP_NAME_MAXLEN );
      return ( NULL );
    }

  newVolume = (Volume *) VipAllocVolumeStructure();
  if(newVolume == PB) return(PB);

  /* necessary to verify consistancy with the future state : */
  newVolume->data = NULL;
  newVolume->tab = NULL;
  newVolume->shfj->start_time = (int *)VipCalloc(tSize,sizeof(int),
                                         "VipDeclare4DVolumeStructure");
  if(newVolume->shfj->start_time==NULL) return((Volume *) NULL);
  newVolume->shfj->dur_time   = (int *)VipCalloc(tSize,sizeof(int),
                                         "VipDeclare4DVolumeStructure");
  if(newVolume->shfj->dur_time==NULL) return((Volume *) NULL);

#ifndef VIP_CARTO_VOLUME_WRAPPING
  newVolume->swapfile = NULL;
#endif

  newVolume->size3Db = 0;

  /* necessary to set caracteristics : */
  newVolume->state = STATE_DECLARED;
  VipSetType ( newVolume, type );
  VipSet4DSize ( newVolume, xSize, ySize, zSize, tSize);
  VipSet4DVoxelSize ( newVolume, xVoxelSize, yVoxelSize, zVoxelSize, tVoxelSize);
  strcpy ( newVolume->name, name);
  VipSetBorderWidth ( newVolume, borderWidth );

  /* reference (Tivoli specific)*/
  (void) strcpy ( newVolume->ref.name, "" );
  newVolume->ref.x = 0;
  newVolume->ref.y = 0;
  newVolume->ref.z = 0;
  newVolume->ref.t = 0;

  /* if all is right : */
  newVolume->id = ++id;


  return ( newVolume );
}

/*----------------------------------------------------------------------------*/
	Volume	    *VipDuplicateVolumeStructure

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	char    *name
)
{
    int s;
    Volume	    *newVolume;

    if ( !VipVerifyAll ( volume ) )
	{
	    VipPrintfError ( "(volume)VipDuplicateVolumeStructure" );
	    return ( NULL );
	}

    newVolume = VipDeclare4DVolumeStructure (
					     volume->size.x, volume->size.y, volume->size.z, volume->size.t,
					     volume->voxelSize.x, volume->voxelSize.y, volume->voxelSize.z, 
					     volume->voxelSize.t, volume->type, name, volume->borderWidth );

    /*tivoli specific*/

    (void) strcpy ( newVolume->ref.name, volume->ref.name );
    newVolume->ref.x = volume->ref.x;
    newVolume->ref.y = volume->ref.y;
    newVolume->ref.z = volume->ref.z;
    newVolume->ref.t = volume->ref.t;


    if ( volume->shfj )
    {
    newVolume->shfj->unite = volume->shfj->unite; /*on espere que ca marche*/
    newVolume->shfj->VmdType = volume->shfj->VmdType;
    newVolume->shfj->scale = volume->shfj->scale;
    newVolume->shfj->offset = volume->shfj->offset;

    if (newVolume->size.t > 1)
	{
	    if (!newVolume->shfj->start_time)
		newVolume->shfj->start_time = (int *)VipCalloc(newVolume->size.t,sizeof(int),"");
	    if (!newVolume->shfj->dur_time)
		newVolume->shfj->dur_time = (int *)VipCalloc(newVolume->size.t,sizeof(int),"");
            if ( volume->shfj->start_time && volume->shfj->dur_time )
            {

              for (s=0; s<newVolume->size.t; s++)
		{
		    newVolume->shfj->start_time[s] = volume->shfj->start_time[s];
		    newVolume->shfj->dur_time[s] = volume->shfj->dur_time[s];
		}

            }
            else
            {

              for (s=0; s<newVolume->size.t; s++)
		{
		    newVolume->shfj->start_time[s] = s;
		    newVolume->shfj->dur_time[s] = 1;
		}

            }

	}
     }

#ifdef VIP_CARTO_VOLUME_WRAPPING
    VipVolumeCartoCopyStruct( volume, newVolume );
#endif

    return ( newVolume );
}

/*----------------------------------------------------------------------------*/
void      *VipCopyVolumeHeader

/*----------------------------------------------------------------------------*/
(
        Volume      *sourceVolume,
        Volume      *destVolume
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
    VipVolumeCartoCopyHeader( sourceVolume, destVolume );
#endif
}

/*----------------------------------------------------------------------------*/
	int			VipAllocateVolumeData
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  int   nxb, nyb, nzb;
  int   bW;

#ifndef VIP_CARTO_VOLUME_WRAPPING
  long  mmThreshold;
  char *mmpfx = NULL, *tmpdir;

  swapfileNode		*p;
  int                    file;
#endif

  if ( !VipVerifyAll ( volume ) ||
       !VipTestState ( volume, STATE_DECLARED ) )
    {
      VipPrintfExit ( "(volume.c)VipAllocateVolumeData" );
      return(PB);
    }
  bW = VipBorderWidthOf(volume);
  nxb = volume->size.x + 2 * bW;
  nyb = volume->size.y + 2 * bW;
  nzb = volume->size.z + 2 * bW;
  volume->size3Db = volume->size.t*nzb * nyb * nxb * VipSizeofType ( volume );

#ifdef VIP_CARTO_VOLUME_WRAPPING
  /* for now, release any carto::Volume and switch to regular Vip volume */
  /* VipVolumeCartoFreeStruct( volume ); */
  return VipVolumeCartoAllocData( volume );

#else

  /*Warning, exit() call remains here... */
  assert(volume->swapfile == NULL);
  if ( VipGetRTConfigLong("VIP_MMAP_THRESHOLD", &mmThreshold) != PB)
    if ( (mmpfx = VipGetRTConfigString("VIP_MMAP_PREFIXDIR")) != PB) 
      VipMemoryMapping = VTRUE;

#ifndef _WIN32
  /* TODO: memory mapping is different on windows (FileMapping functions) */
  if ( (VipMemoryMapping == VTRUE) && (volume->size3Db > (size_t)mmThreshold) )
    {
      VipPrintfInfo ( "Memory mapping" );
      (void) fprintf ( stdout, "Creating swap file for volume '%s'.\n",
			 volume->name );
      
     
      tmpdir = VipCalloc(strlen(mmpfx)+1+strlen("TMPDIR="), sizeof(char),"");
      sprintf(tmpdir,"TMPDIR=%s", mmpfx);
      if (tmpdir == NULL)
	{
	  perror(NULL);
	  exit(EXIT_FAILURE);
	}
      putenv(tmpdir);
      VipFree(tmpdir); VipFree(mmpfx);

      volume->swapfile = tempnam(NULL, "Vip");
      if (volume->swapfile == NULL) {
	perror(NULL);
	exit(EXIT_FAILURE);
      }
      file = open(volume->swapfile, O_RDWR | O_CREAT | O_TRUNC, 666);
      if (file == -1) {
	perror(volume->swapfile);
	exit(EXIT_FAILURE);
      }
#ifndef _WIN32
      if (fchmod(file, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH))
	{
	  perror(volume->swapfile);
	  exit(EXIT_FAILURE);
	}
#endif
  
      /* add it at the start of the list */
      p = VipMalloc(sizeof(swapfileNode),"VipAllocateVolumeData(p)");
      if(p==NULL) return(PB);

      p->swapfile = volume->swapfile;
      p->next = swapfileList;
      swapfileList = p;

      for (p = swapfileList; p != NULL; p = p->next);
      p = VipMalloc(sizeof(swapfileNode),"VipAllocateVolumeData(p')");
      if(p==NULL) return(PB);

      p->next = NULL;
      p->swapfile = volume->swapfile;
#ifndef _WIN32
      /* signal handling is different on windows */
      if (!swapfileFlag) {
	if (
	    signal(SIGHUP, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGINT, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGQUIT, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGILL, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGTRAP, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGABRT, swapfileAtSignal) == SIG_ERR ||
	    /* 	    signal(SIGEMT, swapfileAtSignal) == SIG_ERR || */
	    signal(SIGFPE, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGBUS, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGSEGV, swapfileAtSignal) == SIG_ERR ||
	    /*	    signal(SIGSYS, swapfileAtSignal) == SIG_ERR ||*/
	    signal(SIGPIPE, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGALRM, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGTERM, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGUSR1, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGUSR2, swapfileAtSignal) == SIG_ERR ||
	    /* signal(SIGPOLL, swapfileAtSignal) == SIG_ERR || */
	    signal(SIGPROF, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGXCPU, swapfileAtSignal) == SIG_ERR ||
	    signal(SIGXFSZ, swapfileAtSignal) == SIG_ERR)
	  {
	    perror(NULL);
	    exit(EXIT_FAILURE);
	  }
#endif	/* _WIN32 */
	if (atexit(&swapfileAtExit)) {
	  fputs("'atexit()' problem\n", stderr);
	  exit(EXIT_FAILURE);
	}
	swapfileFlag = 1;
      }

      if (ftruncate(file, volume->size3Db)) {
	perror(volume->swapfile);
	exit(EXIT_FAILURE);
      }

      volume->data = mmap(
			  0,
			  volume->size3Db,
			  PROT_WRITE | PROT_READ,
			  MAP_SHARED,
			  file,
			  0);
      if (volume->data == MAP_FAILED) {
	perror(NULL);
	exit(EXIT_FAILURE);
      }

      if (close(file)) {
	perror(volume->swapfile);
	exit(EXIT_FAILURE);
      }
    }
  else
#endif	/* _WIN32 */
    volume->data = (char *) VipCalloc((volume->size.x+2*volume->borderWidth)
				      * (volume->size.y+2*volume->borderWidth)
				      * (volume->size.z+2*volume->borderWidth)
				      * volume->size.t,VipSizeofType ( volume ),"VipAllocateVolumeData(data)");
  if(volume->data==NULL) return(PB);

  /*bnew: adapted to make first point at [0][0][0]*/
  volume->tab = NULL;
  /*******************************************************
  ** A ete mis en commentaire car fuite de memoire
  ** j.-F. Mangin, F. Poupon  le 05/03/1999
  bytesPerVoxel = VipSizeofType ( volume );
        
  nyb1 = volume->size.y + bW;
  nzb1 = volume->size.z + bW;

  oL = VipOffsetLine ( volume ) * bytesPerVoxel;

  volume->tab = (char ***) VipMalloc ( nzb * sizeof(char **),
				       "VipAllocateVolumeData(tab)" );
  if(volume->tab==NULL) 
    {
      if (volume->data) VipFree(volume->data);
      return(PB);
    }

  volume->tab = volume->tab + bW;

  ptrData = volume->data; *//* + bW*bytesPerVoxel; <-- Y'a un bug (D.Riviere) */

  /*for ( iz = -bW; iz < nzb1; ++iz )
    {
      volume->tab[iz] = (char **) VipMalloc ( nyb * sizeof(char *), 
					      "VipAllocateVolumeData(tab)");
      if (volume->tab[iz]==NULL) 
	{
	  for (;iz--;)
	    {
	      if (volume->tab[iz]) VipFree(volume->tab[iz]);
	    }
	  if (volume->tab) VipFree(volume->tab);
	  if (volume->data) VipFree(volume->data);
	  return(PB);
	}

      volume->tab[iz] = volume->tab[iz] + bW;
      for ( iy = -bW; iy < nyb1; ++iy )
	{
	  volume->tab[iz][iy] = ptrData;
	  ptrData += oL;
	}
    }
  **********************************************************/

  VipSetState ( volume, STATE_ALLOCATED );
  return(OK);

#endif // VIP_CARTO_VOLUME_WRAPPING
}


#ifndef VIP_CARTO_VOLUME_WRAPPING
#ifndef _WIN32
/*---------------------------------------------------------------------------*/
static 
void swapfileAtSignal
/*---------------------------------------------------------------------------*/
(
	int a
)
{
  VipPrintfError ( "Memory mapping" );
  fputs("Caught ", stderr);
  switch(a) {
  case SIGHUP:
    fputs("SIGHUP", stderr);
    break;
  case SIGINT:
    fputs("SIGINT", stderr);
    break;
  case SIGQUIT:
    fputs("SIGQUIT", stderr);
    break;
  case SIGILL:
    fputs("SIGILL", stderr);
    break;
  case SIGTRAP:
    fputs("SIGTRAP", stderr);
    break;
  case SIGABRT:
    fputs("SIGABRT", stderr);
    break;
/*   case SIGEMT: */
/*     fputs("SIGEMT", stderr); */
/*     break; */
  case SIGFPE:
    fputs("SIGFPE", stderr);
    break;
  case SIGKILL:
    fputs("SIGKILL", stderr);
    break;
  case SIGBUS:
    fputs("SIGBUS", stderr);
    break;
  case SIGSEGV:
    fputs("SIGSEGV", stderr);
    break;
    /*  case SIGSYS:
    fputs("SIGSYS", stderr);
    break;*/
  case SIGPIPE:
    fputs("SIGPIPE", stderr);
    break;
  case SIGALRM:
    fputs("SIGALRM", stderr);
    break;
  case SIGTERM:
    fputs("SIGTERM", stderr);
    break;
  case SIGUSR1:
    fputs("SIGUSR1", stderr);
    break;
  case SIGUSR2:
    fputs("SIGUSR2", stderr);
    break;
/*  case SIGPOLL:
    fputs("SIGPOLL", stderr);
    break; */
  case SIGPROF:
    fputs("SIGPROF", stderr);
    break;
  case SIGXCPU:
    fputs("SIGXCPU", stderr);
    break;
  case SIGXFSZ:
    fputs("SIGXFSZ", stderr);
    break;
  default:
    fputs("unknown", stderr);
    break;
  }
  fputs(" signal\n", stderr);

  switch(a) {
  case SIGHUP:
  case SIGINT:
  case SIGKILL:
  case SIGPIPE:
  case SIGALRM:
  case SIGTERM:
    exit(EXIT_FAILURE);
    break;
  default:
    swapfileAtExit();
    break;
  }
}
#endif	/* _WIN32 */

/*---------------------------------------------------------------------------*/

	static void swapfileAtExit
	
/*---------------------------------------------------------------------------*/
(
	void
)
{
  swapfileNode *p;

  if ( swapfileList != NULL )
    {
      fputs("Cleaning up swap files...\n", stderr);
      for (p = swapfileList; p != NULL; p = p->next)
	if (unlink(p->swapfile))
	  perror(p->swapfile);
    }
}
#endif


/*---------------------------------------------------------------------------*/

Volume	    *VipCreate3DVolume
	
/*---------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	int			type,
	char    *name,
	int			borderWidth
)
{
  Volume	    *newVolume;
	
  newVolume = VipDeclare4DVolumeStructure (
					   xSize, ySize, zSize, 1,
					   xVoxelSize, yVoxelSize, zVoxelSize, 1.0,
					   type, name, borderWidth );
  if(newVolume==NULL) return((Volume *)NULL);

  if (VipAllocateVolumeData ( newVolume ) == PB) 
    {
      VipFree(newVolume);
      return((Volume *)NULL);
    }
	
  return ( newVolume );
}

/*----------------------------------------------------------------------------*/

Volume	    *VipCreate4DVolume
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	int			tSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	float		tVoxelSize,
	int			type,
	char    *name,
	int			borderWidth
)
{
  Volume	    *newVolume;
	
  newVolume = VipDeclare4DVolumeStructure (
					   xSize, ySize, zSize, tSize,
					   xVoxelSize, yVoxelSize, zVoxelSize, tVoxelSize,
					   type, name, borderWidth );

  if(newVolume==NULL) return(NULL);

  if (VipAllocateVolumeData ( newVolume ) == PB) 
    {
      VipFree(newVolume);
      return(NULL);
    }
	
  return ( newVolume );
}


/*----------------------------------------------------------------------------*/

int			VipTransferVolumeData 
/*
VipTransferVolumeData ( fromVolume, toVolume );
Copies the contents of 'fromVolume' to 'toVolume' (when
enough place is present).  It does not copy the borders.
*/
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volumeR,
	Volume	    *volumeW
)
{
  int		nx,
    iy, ny,
    iz, nz,
    bytesPerVoxel,
    oLR, oLW,
    oCR, oCW;
  char		*ptrDataR,
    *ptrDataW;
	
  if ( !VipVerifyAll ( volumeR ) ||
       !VipVerifyAll ( volumeW ) ||
       !VipTestFilled ( volumeR ) ||
       !VipTestEqualType ( volumeR, volumeW ) )
    {
      VipPrintfExit ( "(volume.c)VipTransferVolumeData" );
      return(PB);
    }

#ifdef VIP_CARTO_VOLUME_WRAPPING
  if( VipVolumeCartoTransferVolumeData( volumeR, volumeW ) == OK )
    return OK;
#endif

  /* function 'VipTestState' unused to prevent an unexpectable error message */
  if ( volumeW->state == STATE_FILLED )
    {/*
      VipPrintfWarning ( 
			"Destination volume is filled: its data may be lost" );
      (void) fprintf ( stderr, "\tduring 'transferVolumeData'." );*/
    }
        
  if ( volumeW->state == STATE_DECLARED )
    if (VipAllocateVolumeData ( volumeW ) == PB) return (PB);

  if ( volumeW->size.x > volumeR->size.x ||
       volumeW->size.y > volumeR->size.y ||
       volumeW->size.z > volumeR->size.z )
    {
      VipPrintfWarning ( "Destination volume is over-dimensioned" );
      (void) fprintf ( stderr, 
		       "\tThe destination volume will not be completely filled." );
      (void) fprintf ( stderr, "\n\tIn VipTransferVolumeData." );
    }
        
  if ( volumeW->size.x < volumeR->size.x ||
       volumeW->size.y < volumeR->size.y ||
       volumeW->size.z < volumeR->size.z )
    {
      VipPrintfWarning ( "Source volume is over-dimensioned." );
      (void) fprintf ( stderr, 
		       "\tThe source volume will not be completely copied." );
      (void) fprintf ( stderr, "\n\tIn VipTransferVolumeData." );
    }
        

  bytesPerVoxel = VipSizeofType ( volumeR );

  nx = mVipMin ( volumeR->size.x, volumeW->size.x );
  ny = mVipMin ( volumeR->size.y, volumeW->size.y );
  nz = mVipMin ( volumeR->size.z, volumeW->size.z );

  oCR = ( volumeR->size.x - nx + VipOffsetPointBetweenLine ( volumeR ) )
    * bytesPerVoxel;
  oCW = ( volumeW->size.x - nx + VipOffsetPointBetweenLine ( volumeW ) )
    * bytesPerVoxel;
  oLR = ( ( volumeR->size.y - ny ) * VipOffsetLine ( volumeR ) +
	  VipOffsetLineBetweenSlice ( volumeR ) ) * bytesPerVoxel;
  oLW = ( ( volumeW->size.y - ny ) * VipOffsetLine ( volumeW ) +
	  VipOffsetLineBetweenSlice ( volumeW ) ) * bytesPerVoxel;

  ptrDataR = volumeR->data + VipOffsetFirstPoint ( volumeR ) * bytesPerVoxel;
  ptrDataW = volumeW->data + VipOffsetFirstPoint ( volumeW ) * bytesPerVoxel;

  nx *= bytesPerVoxel;
  oCR += nx;
  oCW += nx;

  for ( iz = 0; iz < nz; iz++ )
    {
      for ( iy = 0; iy < ny; iy++ )
	{
	  memcpy ( (void*)ptrDataW, (void*)ptrDataR, nx );
	  ptrDataR += oCR;
	  ptrDataW += oCW;
	}
      ptrDataR += oLR;
      ptrDataW += oLW;
    }

  VipSetState ( volumeW, STATE_FILLED );
	
  return(OK);
}

/*----------------------------------------------------------------------------*/
	Volume	    *VipCopyVolume
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	char    *name
)
{
  Volume	    *newVolume;

  if ( !VipVerifyAll ( volume ) )
    {
      VipPrintfError ( "(volume.c)VipCopyVolume" );
      return(NULL);
    }
  newVolume = VipDuplicateVolumeStructure ( volume, name );

  if(newVolume==NULL) return(NULL);

  if (VipTransferVolumeData ( volume, newVolume ) == PB)
    {
      VipFreeVolume( newVolume );
      VipFree( newVolume );
      return(NULL);
    }

  return ( newVolume );
}

/*----------------------------------------------------------------------------*/
	int			VipFreeVolumeData

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
#ifndef VIP_CARTO_VOLUME_WRAPPING
  swapfileNode		*p;
  swapfileNode		*pold;
#endif

  if ( !VipVerifyAll ( volume ) ||
       !VipTestState ( volume, STATE_FILLED ) )
    {
      VipPrintfExit ( "(volume.c)VipFreeVolumeData" );
      return(PB);
    }
  volume->tab = NULL;

#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoFreeData( volume );
#else

#ifndef _WIN32
    if (volume->swapfile)
      {
        if (munmap((caddr_t) volume->data, volume->size3Db))
          {
            perror(NULL);
            VipPrintfExit ( "(volume.c)VipFreeVolumeData(MEMORYMAP)" );
            exit(EXIT_FAILURE);
          }

        assert(swapfileList);

        if (swapfileList->swapfile == volume->swapfile)
          {
            p = swapfileList;
            swapfileList = swapfileList->next;
            VipFree(p);
          }
        else
          {
            assert(swapfileList->next);
            for (pold = swapfileList, p = swapfileList->next;
                 p != NULL;
                 pold = p, p = p->next)
              {
                if (p->swapfile == volume->swapfile)
                  {
                    pold->next = p->next;
                    VipFree(p);
                    break;
                  }
              }
          }
                

        if (unlink(volume->swapfile))
          {
            perror(volume->swapfile);
            VipPrintfExit ( "(volume.c)VipFreeVolumeData(MEMORYMAP)" );
            exit(EXIT_FAILURE);
          }
        VipFree(volume->swapfile);
        volume->swapfile = NULL;
      }
    else
#endif	/* _WIN32 */
      VipFree(volume->data);

  volume->data = NULL;
  volume->size3Db = 0;

  VipSetState ( volume, STATE_DECLARED );

  return(OK);
#endif // VIP_CARTO_VOLUME_WRAPPING
}

/*----------------------------------------------------------------------------*/
	int			VipFreeVolume

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  if ( volume==NULL )
    VipPrintfExit ( "(volume.c)VipFreeVolume" );
  if ( volume->state == STATE_FREED )
    {        
      VipPrintfWarning ( "You try to free a volume that was already freed" );
      (void) fprintf ( stderr, "\tIt was called '%s' before",
                       volume->name );
      (void) fprintf ( stderr, "\n\tI do not change anything.\n" );
    }
  else
    {
      switch ( volume->state )
	{
	case STATE_DECLARED:
            
	  VipPrintfWarning ( "You try to free a volume that was not allocated" );
	  (void) fprintf ( stderr, "\tVolume '%s' was only declared.",
			   volume->name );
	  (void) fprintf ( stderr, "\n\tBut OK, I'll free it for you.\n" );
	  break;
            
	case STATE_ALLOCATED:
            
	  VipPrintfWarning ( "You try to free a volume that was not filled" );
	  (void) fprintf ( stderr, "\tVolume '%s' was only allocated.",
			   volume->name );
	  (void) fprintf ( stderr, "\n\tBut OK, I'll free it for you.\n" );
	  volume->state = STATE_FILLED;
	  VipFreeVolumeData ( volume );
	  break;
            
	case STATE_FILLED:

	  VipFreeVolumeData ( volume );
	  break;

	default:

	  VipVerifyState ( volume->state );
	  VipPrintfExit ( "(volume.c)VipFreeVolume" );
	  return(PB);
	}
          
          
      volume->size.x = volume->size.y = volume->size.z = volume->size.t = 0;
      volume->type = 0;
      volume->voxelSize.x = volume->voxelSize.y = volume->voxelSize.z = 
	volume->voxelSize.t = 0.;
      strcpy ( volume->ref.name, "" );
      volume->ref.x = 0;
      volume->ref.y = 0;
      volume->ref.z = 0;
      volume->ref.t = 0;


      if (volume->shfj)
        {
	  if (volume->shfj->start_time) VipFree(volume->shfj->start_time);
	  if (volume->shfj->dur_time)   VipFree(volume->shfj->dur_time);
	  VipFree(volume->shfj);
        }
          
      if (volume->enst)
	{
	  VipFree(volume->enst);
	}

      VipSetState ( volume, STATE_FREED );
    }
  /* possible Modification relatively to Tivoli, to free the volume structure,
     Is it a good Idea? It's not really important I think but perhaps It
     would be better for the VIP/IDL interface. Anyway, it's not a big
     amount of memory
     */
  return(OK);
}





/*==========================*
 |                          |
 |  input-output functions  |
 |                          |
 *==========================*/

/*----------------------------------------------------------------------------*/
	int	    VipTestImageFileExist				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING

  return VipTestCartoImageFileExist( ima_name );

#else

    int ima_format;
    char extension[16];
    FILE *header_file;
    char header_name[1024];

    ima_format = PB;

    if ( ima_name == NULL || strlen ( ima_name ) == 0 )
	{
	    VipPrintfError ( "Invalid image name (NULL or length 0) (VipTestImageFileExist)" );
	    return(PB);
	}
    strcpy(header_name,ima_name);
    strcpy(extension,".dim"); /*JFM, 25-9-2000 Tivoli becomes first*/
    strcat(header_name,extension);
    header_file = fopen(header_name,"r");
    if(header_file!=NULL)
	{
	    ima_format = TIVOLI;
	    fclose(header_file);
	}
    else
	{
	    strcpy(header_name,ima_name);
	    strcpy(extension,".vhdr");
	    strcat(header_name,extension);
	    header_file = fopen(header_name,"r");
	    if(header_file!=NULL)
		{
		    ima_format = VIDA;
		    fclose(header_file);
		}
	    else  
		{
		    strcpy(header_name,ima_name);
		    strcpy(extension,".hdr");
		    strcat(header_name,extension);
		    header_file = fopen(header_name,"r"); 
	  
		    if(header_file!=NULL)
			{
			    ima_format = SPM;
			    fclose(header_file);
			}
		    else
			{

			  strcpy(header_name,ima_name);
			  strcpy(extension,".mnc");
			  strcat(header_name,extension);
			  header_file = fopen(header_name,"r"); 
	  
			  if(header_file!=NULL)
			    {
			      ima_format = MINC;
			      fclose(header_file);
			    }
			  else
			    {	
			      return(PB);
			    }
			}
		}
	}

    return(ima_format);
#endif
}

/*----------------------------------------------------------------------------*/
	int	    VipGetImageFormat				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
)
{

  int ima_format;

  char extension[16];
  FILE *header_file;
  char header_name[1024];

  ima_format = PB;

  if ( ima_name == NULL || strlen ( ima_name ) == 0 )
    {
      VipPrintfError ( "Invalid image name (NULL or length 0) (VipGetImageFormat)" );
      return(PB);
    }

  strcpy(header_name,ima_name);
  strcpy(extension,".vhdr");
  strcat(header_name,extension);
  header_file = fopen(header_name,"r");
  if(header_file!=NULL)
    {
      ima_format = VIDA;
    }
  else
    {
      strcpy(header_name,ima_name);
      strcpy(extension,".dim");
      strcat(header_name,extension);
      header_file = fopen(header_name,"r");
      if(header_file!=NULL)
	{
	  ima_format = TIVOLI;
	}
      else
	  {
	      strcpy(header_name,ima_name);
	      strcpy(extension,".hdr");
	      strcat(header_name,extension);
	      header_file = fopen(header_name,"r"); 
	  
	      if(header_file!=NULL)
		  {
		      ima_format = SPM;
		  }
	      else
		  {
		    strcpy(header_name,ima_name);
		    strcpy(extension,".mnc");
		    strcat(header_name,extension);
		    header_file = fopen(header_name,"r"); 
	  
		    if(header_file!=NULL)
		      {
			ima_format = MINC;
		      }
		    else
		      {
			VipPrintfError ( "Image format or read error" );
			ima_format = PB;
		      }
		  }
	  }
    }

  if(ima_format!=PB) fclose(header_file);

  return(ima_format);
}

/*----------------------------------------------------------------------------*/
	Volume	    *VipReadVolume		
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
)
{
  return (VipReadVolumeWithBorder(ima_name,0));
}

	

 /*----------------------------------------------------------------------------*/
	Volume	    *VipReadVolumeWithBorder			
/* Switch towards format dedicated I/O procedures	*/
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
)
{
#ifndef VIP_CARTO_VOLUME_WRAPPING
  VIP_DEC_VOLUME(vol);
  int ima_format;
#endif

  char error[VIP_NAME_MAXLEN];


  if ( !VipVerifyBorderWidth ( borderWidth ) )
    {
      sprintf(error,"(VipReadVolumeWithBorder)Bad border width (%d) for image: %s",borderWidth,ima_name);
      VipPrintfError(error);
      return(NULL);
    }
		
  if ( ima_name == NULL || strlen ( ima_name ) == 0 )
    {
      VipPrintfError ( "Invalid image name (NULL or length 0)" );
      return(NULL);
    }
  if ( strlen ( ima_name ) > (size_t)VIP_NAME_MAXLEN )
    {
      VipPrintfError ( "Invalid image name (more than 1024 chars...)" );
      return(NULL);
    }

#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoRead( ima_name, borderWidth, ANY_TYPE, -1 );
#else

  ima_format = VipGetImageFormat(ima_name);

  if(ima_format==TIVOLI)
    {
      vol = VipReadTivoliVolumeWithBorder(ima_name, borderWidth);
    }
  else if(ima_format==VIDA)
    {
      vol = VipReadVidaVolumeWithBorder(ima_name , borderWidth);
    }
  else if(ima_format==SPM)
    {
      vol = VipReadSPMVolumeWithBorder(ima_name , borderWidth);
    }
  else if(ima_format==MINC)
    {
      vol = VipReadMincVolumeWithBorder(ima_name , borderWidth);
    }  else
    {
      sprintf(error,"(VipReadVolumeWithBorder)Can not read this image: %s", ima_name);
      VipPrintfError(error);
      return(NULL);
    }

  return(vol);
#endif // VIP_CARTO_VOLUME_WRAPPING
}

/*----------------------------------------------------------------------------*/

Volume	    *VipReadTivoliVolume
/*switch toward Tivoli I/O*/			
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipReadVolume( ima_name );
#else
  VIP_DEC_VOLUME(vol);

  vol = VipReadTivoliVolumeWithBorder(ima_name,0);
  return(vol);
#endif
}
/*----------------------------------------------------------------------------*/

Volume	    *VipReadTivoliVolumeWithBorder
/*switch toward Tivoli I/O*/			
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoRead( ima_name, borderWidth, ANY_TYPE, -1 );
#else

  VIP_DEC_VOLUME(vol);
  char message[VIP_NAME_MAXLEN];

  printf("Reading Tivoli/GIS image format...\n");


#ifdef TIVOLI_LIB
	
  vol = readVolume(ima_name,borderWidth);
  if(vol!=PB)
      {
	  printf("Volume geometry:\n");
	  printf("X size:%d, Y size:%d, Z size:%d, T size:%d\n",
		 mVipVolSizeX(vol),mVipVolSizeY(vol),mVipVolSizeZ(vol),mVipVolSizeT(vol));
	  printf("X voxsize:%5.3fmm, Y voxsize:%5.3fmm, Z voxsize:%5.3fmm, T voxsize:%5.3fs\n",
		 mVipVolVoxSizeX(vol),mVipVolVoxSizeY(vol),mVipVolVoxSizeZ(vol),mVipVolVoxSizeT(vol));
      }
  sprintf(message,"pour warning");

  return(vol);
#else

  sprintf(message,"image %s is in Tivoli format and your Vip version\n\
has not been compiled to be Tivoli compatible (use #define TIVOLI_LIB)",
	  ima_name);
  VipPrintfError(message);
  return(NULL);

#endif
#endif // VIP_CARTO_VOLUME_WRAPPING
}


/*----------------------------------------------------------------------------*/

Volume      *VipReadVidaVolume
/*switch toward Tivoli I/O*/
/*----------------------------------------------------------------------------*/
(
        const char    *ima_name
)
{
  VIP_DEC_VOLUME(vol);

  vol = VipReadVidaVolumeWithBorder(ima_name,0);
  return(vol);
}

/*----------------------------------------------------------------------------*/
	Volume	    *VipReadVidaVolumeWithBorder
/* the volume is systematically return rescaled in S16BIT,
To get perfect rescaling in VFLOAT, use VipReadVidaFloatVolume */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoRead( ima_name, borderWidth, S16BIT, -1 );
#else

  VIP_DEC_VOLUME(vol);

  printf("Reading VIDA image format to short 16 bits volume...\n");

  vol = ReadVidaVolume(ima_name, borderWidth);
  if(vol!=PB)
      {
	  printf("Volume geometry:\n");
	  printf("X size:%d, Y size:%d, Z size:%d, T size:%d\n",
		 mVipVolSizeX(vol),mVipVolSizeY(vol),mVipVolSizeZ(vol),mVipVolSizeT(vol));
	  printf("X voxsize:%5.3fmm, Y voxsize:%5.3fmm, Z voxsize:%5.3fmm, T voxsize:%5.3fs\n",
		 mVipVolVoxSizeX(vol),mVipVolVoxSizeY(vol),mVipVolVoxSizeZ(vol),mVipVolVoxSizeT(vol));
      }
  return(vol);
#endif // VIP_CARTO_VOLUME_WRAPPING
}


/*----------------------------------------------------------------------------*/

Volume      *VipReadSPMVolume
/*switch toward Tivoli I/O*/
/*----------------------------------------------------------------------------*/
(
        const char    *ima_name
)
{
  VIP_DEC_VOLUME(vol);

  vol = VipReadSPMVolumeWithBorder(ima_name,0);
  return(vol);
}

/*----------------------------------------------------------------------------*/
	Volume	    *VipReadSPMVolumeWithBorder
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoRead( ima_name, borderWidth, ANY_TYPE, -1 );
#else

  VIP_DEC_VOLUME(vol);

  printf("Reading SPM image format...\n");

  vol = ReadSPMVolume(ima_name, borderWidth);

  if(vol!=PB)
      {
	  printf("Volume geometry:\n");
	  printf("X size:%d, Y size:%d, Z size:%d, T size:%d\n",
		 mVipVolSizeX(vol),mVipVolSizeY(vol),mVipVolSizeZ(vol),mVipVolSizeT(vol));
	  printf("X voxsize:%5.3fmm, Y voxsize:%5.3fmm, Z voxsize:%5.3fmm, T voxsize:%5.3fs\n",
		 mVipVolVoxSizeX(vol),mVipVolVoxSizeY(vol),mVipVolVoxSizeZ(vol),mVipVolVoxSizeT(vol));
      }
  return(vol);
#endif // VIP_CARTO_VOLUME_WRAPPING
}





/*----------------------------------------------------------------------------*/
	Volume	    *VipReadVidaHeader
/* get the dimensions and the pixelsize of the vida file      */
				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
)
{
  VIP_DEC_VOLUME(vol);

  vol = ReadVidaVolumeDim(ima_name);
  if(vol!=PB)
      {
	  printf("Volume geometry:\n");
	  printf("X size:%d, Y size:%d, Z size:%d, T size:%d\n",
		 mVipVolSizeX(vol),mVipVolSizeY(vol),mVipVolSizeZ(vol),mVipVolSizeT(vol));
	  printf("X voxsize:%5.3fmm, Y voxsize:%5.3fmm, Z voxsize:%5.3fmm, T voxsize:%5.3fs\n",
		 mVipVolVoxSizeX(vol),mVipVolVoxSizeY(vol),mVipVolVoxSizeZ(vol),mVipVolVoxSizeT(vol));
      }
  return(vol);
}


/*----------------------------------------------------------------------------*/
	Volume	    *VipReadVidaFloatVolume
/* the volume is return in Float Perfectly scaled */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
)
{
  VIP_DEC_VOLUME(vol);

  vol = VipReadVidaFloatVolumeWithBorder(ima_name,0);
  return(vol);
}

/*----------------------------------------------------------------------------*/
	Volume	    *VipReadVidaFloatVolumeWithBorder
/* the volume is return in Float Perfectly scaled */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoRead( ima_name, borderWidth, VFLOAT, -1 );
#else

  int ima_format;

  VIP_DEC_VOLUME(vol);

  ima_format = VipGetImageFormat(ima_name);
  if(ima_format==TIVOLI) return(PB);

  printf("Reading VIDA image format to float volume...\n");

  vol = ReadVidaFloatVolumeWithBorder(ima_name, borderWidth);  

  printf("Volume geometry:\n");
  printf("X size:%d, Y size:%d, Z size:%d, T size:%d\n",
	 mVipVolSizeX(vol),mVipVolSizeY(vol),mVipVolSizeZ(vol),mVipVolSizeT(vol));
  printf("X voxsize:%5.3fmm, Y voxsize:%5.3fmm, Z voxsize:%5.3fmm, T voxsize:%5.3fs\n",
	 mVipVolVoxSizeX(vol),mVipVolVoxSizeY(vol),mVipVolVoxSizeZ(vol),mVipVolVoxSizeT(vol));
  return(vol);
#endif // VIP_CARTO_VOLUME_WRAPPING
}
/*----------------------------------------------------------------------------*/
	Volume	    *VipReadVidaFloatFrame
/* the volume is return in Float Perfectly scaled */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int	fr1,             /* 1ere frame a lire      */
	int	fr2              /* derniere frame a lire  */
)
{
  int ima_format;

  VIP_DEC_VOLUME(vol);

#ifdef VIP_CARTO_VOLUME_WRAPPING
  if( fr1 == fr2 )
    return VipVolumeCartoRead( ima_name, 0, VFLOAT, fr1 );
#endif

  ima_format = VipGetImageFormat(ima_name);
  if(ima_format==TIVOLI) return(PB);

  vol = ReadVidaFloatFrame(ima_name, fr1,fr2);   
  return(vol);
}

/*----------------------------------------------------------------------------*/
	Volume	    *VipReadVidaFrame
/* the volume is systematically return rescaled in S16BIT,
To get perfect rescaling in VFLOAT, use VipReadVidaFloatVolume */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int	fr1,             /* 1ere frame a lire      */
	int	fr2              /* derniere frame a lire  */
)
{
  int ima_format;

  VIP_DEC_VOLUME(vol);

#ifdef VIP_CARTO_VOLUME_WRAPPING
  if( fr1 == fr2 )
    return VipVolumeCartoRead( ima_name, 0, S16BIT, fr1 );
#endif

  ima_format = VipGetImageFormat(ima_name);
  if(ima_format==TIVOLI) return(PB);


  vol = ReadVidaFrame(ima_name, fr1,fr2);   
  return(vol);
}

/*----------------------------------------------------------------------------*/
int VipWriteVolume
/* The volume is systematically writen in VIDA without scale factors*/
/*----------------------------------------------------------------------------*/
(
 Volume	    *volume,
 const char    *ima_name
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoWrite( volume, ima_name, ANY_FORMAT );
#else
  return( WriteVidaVolume(volume, ima_name) );
#endif // VIP_CARTO_VOLUME_WRAPPING
}

/*----------------------------------------------------------------------------*/
int VipWriteSPMVolume
/*----------------------------------------------------------------------------*/
(
 Volume	    *volume,
 const char    *ima_name
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoWrite( volume, ima_name, SPM );
#else
  return( WriteSPMVolume(volume, ima_name) );
#endif // VIP_CARTO_VOLUME_WRAPPING
}

/*----------------------------------------------------------------------------*/
int VipWriteVidaScaledVolume
/* The volume is writen in VIDA format with optimal scale factors*/
/*----------------------------------------------------------------------------*/
(
 Volume	    *volume,
 const char    *ima_name
)
{
#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoWrite( volume, ima_name, ANY_FORMAT /* VIDA */ );
#else
  return( WriteVidaScaledVolume(volume, ima_name) );
#endif // VIP_CARTO_VOLUME_WRAPPING
}

/*----------------------------------------------------------------------------*/
int VipWriteTivoliVolume
/* The volume is writen in Tivoli format */
/*----------------------------------------------------------------------------*/
(
 Volume	    *volume,
 const char    *ima_name
)
{

#ifdef VIP_CARTO_VOLUME_WRAPPING
  return VipVolumeCartoWrite( volume, ima_name, ANY_FORMAT /* TIVOLI */ );
#else

#ifdef TIVOLI_LIB
	
  writeVolume(volume,ima_name) ;
  return(OK);

#else
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"write image %s: your Vip version\n\
has not been compiled to be Tivoli compatible (use #define TIVOLI_LIB)",
	  ima_name);
  VipPrintfExit(message);
  return(PB);

#endif
#endif // VIP_CARTO_VOLUME_WRAPPING
}
