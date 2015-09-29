/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Tivoli_util.c        * TYPE     : Source
 * AUTHOR      : see below            * CREATION : 12/09/1994
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : B. Verdonck, T. Geraud, J.F. Mangin
 *               modified 15/06/95 by T.Geraud (transferVolumeData)
 *               modified 16/01/95 by B.Verdonck 
 *                                             (readVolume, extensions)
 *               modified 23/11/95 by A.Robert (offsetBox)
 *               modified 12/95 by T.Geraud 
 *                                             (new file format .dim)
 *               modified 12/95 by D.Orfanos (memory mapping)
 *               modified 15/01/96 by B.Verdonck (count from 0 --> n-1
 *                                  and tab[0][0][0] is always the first point)
 *               modified 22/01/96 by B.Verdonck
 *                                   (reorganisation and documentation,
 *                                    minor corrections)
 *
 *
 *               Copyright (c) 1994
 *               Dept. IMA  TELECOM Paris
 *               46, rue Barrault  F-75634 PARIS Cedex 13
 *               All Rights Reserved
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


#include <assert.h>

#ifdef MEMORYMAP
 #ifndef _XOPEN_SOURCE /* necessaire pour utiliser 'tempnam()', qui n'est */
 #define _XOPEN_SOURCE /* pas C ISO, mais est necessaire au memory mapping */
 #include <stdio.h>
 #undef _XOPEN_SOURCE
 #endif
#else
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#ifdef MEMORYMAP
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

/*bnew*/
/* #include <sys/types.h> */ /* necessary ?? */

#include <vip/TivoliIO.h>

/*  static flags for (Volume*)->state  */

#define STATE_DECLARED	1	/* structure declared */
#define STATE_ALLOCATED	2	/* memory allocated for data */
#define STATE_FILLED	3	/* significant data */
#define STATE_FREED	4	/* structure freed */	

#ifdef MEMORYMAP
/* swap file management */

typedef struct swapfileNode {
	struct swapfileNode *next;
	const char *swapfile;
} swapfileNode;

static swapfileNode *swapfileList = NULL;

static int swapfileFlag = 0;

static void swapfileAtSignal(int);
static void swapfileAtExit(void);
#endif


/* local functions: declarations */ 

static void notImplementedFor ( int type );

static int verifyState ( int state );
static int verifySizes ( int nx, int ny, int nz, int nt ); 
static int verifyVoxelSizes ( float dx, float dy, float dz, float dt );
static int verifyBorderWidth ( int b );

static int testState ( Volume *volume, int state );

static int verifyType ( int type );

static void setState ( Volume *volume, int state );

static int sizeOf ( int type );

static void scanNflush ( char **str, char *strRef );

static char *imaFile(const char *filename);
static char *dimFile(const char *filename);
static void errorDuplicateFields ( char *s );

#ifdef MEMORYMAP 
static void swapfileAtSignal ( int a ); 
static void swapfileAtExit ( void );
#endif

static void addBorder_U8  ( Volume *volume );
static void addBorder_U16 ( Volume *volume );
static void addBorder_S32 ( Volume *volume );

/* NOT USED */
/*
static int verify2Dformat ( Volume *volume );
static long int getSizeInVoxels ( Volume *volume );
static long int getSizeInBytes ( Volume *volume );
static int testStateN ( Volume *volume, int n, ...  );
*/



/*==================================================/
 |                                                  /
 |  printing functions (print to stdout or stderr)  /
 |                                                  /
 *=================================================*/

/*----------------------------------------------------------------------------*/
static
void notImplementedFor ( int type )

/*----------------------------------------------------------------------------*/
{  
  if ( verifyType ( type ) == OK )
  {
    printfError ( "" );
    (void) fprintf ( stderr, 
                    "This function is not yet implemented for type '%s'.\n", 
                    typeName ( type ) );
  }
}

/*----------------------------------------------------------------------------*/

void printfError ( char *s )

/*----------------------------------------------------------------------------*/
{
  (void) fflush ( stdout );
  (void) fprintf ( stderr, "\n------------------------------" );
  (void) fprintf ( stderr, "\n!! Tivoli error" );
  if (s!=NULL && strlen(s)!=0)
      (void) fprintf ( stderr, ":\n\t %s", s );
  (void) fprintf( stderr, "\n------------------------------\n" );
  (void) fflush ( stderr );
}


/*----------------------------------------------------------------------------*/

void printfWarning ( char *s )

/*----------------------------------------------------------------------------*/
{
  (void) fflush ( stdout );
  (void) fprintf ( stderr, "\n------------------------------" );
  (void) fprintf ( stderr, "\n! Tivoli warning" );
  if (s!=NULL && strlen(s)!=0)
      (void) fprintf ( stderr, ":\n\t %s", s );
  (void) fprintf( stderr, "\n------------------------------\n" );
  (void) fflush ( stderr );
  printftrace ( MSG, "%s", s );
}


/*----------------------------------------------------------------------------*/

void printfInfo ( char *s )

/*----------------------------------------------------------------------------*/
{
  if (VERBOSE)
  {
    (void) fflush ( stderr );
    (void) fprintf ( stdout, "\n------------------------------" );
    (void) fprintf ( stdout, "\nTivoli info" );
    if (s!=NULL && strlen(s)!=0)
        (void) fprintf ( stdout, ":\n\t %s", s );
    (void) fprintf( stdout, "\n------------------------------\n" );
    (void) fflush ( stdout );
  }
}



/*----------------------------------------------------------------------------*/
	
/*	void			printfvolume*/

/*----------------------------------------------------------------------------*/
/******************************* A ete mis en commentaire le 05/03/1999
(
	Volume	    *volume,
	int			n,
	int			mode
)
{
	int			ix, bx, ex,
				iy, by, ey,
				iz, bz, ez,
				borderWidth; 
	U8BIT_t	  ***tabU8BIT;
	S32BIT_t	  ***tabS32BIT;
	DOUBLE_t  ***tabDOUBLE;
        
        printftrace ( IN, "printfvolume" );
        
	if ( !testFilled ( volume ) )
		goto abort;

        if ( n < 1 )
        {
          printfError ( "Parameter n < 1 has no sense !" );
          fprintf ( stderr, "\tn = %d\n", n );
          goto abort;
        }
        
		
	borderWidth = volume->borderWidth;
        printfInfo ( "Contents of a volume" );
        switch ( mode )
	{
		case 0 :
			printf ( "Beginning of '%s' (n=%d):\n", 
                                volume->name, n );
			bx = by = bz = -borderWidth;
			ex = ( bx + n < volume->size.x + borderWidth ? bx + n :
					volume->size.x + borderWidth );
			ey = ( by + n < volume->size.y + borderWidth ? by + n :
					volume->size.y + borderWidth );
			ez = ( bz + n < volume->size.z + borderWidth ? bz + n :
					volume->size.z + borderWidth );
			break;
		case 1 :
			printf ( "Ending of '%s' (n=%d):\n", 
                                volume->name, n );
			ex = volume->size.x + borderWidth;
			ey = volume->size.y + borderWidth;
			ez = volume->size.z + borderWidth;
			bx = ( ex - n < -borderWidth ? -borderWidth : ex - n );
			by = ( ey - n < -borderWidth ? -borderWidth : ey - n );
			bz = ( ez - n < -borderWidth ? -borderWidth : ez - n );
			break;
		default :
			printfError ( "Mode unknown (should be 0 or 1) !" );
                        (void) fprintf ( stderr, "\tmode = %d\n", mode );
                        goto abort;
	}
	switch ( volume->type )
	{
		case U8BIT :
			tabU8BIT = tab_U8BIT ( volume );
if ( EXEC )
			for ( iz = bz; iz < ez; iz++ )
			{
				for ( iy = by; iy < ey; iy++ )
				{
					for ( ix = bx; ix < ex; ix++ )
						printf ( "%3u ", tabU8BIT[iz][iy][ix] );
					printf ( "\n" );
				}
				printf ( "\n" );
			}
			break;
		case S32BIT :
			tabS32BIT = tab_S32BIT ( volume );
if ( EXEC )
			for ( iz = bz; iz < ez; iz++ )
			{
				for ( iy = by; iy < ey; iy++ )
				{
					for ( ix = bx; ix < ex; ix++ )
						printf ( "%.3ld ", tabS32BIT[iz][iy][ix] );
					printf ( "\n" );
				}
				printf ( "\n" );
			}
			break;
		case VDOUBLE :
			tabDOUBLE = tab_DOUBLE ( volume );
if ( EXEC )
			for ( iz = bz; iz < ez; iz++ )
			{
				for ( iy = by; iy < ey; iy++ )
				{
					for ( ix = bx; ix < ex; ix++ )
						printf ( "%8f ", tabDOUBLE[iz][iy][ix] );
					printf ( "\n" );
				}
				printf ( "\n" );
			}
			break;
                default:
                        notImplementedFor ( volume->type );
                        goto abort;
        }
	printf ( "\n" );

        printftrace ( OUT, "" );
        return;
abort :
	printfexit ( "(volume)printfvolume" );
}
*******************************/



/*=================================================*
 |                                                 |
 |  testing functions  (return 0 (PB) on failure)  |
 |                                                 |
 *================================================*/


/*----------------------------------------------------------------------------*/
	static
	int			verifySizes		/*NEW*/	/*REPLACE*/

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
                nt < 1 ||                  /*bnew*/
		nx > IMAGE_MAXSIZE ||
		ny > IMAGE_MAXSIZE ||
		nz > IMAGE_MAXSIZE ||
		nt > IMAGE_MAXSIZE )
	{
          printfWarning ( "Improper volume dimension" );
          (void) fprintf ( stderr, "Volume dimension should be in [1,%d].\n", 
                   IMAGE_MAXSIZE );
          (void) fprintf ( stderr, "You tried to use [%d, %d, %d, %d].\n",
                   nx, ny, nz, nt );
          return ( PB );
	}
	return ( OK );
}





/*----------------------------------------------------------------------------*/
	static
	int			verifyVoxelSizes		/*NEW*/	/*REPLACE*/
	
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
		dt <  0. )
	{
          printfWarning ( "Improper voxel size" );
          (void) fprintf ( stderr, "Voxel sizes should be positive.\n");
          /*?theo: 'strictly' positive and 0 is allowed for z and t ??*/
          (void) fprintf ( stderr, "You tried to use [%f, %f, %f, %f].\n",
                   dx, dy, dz, dt );
          return ( PB );
	}
	return ( OK );
}





/*----------------------------------------------------------------------------*/
	static
	int			verifyBorderWidth

/*----------------------------------------------------------------------------*/
(
	int			b
)
{
	if ( b < 0 || b > BORDER_MAXWIDTH )
	{
          printfWarning ( "Improper volume border width" );
          (void) fprintf ( stderr, "Border widths should be in [0,%d])\n", 
                          BORDER_MAXWIDTH );
          (void) fprintf ( stderr, "You tried to use %d\n", b );
          return ( PB );
	}
	return ( OK );
}



/*----------------------------------------------------------------------------*/
	static
	int			testState

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			state
)
{
	if ( !testExist ( volume ) ||
		!verifyState ( state ) )
		return ( PB );
	if ( volume->state != state )
	{
          printfWarning ( "Volume state problem" );
          (void) fprintf ( stderr,
		"Volume '%s' (%s) has not the expected state (%s).\n",
		volume->name, stateName ( volume->state ), stateName ( state ) );
          return ( PB );
	}
	return ( OK );
}



#if 0 // this function is never used
/*----------------------------------------------------------------------------*/
	static
	int			testStateN

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			n, ...
)
{
	va_list		ap;
	int			i = 0,
				state,
				found = VFALSE;
	if ( !testExist ( volume ) )
		return ( PB );
	va_start ( ap, n );
	for ( i = 0; i < n; i++ )
	{
		state = va_arg ( ap, int );
		if ( !verifyState ( state ) )
		{
                  printfWarning ( 
                        "Bad state specifiers in '(volume)testStateN'.\n" );
                  return ( PB );
		}
		if ( state == volume->state )
			found = VTRUE;
	}
	va_end ( ap );
	if ( ! found )
	{
          printfWarning ( "Volume state problem" );
          (void) fprintf ( stderr,
		"Volume '%s' (%s) has not an expected state.\n",
		volume->name, stateName ( volume->state ) );
          return ( PB );
	}
	return ( OK );
}
#endif

/*----------------------------------------------------------------------------*/
	static
	int			verifyType

/*----------------------------------------------------------------------------*/
(
	int			type
)
{
	if ( type != U8BIT  &&
		type != S8BIT  &&
		type != U16BIT &&
		type != S16BIT &&
		type != U32BIT &&
		type != S32BIT &&
		type != VFLOAT  &&
		type != VDOUBLE )
	{
          printfWarning ( "Improper volume type" );
          (void) fprintf ( stderr, "type #%d !\n", type );
          return ( PB );
	}
	return ( OK );
}







/*----------------------------------------------------------------------------*/
	static
	int			verifyState

/*----------------------------------------------------------------------------*/
(
	int			state
)
{
	if ( state != STATE_DECLARED  &&
		state != STATE_ALLOCATED &&
		state != STATE_FILLED    &&
		state != STATE_FREED )
	{
          printfWarning ( "Improper volume state" );
          (void) fprintf ( stderr, "State #%d !\n", state );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/

	int			testExist

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( volume == NULL )
	{
          printfWarning ( "Access to volume denied: pointer to NULL." );
          (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
          return ( PB );
	}
	if ( volume->state == STATE_FREED )
	{
          printfWarning ( "Access to volume denied: volume is freed." );
          (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/

	int			testExistN

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
                  printfWarning ( "Access to volume denied: pointer to NULL." );
                  (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
                  return ( PB );
                }
                if ( volume->state == STATE_FREED )
                {
                  printfWarning ( "Access to volume denied: volume is freed." );
                  (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
                  return ( PB );
                }
	}
	va_end ( ap );
	return ( OK );
}




/*----------------------------------------------------------------------------*/

	int			testType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			type
)
{
	if ( !testExist ( volume ) ||
		!verifyType ( type ) )
		return ( PB );
	if ( volume->type != type )
	{
          printfWarning ( "Type problem" );
          (void) fprintf ( stderr,
		"Volume '%s' (%s) has not the expected type (%s).\n",
                          volume->name, typeName ( volume->type ), 
                          typeName ( type ) );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/

	int			testTypeN

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
	
	if ( !testExist ( volume ) )
		return ( PB );
	va_start ( ap, n );
	for ( i = 0; i < n; i++ )
	{
		type = va_arg ( ap, int );
		if ( !verifyType ( type ) )
		{
                  printfWarning ( "Bad type specifiers in '(volume)testTypeN'" );
                  return ( PB );
		}
		if ( type == volume->type )
			found = VTRUE;
	}
	va_end ( ap );
	if ( ! found )
	{
          printfWarning ( "Type problem" );
          (void) fprintf ( stderr,
                          "Volume '%s' (%s) has not an expected type.\n",
                          volume->name, typeName ( volume->type ) );
          return ( PB );
	}
	return ( OK );
}





/*----------------------------------------------------------------------------*/
	
	int			testFilled

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist ( volume ) )
		return ( PB );
	if ( volume->state != STATE_FILLED )
	{
          printfWarning ( "The volume does not contain significant data." );
          (void) fprintf ( stderr,
		"Volume name: '%s'\n", volume->name );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/

	int			testEqualSize		/*REPLACE*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
	if ( !testExistN ( 2, volume1, volume2 ) )
		return ( PB );
	if ( volume1->size.x != volume2->size.x ||
			volume1->size.y != volume2->size.y ||
			volume1->size.z != volume2->size.z ||
			volume1->size.t != volume2->size.t )
	{
          printfWarning ( "Dimension problem" );
          (void) fprintf ( stderr,
                     "Volumes '%s' and '%s' do not have the same dimensions:\n",
			volume1->name, volume2->name );
          (void) fprintf ( stderr,
                          "\t%d x %d x %d x %d  v.  %d x %d x %d x %d  !\n",
                          volume1->size.x, volume1->size.y,
                          volume1->size.z, volume1->size.t, 
                          volume2->size.x, volume2->size.y,
                          volume2->size.z, volume2->size.t );
/*?theo: j'ai elimine ce qui suit...
          if ( volume1->size.t != 1 || volume2->size.t != 1 )
              (void) fprintf ( stderr,
                              "%d x %d x %d x %d  v.  %d x %d x %d x %d  !\n",
                              volume1->size.x, volume1->size.y,
                              volume1->size.z, volume1->size.t, 
                              volume2->size.x, volume2->size.y,
                              volume2->size.z, volume2->size.t );
          else
              if ( volume1->size.z != 1 || volume2->size.z != 1 )
                  (void) fprintf ( stderr,
                                  "%d x %d x %d  v.  %d x %d x %d  !\n",
                             volume1->size.x, volume1->size.y, volume1->size.z, 
                             volume2->size.x, volume2->size.y, volume2->size.z );
		else
                    (void) fprintf ( stderr,
                                    "%d x %d  v.  %d x %d  !\n",
                                    volume1->size.x, volume1->size.y, 
                                    volume2->size.x, volume2->size.y );
*/
          return ( PB );
	}
	return ( OK );
}





/*----------------------------------------------------------------------------*/

	int			testEqual2DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
	if ( !testExistN ( 2, volume1, volume2 ) )
		return ( PB );
	if ( volume1->size.x != volume2->size.x ||
			volume1->size.y != volume2->size.y )
	{
          printfWarning ( "Dimension problem" );
          (void) fprintf ( stderr,
                          "Volumes '%s' and '%s' do not have the same ", 
                          volume1->name, volume2->name );
          (void) fprintf ( stderr,
                          "dimensions along x and y.\n" );
          (void) fprintf ( stderr,
                          "\t%d x %d  v.  %d x %d  !\n",
                          volume1->size.x, volume1->size.y, 
                          volume2->size.x, volume2->size.y );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/

	int			testEqualType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
	if ( !testExistN ( 2, volume1, volume2 ) )
		return ( PB );
	if ( volume1->type != volume2->type )
	{
          printfWarning ( "Type problem" );
          (void) fprintf ( stderr,
                          "Volumes '%s' (%s) and '%s' (%s) do not ",
                          volume1->name, typeName ( volume1->type ),
                          volume2->name, typeName ( volume2->type ) );
          (void) fprintf ( stderr, "have not the same type.\n" );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/

	int			testEqualBorderWidth

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
)
{
	if ( !testExistN ( 2, volume1, volume2 ) )
		return ( PB );
	if ( volume1->borderWidth != volume2->borderWidth )
	{
          printfWarning ( "Border problem" );
          (void) fprintf ( stderr,
                          "Volumes '%s' (%d) and '%s' (%d) ",
                          volume1->name, volume1->borderWidth,
                          volume2->name, volume2->borderWidth );
          (void) fprintf ( stderr, "do not have the same border width.\n" );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/
	
	int			testSlice

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			slice
)
/*IMA_function: testSlice
IMA_description: This function also considers slices in the border.  
Slices thus can go from -bW to nz-1+bW !
*/
{	
	if ( !testExist ( volume ) )
		return ( PB );
		
	if ( slice < -volume->borderWidth 
            || slice > volume->size.z + volume->borderWidth - 1 )
	{
          printfWarning ( "Presence of a slice" );
          (void) fprintf ( stderr,
                          "Volume '%s' (%d-%d) does not contain slice #%d.\n",
                          volume->name, -volume->borderWidth, 
                          volume->size.z + volume->borderWidth - 1, slice );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/
	
	int			testImageSlice

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			slice
)
/*IMA_function: testImageSlice
IMA_description: Tests for the presence of a slice in a volume.  Slice numbers 
go from 0 to nz-1.

Remark: does not take account of slices in the border !
Use function 'testSlice' to also look into the border.
*/
{
	if ( !testExist ( volume ) )
		return ( PB );
		
        /*bnew: slices count from 0 to n-1 !*/
	if ( slice < 0 || slice >= volume->size.z )
	{
          printfWarning ( "Presence of a slice" );
          (void) fprintf ( stderr,
                          "Volume '%s' (0-%d) does not contain slice #%d.\n",
                          volume->name, volume->size.z-1, slice );
          return ( PB );
	}
	return ( OK );
}




/*----------------------------------------------------------------------------*/
	
	int			verifyAll		/*NEW*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist		( volume ) ||
		!verifyType		( volume->type ) ||
		!verifyState		( volume->state ) ||
		!verifyBorderWidth	( volume->borderWidth ) ||
		!verifySizes		( volume->size.x,
						  volume->size.y,
						  volume->size.z,
						  volume->size.t ) ||
		!verifyVoxelSizes	( volume->voxelSize.x,
						  volume->voxelSize.y,
						  volume->voxelSize.z,
						  volume->voxelSize.t )
	   )
		return ( PB );
	return ( OK );
}




/*Bert----------------------------------------------------------------------*/

int verifyRoi ( Roi *roi, Volume *volume )

/*--------------------------------------------------------------------------*/
{
/*IMA_function: verifyRoi - function for verification of a Roi

IMA_description: This function verifies that the roi falls properly into 
the volume (+ borders).  
If not: the roi is adjusted in order to make it fall into the volume

IMA_return: Returns VFALSE if the roi was changed (in this case the values of the
roi are changed), VTRUE if no incompatibilities occured.
*/

  int roiOK = VTRUE;
  int maxx, maxy, maxz, maxt;
  int h;
  
  /* correct order 1 < 2 */
  if (roi->x2 < roi->x1)
  {       
    h=roi->x1;roi->x1=roi->x2;roi->x2=h;
    roiOK=VFALSE;
  }
  if (roi->y2 < roi->y1)
  {       
    h=roi->y1;roi->y1=roi->y2;roi->y2=h;
    roiOK=VFALSE;
  }
  if (roi->z2 < roi->z1)
  {       
    h=roi->z1;roi->z1=roi->z2;roi->z2=h;
    roiOK=VFALSE;
  }
  if (roi->t2 < roi->t1)
  {       
    h=roi->t1;roi->t1=roi->t2;roi->t2=h;
    roiOK=VFALSE;
  }
  /* check for not negative */
  if (roi->x1 < 0)
  {
    roi->x1 = 0;
    roiOK=VFALSE;
  }
  if (roi->y1 < 0)
  {
    roi->y1 = 0;
    roiOK=VFALSE;
  }
  if (roi->z1 < 0)
  {
    roi->z1 = 0;
    roiOK=VFALSE;
  }
  if (roi->t1 < 0)
  {
    roi->t1 = 0;
    roiOK=VFALSE;
  }
  if (roi->x2 < 0)
  {
    roi->x2 = 0;
    roiOK=VFALSE;
  }
  if (roi->y2 < 0)
  {
    roi->y2 = 0;
    roiOK=VFALSE;
  }
  if (roi->z2 < 0)
  {
    roi->z2 = 0;
    roiOK=VFALSE;
  }
  if (roi->t2 < 0)
  {
    roi->t2 = 0;
    roiOK=VFALSE;
  }
  /* check for not too big */
  getSize4(volume, &maxx, &maxy, &maxz, &maxt);
  h = 2*borderWidthOf(volume)-1;
  maxx += h;
  maxy += h;
  maxz += h;
  maxt += h;
  if (roi->x1 > maxx)
  {
    roi->x1 = maxx;
    roiOK=VFALSE;
  }
  if (roi->y1 > maxy)
  {
    roi->y1 = maxy;
    roiOK=VFALSE;
  }
  if (roi->z1 > maxz)
  {
    roi->z1 = maxz;
    roiOK=VFALSE;
  }
  if (roi->t1 > maxt)
  {
    roi->t1 = maxt;
    roiOK=VFALSE;
  }
  if (roi->x2 > maxx)
  {
    roi->x2 = maxx;
    roiOK=VFALSE;
  }
  if (roi->y2 > maxy)
  {
    roi->y2 = maxy;
    roiOK=VFALSE;
  }
  if (roi->z2 > maxz)
  {
    roi->z2 = maxz;
    roiOK=VFALSE;
  }
  if (roi->t2 > maxt)
  {
    roi->t2 = maxt;
    roiOK=VFALSE;
  }

  return( roiOK );
}


/*----------------------------------------------------------------------------*/
/*bnew: new function*/

int testLevelInType ( double level, int type )

/*----------------------------------------------------------------------------*/
{
/* take care: implementation dependent !!! */
  switch ( type )
  {
  case U8BIT:
    if ( level > 255 || level < 0 || level != (U8BIT_t)level )
    {
      printfWarning ( "" );
      (void) fprintf ( stderr,
                      "Level %f does not fit in U8BIT [0, 255].\n", level );
      return ( PB );
    }
    break;

  case S8BIT:
    if ( level > 127 || level < -128 || level != (S8BIT_t)level )
    {
      printfWarning ( "" );
      (void) fprintf ( stderr,
                      "Level %f does not fit in U8BIT [-128, 127].\n", level );
      return ( PB );
    }
    break;

  case U16BIT:
    if ( level > 65535  || level < 0 || level != (U16BIT_t)level )
    {
      printfWarning ( "" );
      (void) fprintf ( stderr,
                      "Level %f does not fit in U8BIT [0, 65535].\n", level );
      return ( PB );
    }
    break;

  case S16BIT:
    if ( level > 32767 || level < -32768 || level != (S16BIT_t)level )
    {
      printfWarning ( "" );
      (void) fprintf ( stderr,
                      "Level %f does not fit in U8BIT [-32768, 32767].\n", 
                      level );
      return ( PB );
    }
    break;

  case U32BIT:
    if ( level != (U32BIT_t)level )
    {
      printfWarning ( "" );
      (void) fprintf ( stderr,
                      "Level %f does not fit in U32BIT.\n", level );
      return ( PB );
    }
    break;

  case S32BIT:
    if ( level != (S32BIT_t)level )
    {
      printfWarning ( "" );
      (void) fprintf ( stderr,
                      "Level %f does not fit in S32BIT.\n", level );
      return ( PB );
    }
    break;

  case VFLOAT:
    if ( level != (FLOAT_t)level )
    {
      printfWarning ( "" );
      (void) fprintf ( stderr,
                      "Level %f does not fit in FLOAT.\n", level );
      return ( PB );
    }
    break;

  case VDOUBLE:
    break;
  
  default:
    notImplementedFor ( type );
  }

  return ( OK );
}



/*=====================================================*
 |                                                     |
 |  getting functions : access to the volume structure |
 |                                                     |
 *=====================================================*/

/*bnew*/
/*----------------------------------------------------------------------------*/
	
	void			getSize2

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int		    *sizeX,
	int		    *sizeY
)
/* first 2 sizes, relevant for 2D images or size of a slice */
{
	if ( !testExist ( volume ) )
		goto abort;
	*sizeX = volume->size.x;
	*sizeY = volume->size.y;

	return;
abort :
	printfexit ( "(volume)getSize2" );
}
/*----------------------------------------------------------------------------*/
	
	void			getSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int		    *sizeX,
	int		    *sizeY,
	int		    *sizeZ
)
/* 3 sizes */
{
	if ( !testExist ( volume ) )
		goto abort;
	*sizeX = volume->size.x;
	*sizeY = volume->size.y;
	*sizeZ = volume->size.z;
	return;
abort :
	printfexit ( "(volume)getSize" );
}
/*----------------------------------------------------------------------------*/
	
	void			getSize4		/*NEW*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int		    *sizeX,
	int		    *sizeY,
	int		    *sizeZ,
	int		    *sizeT
)
/* All 4 sizes */
{
	if ( !testExist ( volume ) )
		goto abort;
	*sizeX = volume->size.x;
	*sizeY = volume->size.y;
	*sizeZ = volume->size.z;
	*sizeT = volume->size.t;
	return;
abort :
	printfexit ( "(volume)getSize4" );
}



/*----------------------------------------------------------------------------*/
	
	int			sizexOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist ( volume ) )
		goto abort;
	return ( volume->size.x );
abort :
	printfexit ( "(volume)sizexOf" );
	return ( -1 );
}
/*----------------------------------------------------------------------------*/
	
	int			sizeyOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist ( volume ) )
		goto abort;
	return ( volume->size.y );
abort :
	printfexit ( "(volume)sizeyOf" );
	return ( -1 );
}
/*----------------------------------------------------------------------------*/
	
	int			sizezOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist ( volume ) )
		goto abort;
	return ( volume->size.z );
abort :
	printfexit ( "(volume)sizezOf" );
	return ( -1 );
}
/*----------------------------------------------------------------------------*/
	
	int			sizetOf		/*NEW*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist ( volume ) )
		goto abort;
	return ( volume->size.t );
abort :
	printfexit ( "(volume)sizetOf" );
	return ( -1 );
}




/*bnew*/
/*----------------------------------------------------------------------------*/
	
	void			getVoxelSize2

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float	    *voxelSizeX,
	float	    *voxelSizeY
)
{
	if ( !testExist ( volume ) )
		goto abort;
	*voxelSizeX = volume->voxelSize.x;
	*voxelSizeY = volume->voxelSize.y;
	return;
abort :
	printfexit ( "(volume)getVoxelSize2" );
}
/*----------------------------------------------------------------------------*/
	
	void			getVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float	    *voxelSizeX,
	float	    *voxelSizeY,
	float	    *voxelSizeZ
)
/* 3D */
{
	if ( !testExist ( volume ) )
		goto abort;
	*voxelSizeX = volume->voxelSize.x;
	*voxelSizeY = volume->voxelSize.y;
	*voxelSizeZ = volume->voxelSize.z;
	return;
abort :
	printfexit ( "(volume)getVoxelSize" );
}
/*----------------------------------------------------------------------------*/
	
	void			getVoxelSize4

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float	    *voxelSizeX,
	float	    *voxelSizeY,
	float	    *voxelSizeZ,
	float	    *voxelSizeT
)
/* all 4 voxelSizes */
{
	if ( !testExist ( volume ) )
		goto abort;
	*voxelSizeX = volume->voxelSize.x;
	*voxelSizeY = volume->voxelSize.y;
	*voxelSizeZ = volume->voxelSize.z;
	*voxelSizeT = volume->voxelSize.t;
	return;
abort :
	printfexit ( "(volume)getVoxelSize4" );
}



/*----------------------------------------------------------------------------*/
	
	int			borderWidthOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist ( volume ) )
		goto abort;
	return ( volume->borderWidth );
abort :
	printfexit ( "(volume)borderWidthOf" );
	return ( -1 );
}



/*----------------------------------------------------------------------------*/
	
	int			typeOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	if ( !testExist ( volume ) )
		goto abort;
	return ( volume->type );
abort :
	printfexit ( "(volume)typeOf" );
	return ( 0 );
}
/*----------------------------------------------------------------------------*/
/*bnew: made extern */
	char		    *typeName

/*----------------------------------------------------------------------------*/
(
	int			type
)
{
	char *name;
	name = mallocT ( 64 );

	switch ( type )
	{
		case U8BIT  :
			strcpy ( name, "unsigned 8 bit" );
			break;
		case S8BIT  :
			strcpy ( name, "signed 8 bit" );
			break;
		case U16BIT :
			strcpy ( name, "unsigned 16 bit" );
			break;
		case S16BIT :
			strcpy ( name, "signed 16 bit" );
			break;
		case U32BIT :
			strcpy ( name, "unsigned 32 bit" );
			break;
		case S32BIT :
			strcpy ( name, "signed 32 bit" );
			break;
		case VFLOAT  :
			strcpy ( name, "float" );
			break;
		case VDOUBLE :
			strcpy ( name, "double" );
			break;
                default :
                        strcpy ( name, "invalid type" );
                        break;
	}
	return ( name );
}



/*----------------------------------------------------------------------------*/
/*bnew: made extern */
	char		    *stateName

/*----------------------------------------------------------------------------*/
(
	int			state
)
{
	char		    *name;
	name = mallocT ( 64 );
	
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
                default :
                        strcpy ( name, "invalid state" );
			break;
	}
	return ( name );
}




/*----------------------------------------------------------------------------*/

	void	    getMinMax
	
/*----------------------------------------------------------------------------*/
(
        Volume     *volume,
        double     *minVal,
        double     *maxVal
)
/*IMA_function: getMinMax
IMA_description: Search the minimum and maximum intensity value of a volume.
If VERBOSE is on, print these values to stdout (including position of min
   and max.  Looks also in the borders.
*/
{
  int k;

  int minInt=0, maxInt=0, valInt=0;
  double minD=0, maxD=0, valD=0;

  int minPos, maxPos, posZ;
  int oL, oS, oV, bw;
  
  U8BIT_t   *dataU8;
  U16BIT_t   *dataU16;
  S32BIT_t   *dataS32;
  FLOAT_t    *dataF;
  DOUBLE_t   *dataD;

  printftrace( IN, "getMinMax" );
    
  if ( !verifyAll ( volume ) ||
    !testFilled ( volume ) )
    goto abort;

  oV = offsetVolume ( volume );
  minPos = maxPos = 0;
  
  switch  ( volume->type ) 
  {
    case U8BIT:
    dataU8 = data_U8BIT ( volume );
    minInt = maxInt = *dataU8;
    for ( k=0; k<oV; k++ )
    {
      valInt = * ( dataU8++ );
      if  ( valInt < minInt )
      {
        minPos = k; minInt = valInt;
      }
      else if  ( valInt > maxInt )
      { 
        maxPos = k; maxInt = valInt;
      }
    }
    break;
    case U16BIT:
    dataU16 = data_U16BIT ( volume );
    minInt = maxInt = *dataU16;
    for ( k=0; k<oV; k++ )
    {
      valInt = * ( dataU16++ );
      if  ( valInt < minInt )
      {
        minPos = k; minInt = valInt;
      }
      else if  ( valInt > maxInt )
      { 
        maxPos = k; maxInt = valInt;
      }
    }
    break;
    case S32BIT:
    dataS32 = data_S32BIT ( volume );
    minInt = maxInt = *dataS32;
    for ( k=0; k<oV; k++ )
    {
      valInt = * ( dataS32++ );
      if  ( valInt < minInt )
      {
        minPos = k; minInt = valInt;
      }
      else if  ( valInt > maxInt )
      { 
        maxPos = k; maxInt = valInt;
      }
    }
    break;
    case VFLOAT:
    dataF = data_VFLOAT ( volume );
    minD = maxD = *dataF;
    for ( k=0; k<oV; k++ )
    {
      valD = * ( dataF++ );
      if  ( valD < minD )
      {
        minPos = k; minD = valD;
      }
      else if  ( valD > maxD )
      { 
        maxPos = k; maxD = valD;
      }
    }
    break;
    case VDOUBLE:
    dataD = data_VDOUBLE ( volume );
    minD = maxD = *dataD;
    for ( k=0; k<oV; k++ )
    {
      valD = * ( dataD++ );
      if  ( valD < minD )
      {
        minPos = k; minD = valD;
      }
      else if  ( valD > maxD )
      { 
        maxPos = k; maxD = valD;
      }
    }
    break;
    default:
    notImplementedFor ( volume->type );
    goto abort;
  }  

  switch  ( volume->type )
  {
    case U8BIT:
    case U16BIT:
    case S32BIT:
    *minVal = minInt;
    *maxVal = maxInt;
    break;
    case VFLOAT:
    case VDOUBLE:
    *minVal = minD;
    *maxVal = maxD;
    break;
  }

  if  ( VERBOSE )
  {
    bw = borderWidthOf ( volume );
    oL = offsetLine ( volume );
    oS = offsetSlice ( volume );
    posZ = minPos/oS-bw;
    minPos = minPos%oS;
    fflush ( stdout );
    printfInfo ( "Results of the 'getMinMax' function:\n\t\t" );
    switch  ( volume->type )
    {
      case U8BIT:
      case U16BIT:
      case S32BIT:
      printf ( "Minimum = %d at (%d, %d, %d)",
             minInt, minPos%oL-bw, minPos/oL-bw, posZ );
      posZ = maxPos/oS-bw;
      maxPos = maxPos%oS;
      printf ( "\n\t\tMaximum = %d at (%d, %d, %d)\n",
             maxInt, maxPos%oL-bw, maxPos/oL-bw, posZ );
      break;
      case VFLOAT:
      case VDOUBLE:
      printf ( "Minimum = %f at (%d, %d, %d)",
             minD, minPos%oL-bw, minPos/oL-bw, posZ );
      posZ = maxPos/oS-bw;
      maxPos = maxPos%oS;
      printf ( "\n\t\tMaximum = %f at (%d, %d, %d)\n",
             maxD, maxPos%oL-bw, maxPos/oL-bw, posZ );
      break;
    }
  }
  
  printftrace ( OUT, "" );
  return;

abort:
  printfexit( "(volume)getMinMax" );
}



/*============================================================*
 |                                                            |
 |  setting functions : change values in the volume structure |
 |                                                            |
 *============================================================*/



/*----------------------------------------------------------------------------*/
	static
	void			setState

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			state
)
{
	if ( !testExist ( volume ) ||
		!verifyState ( state ) )
		goto abort;
	if ( volume->data != NULL && state == STATE_DECLARED )
	{
          /* possible memory loss ! */
          printfError (
"Volume can not be set to 'STATE_DECLARED' while data is not 'NULL'." );
          goto abort;
	}
	if ( volume->data == NULL && state == STATE_ALLOCATED )
	{
          /* dangerous ! */
          printfError (
"Volume can not be set to 'STATE_ALLOCATED' while data is still 'NULL'." );
          goto abort;
	}
	if ( volume->data == NULL && state == STATE_FILLED )
	{
          /* dangerous ! */
          printfError (
"Volume can not be set to 'STATE_FILLED' while data is still 'NULL'." );
          goto abort;
	}
	volume->state = state;
	return;
abort :
        (void) fprintf ( stderr, "Please contact geraud@ima.enst.fr\n" );
	printfexit ( "(volume)setState" );
}



/*----------------------------------------------------------------------------*/

	void			setSize2

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx,
	int			ny
)
{
	printftrace ( IN, "setSize2" );

	if ( !testState ( volume, STATE_DECLARED ) ||
		!verifySizes ( nx, ny, 1, 1 ) )
		goto abort;
	volume->size.x = nx;
	volume->size.y = ny;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setSize2" );
}
/*----------------------------------------------------------------------------*/

	void			setSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx,
	int			ny,
	int			nz
)
{
	printftrace ( IN, "setSize" );

	if ( !testState ( volume, STATE_DECLARED ) ||
		!verifySizes ( nx, ny, nz, 1 ) )
		goto abort;
	volume->size.x = nx;
	volume->size.y = ny;
	volume->size.z = nz;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setSize" );
}
/*----------------------------------------------------------------------------*/

	void			setSize4

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx,
	int			ny,
	int			nz,
	int			nt
)
{
	printftrace ( IN, "setSize4" );

	if ( !testState ( volume, STATE_DECLARED ) ||
		!verifySizes ( nx, ny, nz, nt ) )
		goto abort;
	volume->size.x = nx;
	volume->size.y = ny;
	volume->size.z = nz;
	volume->size.t = nt;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setSize4" );
}




/*----------------------------------------------------------------------------*/

	void			setSizex		/*REPLACE*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx
)
{
	printftrace ( IN, "setSizex" );

	if ( !testState ( volume, STATE_DECLARED ) ||
		!verifySizes ( nx, 1, 1, 1 ) )
		goto abort;
	volume->size.x = nx;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setSizex" );
}




/*----------------------------------------------------------------------------*/

	void			setSizey		/*REPLACE*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			ny
)
{
	printftrace ( IN, "setSizey" );

	if ( !testState ( volume, STATE_DECLARED ) ||
		!verifySizes ( 1, ny, 1, 1 ) )
		goto abort;
	volume->size.y = ny;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setSizey" );
}




/*----------------------------------------------------------------------------*/

	void			setSizez		/*REPLACE*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nz
)
{
	printftrace ( IN, "setSizez" );

	if ( !testState ( volume, STATE_DECLARED ) ||
		!verifySizes ( 1, 1, nz, 1 ) )
		goto abort;
	volume->size.z = nz;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setSizez" );
}




/*----------------------------------------------------------------------------*/

	void			setSizet		/*NEW*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nt
)
{
	printftrace ( IN, "setSizet" );

	if ( !testState ( volume, STATE_DECLARED ) ||
		!verifySizes ( 1, 1, 1, nt ) )
		goto abort;
	volume->size.t = nt;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setSizet" );
}


/*----------------------------------------------------------------------------*/

	void			setVoxelSize2		/*REPLACE*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float		dx,
	float		dy
)
{
	printftrace ( IN, "setVoxelSize2" );

	if ( !testExist ( volume ) ||
		!verifyVoxelSizes ( dx, dy, 1.0, 1.0 ) )
		goto abort;
	volume->voxelSize.x = dx;
	volume->voxelSize.y = dy;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setVoxelSize2" );
}
/*----------------------------------------------------------------------------*/

	void			setVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float		dx,
	float		dy,
	float		dz
)
{
	printftrace ( IN, "setVoxelSize" );

	if ( !testExist ( volume ) ||
		!verifyVoxelSizes ( dx, dy, dz, 1.0 ) )
		goto abort;
	volume->voxelSize.x = dx;
	volume->voxelSize.y = dy;
	volume->voxelSize.z = dz;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setVoxelSize" );
}
/*----------------------------------------------------------------------------*/

	void			setVoxelSize4		

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float		dx,
	float		dy,
	float		dz,
	float		dt
)
{
	printftrace ( IN, "setVoxelSize4" );

	if ( !testExist ( volume ) ||
		!verifyVoxelSizes ( dx, dy, dz, dt ) )
		goto abort;
	volume->voxelSize.x = dx;
	volume->voxelSize.y = dy;
	volume->voxelSize.z = dz;
	volume->voxelSize.t = dt;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setVoxelSize4" );
}


/*----------------------------------------------------------------------------*/

	void			setType	     /*  modifications by B.Verdonck  */

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			type
)
{
	printftrace ( IN, "setType" );

	if ( !testExist ( volume ) || !verifyType ( type ) )
            goto abort;
        
        if ( volume->state == STATE_ALLOCATED )
        {
          printfWarning ( "The volume was already allocated" );
          (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
          (void)fprintf ( stderr, "I'll free it before changing its type.\n");
          setState ( volume, STATE_FILLED );
          freeVolumeData( volume );
        }
        
        if ( volume->state == STATE_FILLED )
        {
          printfError ( "The volume must not be filled" );
          (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
          goto abort;
        }
        
        /* state can only be STATE_DECLARED */

	volume->type = type;

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)setType" );
}



/*----------------------------------------------------------------------------*/

	void			setBorderWidth

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			borderWidth
)
{
	Volume	    *newVolume;
	int			oTRACE;
	
printftrace ( IN, "setBorderWidth" );
oTRACE = TRACE; TRACE = OFF;

	if ( !testExist ( volume ) )
		goto abort;
		
        /*?theo: comment borderWidth peut etre <0 */
	if ( ( borderWidth >= 0 && volume->borderWidth != borderWidth )
			/* exact value */ ||
		( borderWidth < 0 && volume->borderWidth < -borderWidth )
			/* at least */ )
	{
		borderWidth = abs ( borderWidth );
		switch ( volume->state )
		{
			case STATE_FILLED :
                        printfInfo ( "Changing borderWidth of a filled volume" );
                            (void) fprintf ( stdout, "\tVolume named '%s'",
                                            volume->name );
                            (void) fprintf ( stdout, 
                                            "\n\tBorder from %d to %d\n",
                                            volume->borderWidth, borderWidth );
                            
				newVolume = duplicateVolumeStructure ( volume,
						"setBorderWidth_newVolume" );
				newVolume->borderWidth = borderWidth;
				transferVolumeData ( volume, newVolume );
				/* tricky */
				freeVolumeData ( volume );
				volume->data = newVolume->data;
				volume->tab = newVolume->tab;
				volume->state = STATE_FILLED;
				volume->borderWidth = borderWidth;
#ifdef MEMORYMAP
				volume->swapfile = newVolume->swapfile;
#endif
				volume->size3Db = newVolume->size3Db;
				free ( newVolume ); /* only the structure ! */
				break;
			case STATE_ALLOCATED :
                   printfInfo ( "Changing borderWidth of an allocated volume" );
                            (void) fprintf ( stdout, "\tVolume named '%s'",
                                            volume->name );
                            (void) fprintf ( stdout, 
                                            "\n\tBorder from %d to %d\n",
                                            volume->borderWidth, borderWidth );
				freeVolumeData ( volume );
				volume->borderWidth = borderWidth;
				allocateVolumeData ( volume );
				break;
			case STATE_DECLARED :
				volume->borderWidth = borderWidth;
				break;
/* This is unnecessary since already tested by 'testExist'
			case STATE_FREED :
                                printfError ( 
                                      "Can not access a volume that is freed." );
				goto abort;
*/
			default :
                                verifyState ( volume->state );
				goto abort;
		}
	}
        /* else do nothing */
/*	else
	{	
        printfInfo ( "Set border width used, no change necessary" );
	}
*/

TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort :
	printfexit ( "(volume)setBorderWidth" );
}



/*----------------------------------------------------------------------------*/

	void			setVolumeLevel

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	double			level   /*bnew: long ---> double */
)
{
	U8BIT_t	    *ptU8BIT, levelU8BIT;
	S8BIT_t	    *ptS8BIT, levelS8BIT;
	U16BIT_t	    *ptU16BIT, levelU16BIT;
	S16BIT_t	    *ptS16BIT, levelS16BIT;
	U32BIT_t	    *ptU32BIT, levelU32BIT;
	S32BIT_t	    *ptS32BIT, levelS32BIT;
	FLOAT_t	    *ptFLOAT, levelFLOAT;
	DOUBLE_t	    *ptDOUBLE;
	unsigned long	i, n;
	int			oTRACE;
/*I eliminated this:
        int     oldState;
*/
	
printftrace ( IN, "setVolumeLevel" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) )
/*bnew: I eliminated the following
||
		!testStateN ( volume, 3,
				STATE_DECLARED, STATE_ALLOCATED, STATE_FILLED ) )
unnecessary: STATE_FREED is already tested for in 'verifyAll'
*/
		goto abort;
	
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
		
        if ( !testLevelInType ( level, volume->type ) )
            goto abort;
        
	n =  offsetVolume ( volume );

/*bnew: I eliminated this.  It makes no sense to keep the old state.
	oldState = volume->state;
*/

	switch ( volume->type )
	{
		case U8BIT :
			levelU8BIT = (U8BIT_t)level;
			ptU8BIT = data_U8BIT ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptU8BIT++) = levelU8BIT;
			break;
		case S8BIT :
			levelS8BIT = (S8BIT_t)level;
			ptS8BIT = data_S8BIT ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptS8BIT++) = levelS8BIT;
			break;
		case U16BIT :
			levelU16BIT = (U16BIT_t)level;
			ptU16BIT = data_U16BIT ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptU16BIT++) = levelU16BIT;
			break;
		case S16BIT :
			levelS16BIT = (S16BIT_t)level;
			ptS16BIT = data_S16BIT ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptS16BIT++) = levelS16BIT;
			break;
		case U32BIT :
			levelU32BIT = (U32BIT_t)level;
			ptU32BIT = data_U32BIT ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptU32BIT++) = levelU32BIT;
			break;
		case S32BIT :
			levelS32BIT = (S32BIT_t)level;
			ptS32BIT = data_S32BIT ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptS32BIT++) = levelS32BIT;
			break;
		case VFLOAT :
			levelFLOAT = (FLOAT_t)level;
			ptFLOAT = data_VFLOAT ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptFLOAT++) = levelFLOAT;
			break;
		case VDOUBLE :
			ptDOUBLE = data_VDOUBLE ( volume );
if ( EXEC )
			for ( i = 0; i < n; i++ )
				*(ptDOUBLE++) = level;
			break;
		default :
                        notImplementedFor ( volume->type );
			goto abort;
	}
/*bnew: I eliminate this:
	volume->state = oldState;
*/

TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort :
	printfexit ( "(volume)setVolumeLevel" );
}




/*----------------------------------------------------------------------------*/

	void			setImageLevel

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	double			level
)
{
	U8BIT_t	    *ptU8BIT, levelU8BIT;
	S8BIT_t	    *ptS8BIT, levelS8BIT;
	U16BIT_t	    *ptU16BIT, levelU16BIT;
	S16BIT_t	    *ptS16BIT, levelS16BIT;
	U32BIT_t	    *ptU32BIT, levelU32BIT;
	S32BIT_t	    *ptS32BIT, levelS32BIT;
	FLOAT_t	    *ptFLOAT, levelFLOAT;
	DOUBLE_t	    *ptDOUBLE;
	int			ix, iy, iz,
				nx, ny, nz;
	long int		oPbL, oLbS;
	int			oTRACE;
	
printftrace ( IN, "setImageLevel" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) )
		goto abort;
/*bnew: cancel this test
 ||
		!testStateN ( volume, 3,
				STATE_DECLARED, STATE_ALLOCATED, STATE_FILLED ) )
*/	
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
		
        if ( !testLevelInType ( level, volume->type ) )
            goto abort;
        
	getSize3 ( volume, &nx, &ny, &nz );
	oPbL = offsetPointBetweenLine ( volume );
	oLbS = offsetLineBetweenSlice ( volume );
	switch ( volume->type )
	{
		case U8BIT :
			levelU8BIT = (U8BIT_t)level;
			ptU8BIT = data_U8BIT ( volume );
			ptU8BIT += offsetFirstPoint ( volume );
if ( EXEC )
                        for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptU8BIT++) = levelU8BIT;
			ptU8BIT += oPbL;
			}
			ptU8BIT += oLbS;
			}
			break;
		case S8BIT :
			levelS8BIT = (S8BIT_t)level;
			ptS8BIT = data_S8BIT ( volume );
			ptS8BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptS8BIT++) = levelS8BIT;
			ptS8BIT += oPbL;
			}
			ptS8BIT += oLbS;
			}
			break;
		case U16BIT :
			levelU16BIT = (U16BIT_t)level;
			ptU16BIT = data_U16BIT ( volume );
			ptU16BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptU16BIT++) = levelU16BIT;
			ptU16BIT += oPbL;
			}
			ptU16BIT += oLbS;
			}
			break;
		case S16BIT :
			levelS16BIT = (S16BIT_t)level;
			ptS16BIT = data_S16BIT ( volume );
			ptS16BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptS16BIT++) = levelS16BIT;
			ptS16BIT += oPbL;
			}
			ptS16BIT += oLbS;
			}
			break;
		case U32BIT :
			levelU32BIT = (U32BIT_t)level;
			ptU32BIT = data_U32BIT ( volume );
			ptU32BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptU32BIT++) = levelU32BIT;
			ptU32BIT += oPbL;
			}
			ptU32BIT += oLbS;
			}
			break;
		case S32BIT :
			levelS32BIT = (S32BIT_t)level;
			ptS32BIT = data_S32BIT ( volume );
			ptS32BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptS32BIT++) = levelS32BIT;
			ptS32BIT += oPbL;
			}
			ptS32BIT += oLbS;
			}
			break;
		case VFLOAT :
			levelFLOAT = (FLOAT_t)level;
			ptFLOAT = data_VFLOAT ( volume );
			ptFLOAT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptFLOAT++) = levelFLOAT;
			ptFLOAT += oPbL;
			}
			ptFLOAT += oLbS;
			}
			break;
		case VDOUBLE :
			ptDOUBLE = data_VDOUBLE ( volume );
			ptDOUBLE += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
			for ( iy = 0; iy < ny; iy++ )
			{
			for ( ix = 0; ix < nx; ix++ )
				*(ptDOUBLE++) = level;
			ptDOUBLE += oPbL;
			}
			ptDOUBLE += oLbS;
			}
			break;
		default :
                        notImplementedFor ( volume->type );
                        goto abort;
        }


TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort :
	printfexit ( "(volume)setImageLevel" );
}




/*----------------------------------------------------------------------------*/

	void			setBorderLevel

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	double		level
)
{
	U8BIT_t	    *ptU8BIT, levelU8BIT;
	S8BIT_t	    *ptS8BIT, levelS8BIT;
	U16BIT_t	    *ptU16BIT, levelU16BIT;
	S16BIT_t	    *ptS16BIT, levelS16BIT;
	U32BIT_t	    *ptU32BIT, levelU32BIT;
	S32BIT_t	    *ptS32BIT, levelS32BIT;
	FLOAT_t	    *ptFLOAT, levelFLOAT;
	DOUBLE_t	    *ptDOUBLE;
	int			nB, nS, nE,
				ix, iy, iz,
				nx, ny, nz;
	long	int		oL;
	int			oTRACE;
	
printftrace ( IN, "setBorderLevel" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) )
		goto abort;
/*bnew ||
		!testStateN ( volume, 3,
				STATE_DECLARED, STATE_ALLOCATED, STATE_FILLED ) )
*/

	if ( volume->borderWidth == 0 )
	{
          printfError ( "The volume has no border" );
          (void) fprintf ( stderr, "Volume name: '%s'\n", volume->name );
          goto abort;
	}

        if ( !testLevelInType ( level, volume->type ) )
            goto abort;
        
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );

	nB = offsetFirstPoint ( volume );
	oL = volume->size.x;
	nS = offsetLineBetweenSlice ( volume );
	nx = 2 * volume->borderWidth;
	ny = volume->size.y;
	nz = volume->size.z;
	nE = nB - nx - nS;
	switch ( volume->type )
	{
		case U8BIT :
			levelU8BIT = (U8BIT_t)level;
			ptU8BIT = data_U8BIT ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptU8BIT++) = levelU8BIT;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptU8BIT += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptU8BIT++) = levelU8BIT;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptU8BIT++) = levelU8BIT;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptU8BIT++) = levelU8BIT;
}                        
			break;
		case S8BIT :
			levelS8BIT = (S8BIT_t)level;
			ptS8BIT = data_S8BIT ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptS8BIT++) = levelS8BIT;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptS8BIT += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptS8BIT++) = levelS8BIT;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptS8BIT++) = levelS8BIT;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptS8BIT++) = levelS8BIT;
}                        
			break;
		case U16BIT :
			levelU16BIT = (U16BIT_t)level;
			ptU16BIT = data_U16BIT ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptU16BIT++) = levelU16BIT;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptU16BIT += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptU16BIT++) = levelU16BIT;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptU16BIT++) = levelU16BIT;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptU16BIT++) = levelU16BIT;
}                        
			break;
		case S16BIT :
			levelS16BIT = (S16BIT_t)level;
			ptS16BIT = data_S16BIT ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptS16BIT++) = levelS16BIT;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptS16BIT += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptS16BIT++) = levelS16BIT;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptS16BIT++) = levelS16BIT;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptS16BIT++) = levelS16BIT;
}                        
			break;
		case U32BIT :
			levelU32BIT = (U32BIT_t)level;
			ptU32BIT = data_U32BIT ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptU32BIT++) = levelU32BIT;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptU32BIT += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptU32BIT++) = levelU32BIT;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptU32BIT++) = levelU32BIT;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptU32BIT++) = levelU32BIT;
}                        
			break;
		case S32BIT :
			levelS32BIT = (S32BIT_t)level;
			ptS32BIT = data_S32BIT ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptS32BIT++) = levelS32BIT;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptS32BIT += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptS32BIT++) = levelS32BIT;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptS32BIT++) = levelS32BIT;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptS32BIT++) = levelS32BIT;
}                        
			break;
		case VFLOAT :
			levelFLOAT = (FLOAT_t)level;
			ptFLOAT = data_VFLOAT ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptFLOAT++) = levelFLOAT;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptFLOAT += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptFLOAT++) = levelFLOAT;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptFLOAT++) = levelFLOAT;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptFLOAT++) = levelFLOAT;
}                        
			break;
		case VDOUBLE :
			ptDOUBLE = data_VDOUBLE ( volume );
if (EXEC) {
			for ( ix = 0; ix < nB; ix++ )
				*(ptDOUBLE++) = level;
			for ( iz = 0; iz < nz; iz++ )
			{
				for ( iy = 0; iy < ny; iy++ )
				{
					ptDOUBLE += oL;
					for ( ix = 0; ix < nx; ix++ )
						*(ptDOUBLE++) = level;
				}
				for ( ix = 0; ix < nS; ix++ )
					*(ptDOUBLE++) = level;
			}
			for ( ix = 0; ix < nE; ix++ )
				*(ptDOUBLE++) = level;
}                        
			break;
		default :
                        notImplementedFor ( volume->type );
                        goto abort;
	}

TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort :
	printfexit ( "(volume)setBorderLevel" );
}




/*====================*
 |                    |
 |  offset functions  |
 |                    |
 *====================*/


/*----------------------------------------------------------------------------*/
	static
	int			sizeOf

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

	int			sizeofType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	int			size;
	
        if ( !verifyType ( volume->type ) )
            goto abort;
        size = sizeOf ( volume->type );
	return ( size );
abort :
	printfexit ( "(volume)sizeofType" );
	return ( -1 ); 
}
/* to eliminate in the future: */
/*----------------------------------------------------------------------------*/

	long			offset_forANYtype_BetweenPoint

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( sizeofType( volume ) );
}




/*----------------------------------------------------------------------------*/

	long			offsetFirstPoint

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( ( ( (long)(volume->size.x) + 2 * volume->borderWidth ) *
		( volume->size.y + 2 * volume->borderWidth + 1 ) + 1 ) *
		volume->borderWidth );
}





/*----------------------------------------------------------------------------*/

	long			offsetLine

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( (long)(volume->size.x) + 2 * volume->borderWidth );
}





/*----------------------------------------------------------------------------*/

	long			offsetPointBetweenLine

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( 2 * volume->borderWidth );
}





/*----------------------------------------------------------------------------*/

	long			offsetSlice

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( ( (long)(volume->size.y) + 2 * volume->borderWidth ) *
		( volume->size.x + 2 * volume->borderWidth ) );
}





/*----------------------------------------------------------------------------*/

	long			offsetLineBetweenSlice

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( 2 * (long)(volume->borderWidth) *
		( volume->size.x + 2 * volume->borderWidth ) );
}





/*----------------------------------------------------------------------------*/

	long			offsetVolume

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( ( (long)(volume->size.z) + 2 * volume->borderWidth ) *
		( volume->size.y + 2 * volume->borderWidth ) *
		( volume->size.x + 2 * volume->borderWidth ) );
}





/*----------------------------------------------------------------------------*/

	long			offsetImage

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	return ( (long)(volume->size.z) * volume->size.y * volume->size.x );
}





/*----------------------------------------------------------------------------*/
	
	long int	    *offsetBox
					/*  11/95: modified by A.Robert & T.Geraud  */
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			connexity
)
/*IMA_function: offsetBox
IMA_description: Returns a set of incremental offsets for a number of standard
neighbour systems.  Incremental means that you start with 
'ptrAboutThisPoint + offsetbox[0]',
and move on by adding for each new neighbour i 'offsetbox[i]' to the ptr to the
previous neighbour.
*/
{
	long int	    *offset,
				oL, oS;
	
	printftrace ( IN, "offsetBox" );

	if ( !testExist ( volume ) )
		goto abort;
	
	if ( volume->size.z == 1 )
	{
		if ( connexity != 4 && connexity != 8 )
                {
                  printfWarning ( "Connexity does not match volume size" );
                  (void) fprintf ( stderr, "\tConnexity %d, in volume '%s'",
                                 connexity, volume->name );
                  (void) fprintf ( stderr, "\n\t of size [%d, %d, %d, %d].\n",
                                  volume->size.x, volume->size.y, 
                                  volume->size.z, volume->size.t ); 
                }
        }
	else
	{
		if ( connexity != 6 && connexity != 18 && connexity != 26 
			&& connexity != 98 )
                {
                  printfWarning ( "Connexity does not match volume size" );
                  (void) fprintf ( stderr, "\tConnexity %d, in volume '%s'",
                                 connexity, volume->name );
                  (void) fprintf ( stderr, "\n\t of size [%d, %d, %d, %d].\n",
                                  volume->size.x, volume->size.y, 
                                  volume->size.z, volume->size.t ); 
                }
	}
	offset = (long int *) mallocT ( connexity * sizeof(long int) );
	oL = offsetLine ( volume );
	oS = offsetSlice ( volume );
	switch ( connexity )
	{
	/* 2D */
		case 4 :
			offset[0] = - oL;
			offset[1] = offset[3] = oL - 1;
			offset[2] = 2;
			break;
		case 8 :
			offset[0] = - oL - 1;
			offset[1] = offset[2] = offset[6] = offset[7] = 1;
			offset[3] = offset[5] = oL - 2;
			offset[4] = 2;
			break;
	/* 3D */
		case 6 :
			offset[0] = - oS;
			offset[1] = offset[5] = oS - oL;
			offset[2] = offset[4] = oL - 1;
			offset[3] = 2;
			break;
		case 18 :
			offset[0] = - oS - oL;
			offset[1] = offset[4] = offset[14] = offset[17] = oL - 1;
			offset[2] = offset[3] = offset[6] = offset[7] =
					offset[11] = offset[12] = offset[15] = offset[16] = 1;
			offset[5] = offset[13] = oS - 2 * oL - 1;
			offset[8] = offset[10] = oL - 2;
			offset[9] = 2;
			break;
			
		case 26 :
			offset[0] = - oS - oL - 1;
			offset[1] = offset[2] = offset[4] = offset[5] = offset[7] =
			offset[8] = offset[10] = offset[11] = offset[15] =
			offset[16] = offset[18] = offset[19] = offset[21] =
			offset[22] = offset[24] = offset[25] = 1;
			offset[3] = offset[6] = offset[12] = offset[14] =
					offset[20] = offset[23] = oL - 2;
			offset[9] = offset[17] = oS - 2 * oL - 2;
			offset[13] = 2;
			break;
			
		case 98 :
			offset[3] = offset[4] = offset[5] = offset[6] =
			offset[10] = offset[11] = offset[12] = offset[13] =
			offset[17] = offset[18] = offset[19] = offset[20] =
			offset[22] = offset[23] = offset[24] = offset[25] =
			offset[27] = offset[28] = offset[29] = offset[30] =
			offset[32] = offset[33] = offset[34] = offset[35] =
			offset[37] = offset[38] = offset[39] = offset[40] =
			offset[44] = offset[45] = offset[46] = offset[47] =
			offset[51] = offset[52] = offset[53] = offset[54] =
			offset[58] = offset[59] = offset[60] = offset[61] =
			offset[63] = offset[64] = offset[65] = offset[66] =
			offset[68] = offset[69] = offset[70] = offset[71] =
			offset[73] = offset[74] = offset[75] = offset[76] =
			offset[78] = offset[79] = offset[80] = offset[81] =
			offset[85] = offset[86] = offset[87] = offset[88] =
			offset[92] = offset[93] = offset[94] = offset[95] = 1;

			offset[1] = offset[8] = offset[15] =
			offset[42] = offset[49] = offset[56] = 
			offset[83] = offset[90] = offset[97] = 2;

			offset[7] = offset[14] = 
			offset[48] = offset[55] =
			offset[89] = offset[96] = oL -3;

			offset[2] = offset[9] = 
			offset[43] = offset[50] = 
			offset[84] = offset[91] = oL -3;
			
			offset[16] = offset[57] = oS - 4 * oL - 3;
			offset[41] = offset[82] = oS - 4 * oL -3;
			offset[21] = offset[26] = offset[31] = offset[36] =
			offset[62] = offset[67] = offset[72] = offset[77] = oL -4;
			offset[0] = -2 * oS - 2 * oL -1;
			break;
		default :
                        printfError ( 
			"Connexity (arg #2/2) has to be 4, 6, 8, 18, 26 or 98" );
			goto abort;
	}
	
	printftrace ( OUT, "" );
	return ( offset );
abort :
	printfexit ( "(volume)offsetBox" );
	return ( NULL );
}



/*============================*
 |                            |
 |  data accessing functions  |
 |                            |
 *============================*/


/*----------------------------------------------------------------------------*/

	U8BIT_t	    *data_U8BIT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
/*IMA_function: data_U8BIT - provide a pointer to U8BIT data

IMA_description: This function allocates memory if this has not been done 
before.  It puts the volume state to 'filled'.

There exist similar functions for all types.

IMA_return: A pointer to the U8BIT data of the volume.
*/
{
	printftrace ( IN, "data_U8BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, U8BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (U8BIT_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_U8BIT" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	S8BIT_t	    *data_S8BIT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_S8BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, S8BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (S8BIT_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_S8BIT" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	U16BIT_t	    *data_U16BIT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_U16BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, U16BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (U16BIT_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_U16BIT" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	S16BIT_t	    *data_S16BIT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_S16BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, S16BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (S16BIT_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_S16BIT" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	U32BIT_t	    *data_U32BIT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_U32BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, U32BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (U32BIT_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_U32BIT" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	S32BIT_t	   *data_S32BIT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_S32BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, S32BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (S32BIT_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_S32BIT" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	FLOAT_t	    *data_VFLOAT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_VFLOAT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, VFLOAT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (FLOAT_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_VFLOAT" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	DOUBLE_t	    *data_VDOUBLE

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_VDOUBLE" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, VDOUBLE ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (DOUBLE_t *)(volume->data) );
abort :
	printfexit ( "(volume)data_VDOUBLE" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	ANY_t	    *data_ANY

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	printftrace ( IN, "data_ANY" );

	if ( !verifyAll ( volume ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( volume->data );
abort :
	printfexit ( "(volume)data_ANY" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

/*	U8BIT_t    ***tab_U8BIT*/

/*---------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_U8BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, U8BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (U8BIT_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_U8BIT" );
	return ( NULL );
}
*/


/*----------------------------------------------------------------------------*/

/*	S8BIT_t    ***tab_S8BIT*/

/*----------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_S8BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, S8BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (S8BIT_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_S8BIT" );
	return ( NULL );
}
*/




/*----------------------------------------------------------------------------*/

/*	U16BIT_t    ***tab_U16BIT */

/*----------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_U16BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, U16BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (U16BIT_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_U16BIT" );
	return ( NULL );
}
*/




/*----------------------------------------------------------------------------*/

/*	S16BIT_t    ***tab_S16BIT*/

/*----------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_S16BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, S16BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (S16BIT_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_S16BIT" );
	return ( NULL );
}
*/




/*----------------------------------------------------------------------------*/

/*	U32BIT_t    ***tab_U32BIT */

/*----------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_U32BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, U32BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (U32BIT_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_U32BIT" );
	return ( NULL );
}*/





/*----------------------------------------------------------------------------*/

/*	S32BIT_t   ***tab_S32BIT */

/*----------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_S32BIT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, S32BIT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (S32BIT_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_S32BIT" );
	return ( NULL );
}*/





/*----------------------------------------------------------------------------*/

/*	FLOAT_t    ***tab_FLOAT */

/*----------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_FLOAT" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, VFLOAT ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (FLOAT_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_FLOAT" );
	return ( NULL );
} */





/*----------------------------------------------------------------------------*/

/*	DOUBLE_t    ***tab_DOUBLE */

/*----------------------------------------------------------------------------*/
/*(
	Volume	    *volume
)
{
	printftrace ( IN, "tab_DOUBLE" );

	if ( !verifyAll ( volume ) ||
		!testType ( volume, VDOUBLE ) )
		goto abort;
	if ( volume->state == STATE_DECLARED )
		allocateVolumeData ( volume );
	setState ( volume, STATE_FILLED );

	printftrace ( OUT, "" );
	return ( (DOUBLE_t ***)(volume->tab) );
abort :
	printfexit ( "(volume)tab_DOUBLE" );
	return ( NULL );
}*/





/*==========================*
 |                          |
 |  input-output functions  |
 |                          |
 *==========================*/


/*----------------------------------------------------------------------------*/
	static
	void			scanNflush				/*NEW*/

/*----------------------------------------------------------------------------*/
(
	char		   **str,
	char		    *strRef
)
/* Read the next word from '*str', put it in 'strRef' and move '*str'
behind the word. */
{
        /* read a word from the string '*str' and put it in 'strRef' */
	sscanf ( *str, "%s", strRef );
        /* advance '*str' over leading whitespace */
	while ( **str != *strRef )	/*  perhaps unusefull  */
		(*str)++;
        /* advance '*str' over the word itself */
	while ( *(*str)++ == *strRef++ );
        /* advance '*str' over trailing whitespace */
	while ( **str == ' ' || **str == '\t' || **str == '\n' )
		(*str)++;		/*  flushes end of lines  */
}


/*bnew: I use 'Roi' instead,  look into volume.h
typedef struct
{
	char		    refname[NAME_MAXLEN+1];
	int			xref, yref, zref, tref;
	int			xroi, yroi, zroi, troi;
	int			dxroi, dyroi, dzroi, dtroi;
}
	extraVolume;
*/


/*bnew*/
/*----------------------------------------------------------------------------*/
static
char *imaFile ( const char *filename )

/*----------------------------------------------------------------------------*/
/* Creates a filename ending on '.ima' */
{
  size_t l;
  char *filenameIma, *extIma = ".ima", *extDim = ".dim";

  if ( filename == NULL )
  {
    printfError ( "filename == NULL, contact a Tivoli administrator !" );
    printfexit( "(volume)imaFile" );
  }
  l = strlen ( filename );

  filenameIma = charMalloc ( l + 5 );

  strcpy ( filenameIma, filename );

  /*bnew: added test l<=5 */
  if ( l<=5 || (strcmp ( filenameIma+l-5, extIma ) &&  
      strcmp ( filenameIma+l-5, extDim )) )
  {
    strcat ( filenameIma, extIma );
  }
  else
      strcpy ( filenameIma+l-5, extIma );

  return ( filenameIma );
  
}

/*bnew*/
/*----------------------------------------------------------------------------*/
static
char *dimFile ( const char *filename )

/*----------------------------------------------------------------------------*/
/* Creates a filename ending on '.dim' */
{
  size_t l;
  char *filenameDim, *extIma = ".ima", *extDim = ".dim";

  if ( filename == NULL )
  {
    printfError ( "filename == NULL, contact a Tivoli administrator !" );
    printfexit( "(volume)dimFile" );
  }
  l = strlen ( filename );

  filenameDim = charMalloc ( l + 5 );

  strcpy ( filenameDim, filename );

  /*bnew: added test l<=5 */
  if ( l<=5 || (strcmp ( filenameDim+l-5, extIma ) &&
       strcmp ( filenameDim+l-5, extDim )) )
  {
    strcat ( filenameDim, extDim );
  }
  else
      strcpy ( filenameDim+l-5, extDim );

  return ( filenameDim );
  
}


/*----------------------------------------------------------------------------*/
static
void errorDuplicateFields ( char *s )
/*----------------------------------------------------------------------------*/
{
  printfWarning ( "" );
  (void) fprintf ( stderr,
                  "Several '-%s' fields in dimension file !\n", s );
}

/*----------------------------------------------------------------------------*/
	
	Volume *readVolumeDim

/*----------------------------------------------------------------------------*/
/*IMA_function: readVolumeDim - Read the volume structure using .dim information
IMA_description: This function existed earlier with other parameters
pleas change them !  The dimension information is now returned as
a volume structure.
The old function is renamed to readVolumeDim_OLD but should 
disappear in the future.
*/
(
	const char    *filename,
        Roi        *roi
)
{
	char		    *filenameDim,
				str[1024],
			    *strc,
				strField[1024], strArg[1024], strType[64];
	FILE		    *fileDim;
	int			narg,
				count_dx, count_dy, count_dz, count_dt,
				count_type, count_ref, count_bo, count_om,
                                count_x1, count_x2, count_y1, count_y2, 
                                count_z1, count_z2, count_t1, count_t2;
	char		    *c;
	int	    nx, ny, nz, nt, type;
        float	    dx, dy, dz, dt;
        int         xref, yref, zref, tref;
        int         x1, x2, y1, y2, z1, z2, t1, t2;
        char nameref[NAME_MAXLEN];
	int test_byteswaping = 0x41424344;
	char read_byteswaping[1000];

        DECL_VOLUME(vol);
        
	printftrace ( IN, "readVolumeDim" );

        filenameDim = dimFile ( filename );
	fileDim = fopen ( filenameDim, "r" );
	if ( fileDim == NULL )
	{
          printfError ( "File read error" );
          (void) fprintf ( stderr,
                          "File '%s' can not be opened for reading\n", 
                          filenameDim );
          goto abort;
        }
	(void) fgets ( str, 1023, fileDim );	/*  read the first line  */
	c = str;
	/* verify that first line consists: 0..9, spc and tab */
	while ( *c != '\n' )
	{
		if ( ( *c < '0' || *c > '9' ) && *c != ' ' && *c != '\t' )
                {
                  printfWarning ( 
                        "Non-numerical characters on first line of dim file" );
                  goto abortReturn;
                }
                c++;
	}
	narg = sscanf ( str, "%d %d %d %d", &nx, &ny, &nz, &nt );
        printfInfo ( "Reading .dim file:" );
	switch ( narg )	/*  analyze the first line  */
	{
		case 2 :
                    if ( VERBOSE )  
                        (void) fprintf ( stdout,
                                        "Image %d x %d\n", nx, ny );
                    nz = nt = 1;
                    break;
		case 3 :
                    if ( VERBOSE )  
                        (void) fprintf ( stdout,
                                        "Image %d x %d x %d\n", nx, ny, nz );
                    nt = 1;
                    break;
		case 4 :
                    if ( VERBOSE )  
                        (void) fprintf ( stdout,
                                  "Image %d x %d x %d x %d\n", nx, ny, nz, nt );
                    break;
                default :
                    printfWarning ( 
                             "First line of .dim file is not of Tivoli type." );
                    goto abort;
        }

        if ( !verifySizes ( nx, ny, nz, nt ) )
            goto abort;
        
        /* default values for the other fields */
	dx = dy = dz = dt = 1.0;
	type = 10;
        vol = declareVolumeStructure4( nx, ny, nz, nt, dx, dy, dz, dt,
                                          type, filename, 0 );
	(void) strcpy ( nameref, "" );
	xref = yref = zref = tref = 0;
        vol->ref.x = xref;
        vol->ref.y = yref;
        vol->ref.z = zref;
        vol->ref.t = tref;
        strcpy(vol->ref.name, nameref);
        roi->state = OFF;
        roi->x1 = 0;
        roi->y1 = 0;
        roi->z1 = 0;
        roi->t1 = 0;
        roi->x2 = nx-1;
        roi->y2 = ny-1;
        roi->z2 = nz-1;
        roi->t2 = nt-1;
	vol->shfj->byte_swapping_actif = VFALSE;

        
	count_dx = count_dy = count_dz = count_dt = 0;
	count_type = 0;
	count_ref = 0;
	count_bo = 0;
	count_om = 0;
	count_x1 = count_y1 = count_z1 = count_t1 = 0;
	count_x2 = count_y2 = count_z2 = count_t2 = 0;

	while ( fgets ( str, 1023, fileDim ) )	/*  read a line  */

	if ( *str != '\n' )
	{
		strc = str;
		do
		{
			scanNflush ( &strc, strField );
/* dx */		if ( !strcmp ( strField, "-dx" ) )
			{
				if ( ++count_dx > 1 )
				{
                                  errorDuplicateFields ( "dx" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%f", &dx );
                                vol->voxelSize.x = dx;
				if ( VERBOSE )  
                                    (void)fprintf ( stdout, "dx = %f\n", dx );
			}
			else
/* dy */		if ( !strcmp ( strField, "-dy" ) )
			{
				if ( ++count_dy > 1 )
				{
                                  errorDuplicateFields ( "dy" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%f", &dy );
                                vol->voxelSize.y = dy;
				if ( VERBOSE )  
                                    (void)fprintf ( stdout, "dy = %f\n", dy );
			}
			else
/* dz */		if ( !strcmp ( strField, "-dz" ) )
			{
				if ( narg == 2 )
				{
                                  printfWarning ( 
                                    "dz field incompatible with 2D format !\n" );
                                  goto abort;
				}
				if ( ++count_dz > 1 )
				{
                                  errorDuplicateFields ( "dz" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%f", &dz );
                                vol->voxelSize.z = dz;
				if ( VERBOSE )  
                                    (void)fprintf ( stdout, "dz = %f\n", dz );
			}
			else
/* dt */		if ( !strcmp ( strField, "-dt" ) )
			{
				if ( narg == 2 )
				{
                                  printfWarning ( 
                                    "dt field incompatible with 2D format !\n" );
                                  goto abort;
				}
				if ( narg == 3 )
				{
                                  printfWarning ( 
                                    "dt field incompatible with 3D format !\n" );
                                  goto abort;
				}
				if ( ++count_dt > 1 )
				{
                                  errorDuplicateFields ( "dt" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%f", &dt );
                                vol->voxelSize.t = dt;
				if ( VERBOSE )  
                                    (void)fprintf ( stdout, "dt = %f\n", dt );
			}
			else
/* bo */		if ( !strcmp ( strField, "-bo" ) )
			{
				if ( ++count_bo > 1 )
				{
                                  errorDuplicateFields ( "bo" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%s", read_byteswaping);
				if(read_byteswaping[0]!=((char *)(&test_byteswaping))[0])
				  {
				    vol->shfj->byte_swapping_actif = VTRUE;
				    printf("Byte swapping active\n");
				  }
				else
				  {
				    vol->shfj->byte_swapping_actif = VFALSE;
				    printf("No byte swapping\n");
				  }
				
			}
			else
/* om */		if ( !strcmp ( strField, "-om" ) )
			{
				if ( ++count_om > 1 )
				{
                                  errorDuplicateFields ( "om" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%s", read_byteswaping);
				if(strcmp(read_byteswaping,"binar"))
				  {
				    VipPrintfError("Unsupported -om mode");
				    printf("om mode: %s\n",read_byteswaping);
				  }
			}
			else
/* ref */		if ( !strcmp ( strField, "-ref" ) )
			{
				if ( ++count_ref > 1 )
				{
                                  errorDuplicateFields ( "ref" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
                                /*bnew: the constant 1023 should be equal
                                  to NAME_MAXLEN. Does anyone knows a better
                                  way of coding this ?*/
				sscanf ( strArg, "%1023s", nameref );
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%d", &(xref) );
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%d", &(yref) );
				if ( narg > 2 )
				{
					scanNflush ( &strc, strArg );
					sscanf ( strArg, "%d", &(zref) );
				}
				if ( narg > 3 )
				{
					scanNflush ( &strc, strArg );
					sscanf ( strArg, "%d", &(tref) );
				}
                                vol->ref.x = xref;
                                vol->ref.y = yref;
                                vol->ref.z = zref;
                                vol->ref.t = tref;
                                strcpy ( vol->ref.name, nameref );
				if ( VERBOSE )
                                    (void)fprintf ( stdout, 
                                          "Ref to '%s' at [%d, %d, %d, %d]\n", 
                                                   nameref, 
                                                   xref, yref, zref, tref );
			}
			else
/* x1 */
                            if ( !strcmp ( strField, "-x1" ) )
                            {
                              if ( ++count_x1 > 1 )
                              {
                                errorDuplicateFields ( "x1" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else 
                              {
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &x1 );
                                roi->x1 = x1;
                                roi->state = ON;
                              }
                            }
			else
/* x2 */
                            if ( !strcmp ( strField, "-x2" ) )
                            {
                              if ( ++count_x2 > 1 )
                              {
                                errorDuplicateFields ( "x2" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else
                              {
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &x2 );
                                roi->x2 = x2;
                                roi->state = ON;
                              }
                            }
			else
/* y1 */
                            if ( !strcmp ( strField, "-y1" ) )
                            {
                              if ( ++count_y1 > 1 )
                              {
                                errorDuplicateFields ( "y1" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else
                              {
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &y1 );
                                roi->y1 = y1;
                                roi->state = ON;
                              }
                            }
			else
/* y2 */
                            if ( !strcmp ( strField, "-y2" ) )
                            {
                              if ( ++count_y2 > 1 )
                              {
                                errorDuplicateFields ( "y2" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else
                              {
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &y2 );
                                roi->y2 = y2;
                                roi->state = ON;
                              }
                            }
			else
/* z1 */
                            if ( !strcmp ( strField, "-z1" ) )
                            {
                              if ( ++count_z1 > 1 )
                              {
                                errorDuplicateFields ( "z1" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else
                              {
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &z1 );
                                roi->z1 = z1;
                                roi->state = ON;
                              }
                            }
			else
/* z2 */
                            if ( !strcmp ( strField, "-z2" ) )
                            {
                              if ( ++count_z2 > 1 )
                              {
                                errorDuplicateFields ( "z2" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else
                              {
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &z2 );
                                roi->z2 = z2;
                                roi->state = ON;
                              }
                            }
			else
/* t1 */
                            if ( !strcmp ( strField, "-t1" ) )
                            {
                              if ( ++count_t1 > 1 )
                              {
                                errorDuplicateFields ( "t1" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else
                              {
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &t1 );
                                roi->t1 = t1;
                                roi->state = ON;
                              }
                            }
			else
/* t2 */
                            if ( !strcmp ( strField, "-t2" ) )
                            {
                              if ( ++count_t2 > 1 )
                              {
                                errorDuplicateFields ( "t2" );
                                (void) fprintf ( stderr, "I'll ignore it." );
                              }
                              else
                              { 
                                scanNflush ( &strc, strArg );
                                sscanf ( strArg, "%d", &t2 );
                                roi->t2 = t2;
                                roi->state = ON;
                              }
                            }
			else
/* type */	if ( !strcmp ( strField, "-type" ) )
			{
                                if ( ++count_type > 1 )
				{
                                  errorDuplicateFields ( "type" );
                                  goto abort;
				}
				scanNflush ( &strc, strArg );
				sscanf ( strArg, "%s", strType );
				if ( !strcmp ( strType, "U8" ) )
					type = 10;
				else
				if ( !strcmp ( strType, "S8" ) )
					type = 11;
				else
				if ( !strcmp ( strType, "U16" ) )
					type = 20;
				else
				if ( !strcmp ( strType, "S16" ) )
					type = 21;
				else
				if ( !strcmp ( strType, "U32" ) )
					type = 40;
				else
				if ( !strcmp ( strType, "S32" ) )
					type = 41;
				else
				if ( !strcmp ( strType, "FLOAT" ) )
					type = 400;
				else
				if ( !strcmp ( strType, "DOUBLE" ) )
					type = 800;
				else
				{
                                  printfWarning ( "Type name unknown" );
                                  (void) fprintf ( stderr, "Type '%s' ?\n", 
                                                  strType ); 
                                  goto abort;
				}
                                vol->type = type;
				if ( VERBOSE )
                                    (void)fprintf ( stdout, "type = %s\n", 
                                                   strType );
			}
			else
/* txt */		if ( !strcmp ( strField, "-txt" ) )
			{
				if ( VERBOSE )  
                                    (void) fprintf ( stdout, "txt = \"" );
				c = strc;
				while ( *c != '-' )
				{
					if ( *c != '\n' )
					{
                                          if ( VERBOSE )
                                              (void)fprintf ( stdout, "%c", *c );
                                          c++;
					}
					else
					{
                                          if ( VERBOSE )  
                                              (void) fprintf ( stdout, "\n" );
                                          if ( ! fgets ( str, 1023, fileDim ) )
                                          {
                                            printfError ( 
                                            "No end of text in .dim file !" );
                                            goto abort;
                                          }
                                          c = str;
					}
				}
				strc = c;
				scanNflush ( &strc, strArg );
				if ( strcmp ( strArg, "-endtxt" ) )
				{
                                  printfError ( 
                                         "Bad txt format in .dim file !" );
                                  goto abort;
				}
				if ( VERBOSE )  
                                    (void) fprintf ( stdout, "\"\n" );
			}
			else
                        {/* Jeff bidouille pour autoriser les autres champs GIS
                          printfError ( "Unknown field in dim file" );
                          (void) fprintf ( stderr, "Field: %s\n", 
                                          strField );
                          goto abortReturn;*/
                        }
		}
		while ( strc < str + strlen(str) );
	}
	(void) fflush ( stdout );
	fclose ( fileDim );
        
        if ( !verifyVoxelSizes ( dx, dy, dz, dt ) )
            goto abort;

        if ( roi->state == ON && verifyRoi( roi, vol ) == VFALSE )
        {
          printfWarning ( "I changed the region-of-interest specification" );
          (void)fprintf ( stderr, "\tto [ %d-%d, %d-%d, %d-%d, %d-%d ]\n",
                           roi->x1, roi->x2, roi->y1, roi->y2,  
                           roi->z1, roi->z2, roi->t1, roi->t2 );
        }
        else if ( VERBOSE && roi->state == ON )
        {
          printfInfo ( "Region-of-interest specification" );
          (void)fprintf ( stdout, "\t [ %d-%d, %d-%d, %d-%d, %d-%d ]\n",
                           roi->x1, roi->x2, roi->y1, roi->y2, 
                           roi->z1, roi->z2, roi->t1, roi->t2 );
        }
                
        free ( filenameDim );
	/*	printf("%p (Tfree)\n",filenameDim);*/

        
	printftrace ( OUT, "" );
	return ( vol );
        
abortReturn:
        return ( NULL );
        
abort:    
        (void) fprintf ( stderr, "Dimension file: '%s'\n", 
                        filenameDim );
	printfexit ( "(volume)readVolumeDim" );
	return ( NULL );
}





/* Please refer to 'readVolumeDim', because the following function is old and
   should disappear in the future.
*/
/*----------------------------------------------------------------------------*/

	int			readVolumeDim_OLD		

/*----------------------------------------------------------------------------*/
(
	const char    *filename,
	int		    *nx,
	int		    *ny,
	int		    *nz,
	int		    *type,
	float	    *dx,
	float	    *dy,
	float	    *dz
)
{
        char		    *filenameDim;
  	FILE		    *fileDim;
	int			narg;
	float		f;

	printftrace ( IN, "readVolumeDim_OLD" );

        filenameDim = dimFile ( filename );
        fileDim = fopen ( filenameDim, "r" );

	*nx = *ny = *nz = 0;
	*dx = *dy = *dz = 0.;
	*type = 0; /* UNKNOWN */
	if ( fileDim == NULL )
	{
		(void) fprintf ( stderr,
				"file '%s' can not be accessed\n", filenameDim );
		goto abort;
        }
	narg = fscanf ( fileDim, "%d %d %d\n%d\n%f %f %f %f",
			nx, ny, nz, type, dx, dy, dz, &f );
			
/*  ?? the file contents must be verified; testing narg is not a proof...  */
	
	switch ( narg )
	{
		case 2 :		/*  old 2d format  */
		
			if ( !verifySizes ( *nx, *ny, 1, 1 ) )
				goto abort;
			*nz = 1;	
			*type = U8BIT;
			*dx = *dy = 1.;
			/* only difference with the one-sliced old 3d format: */
			*dz = 0.;
			if ( VERBOSE )
				(void) fprintf ( stdout,
				"2d image (old format): %d x %d\n", *nx, *ny );
			break;
			
		case 3 :		/*  short 3d format (very old tivoli format)  */
			
			if ( !verifySizes ( *nx, *ny, *nz, 1 ) )
				goto abort;
			*type = U8BIT;
			*dx = *dy = *dz = 1.;
			if ( VERBOSE )
				(void) fprintf ( stdout,
			"3d image (very old format): %d x %d x %d\n", *nx, *ny, *nz );
			break;
			
		case 7 :		/*  long 3d format (old tivoli format)  */
			
			if ( !verifySizes ( *nx, *ny, *nz, 1 ) ||
				!verifyVoxelSizes ( *dx, *dy, *dz, 1.0 ) ||
				!verifyType ( *type ) )
				goto abort;
			if ( *dz == 0 )
			{
				if ( *nz != 1 )
				{
					(void)fprintf ( stderr,
				"bad format (no slice thickness but several slices) !\n" );
					goto abort;
				}
				if ( *type != U8BIT )
				{
					(void)fprintf ( stderr,
				"bad format (no slice thickness and not U8BIT data) !\n" );
					goto abort;
				}
			}
			if ( VERBOSE )
			{
				(void) fprintf ( stdout,
				"3d image (old format): %d x %d x %d\n", *nx, *ny, *nz );
				(void) fprintf ( stdout,
				"type: %s\n", typeName ( *type ) );
				if ( *dx != 1 || *dy != 1 || *dz != 1 )
					(void) fprintf ( stdout,
					"voxel size: %f x %f x %f\n", *dx, *dy, *dz );
			}
			break;
		default :
			(void)fprintf ( stderr, "bad .dim format !\n" );
			goto abort;
	}
	fclose ( fileDim );

	printftrace ( OUT, "" );
	return ( OK );
abort :
	printfexit ( "(volume)readVolumeDim_OLD" );
	return ( PB );
}




/*----------------------------------------------------------------------------*/

	Volume	    *readVolume				/*NEW*/
					/*  12/95: modified by T.Geraud  */
/*----------------------------------------------------------------------------*/
(
	const char    *filename,
	int			borderWidth
)
{
  char		    *filenameIma,
    charP, char5, charD;
  FILE		    *fileIma;
  int			iy, iz, it,
    nx, ny, nz,nt=0,
    oL, oLbS, osbv, offset1, offset2,
    type, other,
    bytesPerVoxel;
  float		dx, dy, dz;
  long			supLevel,
    fileSize,
    pct;
  Volume	    *newVolume;
  U8BIT_t	    *ptU8BIT;
  S8BIT_t	    *ptS8BIT;
  U16BIT_t	    *ptU16BIT;
  S16BIT_t	    *ptS16BIT;
  U32BIT_t	    *ptU32BIT;
  S32BIT_t	    *ptS32BIT;
  FLOAT_t	    *ptFLOAT;
  DOUBLE_t	    *ptDOUBLE;
  Roi        roi;
  int              dxroi=0, dyroi=0, dzroi=0, dtroi=0;
  int			oTRACE;

  printftrace ( IN, "readVolume" );
  oTRACE = TRACE; TRACE = OFF;

  if ( !verifyBorderWidth ( borderWidth ) )
    goto abort;

  if ( filename == NULL || strlen ( filename ) == 0 )
    {
      printfError ( "Invalid filename (NULL or length 0)" );
      goto abort;
    }
        
  /* opening file .ima */	
  filenameIma = imaFile ( filename );
  fileIma = fopen ( filenameIma, "rb" );

  if ( fileIma == NULL )
    {
      printfError ( "File read error" );
      (void) fprintf ( stderr,
		       "File '%s' can not be opened for reading\n", 
		       filenameIma );
      goto abort;
    }
  if ( fseek ( fileIma, 0L, SEEK_END ) )
    {
      printfError ( "Going to the end of file with 'fseek' failed" );
      goto abort;
    }
  fileSize = ftell ( fileIma );
  rewind ( fileIma );
	
  nz=1;
  dz=0.;
  roi.state = OFF;

  other = VFALSE;

  newVolume = readVolumeDim ( filename, &roi );
  if ( newVolume != NULL )
    { 
      /* OK, read .dim without problems */
      nx = newVolume->size.x;
      ny = newVolume->size.y;
      nz = newVolume->size.z;
      nt = newVolume->size.t;
      type = newVolume->type;
      newVolume->borderWidth = borderWidth;
      if ( roi.state == ON )
	{
	  /* adapt newVolume to receive only the ROI */
	  dxroi = roi.x2 - roi.x1 + 1;
	  dyroi = roi.y2 - roi.y1 + 1;
	  dzroi = roi.z2 - roi.z1 + 1;
	  dtroi = roi.t2 - roi.t1 + 1;
            
	  setSize4 ( newVolume, dxroi, dyroi, dzroi, dtroi );
            
	  (void) strcpy ( newVolume->ref.name, newVolume->name );
	  if ( strlen( newVolume->name ) < NAME_MAXLEN-4 )
	    (void) strcat ( newVolume->name, "ROI" );
            
	  newVolume->ref.x = roi.x1;
	  newVolume->ref.y = roi.y1;
	  newVolume->ref.z = roi.z1;
	  newVolume->ref.t = roi.t1;
	}
    }          
  else  /* try the old tivoli-type */
    if ( readVolumeDim_OLD ( filename,
			     &nx, &ny, &nz, &type, &dx, &dy, &dz ) )
      {
	newVolume = createVolume4 ( nx, ny, nz, 1, dx, dy, dz, 1.0,
				    type, filename, borderWidth );
      }

    else		/* other formats: xv, old IMA */
      {
	fscanf ( fileIma, "%c%c\n%c", &charP, &char5, &charD );
	if ( charP == 'P' && char5 == '5' && charD == '#' )
	  /* xv header for a 2D PBM(raw)-Greyscale image */
	  {
	    do
	      fscanf ( fileIma, "%c", &charD );
	    while ( charD != '\n' );			
	    fscanf ( fileIma, "%d%d\n", &nx, &ny );
	    supLevel = 0;
	    charD = '0';
	    do
	      {
		supLevel = supLevel * 10 + (int)(charD - '0');
		fscanf ( fileIma, "%c", &charD );
	      }
	    while ( charD != '\n' );
	    nz = 1;
	    dx = dy = 1.;
	    dz = 0.; /* 2d format */
	    other = VTRUE;
	    if ( supLevel == 256 )
	      type = U8BIT;
	    else
	      {
		printfError ( 
			     "Error in xv PBM(raw)-Greyscale format !" );
		goto abort;
	      }
	    newVolume = createVolume4 ( nx, ny, nz, 1, 
					dx, dy, dz, 1.0,
					type, filename, 
					borderWidth );
	  }
	else
	  {
	    rewind ( fileIma );
	    /* insert here new formats */
	    printfError ( "Unknown file format" );
	    goto abort;
	  }
      }

  /*?theo: no test for other formats ?? *//*JEFF nt 6-5-97*/
  if ( !other  &&  fileSize != (long)nx * ny * nz * nt * sizeOf ( type ) )
    {
      printfError ( "Size of .ima does not match specifications in .dim" );
      fprintf ( stderr, "\tFilesize = %ld bytes\n", fileSize );
      fprintf ( stderr, "\tThis should be %ld\n.",  
		(long) nx * ny * nz * nt * sizeOf ( type ) );
      goto abort;
    }

  bytesPerVoxel = sizeofType ( newVolume );
  oL = offsetLine ( newVolume );
  oLbS = offsetLineBetweenSlice( newVolume);
  osbv = (2*borderWidth) *( ny + 2*borderWidth) *(nx+2*borderWidth) ;
	

  if ( roi.state == OFF )
    {
      /* read the complete volume */
      switch ( newVolume->type )
	{
	case U8BIT :
	  ptU8BIT = data_U8BIT ( newVolume );
	  ptU8BIT += offsetFirstPoint ( newVolume );
	  if ( EXEC )
  	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptU8BIT, bytesPerVoxel, nx, fileIma );
			ptU8BIT += oL;
		      }
		    ptU8BIT += oLbS;
		  }
		ptU8BIT += osbv;
	      }
	  break;
	case S8BIT :
	  ptS8BIT = data_S8BIT ( newVolume );
	  ptS8BIT += offsetFirstPoint ( newVolume );
	  if ( EXEC )
  	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptS8BIT, bytesPerVoxel, nx, fileIma );
			ptS8BIT += oL;
		      }
		    ptS8BIT += oLbS;
		  }
		ptS8BIT += osbv;
	      }
	  break;
	case U16BIT :
	  ptU16BIT = data_U16BIT ( newVolume );
	  ptU16BIT += offsetFirstPoint ( newVolume );
	  if ( EXEC )
	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptU16BIT, bytesPerVoxel, nx, fileIma );
			if(newVolume->shfj->byte_swapping_actif==VTRUE)
			  VipByteSwapping_U16BIT( ptU16BIT, bytesPerVoxel, nx);
			ptU16BIT += oL;
		      }
		    ptU16BIT += oLbS;
		  }
		ptU16BIT += osbv;
	      }
	  break;
	case S16BIT :
	  ptS16BIT = data_S16BIT ( newVolume );
	  ptS16BIT += offsetFirstPoint ( newVolume );
	  if ( EXEC )
	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptS16BIT, bytesPerVoxel, nx, fileIma );
			if(newVolume->shfj->byte_swapping_actif==VTRUE)
			  VipByteSwapping_S16BIT( ptS16BIT, bytesPerVoxel, nx);
			ptS16BIT += oL;
		      }
		    ptS16BIT += oLbS;
		  }
		ptS16BIT += osbv;
	      }
	  break;
	case U32BIT :
	  ptU32BIT = data_U32BIT ( newVolume );
	  ptU32BIT += offsetFirstPoint ( newVolume );
	 
	  if ( EXEC )
	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptU32BIT, bytesPerVoxel, nx, fileIma );
			if(newVolume->shfj->byte_swapping_actif==VTRUE) 
			  /*truande a cause de la suppression du 32BIT, j'espere que ca marche...*/
			  VipByteSwapping_S32BIT( (Vip_S32BIT *)ptU32BIT, bytesPerVoxel, nx);
			ptU32BIT += oL;
		      }
		    ptU32BIT += oLbS;
		  }
		ptU32BIT += osbv;
	      }
	  break;
	case S32BIT :
	  ptS32BIT = data_S32BIT ( newVolume );
	  ptS32BIT += offsetFirstPoint ( newVolume );
	  if ( VERBOSE )
	    (void)fprintf ( stdout,
			    "\nReading '%s' : 00 %%", 
			    filename );
	  if ( EXEC )
	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    if ( VERBOSE )
		      {
			pct = 100 * (long int)iz / (long int)nz;
			(void)fprintf ( stdout,
					"\b\b\b\b%2ld %%", pct );
			fflush ( stdout );
		      }
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptS32BIT, bytesPerVoxel, nx, fileIma );
			if(newVolume->shfj->byte_swapping_actif==VTRUE)
			  VipByteSwapping_S32BIT( ptS32BIT, bytesPerVoxel, nx);
			ptS32BIT += oL;
		      }
		    ptS32BIT += oLbS;
		  }
		ptS32BIT += osbv;
	      }
	  if ( VERBOSE )
	    (void)fprintf ( stdout, "\b\b\b\b\b\bcompleted.\n" );
	  break;
	case VFLOAT :
	  ptFLOAT = data_VFLOAT ( newVolume );
	  ptFLOAT += offsetFirstPoint ( newVolume );
	  if ( EXEC )
	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptFLOAT, bytesPerVoxel, nx, fileIma );
			if(newVolume->shfj->byte_swapping_actif==VTRUE)
			  VipByteSwapping_VFLOAT( ptFLOAT, bytesPerVoxel, nx);
			ptFLOAT += oL;
		      }
		    ptFLOAT += oLbS;
		  }
		ptFLOAT += osbv;
	      }
	  break;
	case VDOUBLE :
	  ptDOUBLE = data_VDOUBLE ( newVolume );
	  ptDOUBLE += offsetFirstPoint ( newVolume );
	  if ( EXEC )
	    for ( it = 0; it < nt; it++ )
	      {
		for ( iz = 0; iz < nz; iz++ )
		  {
		    for ( iy = 0; iy < ny; iy++ )
		      {
			fread ( ptDOUBLE, bytesPerVoxel, nx, fileIma );
			if(newVolume->shfj->byte_swapping_actif==VTRUE)
			  VipByteSwapping_VDOUBLE( ptDOUBLE, bytesPerVoxel, nx);
			ptDOUBLE += oL;
		      }
		    ptDOUBLE += oLbS;
		  }
		ptDOUBLE += osbv;
	      }
	  break;
	default :
	  notImplementedFor ( newVolume->type );
	  goto abort;
	}
    }
  else /* when roi.state is ON, only read the roi (4D not implemented JEFF)*/
    {
      roi.x1 *= bytesPerVoxel;
      roi.y1 *= bytesPerVoxel;
      roi.z1 *= bytesPerVoxel;
      offset1 = (nx-roi.x2-1)*bytesPerVoxel;
      offset2 = (ny-roi.y2-1)*bytesPerVoxel;

      switch ( newVolume->type )
	{
	case U8BIT :
	  ptU8BIT = data_U8BIT ( newVolume );
	  ptU8BIT += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptU8BIT, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptU8BIT += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptU8BIT += oLbS;
	      }
	  break;
	case S8BIT :
	  ptS8BIT = data_S8BIT ( newVolume );
	  ptS8BIT += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptS8BIT, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptS8BIT += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptS8BIT += oLbS;
	      }
	  break;
	case U16BIT :
	  ptU16BIT = data_U16BIT ( newVolume );
	  ptU16BIT += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptU16BIT, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptU16BIT += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptU16BIT += oLbS;
	      }
	  break;
	case S16BIT :
	  ptS16BIT = data_S16BIT ( newVolume );
	  ptS16BIT += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptS16BIT, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptS16BIT += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptS16BIT += oLbS;
	      }
	  break;
	case U32BIT :
	  ptU32BIT = data_U32BIT ( newVolume );
	  ptU32BIT += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptU32BIT, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptU32BIT += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptU32BIT += oLbS;
	      }
	  break;
	case S32BIT :
	  ptS32BIT = data_S32BIT ( newVolume );
	  ptS32BIT += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptS32BIT, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptS32BIT += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptS32BIT += oLbS;
	      }
	  break;
	case VFLOAT :
	  ptFLOAT = data_VFLOAT ( newVolume );
	  ptFLOAT += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptFLOAT, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptFLOAT += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptFLOAT += oLbS;
	      }
	  break;
	case VDOUBLE :
	  ptDOUBLE = data_VDOUBLE ( newVolume );
	  ptDOUBLE += offsetFirstPoint ( newVolume );
	  fseek( fileIma, roi.z1*nx*ny, SEEK_SET );
	  if ( EXEC )
	    for ( iz = 0; iz < dzroi; iz++ )
	      {
		fseek( fileIma, roi.y1*nx, SEEK_CUR );
		for ( iy = 0; iy < dyroi; iy++ )
		  {
		    fseek( fileIma, roi.x1, SEEK_CUR );
		    fread ( ptDOUBLE, bytesPerVoxel, dxroi, fileIma );
		    fseek( fileIma, offset1, SEEK_CUR );
		    ptDOUBLE += oL;
		  }
		fseek( fileIma, offset2, SEEK_CUR );
		ptDOUBLE += oLbS;
	      }
	  break;
	default :
	  notImplementedFor ( newVolume->type );
	  goto abort;
	}
    }
  /*bnew: unnecessary: is already done in the 'data_...' functions
    setState ( newVolume, STATE_FILLED );
    */

  fclose ( fileIma );

  free ( filenameIma );
  /*printf("%p (Tfree)\n",filenameIma);*/
	
  TRACE = oTRACE;
  printftrace ( OUT, "" );

  return ( newVolume );
  abort :
    printfexit ( "(volume)readVolume" );
  return ( NULL );
}




/*bnew*/
/*----------------------------------------------------------------------------*/

void			writeVolumeDim		       	/*NEW*/

/*----------------------------------------------------------------------------*/
(
 Volume	    *volume,
 const char    *filename
 )
{
  char		    *filenameDim;
  FILE		    *fileDim;
  int		    oTRACE;
  int test_byteswaping = 0x41424344;

  printftrace ( IN, "writeVolumeDim" );
  oTRACE = TRACE; TRACE = OFF;

  if ( !verifyAll ( volume ) )
      goto abort;

  filenameDim = dimFile( filename );
  
  fileDim = fopen ( filenameDim, "w" );
  if ( fileDim == NULL )
  {
    printfError ( "File write error" );
    (void) fprintf ( stderr,
                    "File '%s' can not be opened for writing.\n", 
                    filenameDim );
    goto abort;
  }

  if ( volume->voxelSize.z == 0. )
  {
    if ( volume->type == U8BIT ) /* old 2D format */
    {
      printfInfo ( "Writing image with old 2D format" );
      (void) fprintf ( fileDim, "%d %d\n",
                      volume->size.x, volume->size.y );
    }
    else
    {
      printfError ( "Unknown volume format" );
      (void) fprintf ( stderr, "Voxelsize.z = 0 and type != U8" );
      goto abort;
    }
  }
  else                          /* tivoli format */
  {
    /*  first line  */
		
    (void) fprintf ( fileDim, "%d %d", volume->size.x, volume->size.y );
    /*if ( volume->size.z != 1 )*/
        (void) fprintf ( fileDim, " %d", volume->size.z );
    if ( volume->size.t != 1 )
        (void) fprintf ( fileDim, " %d", volume->size.t );
    (void) fprintf ( fileDim, "\n" );
		
    /*  type  */
		
    (void) fprintf ( fileDim, "-type " );
    switch ( volume->type )
    {
      case 10 :
      (void) fprintf ( fileDim, "U8" );
      break;
      case 11 :
      (void) fprintf ( fileDim, "S8" );
      break;

      case 20 :
      (void) fprintf ( fileDim, "U16" );
      break;
      case 21 :
      (void) fprintf ( fileDim, "S16" );
      break;

      case 40 :
      (void) fprintf ( fileDim, "U32" );
      break;
      case 41 :
      (void) fprintf ( fileDim, "S32" );
      break;

      case 400 :
      (void) fprintf ( fileDim, "FLOAT" );
      break;
      case 800 :
      (void) fprintf ( fileDim, "DOUBLE" );
      break;
    }
    (void) fprintf ( fileDim, "\n" );
		
    /*  sizes  */
		
    (void) fprintf ( fileDim, "-dx %f  -dy %f",
                    volume->voxelSize.x, volume->voxelSize.y );
    /* if ( volume->size.z != 1 ) */
        (void) fprintf ( fileDim, "  -dz %f", volume->voxelSize.z );
    if ( volume->size.t != 1 )
        (void) fprintf ( fileDim, "  -dt %f", volume->voxelSize.t );
    (void) fprintf ( fileDim, "\n" );
	
    /*byte swaping  */

    fprintf( fileDim, "-bo ");
    fwrite( (char *)(&test_byteswaping), 1,  4,fileDim);
    fprintf( fileDim, "\n");

    /*binar/ASCII (ASCII not supported by VIP but by Aims*/

    fprintf( fileDim, "-om binar\n");
	
    /* reference */

    if ( strcmp ( volume->ref.name, "" ) )
    {
      (void) fprintf ( fileDim, "  -ref %s %d %d", 
                      volume->ref.name, volume->ref.x, volume->ref.y );
      
      if ( volume->size.z != 1 )
          (void) fprintf ( fileDim, " %d", volume->ref.z );
      
      if ( volume->size.t != 1 )
          (void) fprintf ( fileDim, " %d", volume->ref.t );
      
      (void) fprintf ( fileDim, "\n" );
    }
    
  }

  fclose ( fileDim );

  TRACE = oTRACE;
  printftrace ( OUT, "" );
  return;
  
abort :
	printfexit ( "(volume)writeVolumeDim" );
}



/*----------------------------------------------------------------------------*/

	void			writeVolume				/*NEW*/

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	const char    *filename
)
{
  char		    *filenameIma;
  FILE		    *fileIma;
  int			oL, oLbS,
    iy,
    nx,
    ny, nt, it, osbv, /*JEFF 97*/
    iz, nz,
    bytesPerVoxel;
  int bw;
  U8BIT_t	    *ptU8BIT;
  S8BIT_t	    *ptS8BIT;
  U16BIT_t	    *ptU16BIT;
  S16BIT_t	    *ptS16BIT;
  U32BIT_t	    *ptU32BIT;
  S32BIT_t	    *ptS32BIT;
  FLOAT_t	    *ptFLOAT;
  DOUBLE_t	    *ptDOUBLE;
  int			oTRACE;

  printftrace ( IN, "writeVolume" );
  oTRACE = TRACE; TRACE = OFF;

  if ( !verifyAll ( volume ) ||
       !testState ( volume, STATE_FILLED ) )
    goto abort;
        
  writeVolumeDim ( volume, filename );
        
  filenameIma = imaFile( filename );
  fileIma = fopen ( filenameIma, "wb" );
  if ( fileIma == NULL )
    {
      printfError ( "File write error" );
      (void) fprintf ( stderr,
		       "File '%s' can not be opened for writing.\n", 
		       filenameIma );
      goto abort;
    }		

  bytesPerVoxel = sizeofType ( volume );
  oL = offsetLine ( volume );
  oLbS = offsetLineBetweenSlice ( volume );
  bw = volume->borderWidth;
  nx = volume->size.x;
  ny = volume->size.y;
  nz = volume->size.z;
  nt = volume->size.t;
  osbv = 2*bw * (nx+2*bw) * (ny + 2*bw);


  switch ( volume->type )
    {
    case U8BIT :
      ptU8BIT = data_U8BIT ( volume );
      ptU8BIT += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptU8BIT, bytesPerVoxel, nx, fileIma );
		    ptU8BIT += oL;
		  }
		ptU8BIT += oLbS;
	      }
	    ptU8BIT += osbv;
	  }
      break;
    case S8BIT :
      ptS8BIT = data_S8BIT ( volume );
      ptS8BIT += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptS8BIT, bytesPerVoxel, nx, fileIma );
		    ptS8BIT += oL;
		  }
		ptS8BIT += oLbS;
	      }
	    ptS8BIT += osbv;
	  }
      break;
    case U16BIT :
      ptU16BIT = data_U16BIT ( volume );
      ptU16BIT += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptU16BIT, bytesPerVoxel, 
			     nx, fileIma );
		    ptU16BIT += oL;
		  }
		ptU16BIT += oLbS;
	      }
	    ptU16BIT += osbv;
	  }
      break;
    case S16BIT :
      ptS16BIT = data_S16BIT ( volume );
      ptS16BIT += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptS16BIT, bytesPerVoxel, nx, fileIma );
		    ptS16BIT += oL;
		  }
		ptS16BIT += oLbS;
	      }
	    ptS16BIT += osbv;
	  }
      break;
    case U32BIT :
      ptU32BIT = data_U32BIT ( volume );
      ptU32BIT += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptU32BIT, bytesPerVoxel, 
			     nx, fileIma );
		    ptU32BIT += oL;
		  }
		ptU32BIT += oLbS;
	      }
	    ptU32BIT += osbv;
	  }
      break;
    case S32BIT :
      ptS32BIT = data_S32BIT ( volume );
      ptS32BIT += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptS32BIT, bytesPerVoxel, 
			     nx, fileIma );
		    ptS32BIT += oL;
		  }
		ptS32BIT += oLbS;
	      }
	    ptS32BIT += osbv;
	  }
      break;
    case VFLOAT :
      ptFLOAT = data_VFLOAT ( volume );
      ptFLOAT += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptFLOAT, bytesPerVoxel, nx, fileIma );
		    ptFLOAT += oL;
		  }
		ptFLOAT += oLbS;
	      }
	    ptFLOAT += osbv;
	  }
      break;
    case VDOUBLE :
      ptDOUBLE = data_VDOUBLE ( volume );
      ptDOUBLE += offsetFirstPoint ( volume );
      if ( EXEC )
	for(it=0;it<nt;it++)
	  {
	    for ( iz = 0; iz < nz; iz++ )
	      {
		for ( iy = 0; iy < ny; iy++ )
		  {
		    fwrite ( ptDOUBLE, bytesPerVoxel, 
			     nx, fileIma );
		    ptDOUBLE += oL;
		  }
		ptDOUBLE += oLbS;
	      }
	    ptDOUBLE += osbv;
	  }
      break;
    default :
      notImplementedFor ( volume->type );
      goto abort;
    }

  fclose ( fileIma );
  free ( filenameIma );

  TRACE = oTRACE;
  printftrace ( OUT, "" );

  return;
  abort :
    printfexit ( "(volume)writeVolume" );
}




/*----------------------------------------------------------------------------*/

	void			writeVolumeSlices

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			firstSlice, /*  */
	int			nSlices, 
	const char    *filename
)
/*
IMA_function: writeVolumeSlices - writes a series of slices 
IMA_synopsis: writeVolumeSlices ( volume, firstSlice, nSlices, filename );
IMA_parameters: 
firstSlice: The slice number to start from.  Slices go from 0 to n-1 
independent of the presence of a border.

nSlices: The number of consecutive slices to write.

filename: A string with the filename.
*/
{
	Volume	    *newVolume;
	int			oTRACE;

printftrace ( IN, "writeVolumeSlices" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) ||
		!testFilled ( volume ) ||
		!testImageSlice ( volume, firstSlice ) ||
		!testImageSlice ( volume, firstSlice + nSlices - 1 ) )
		goto abort;
		
	/* tricky */
	newVolume = duplicateVolumeStructure
			( volume, "duplicateVolumeStructure" );
	newVolume->data = volume->data + firstSlice * offsetSlice ( volume );
	newVolume->size.z = nSlices;
	newVolume->state = STATE_FILLED;
	writeVolume ( newVolume, filename );
        /* only free the structure... not the data */
	/* free ( newVolume );  */
        /*bnew: keep the structure, but put it on FREED*/
        newVolume->state = STATE_FREED;
        
TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort :
	printfexit ( "(volume)writeVolumeSlices" );
}





/*----------------------------------------------------------------------------*/

	void			writeVolumeSlice

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume, 
	int			slice,   /* slices go from 0 to nz-1 */
	const char    *filename
)
/* writes one single slice */
{
	int			oTRACE;

printftrace ( IN, "writeVolumeSlice" );
oTRACE = TRACE; TRACE = OFF;

	writeVolumeSlices ( volume, slice, 1, filename );
	
TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
}





/*==========================*
 |                          |
 |  volume creators         |
 |                          |
 *==========================*/




/*----------------------------------------------------------------------------*/

	Volume	    *declareVolumeStructure
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	int			type,
	const char    *name,
	int			borderWidth
)
{
	static long	id = 0;
	Volume	    *newVolume;
	int			oTRACE;
	
printftrace ( IN, "declareVolumeStructure" );
oTRACE = TRACE; TRACE = OFF;

	if ( name == NULL )
        {
          printfError ( "Volume name is a NULL pointer" );
          goto abort;
	}
	if ( strlen ( name ) == 0 ) {
          printfError ( "Volume name is empty" );
          goto abort;
	}
	if ( strlen ( name ) > NAME_MAXLEN ) 
        {
          /*bnew: error --> warning, and truncate it */
          printfWarning ( "Volume name is too long" );
          (void) fprintf ( stderr, 
                        "Volume names must be no longer than %d characters.\n",
                         NAME_MAXLEN );
          (void) fprintf ( stderr, "I'll truncate it !\n" );
        }

	newVolume = (Volume *) VipAllocVolumeStructure();
	if(newVolume == PB) return(PB);

	/* necessary to verify consistancy with the future state : */
	newVolume->data = NULL;
	newVolume->tab = NULL;

#ifdef MEMORYMAP
	newVolume->swapfile = NULL;
#endif

	newVolume->size3Db = 0;

	/* necessary to set caracteristics : */
	newVolume->state = STATE_DECLARED;
	setType ( newVolume, type );
	setSize4 ( newVolume, xSize, ySize, zSize, 1 );
	setVoxelSize4 ( newVolume, xVoxelSize, yVoxelSize, zVoxelSize, 1.0 );
        /*bnew: strcpy --> strncpy */
	strncpy ( newVolume->name, name, NAME_MAXLEN );
	setBorderWidth ( newVolume, borderWidth );

	/* reference */
	(void) strcpy ( newVolume->ref.name, "" );
	newVolume->ref.x = 0;
	newVolume->ref.y = 0;
	newVolume->ref.z = 0;
	newVolume->ref.t = 0;

	/* if all is right : */
	newVolume->id = ++id;

TRACE = oTRACE;
printftrace ( OUT, "" );

	return ( newVolume );
abort :
	printfexit ( "(volume)declareVolumeStructure" );
	return ( NULL );
}
/*----------------------------------------------------------------------------*/

	Volume	    *declareVolumeStructure4
	
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
	const char    *name,
	int			borderWidth
)
{
	static long	id = 0;
	Volume	    *newVolume;
	int			oTRACE;
	
printftrace ( IN, "declareVolumeStructure4" );
oTRACE = TRACE; TRACE = OFF;

	if ( name == NULL )
        {
          printfError ( "Volume name is a NULL pointer" );
          goto abort;
	}
	if ( strlen ( name ) == 0 ) {
          printfError ( "Volume name is empty" );
          goto abort;
	}
	if ( strlen ( name ) > NAME_MAXLEN ) 
        {
          /*bnew: error --> warning, and truncate it */
          printfWarning ( "Volume name is too long" );
          (void) fprintf ( stderr, 
                        "Volume names must be no longer than %d characters.\n",
                         NAME_MAXLEN );
          (void) fprintf ( stderr, "I'll truncate it !\n" );
        }

	newVolume = (Volume *) VipAllocVolumeStructure();
	if(newVolume == PB) return(PB);

	/* necessary to verify consistancy with the future state : */
	newVolume->data = NULL;
	newVolume->tab = NULL;

#ifdef MEMORYMAP
	newVolume->swapfile = NULL;
#endif

	newVolume->size3Db = 0;

	/* necessary to set caracteristics : */
	newVolume->state = STATE_DECLARED;
	setType ( newVolume, type );
	setSize4 ( newVolume, xSize, ySize, zSize, tSize );
	setVoxelSize4 ( newVolume, xVoxelSize, yVoxelSize,
			zVoxelSize, tVoxelSize );
        /*bnew: strcpy --> strncpy */
	strncpy ( newVolume->name, name, NAME_MAXLEN );
	setBorderWidth ( newVolume, borderWidth );

	/* reference */
	(void) strcpy ( newVolume->ref.name, "" );
	newVolume->ref.x = 0;
	newVolume->ref.y = 0;
	newVolume->ref.z = 0;
	newVolume->ref.t = 0;

	/* if all is right : */
	newVolume->id = ++id;

TRACE = oTRACE;
printftrace ( OUT, "" );

	return ( newVolume );
abort :
	printfexit ( "(volume)declareVolumeStructure4" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	Volume	    *duplicateVolumeStructure

/*----------------------------------------------------------------------------*/	(
	Volume	    *volume,
	const char    *name
)
{
	Volume	    *newVolume;
	int			oTRACE;

printftrace ( IN, "duplicateVolumeStructure" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) )
		goto abort;

	newVolume = declareVolumeStructure4 (
		volume->size.x, volume->size.y, volume->size.z, volume->size.t,
		volume->voxelSize.x, volume->voxelSize.y, volume->voxelSize.z, 
                volume->voxelSize.t, volume->type, name, volume->borderWidth );

	(void) strcpy ( newVolume->ref.name, volume->ref.name );
	newVolume->ref.x = volume->ref.x;
	newVolume->ref.y = volume->ref.y;
	newVolume->ref.z = volume->ref.z;
	newVolume->ref.t = volume->ref.t;
        
TRACE = oTRACE;
printftrace ( OUT, "" );

	return ( newVolume );
abort :
	printfexit ( "(volume)duplicateVolumeStructure" );
	return ( NULL );
}





/*----------------------------------------------------------------------------*/

	void			allocateVolumeData
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	/*int			oL,
				iy,
				iz,*/
	int		nxb, nyb, nzb, nt;
	/*int			nyb1, nzb1;*/
   int              bW;
	/*char		    *ptrData;
        int        bytesPerVoxel;*/

#ifdef MEMORYMAP
	swapfileNode		*p;
#endif

	printftrace ( IN, "allocateVolumeData" );

	if ( !verifyAll ( volume ) ||
		!testState ( volume, STATE_DECLARED ) )
		goto abort;

        /*bnew: the following cannot occur, since tested in 'verifyAll' */
/*
	if ( volume->size.x > IMAGE_MAXSIZE &&
		volume->size.y > IMAGE_MAXSIZE &&
		volume->size.z > IMAGE_MAXSIZE &&
		volume->size.t > IMAGE_MAXSIZE )
	{
          
		(void) fprintf ( stderr,
		"warning: too large data can create problems ..." );
	}
*/

        bW = borderWidthOf(volume);
        nxb = volume->size.x + 2 * bW;
	nyb = volume->size.y + 2 * bW;
	nzb = volume->size.z + 2 * bW;
	nt = volume->size.t; /*JEFF 97*/
	volume->size3Db = nzb * nyb * nxb * nt * sizeofType ( volume );

#ifdef MEMORYMAP
	assert(volume->swapfile == NULL);
        /*bnew: only for large volumes*/
	if ( volume->size3Db > MEMORYMAP_THRESHOLD ) {
                /*
                printfInfo ( "Memory mapping" );
                (void) fprintf ( stdout, "Creating swap file for volume '%s'.\n",
                                volume->name );
                */

		volume->swapfile = tempnam(NULL, "tivol");
		if (volume->swapfile == NULL) {
			perror(NULL);
			exit(EXIT_FAILURE);
		}
		file = open(volume->swapfile, O_RDWR | O_CREAT | O_TRUNC, 666);
		if (file == -1) {
			perror(volume->swapfile);
			exit(EXIT_FAILURE);
		}
		if (fchmod(file, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH))
		{
			perror(volume->swapfile);
			exit(EXIT_FAILURE);
		}
                
                /* add it at the start of the list */
		p = mallocT(sizeof(swapfileNode));
		p->swapfile = volume->swapfile;
		p->next = swapfileList;
		swapfileList = p;

		for (p = swapfileList; p != NULL; p = p->next);
		p = mallocT(sizeof(swapfileNode));
		p->next = NULL;
		p->swapfile = volume->swapfile;
		if (!swapfileFlag) {
			if (
				signal(SIGHUP, swapfileAtSignal) == SIG_ERR ||
				signal(SIGINT, swapfileAtSignal) == SIG_ERR ||
				signal(SIGQUIT, swapfileAtSignal) == SIG_ERR ||
				signal(SIGILL, swapfileAtSignal) == SIG_ERR ||
				signal(SIGTRAP, swapfileAtSignal) == SIG_ERR ||
				signal(SIGABRT, swapfileAtSignal) == SIG_ERR ||
				signal(SIGEMT, swapfileAtSignal) == SIG_ERR ||
				signal(SIGFPE, swapfileAtSignal) == SIG_ERR ||
				signal(SIGBUS, swapfileAtSignal) == SIG_ERR ||
				signal(SIGSEGV, swapfileAtSignal) == SIG_ERR ||
				signal(SIGSYS, swapfileAtSignal) == SIG_ERR ||
				signal(SIGPIPE, swapfileAtSignal) == SIG_ERR ||
				signal(SIGALRM, swapfileAtSignal) == SIG_ERR ||
				signal(SIGTERM, swapfileAtSignal) == SIG_ERR ||
				signal(SIGUSR1, swapfileAtSignal) == SIG_ERR ||
				signal(SIGUSR2, swapfileAtSignal) == SIG_ERR ||
				signal(SIGPOLL, swapfileAtSignal) == SIG_ERR ||
				signal(SIGPROF, swapfileAtSignal) == SIG_ERR ||
				signal(SIGXCPU, swapfileAtSignal) == SIG_ERR ||
				signal(SIGXFSZ, swapfileAtSignal) == SIG_ERR)
			{
				perror(NULL);
				exit(EXIT_FAILURE);
			}
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
#endif
            volume->data = (char *) mallocT(volume->size3Db);

        /*bnew: adapted to make first point at [0][0][0]
        bytesPerVoxel = sizeofType ( volume );
        
	nyb1 = volume->size.y + bW;
	nzb1 = volume->size.z + bW;

	oL = offsetLine ( volume ) * bytesPerVoxel; */

	volume->tab = NULL;

	/*volume->tab = (char ***) mallocT ( nzb * sizeof(char **) );
        volume->tab = volume->tab + bW;

        ptrData = volume->data + bW * bytesPerVoxel;

	for ( iz = -bW; iz < nzb1; iz++ )
	{
		volume->tab[iz] = (char **) mallocT ( nyb * sizeof(char *) );
                if ( volume->tab[iz] == NULL )
		{
			free ( volume->data );
			for ( iz2 = -bW; iz2 < iz; iz2++ )
				free ( volume->tab[iz2] );
		}
                volume->tab[iz] = volume->tab[iz] + bW;
		for ( iy = -bW; iy < nyb1; iy++ )
		{
                  volume->tab[iz][iy] = ptrData;
                  ptrData += oL;
		}
	} */

	setState ( volume, STATE_ALLOCATED );

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)allocateVolumeData" );
}


#ifdef MEMORYMAP
/*----------------------------------------------------------------------------*/
static 
void swapfileAtSignal
/*----------------------------------------------------------------------------*/
(
	int a
)
{
        printfError ( "Memory mapping" );
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
	case SIGEMT:
		fputs("SIGEMT", stderr);
		break;
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
	case SIGSYS:
		fputs("SIGSYS", stderr);
		break;
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
	case SIGPOLL:
		fputs("SIGPOLL", stderr);
		break;
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


/*----------------------------------------------------------------------------*/

	static void swapfileAtExit
	
/*----------------------------------------------------------------------------*/
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





/*----------------------------------------------------------------------------*/

	Volume	    *createVolume
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	int			type,
	const char    *name,
	int			borderWidth
)
{
	Volume	    *newVolume;
	int			oTRACE;
	
printftrace ( IN, "createVolume" );
oTRACE = TRACE; TRACE = OFF;

	newVolume = declareVolumeStructure4 (
			xSize, ySize, zSize, 1,
			xVoxelSize, yVoxelSize, zVoxelSize, 1.0,
			type, name, borderWidth );

	allocateVolumeData ( newVolume );
	
TRACE = oTRACE;
printftrace ( OUT, "" );

	return ( newVolume );
}
/*----------------------------------------------------------------------------*/

	Volume	    *createVolume4
	
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
	const char    *name,
	int			borderWidth
)
{
	Volume	    *newVolume;
	int			oTRACE;
	
printftrace ( IN, "createVolume4" );
oTRACE = TRACE; TRACE = OFF;

	newVolume = declareVolumeStructure4 (
			xSize, ySize, zSize, tSize,
			xVoxelSize, yVoxelSize, zVoxelSize, tVoxelSize,
			type, name, borderWidth );

	allocateVolumeData ( newVolume );
	
TRACE = oTRACE;
printftrace ( OUT, "" );

	return ( newVolume );
}





/*----------------------------------------------------------------------------*/

	void			transferVolumeData 
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volumeR,
	Volume	    *volumeW
)
/*
IMA_function: transferVolumeData
IMA_synopsis: transferVolumeData ( fromVolume, toVolume );
IMA_description:  Copies the contents of 'fromVolume' to 'toVolume' (when
enough place is present).  It does not copy the borders.
*/
{
	int			nx,
				iy, ny,
				iz, nz,
				bytesPerVoxel,
				oLR, oLW,
				oCR, oCW;
	char		    *ptrDataR,
			    *ptrDataW;
	int			oTRACE;
	
printftrace ( IN, "transferVolumeData" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volumeR ) ||
		!verifyAll ( volumeW ) ||
		!testFilled ( volumeR ) ||
		!testEqualType ( volumeR, volumeW ) )
		goto abort;
		
	/* function 'testState' unused to prevent an unexpectable error 
           message */
	if ( volumeW->state == STATE_FILLED )
        {
          printfWarning ( 
                         "Destination volume is filled: its data may be lost" );
          (void) fprintf ( stderr, "\tduring 'transferVolumeData'." );
        }
        
	if ( volumeW->state == STATE_DECLARED )
		allocateVolumeData ( volumeW );

	if ( volumeW->size.x > volumeR->size.x ||
			volumeW->size.x > volumeR->size.x ||
			volumeW->size.z > volumeR->size.z )
        {
          printfWarning ( "Destination volume is over-dimensioned" );
          (void) fprintf ( stderr, 
                    "\tThe destination volume will not be completely filled." );
          (void) fprintf ( stderr, "\n\tIn 'transferVolumeData'." );
        }
        
        if ( volumeW->size.x < volumeR->size.x ||
			volumeW->size.x < volumeR->size.x ||
			volumeW->size.z < volumeR->size.z )
        {
          printfWarning ( "Source volume is over-dimensioned." );
          (void) fprintf ( stderr, 
                         "\tThe source volume will not be completely copied." );
          (void) fprintf ( stderr, "\n\tIn 'transferVolumeData'." );
        }
        

	bytesPerVoxel = sizeofType ( volumeR );

	nx = vinilum ( volumeR->size.x, volumeW->size.x );
	ny = vinilum ( volumeR->size.y, volumeW->size.y );
	nz = vinilum ( volumeR->size.z, volumeW->size.z );

	oCR = ( volumeR->size.x - nx + offsetPointBetweenLine ( volumeR ) )
			* bytesPerVoxel;
	oCW = ( volumeW->size.x - nx + offsetPointBetweenLine ( volumeW ) )
			* bytesPerVoxel;
	oLR = ( ( volumeR->size.y - ny ) * offsetLine ( volumeR ) +
			offsetLineBetweenSlice ( volumeR ) ) * bytesPerVoxel;
	oLW = ( ( volumeW->size.y - ny ) * offsetLine ( volumeW ) +
			offsetLineBetweenSlice ( volumeW ) ) * bytesPerVoxel;

	ptrDataR = volumeR->data + offsetFirstPoint ( volumeR ) * bytesPerVoxel;
	ptrDataW = volumeW->data + offsetFirstPoint ( volumeW ) * bytesPerVoxel;

	nx *= bytesPerVoxel;
        oCR += nx;
        oCW += nx;

if (EXEC )
        for ( iz = 0; iz < nz; iz++ )
	{
		for ( iy = 0; iy < ny; iy++ )
		{
			memcpy ( (void*)ptrDataW, (void*)ptrDataR, nx );
/*
			for ( ix = 0; ix < nx; ix++ )
				*(ptrDataW++) = *(ptrDataR++);
*/
			ptrDataR += oCR;
			ptrDataW += oCW;
		}
		ptrDataR += oLR;
		ptrDataW += oLW;
	}

	setState ( volumeW, STATE_FILLED );
	
TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort  :
	printfexit ( "(volume)transferVolumeData" );
}





/*----------------------------------------------------------------------------*/

	Volume	    *copyVolume
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	const char    *name
)
{
	Volume	    *newVolume;
	int			oTRACE;

printftrace ( IN, "copyVolume" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) )
		goto abort;

	newVolume = duplicateVolumeStructure ( volume, name );

	transferVolumeData ( volume, newVolume );

TRACE = oTRACE;
printftrace ( OUT, "" );

	return ( newVolume );
abort :
	printfexit ( "(volume)copyVolume" );
	return ( NULL );
}






/*----------------------------------------------------------------------------*/
	
	Volume	    *vol3DtoVol2D

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			first,   /* from 0 to n-1 */
	int			step,
	int			number,
	int			width
)
{
	long int		oPbL, oS, obS, oPbL_2D;
	int			ix, iy, iz, nx, ny;
        Volume	    *volume2D = NULL;
	U8BIT_t	    *ptVolume_U8, *ptVolume2D_U8, *pt1Volume2D_U8;
	U16BIT_t	    *ptVolume_U16, *ptVolume2D_U16, *pt1Volume2D_U16;
	S16BIT_t	    *ptVolume_S16, *ptVolume2D_S16, *pt1Volume2D_S16;
        DOUBLE_t    *ptVolume_D, *ptVolume2D_D, *pt1Volume2D_D;
	int			height;
	char    *name;
        
	printftrace ( IN, "vol3DtoVol2D" );

	if ( !testFilled ( volume ) ||
		!testSlice ( volume, first ) )
            goto abort;

	if ( step <= 0 || number <= 0 || width <= 0 )
	{
          printfError ( "Arguments # 3 to 5 have to be strictly positive" );
          (void) fprintf ( stderr, "You tried: step between slices %d", step );
          (void) fprintf ( stderr, "\n\tnumber of slices %d", number );
          (void) fprintf ( stderr, "\n\tnumber of slices on a row %d\n", width );
          goto abort;
	}
	
	if ( !testSlice ( volume, first + ( number - 1 ) * step ) )
	{
          printfError ( "Improper argument(s) among #2, #3 and #4" );
          (void) fprintf ( stderr, "You tried: first slice %d", first );
          (void) fprintf ( stderr, "\n\tstep between slices %d", step );
          (void) fprintf ( stderr, "\n\tnumber of slices %d\n", number );
          goto abort;
	}

	height = 1 + ( number - 1 ) / width;

        /*bnew: a better name*/
        name = composeName ( volume->name, "2D", "" );
        volume2D = duplicateVolumeStructure ( volume, name );
        free ( name );

	getSize2 ( volume, &nx, &ny );
        setSize4 ( volume2D, nx * width, ny * height, 1, 1 );
        setBorderWidth ( volume2D, 0 );

	oPbL = offsetPointBetweenLine ( volume );
	oS = offsetSlice ( volume );
	obS = offsetLineBetweenSlice ( volume ) + ( step - 1 ) * oS;
        oPbL_2D = nx * ( width - 1 );

        setVolumeLevel ( volume2D, 0 );

	switch ( typeOf ( volume ) )
	{
          /*bnew: (first-1) ---> first*/
	case U8BIT :
                ptVolume_U8 = data_U8BIT ( volume ) + 
                    offsetFirstPoint ( volume ) + first * oS;
		pt1Volume2D_U8 = data_U8BIT ( volume2D );
if ( EXEC )
                for ( iz = 0; iz < number; iz++ )
		{
			ptVolume2D_U8 = pt1Volume2D_U8 +
				nx * width * ny * ( iz / width ) + nx * ( iz % width );
			for ( iy = 0; iy < ny; iy++ )
			{
				for ( ix = 0; ix < nx; ix++ )
					*ptVolume2D_U8++ = *ptVolume_U8++;
				ptVolume_U8 += oPbL;
				ptVolume2D_U8 += oPbL_2D;
			}
			ptVolume_U8 += obS ;
		}
		break;
	case U16BIT :
		ptVolume_U16 = data_U16BIT ( volume ) + 
                    offsetFirstPoint ( volume ) + first * oS;
		pt1Volume2D_U16 = data_U16BIT ( volume2D );
if ( EXEC )
		for ( iz = 0; iz < number; iz++ )
		{
			ptVolume2D_U16 = pt1Volume2D_U16 +
				nx * width * ny * ( iz / width ) + nx * ( iz % width );
			for ( iy = 0; iy < ny; iy++ )
			{
				for ( ix = 0; ix < nx; ix++ )
					*ptVolume2D_U16++ = *ptVolume_U16++;
				ptVolume_U16 += oPbL;
				ptVolume2D_U16 += oPbL_2D;
			}
			ptVolume_U16 += obS ;
		}
		break;
	case S16BIT :
		ptVolume_S16 = data_S16BIT ( volume ) + 
                    offsetFirstPoint ( volume ) + first * oS;
		pt1Volume2D_S16 = data_S16BIT ( volume2D );
if ( EXEC )
		for ( iz = 0; iz < number; iz++ )
		{
			ptVolume2D_S16 = pt1Volume2D_S16 +
				nx * width * ny * ( iz / width ) + nx * ( iz % width );
			for ( iy = 0; iy < ny; iy++ )
			{
				for ( ix = 0; ix < nx; ix++ )
					*ptVolume2D_S16++ = *ptVolume_S16++;
				ptVolume_S16 += oPbL;
				ptVolume2D_S16 += oPbL_2D;
			}
			ptVolume_S16 += obS ;
		}
		break;
	case VDOUBLE :
		ptVolume_D = data_VDOUBLE ( volume ) + 
                    offsetFirstPoint ( volume ) + first * oS;
		pt1Volume2D_D = data_VDOUBLE ( volume2D );
if ( EXEC )
		for ( iz = 0; iz < number; iz++ )
		{
			ptVolume2D_D = pt1Volume2D_D +
				nx * width * ny * ( iz / width ) + nx * ( iz % width );
			for ( iy = 0; iy < ny; iy++ )
			{
				for ( ix = 0; ix < nx; ix++ )
					*ptVolume2D_D++ = *ptVolume_D++;
				ptVolume_D += oPbL;
				ptVolume2D_D += oPbL_2D;
			}
			ptVolume_D += obS ;
		}
	break;
                default:
                notImplementedFor ( volume->type );
                goto abort;
        }

	printftrace ( OUT, "" );
	return ( volume2D );
abort :
	printfexit ( "(volume)vol3DtoVol2D" );
	return ( NULL );
}








/*----------------------------------------------------------------------------*/

	void			freeVolumeData

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
  /*int			nzb1, bW;
  int			i;*/
  int			oTRACE;
#ifdef MEMORYMAP
  swapfileNode		*p;
  swapfileNode		*pold;
#endif

printftrace ( IN, "freeVolumeData" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) ||
		!testState ( volume, STATE_FILLED ) )
            goto abort;

     /*   bW = volume->borderWidth;
        nzb1 = volume->size.z + bW;
	for ( i = -bW; i < nzb1; i++ )
		free ( volume->tab[i] - bW );
	free ( volume->tab - bW );*/
	volume->tab = NULL;

#ifdef MEMORYMAP
	if (volume->swapfile)
	{
		if (munmap((caddr_t) volume->data, volume->size3Db))
		{
			perror(NULL);
			exit(EXIT_FAILURE);
		}

		assert(swapfileList);
/*bnew*/               
		if (swapfileList->swapfile == volume->swapfile)
		{
			p = swapfileList;
			swapfileList = swapfileList->next;
			free(p);
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
                      free(p);
                      break;
                    }
                  }
                }
                

/*old from Dimitri:  this crashed
		if (swapfileList->swapfile == volume->swapfile)
		{
			p = swapfileList;
			swapfileList = swapfileList->next;
			free(p);
		}
		for (pold = swapfileList, p = swapfileList->next;
			p != NULL;
			pold = p, p = p->next)
		{
			if (p->swapfile == volume->swapfile)
			{
				pold->next = p->next;
				free(p);
				break;
			}
		}
*/
		if (unlink(volume->swapfile))
		{
			perror(volume->swapfile);
			exit(EXIT_FAILURE);
		}
		free(volume->swapfile);
		volume->swapfile = NULL;
	}
	else
#endif    
            free(volume->data);

	volume->data = NULL;
	volume->size3Db = 0;

	setState ( volume, STATE_DECLARED );

TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort :
	printfexit ( "(volume)freeVolumeData" );
}





/*----------------------------------------------------------------------------*/

	void			freeVolume

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	int			oTRACE;

printftrace ( IN, "freeVolume" );
oTRACE = TRACE; TRACE = OFF;

/*bnew: I refined the following
	if ( !verifyAll ( volume ) )
		goto abort;
*/
        if ( volume==NULL )
            goto abort;
        if ( volume->state == STATE_FREED )
        {        
          printfWarning ( "You try to free a volume that was already freed" );
          (void) fprintf ( stderr, "\tIt was called '%s' before",
                          volume->name );
          (void) fprintf ( stderr, "\n\tI do not change anything.\n" );
        }
        else
        {
          switch ( volume->state )
          {
            case STATE_DECLARED:
            
            printfWarning ( "You try to free a volume that was not allocated" );
            (void) fprintf ( stderr, "\tVolume '%s' was only declared.",
                            volume->name );
            (void) fprintf ( stderr, "\n\tBut OK, I'll free it for you.\n" );
            break;
            
            case STATE_ALLOCATED:
            
            printfWarning ( "You try to free a volume that was not filled" );
            (void) fprintf ( stderr, "\tVolume '%s' was only allocated.",
                            volume->name );
            (void) fprintf ( stderr, "\n\tBut OK, I'll free it for you.\n" );
            volume->state = STATE_FILLED;
            freeVolumeData ( volume );
            break;
            
            case STATE_FILLED:

            freeVolumeData ( volume );
            break;

            default:

            verifyState ( volume->state );
            goto abort;
          }
          
          
          /*bnew: added .t */
          volume->size.x = volume->size.y = volume->size.z = volume->size.t = 0;
          volume->type = 0;
          /*bnew: added .t */
          volume->voxelSize.x = volume->voxelSize.y = volume->voxelSize.z = 
              volume->voxelSize.t = 0.;
          /*bnew: added ref */
          strcpy ( volume->ref.name, "" );
          volume->ref.x = 0;
          volume->ref.y = 0;
          volume->ref.z = 0;
          volume->ref.t = 0;
          
          setState ( volume, STATE_FREED );
        }
        
TRACE = oTRACE;
printftrace ( OUT, "" );

	return;
abort :
	printfexit ( "(volume)freeVolume" );
}









/*----------------------------------------------------------------------------*/
	static
	void			addBorder_U8
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
/* for internal use of 'addBorder' */
{
	U8BIT_t	    *ptVolumeR, *ptVolumeW, *ptVolume1, *ptVolume0;
	long int		i, n;
	int			ix, nx, nxx,
				iy, ny,
				iz, nz;
				
	ptVolume0 = data_U8BIT ( volume );
	ptVolume1 = ptVolume0 + offsetFirstPoint ( volume );
	getSize3 ( volume, &nx, &ny, &nz );
	nxx = nx + 2;
	
        /* copy columns at the left and at the right */
	ptVolumeR = ptVolume1;
	ptVolumeW = ptVolumeR - 1;
	for ( iz = 0; iz < nz; iz++ )
	{
		for ( iy = 0; iy < ny; iy++ )
		{
			*ptVolumeW = *ptVolumeR;
			ptVolumeR += nx - 1;
			ptVolumeW += nx + 1;
			*ptVolumeW = *ptVolumeR;
			ptVolumeR += 3;
			ptVolumeW += 1;
		}
		ptVolumeR += 2 * (unsigned)nxx;
		ptVolumeW += 2 * (unsigned)nxx;
	}
	
        /* copy rows above and below */
	n = offsetSlice ( volume );
	ptVolumeR = ptVolume1 - 1;
	ptVolumeW = ptVolumeR - nxx;
	for ( iz = 0; iz < nz; iz++ )
	{
		for ( ix = 0; ix < nxx; ix++ )
			*ptVolumeW++ = *ptVolumeR++;
		ptVolumeW += n - 2 * nxx;
		ptVolumeR = ptVolumeW - nxx;
		for ( ix = 0; ix < nxx; ix++ )
			*ptVolumeW++ = *ptVolumeR++;
		ptVolumeR += 2 * (unsigned)nxx;
	}
	
        /* copy slices at the bottom and at the top */
	ptVolumeW = ptVolume0;
	ptVolumeR = ptVolumeW + n;
	for ( i = 0; i < n; i++ )
		*ptVolumeW++ = *ptVolumeR++;
	ptVolumeW = ptVolume0 + offsetVolume ( volume ) - 1;
	ptVolumeR = ptVolumeW - n;
	for ( i = 0; i < n; i++ )
		*ptVolumeW-- = *ptVolumeR--;
}




/*----------------------------------------------------------------------------*/
	static
	void			addBorder_U16
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
/* for internal use of 'addBorder' */
{
	U16BIT_t	    *ptVolumeR, *ptVolumeW, *ptVolume1, *ptVolume0;
	long int		i, n;
	int			ix, nx, nxx,
				iy, ny,
				iz, nz;
				
	ptVolume0 = data_U16BIT ( volume );
	ptVolume1 = ptVolume0 + offsetFirstPoint ( volume );
	getSize3 ( volume, &nx, &ny, &nz );
	nxx = nx + 2;
	
	ptVolumeR = ptVolume1;
	ptVolumeW = ptVolumeR - 1;
	for ( iz = 0; iz < nz; iz++ )
	{
		for ( iy = 0; iy < ny; iy++ )
		{
			*ptVolumeW = *ptVolumeR;
			ptVolumeR += nx - 1;
			ptVolumeW += nx + 1;
			*ptVolumeW = *ptVolumeR;
			ptVolumeR += 3;
			ptVolumeW += 1;
		}
		ptVolumeR += 2 * (unsigned)nxx;
		ptVolumeW += 2 * (unsigned)nxx;
	}
	
	n = offsetSlice ( volume );
	ptVolumeR = ptVolume1 - 1;
	ptVolumeW = ptVolumeR - nxx;
	for ( iz = 0; iz < nz; iz++ )
	{
		for ( ix = 0; ix < nxx; ix++ )
			*ptVolumeW++ = *ptVolumeR++;
		ptVolumeW += n - 2 * nxx;
		ptVolumeR = ptVolumeW - nxx;
		for ( ix = 0; ix < nxx; ix++ )
			*ptVolumeW++ = *ptVolumeR++;
		ptVolumeR += 2 * (unsigned)nxx;
	}
	
	ptVolumeW = ptVolume0;
	ptVolumeR = ptVolumeW + n;
	for ( i = 0; i < n; i++ )
		*ptVolumeW++ = *ptVolumeR++;
	ptVolumeW = ptVolume0 + offsetVolume ( volume ) - 1;
	ptVolumeR = ptVolumeW - n;
	for ( i = 0; i < n; i++ )
		*ptVolumeW-- = *ptVolumeR--;
}




/*----------------------------------------------------------------------------*/
	static
	void			addBorder_S32
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
/* for internal use of 'addBorder' */
{
	S32BIT_t	    *ptVolumeR, *ptVolumeW, *ptVolume1, *ptVolume0;
	long int		i, n;
	int			ix, nx, nxx,
				iy, ny,
				iz, nz;
				
	ptVolume0 = data_S32BIT ( volume );
	ptVolume1 = ptVolume0 + offsetFirstPoint ( volume );
	getSize3 ( volume, &nx, &ny, &nz );
	nxx = nx + 2;
	
	ptVolumeR = ptVolume1;
	ptVolumeW = ptVolumeR - 1;
	for ( iz = 0; iz < nz; iz++ )
	{
		for ( iy = 0; iy < ny; iy++ )
		{
			*ptVolumeW = *ptVolumeR;
			ptVolumeR += nx - 1;
			ptVolumeW += nx + 1;
			*ptVolumeW = *ptVolumeR;
			ptVolumeR += 3;
			ptVolumeW += 1;
		}
		ptVolumeR += 2 * (unsigned)nxx;
		ptVolumeW += 2 * (unsigned)nxx;
	}
	
	n = offsetSlice ( volume );
	ptVolumeR = ptVolume1 - 1;
	ptVolumeW = ptVolumeR - nxx;
	for ( iz = 0; iz < nz; iz++ )
	{
		for ( ix = 0; ix < nxx; ix++ )
			*ptVolumeW++ = *ptVolumeR++;
		ptVolumeW += n - 2 * nxx;
		ptVolumeR = ptVolumeW - nxx;
		for ( ix = 0; ix < nxx; ix++ )
			*ptVolumeW++ = *ptVolumeR++;
		ptVolumeR += 2 * (unsigned)nxx;
	}
	
	ptVolumeW = ptVolume0;
	ptVolumeR = ptVolumeW + n;
	for ( i = 0; i < n; i++ )
		*ptVolumeW++ = *ptVolumeR++;
	ptVolumeW = ptVolume0 + offsetVolume ( volume ) - 1;
	ptVolumeR = ptVolumeW - n;
	for ( i = 0; i < n; i++ )
		*ptVolumeW-- = *ptVolumeR--;
}




/*----------------------------------------------------------------------------*/
	
	void			addBorder
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
/*
IMA_function: addBorder
IMA_synopsis: addBorder ( volume );
IMA_description: Adds a border of width 1 to the volume, and fills it with 
mirrored image values.
*/
{
	printftrace ( IN, "addBorder" );

	if ( !testFilled ( volume ) )
		goto abort;
	
	setBorderWidth ( volume, 1 );
	
if ( EXEC )
{
	switch ( volume->type )
	{
		case U8BIT :
			addBorder_U8 ( volume );
			break;
		case U16BIT :
			addBorder_U16 ( volume );
			break;
		case S32BIT :
			addBorder_S32 ( volume );
			break;
                default:
                        notImplementedFor ( volume->type );
                        goto abort;
        }
}

	printftrace ( OUT, "" );
	return;
abort :
	printfexit ( "(volume)addBorder" );
}





/*----------------------------------------------------------------------------*/

	Volume	    *extractSlice
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			slice   /* between 0 and nz-1 */
)
/* 
IMA_function: extractSlice
IMA_synopsis: oneSliceVolume = extractSlice ( volume, sliceNr );
IMA_parameters:
sliceNr: The number of the slice to extract: between -borderWidth and 
nz-1+borderWidth, i.e. between 0 and nz-1 when no border is present.
*/
{
	Volume	    *newVolume;
	int			oTRACE,
				ix, iy, nx, ny, nz;
	long int		oPbL, oP1;
	U8BIT_t	    *ptVolumeU8,
			    *ptNewVolumeU8;
	U16BIT_t	    *ptVolumeU16,
			    *ptNewVolumeU16;
        char name[NAME_MAXLEN+10];
                  
printftrace ( IN, "extractSlice" );
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) ||
		!testFilled ( volume ) ||
		!testSlice ( volume, slice ) )
		goto abort;
	
        /*bnew: a better name*/
        sprintf ( name, "%sSlice%5d", volume->name, slice );
        newVolume = duplicateVolumeStructure ( volume, name );
        
	newVolume->size.z = 1;
	newVolume->borderWidth = 0;
	
	getSize3 ( volume, &nx, &ny, &nz );
	oPbL = offsetPointBetweenLine ( volume );
	oP1 = offsetFirstPoint ( volume )
			+ slice * offsetSlice ( volume );
	
	switch ( volume->type )
	{
	  case U8BIT :
                  ptVolumeU8 = data_U8BIT ( volume ) + oP1;
                  ptNewVolumeU8 = data_U8BIT ( newVolume );
if (EXEC)
                  for ( iy = 0; iy < ny; iy++ )
                  {	
                    for ( ix = 0; ix < nx; ix++ )
                        *ptNewVolumeU8++ = *ptVolumeU8++;
                    ptVolumeU8 += oPbL;
                  }
            break;
	  case U16BIT :
                  ptVolumeU16 = data_U16BIT ( volume ) + oP1;
                  ptNewVolumeU16 = data_U16BIT ( newVolume );
if (EXEC)
                  for ( iy = 0; iy < ny; iy++ )
                  {	
                    for ( ix = 0; ix < nx; ix++ )
                        *ptNewVolumeU16++ = *ptVolumeU16++;
                    ptVolumeU16 += oPbL;
                  }
            break;
          default:
                  notImplementedFor ( volume->type );
                  goto abort;
          }

TRACE = oTRACE;
printftrace ( OUT, "" );

	return ( newVolume );
abort :
	printfexit ( "(volume)extractSlice" );
	return ( NULL );
}


/*----------------------------------------------------------------------------*/

	Volume	    *extractRoi
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
        Roi      *roi
)
/* 
IMA_function: extractRoi 
IMA_description: It copies a subvolume out of a
volume into a new volume.  It treats borders in an appropriate way.  The
subvolume has to be described by a 'roi' structure (roi.x1, roi.x2, roi.y1,
roi.y2, roi.z1, roi.z2 are integers, counted in number of voxels !).

Remark: If the demanded subvolume falls out of the fromVolume, than it is 
     truncated at the fromVolume borders.

Implementation state: only for U8 and U16

IMA_parameters: 
     Volume *fromVolume,
     Roi    *roi,             Roi is the name of the 'roi'-structure

IMA_return: a pointer to the subvolume of the same type as the original volume 

Abort: if fromVolume not filled
       if fromVolume is not U8 or U16
*/
{
        DECL_VOLUME ( sub );
  	long int		oL1, oL2, oP1, oP2, oBP, oFP, size;
	int			nx, dx,
				iy, ny, dy,
				iz, nz, dz;
	ANY_t	    *ptVolume, *ptSub;
	
        printftrace ( IN, "extractRoi" );
        
        if ( !verifyAll ( volume ) )
            goto abort;

        if ( verifyRoi( roi, volume ) == VFALSE )
        {
          printfWarning ( "I changed the region-of-interest specification" );
          (void)fprintf ( stderr, "\tto [ %d-%d, %d-%d, %d-%d, %d-%d ]\n",
                           roi->x1, roi->x2, roi->y1, roi->y2,  
                           roi->z1, roi->z2, roi->t1, roi->t2 );
        }
        	
	getSize ( volume, &nx, &ny, &nz );
	sub = duplicateVolumeStructure ( volume, "sub" );
	dx = roi->x2 - roi->x1 + 1;
	dy = roi->y2 - roi->y1 + 1;
	dz = roi->z2 - roi->z1 + 1;
	setSize ( sub, dx, dy, dz );
	
	oBP = offset_forANYtype_BetweenPoint ( volume );
	
	oFP = ( roi->z1 * offsetSlice ( volume ) ) * oBP;
	
	oL1 = roi->y1 * offsetLine ( volume ) * oBP;
	oL2 = ( ny - 1 - roi->y2 ) * offsetLine ( volume ) * oBP;
	
	oP1 = roi->x1 * oBP;
	size = dx * oBP;
	oP2 = size + ( nx - 1 - roi->x2 ) * oBP;
	
	oFP += oL1 + oP1;
	oL2 += oL1;
	oP2 += oP1;

	ptSub = data_ANY ( sub );
	ptVolume = data_ANY ( volume ) + oFP;
	for ( iz = 0; iz < dz; iz++ )
	{
		for ( iy = 0; iy < dy; iy++ )
		{
			memcpy ( ptSub, ptVolume, size );
			ptSub += size;
			ptVolume += oP2;
		}
		ptVolume += oL2;
	}
	return ( sub );

abort:
        printfexit ( "(volume)extractRoi" );
        return((Volume*)NULL);
}





/*----------------------------------------------------------------------------*/

	void			appendVolume

/*----------------------------------------------------------------------------*/
(
	const char    *filename,
	Volume	    *volume
)
/*
IMA_function: appendVolume
IMA_synopsis: appendVolume ( filename, volume );
IMA_description: Appends a 3D volume to another 3D volume in a file.  
The 2D sizes should match. 
*/
/* Not for 4D. */
{
        char		    *filenameIma;
  	FILE		    *fileIma;
        DECL_VOLUME(dimVol);
	size_t			oL, oLbS,
				iy,
				nx,
				ny,
				iz, nz,
				bytesPerVoxel;
        Roi       roi;
        
	U8BIT_t	    *ptU8BIT;
	S8BIT_t	    *ptS8BIT;
	U16BIT_t	    *ptU16BIT;
	S16BIT_t	    *ptS16BIT;
	U32BIT_t	    *ptU32BIT;
	S32BIT_t	    *ptS32BIT;
	FLOAT_t	    *ptFLOAT;
	DOUBLE_t	    *ptDOUBLE;
	int			oTRACE;
        
printftrace (IN, "appendVolume");
oTRACE = TRACE; TRACE = OFF;

	if ( !verifyAll ( volume ) ||
		!testFilled ( volume ) )
		goto abort; 

        filenameIma = imaFile ( filename );
        fileIma = fopen ( filenameIma, "ab" );
	if ( fileIma == NULL )
	{
          printfError ( "File append error" );
          (void) fprintf ( stderr,
                          "File '%s' can not be opened for appending.\n", 
                          filenameIma );
          goto abort;
	}
        free ( filenameIma );
        
	/* mise a jour du fichier .dim */
	/*=============================*/
	dimVol = readVolumeDim ( filename, &roi );	/*bnew: HERE*/

	nx = dimVol->size.x;
	ny = dimVol->size.y;
	nz = dimVol->size.z;
	
	/* old 2D format has to be compatible with 3d format : */
        /*?theo: necessaire ? et pour t ?*/
	if (dimVol->voxelSize.z == 0.) dimVol->voxelSize.z = 1.;
	if (volume->voxelSize.z == 0.) volume->voxelSize.z = 1.;
	
	
	if ((volume->size.x != (int)nx) || (volume->size.y != (int)ny))
		{
                  printfError ( 
"Volume to append and volume to be appended do not have the same size" );
                  goto abort;
		}

	if (volume->type != dimVol->type) 
		{
                  printfError (
"Volume to append and volume to be appended do not have the same type" );
                  goto abort;
		}
	if ((volume->voxelSize.x != dimVol->voxelSize.x) || 
            (volume->voxelSize.y != dimVol->voxelSize.y) || 
            (volume->voxelSize.z != dimVol->voxelSize.z))
		{
                  printfError (
"Volume to append and volume to be appended do not have the same voxelsize" );
                  goto abort;
		}

        nz = volume->size.z;
	dimVol->size.z += nz;
        writeVolumeDim( dimVol, filename );	/*bnew: HERE*/

	/* mise a jour du fichier .ima */
	/*=============================*/

	bytesPerVoxel = sizeofType ( volume );
	oL = offsetLine ( volume );
	oLbS = offsetLineBetweenSlice ( volume );

	switch ( volume->type )
	{		
		case U8BIT :
			ptU8BIT = data_U8BIT ( volume );
			ptU8BIT += offsetFirstPoint ( volume );
if ( EXEC )
                        for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptU8BIT, bytesPerVoxel, nx, fileIma ) 
                                !=nx) goto abort;
                            ptU8BIT += oL;
                          }
                          ptU8BIT += oLbS;
			}
			break;

		case S8BIT :
			ptS8BIT = data_S8BIT ( volume );
			ptS8BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptS8BIT, bytesPerVoxel, nx, fileIma ) 
                                !=nx) goto abort;
                            ptS8BIT += oL;
                          }
                          ptS8BIT += oLbS;
			}
			break;

		case U16BIT :
			ptU16BIT = data_U16BIT ( volume );
			ptU16BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptU16BIT, bytesPerVoxel, nx, fileIma ) 
                                !=nx) goto abort;
                            ptU16BIT += oL;
                          }
                          ptU16BIT += oLbS;
			}
			break;

		case S16BIT :
			ptS16BIT = data_S16BIT ( volume );
			ptS16BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptS16BIT, bytesPerVoxel, nx, fileIma ) 
                                != nx) goto abort;
                            ptS16BIT += oL;
                          }
                          ptS16BIT += oLbS;
			}
			break;

		case U32BIT :
			ptU32BIT = data_U32BIT ( volume );
			ptU32BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptU32BIT, bytesPerVoxel, nx, fileIma ) 
                                != nx) goto abort;
                            ptU32BIT += oL;
                          }
                          ptU32BIT += oLbS;
			}
			break;

		case S32BIT :
			ptS32BIT = data_S32BIT ( volume );
			ptS32BIT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptS32BIT, bytesPerVoxel, nx, fileIma ) 
                                != nx) goto abort;
                            ptS32BIT += oL;
                          }
                          ptS32BIT += oLbS;
			}
			break;

		case VFLOAT :
			ptFLOAT = data_VFLOAT ( volume );
			ptFLOAT += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptFLOAT, bytesPerVoxel, nx, fileIma ) 
                                != nx) goto abort;
                            ptFLOAT += oL;
                          }
                          ptFLOAT += oLbS;
			}
			break;

		case VDOUBLE :
			ptDOUBLE = data_VDOUBLE ( volume );
			ptDOUBLE += offsetFirstPoint ( volume );
if ( EXEC )
			for ( iz = 0; iz < nz; iz++ )
			{
                          for ( iy = 0; iy < ny; iy++ )
                          {
                            if (fwrite ( ptDOUBLE, bytesPerVoxel, nx, fileIma )
                                != nx) goto abort;
                            ptDOUBLE += oL;
                          }
                          ptDOUBLE += oLbS;
			}
			break;


		default :
                        notImplementedFor ( volume->type );
                        goto abort;
	} 

	fclose ( fileIma );

TRACE = oTRACE;
printftrace ( OUT, "");

	return;

abort :
	printfexit( "(volume)appendVolume" );	
}




/*----------------------------------------------------------------------------*/

	void		     copySlice
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *v1,
	int			s1,
	Volume	    *v2,
	int			s2
)
{
	int			oTRACE,
  				ix, iy,
  	 			nx, ny, nz;

	long int		oPBL1, oPBL2, oP1, oP2;
	U8BIT_t	    *ptv1U8, *ptv2U8;
	U16BIT_t	    *ptv1U16, *ptv2U16;

	printftrace ( IN, "copySlice" );
	oTRACE = TRACE; TRACE = OFF;

	if ( !testFilled ( v1 ) ||
		!testTypeN ( v1, 2, U8BIT, U16BIT ) ||
		!testEqualType ( v1, v2 ) ||
		!testSlice ( v1, s1 ) ||
		!testSlice ( v2, s2 ) ||
		!testEqual2DSize ( v1, v2 ) )
		goto abort;
  
  getSize3 ( v1, &nx, &ny, &nz );
  oPBL1 = offsetPointBetweenLine ( v1 );
  oPBL2 = offsetPointBetweenLine ( v2 );
  /*bnew: (s1-1) ---> s1 */
  oP1   = offsetFirstPoint ( v1 ) + s1 * offsetSlice ( v1 );
  /*bnew: (s2-1) ---> s2 */
  oP2   = offsetFirstPoint ( v2 ) + s2 * offsetSlice ( v2 );
  
  switch ( typeOf ( v1 ) )
  {
    case U8BIT :
          ptv1U8 = data_U8BIT ( v1 ) + oP1;
          ptv2U8 = data_U8BIT ( v2 ) + oP2;
          for ( iy = 0; iy < ny; iy++ )
          {	
            for ( ix = 0; ix < nx; ix++ )
            {
              *ptv2U8++ = *ptv1U8++;
            }
            ptv1U8 += oPBL1;
            ptv2U8 += oPBL2;
          }
          break;

    case U16BIT :
          ptv1U16 = data_U16BIT ( v1 ) + oP1;
          ptv2U16 = data_U16BIT ( v2 ) + oP2;
          for ( iy = 0; iy < ny; iy++ )
          {	
            for ( ix = 0; ix < nx; ix++ )
            {
              *ptv2U16++ = *ptv1U16++;
            }
            ptv1U16 += oPBL1;
            ptv2U16 += oPBL2;
          }
          break;

    default:
          notImplementedFor ( v1->type );
          goto abort;
  }
  
  TRACE = oTRACE;
  printftrace ( OUT, "" );
  return;
abort:
  printfexit ( "(volume)copySlice" );
}

/*----------------------------------------------------------------------------*/

	Volume	    *axial2coronal
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
/* extended on 23/10/95 for U16 by BV */
{
	Volume	    *newVolume;
	int			nx, iy, ny, iz, nz;
	long int		oL, oS;
	U8BIT_t	    *ptrU8, *ptr1_U8, *ptrNewU8;
	U16BIT_t    *ptrU16, *ptr1_U16, *ptrNewU16;
	
        if ( !verifyAll ( volume ) )
            goto abort;
        if ( volume->borderWidth != 0 )
        {
          printfError ( "This function only works without borders" );
          goto abort;
        }
        
	newVolume = duplicateVolumeStructure ( volume, "coronal" );
	getSize3 ( volume, &nx, &ny, &nz );
	setSize4 ( newVolume, nx, nz, ny, 1 );

	oL = offsetLine ( volume );
	oS = offsetSlice ( volume );
        
        (void)fprintf ( stdout, "Axial to coronal : 00 %%" );
        switch ( typeOf( volume ) )
        {
          case U8BIT:
                 ptr1_U8 = data_U8BIT ( volume ) + ( nz - 1 ) * oS;
                 ptrNewU8 = data_U8BIT ( newVolume );
if ( EXEC )
                 for ( iy = 0; iy < ny; iy++ )
                 {
                   (void)fprintf ( stdout,
                                  "\b\b\b\b%2ld %%", 100 * (long)iy / (long)ny );
                   (void)fflush ( stdout );
                   ptrU8 = ptr1_U8;
                   for ( iz = 0; iz < nz; iz++ )
                   {
                     memcpy ( ptrNewU8, ptrU8, nx );
                     ptrNewU8 += nx;
                     ptrU8 -= oS;
                   }
                   ptr1_U8 += oL;
                 }
                 break;

          case U16BIT:
                 ptr1_U16 = data_U16BIT ( volume ) + ( nz - 1 ) * oS;
                 ptrNewU16 = data_U16BIT ( newVolume );
if ( EXEC )
                 for ( iy = 0; iy < ny; iy++ )
                 {
                   (void)fprintf ( stdout,
                                  "\b\b\b\b%2ld %%", 100 * (long)iy / (long)ny );
                   (void)fflush ( stdout );
                   ptrU16 = ptr1_U16;
                   for ( iz = 0; iz < nz; iz++ )
                   {
                     memcpy ( ptrNewU16, ptrU16, nx );
                     ptrNewU16 += nx;
                     ptrU16 -= oS;
                   }
                   ptr1_U16 += oL;
                 }
                 break;

          default:
                 notImplementedFor ( volume->type );
                 goto abort;
        }
        (void)fprintf ( stdout, "\b\b\b\b\b\bcompleted\n" );

	return ( newVolume );
abort:
        printfexit ( "(volume)axial2coronal" );
        return ( NULL );
}

/*----------------------------------------------------------------------------*/

	Volume	    *coronal2axial
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	Volume	    *newVolume;
	int			nx, iy, ny, iz, nz;
	long int		oL, oS;
	U8BIT_t	    *ptrU8, *ptr1_U8, *ptrNewU8;
	U16BIT_t    *ptrU16, *ptr1_U16, *ptrNewU16;
	
        if ( !verifyAll ( volume ) )
            goto abort;
        if ( volume->borderWidth != 0 )
        {
          printfError ( "This function only works without borders" );
          goto abort;
        }
       
	newVolume = duplicateVolumeStructure ( volume, "coronal" );
	getSize3 ( volume, &nx, &ny, &nz );
	setSize4 ( newVolume, nx, nz, ny, 1 );

	oL = offsetLine ( volume );
	oS = offsetSlice ( volume );
       
        (void)fprintf ( stdout, "Coronal to axial : 00 %%" );
        switch (typeOf( volume ))
        {
        case U8BIT:
          ptr1_U8 = data_U8BIT ( volume ) + ( ny - 1 ) * oL;
          ptrNewU8 = data_U8BIT ( newVolume );
 if ( EXEC )
          for ( iy = 0; iy < ny; iy++ )
          {
            (void)fprintf ( stdout,
                           "\b\b\b\b%2ld %%", 100 * (long)iy / (long)ny );
            (void)fflush ( stdout );
            ptrU8 = ptr1_U8;
		for ( iz = 0; iz < nz; iz++ )
		{
                  memcpy ( ptrNewU8, ptrU8, nx );
                  ptrNewU8 += nx;
                  ptrU8 += oS;
		}
            ptr1_U8 -= oL;
;
            
          }
          break;

        case U16BIT:
          ptr1_U16 = data_U16BIT ( volume ) + ( ny - 1 ) * oL;
          ptrNewU16 = data_U16BIT ( newVolume );
if ( EXEC )
          for ( iy = 0; iy < ny; iy++ )
          {
            (void)fprintf ( stdout,
                           "\b\b\b\b%2ld %%", 100 * (long)iy / (long)ny );
            (void)fflush ( stdout );
            ptrU16 = ptr1_U16;
		for ( iz = 0; iz < nz; iz++ )
		{
                  memcpy ( ptrNewU16, ptrU16, nx );
                  ptrNewU16 += nx;
                  ptrU16 += oS;
		}
            ptr1_U16 -= oL;
          }
          break;

          default:
                 notImplementedFor ( volume->type );
                 goto abort;
        }


          (void)fprintf ( stdout, "\b\b\b\b\b\bcompleted\n" );
	return ( newVolume );
abort:
        printfexit ( "(volume)coronal2axial" );
        return ( NULL );
}

/*----------------------------------------------------------------------------*/
	
	Volume	    *axial2sagittal
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
/* extended on 23/10/95 for U16 by BV */
{
	Volume	    *newVolume;
	int			ix, nx, iy, ny, iz, nz;
	long int		oL, oS;
	U8BIT_t	    *ptrU8, *ptr1_U8, *ptrNewU8;
	U16BIT_t    *ptrU16, *ptr1_U16, *ptrNewU16;
		
        if ( !verifyAll ( volume ) )
            goto abort;
        if ( volume->borderWidth != 0 )
        {
          printfError ( "This function only works without borders" );
          goto abort;
        }
         
	newVolume = duplicateVolumeStructure ( volume, "sagittal" );
	getSize3 ( volume, &nx, &ny, &nz );
	setSize4 ( newVolume, ny, nz, nx, 1 );

	oL = offsetLine ( volume );
	oS = offsetSlice ( volume );

        (void)fprintf ( stdout, "Axial to sagittal : 00 %%" );
        switch (typeOf( volume ))
        {
          case U8BIT:
          ptr1_U8 = data_U8BIT ( volume ) + ( nz - 1 ) * oS;
          oS *= 2;
          ptrNewU8 = data_U8BIT ( newVolume );
if ( EXEC )
          for ( ix = 0; ix < nx; ix++ )
          {
            (void)fprintf ( stdout,
                           "\b\b\b\b%2ld %%", 100 * (long)ix / (long)nx );
            (void)fflush ( stdout );
            ptrU8 = ptr1_U8++;
            for ( iz = 0; iz < nz; iz++ )
            {
              for ( iy = 0; iy < ny; iy++ )
              {
                *ptrNewU8++ = *ptrU8;
                ptrU8 += oL;
              }
              ptrU8 -= oS;
            }
          }
          break;

        case U16BIT:
          ptr1_U16 = data_U16BIT ( volume ) + ( nz - 1 ) * oS;
          oS *= 2;
          ptrNewU16 = data_U16BIT ( newVolume );
if ( EXEC )
          for ( ix = 0; ix < nx; ix++ )
          {
            (void)fprintf ( stdout,
                           "\b\b\b\b%2ld %%", 100 * (long)ix / (long)nx );
            (void)fflush ( stdout );
            ptrU16 = ptr1_U16++;
            for ( iz = 0; iz < nz; iz++ )
            {
              for ( iy = 0; iy < ny; iy++ )
              {
                *ptrNewU16++ = *ptrU16;
                ptrU16 += oL;
              }
              ptrU16 -= oS;
            }
          }
          break;          

          default:
                 notImplementedFor ( volume->type );
                 goto abort;
        }
        fprintf ( stdout, "\b\b\b\b\b\bcompleted\n" );

	return ( newVolume );
abort:
        printfexit ( "(volume)axial2sagittal" );
        return ( NULL );
}
/*----------------------------------------------------------------------------*/
	
	Volume	    *sagittal2axial
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
)
{
	Volume	    *newVolume;
	int			ix, nx, iy, ny, iz, nz;
	long int		oL, oS;
	U8BIT_t	    *ptrU8, *ptr1_U8, *ptrNewU8;
	U16BIT_t    *ptrU16, *ptr1_U16, *ptrNewU16;
		
        if ( !verifyAll ( volume ) )
            goto abort;
        if ( volume->borderWidth != 0 )
        {
          printfError ( "This function only works without borders" );
          goto abort;
        }
         
	newVolume = duplicateVolumeStructure ( volume, "sagittal" );
	getSize3 ( volume, &nx, &ny, &nz );
	setSize4 ( newVolume, nz, nx, ny, 1 );

	oL = offsetLine ( volume );
	oS = offsetSlice ( volume );

        (void)fprintf ( stdout, "Sagittal to axial : 00 %%" );
        switch (typeOf( volume ))
        {
        case U8BIT:
          ptr1_U8 = data_U8BIT ( volume ) + ( ny - 1 ) * oL      ;
          ptrNewU8 = data_U8BIT ( newVolume );
if ( EXEC )
          for ( iy = 0; iy < ny; iy++ )
          {
            (void)fprintf ( stdout,
                           "\b\b\b\b%2ld %%", 100 * (long)iy / (long)ny );
            (void)fflush ( stdout );
	  
            ptrU8 = ptr1_U8;
	    ptr1_U8 -= oL;
            for ( ix = 0; ix < nx; ix++ )
            {
	
              for ( iz = 0; iz < nz; iz++ )
              {
                *ptrNewU8++ = *ptrU8;
                ptrU8 += oS;
              }
               ptrU8 = ptrU8 - nz * oS + 1; 
            }
          }
          break;

        case U16BIT:
          ptr1_U16 = data_U16BIT ( volume ) + ( ny - 1 ) * oL;
          ptrNewU16 = data_U16BIT ( newVolume );
if ( EXEC )
          for ( iy = 0; iy < ny; iy++ )
          {
            (void)fprintf ( stdout,
                           "\b\b\b\b%2ld %%", 100 * (long)iy / (long)ny );
            (void)fflush ( stdout );
            ptrU16 = ptr1_U16;
	    ptr1_U16 -=oL;
            for ( ix = 0; ix < nx; ix++ )
            {
              for ( iz = 0; iz < nz; iz++ )
              {
                *ptrNewU16++ = *ptrU16;
                ptrU16 += oS;
              }
              ptrU16 = ptrU16 - nz * oS + 1 ;
            }
          }
          break;          

          default:
                 notImplementedFor ( volume->type );
                 goto abort;
        }
        
        fprintf ( stdout, "\b\b\b\b\b\bcompleted\n" );

	return ( newVolume );
abort:
        printfexit ( "(volume)sagittal2axial" );
        return ( NULL );
}
