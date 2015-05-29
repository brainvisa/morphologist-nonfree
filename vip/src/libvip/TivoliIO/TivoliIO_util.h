/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/TivoliIO_util.h  * TYPE     : Header
 * AUTHOR      : see below            * CREATION : 12/09/1994
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : B. Verdonck, T. Geraud, J.F. Mangin
 *               modified //
 *
 *               Copyright (c) 1994
 *               Dept. IMA  TELECOM Paris
 *               46, rue Barrault  F-75634 PARIS Cedex 13
 *               All Rights Reserved
 *
 *               This file groups general utility functions used allover the
 *               tivoli library.  Please contact B.V. before adding additional
 *               functions.
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


#ifndef VIP_TIVOLI_UTIL_H
#define VIP_TIVOLI_UTIL_H

#include <stdlib.h>
#include <stdio.h>

/*JFM*/
/*define bete et mechants*/

#define SHOW_LIM 1
#define HIDE_LIM 0

/* boolean values */

#define VTRUE	1
#define VFALSE	0

#define YES     1
#define NO      0

#define ONE     1
#define ZERO    0

/* value returned by testing functions */

#define OK	1
#define PB	0

/* first argument of tracing function 'printftrace' */

#define IN  1
#define MSG 2
#define OUT 3

/* flag modes */

#define ON	1
#define OFF	0

/* default dir for help and usage files */
#define T_HELPDIR  "T_HELPDIR"
/* this has changed: this constant now contains the name of the
 * environment-variable in which to look for the help-directory 
 */

/* Bert:   (see also volmanip.h) */

#define WORLD  1
#define DATA   2

typedef struct {
  double x, y, z;
  int state;   } Coord;

typedef struct {
  float x, y, z; } CoordF;

typedef struct {
  Coord p1, p2; } LineCoord;

typedef struct {
  Coord p1, p2, p3; } PlaneCoord;

typedef struct {
  double phi, theta, cosphi, sinphi, costheta, sintheta; 
  int state;   } Angle;
                                /* phi: in the xy-plane */
                                /* theta: perp. to xy-plane */
/* def of Coord, LineCoord, PlaneCoord, Angle */

#ifdef __cplusplus
extern "C" {
#endif


/* global flags */

extern int	TRACE;	/* trace mode by default */
extern int	VERBOSE;	/* verb11ose mode by default */
extern int	EXEC;	/* execution mode by default */

/***************************/
/*                         */
/*    handy MACROS         */
/*                         */
/***************************/

/*----------------------------------------------------------------------------*/

	#define vinilum(a,b) ((a)<(b)?(a):(b))
	#define vaxilum(a,b) ((a)>(b)?(a):(b))

/*----------------------------------------------------------------------------*/


/***************************/
/*                         */
/*    safe ALLOCATION      */
/*                         */
/***************************/


/*----------------------------------------------------------------------------*/
	extern
	void		    *mallocT
/*
	abort program execution if there is no more memory available ;
	otherwise do 'malloc'
*/
/*----------------------------------------------------------------------------*/
(
	size_t		size
);


/*----------------------------------------------------------------------------*/
	extern
	void		    *callocT
/*
	abort program execution if there is no more memory available ;
	otherwise do 'calloc'
*/
/*----------------------------------------------------------------------------*/
(
	size_t		nelem,
	size_t		elsize
);


/*-----------------------------------------------------------------------------*/
        extern 
        void             **matAlloc

/*      allocate (if possible) a consecutive memory segment for a matrix by using
        callocT.
*/ 
/*-----------------------------------------------------------------------------*/
(             
                 int lines,
                 int columns,
                 int elem_size
);

/*-----------------------------------------------------------------------------*/
        extern 
        void             matFree

/*      Free memory allocated to a matrix.
*/ 
/*-----------------------------------------------------------------------------*/
(             
                 void **   mat
);

/*----------------------------------------------------------------------------*/

char *charMalloc(size_t  length);
/*----------------------------------------------------------------------------*/


/*==============================*
 |                              |
 |  error processing functions  |
 |                              |
 *==============================*/

/*----------------------------------------------------------------------------*/
        extern
        void exitError ( char *errorText );

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
	extern
	void			printfexit
/*
	abort program execution after printing the name of the function
	where problems occure
*/
/*----------------------------------------------------------------------------*/
(
	char		    *fname
);


/*----------------------------------------------------------------------------*/
	extern
	void			printftrace
/*
	if 'TRACE' is 'ON', print in file "./trace.txt" a trace message ;
	otherwise, do nothing
*/
/*----------------------------------------------------------------------------*/
(
	int			mode,
	char		    *fmt, ...
);

/***************************/
/*                         */
/*    USER I/O- functions  */
/*                         */
/***************************/

/* Ask for a float */
/**********************/
float ask_float(float *def, float *mini, float *maxi, char *comment);

/*-----------------------------------------------*/
/*demande un entier en specifiant eventuellement les bornes (qui
sont toujours actives) selon la valeur de show (true,false)*/
extern int getMyInt( int liminf, int limsup, int show);
/*JFM*/

/*----------------------------------------------------*/
/*demande un float (meme convention que getMyInt)*/
extern float getMyFloat(  float liminf, float limsup, int show);
/*JFM*/

/*-------------------------------------------------*/
/*demande une chaine de caractere en posant une question*/
extern char *getMyName( char *question, int maxsize);
/*JFM*/

/*---------------------------------------------------------*/
/*demande une reponse oui ou non a une question*/
extern int getManicheanRep(char *question);
/*JFM*/


/***************************/
/*                         */
/*    DISK I/O- functions  */
/*                         */
/***************************/

/*----------------------------------------------------------------------------*/
        extern
        void            matPrint
/* Function for printing data arrays in MATLAB format.
*/
/*----------------------------------------------------------------------------*/
(
                char     name[],
                int        rows,
                int        cols,
                double  **array
);

extern int file_open_read(FILE **fp, const char *name);
extern int file_open_write(FILE **fp, const char *name);
extern int file_open_append(FILE **fp, const char *name);
extern int file_close(FILE **fp);

extern int file_write(void *ptr, size_t size, size_t length, FILE *fp);

extern int m_filename(char *base, char *name, int number);
extern int m_point_filename(char *base, char *name, int number);
extern int m_zero_filename(char *base, char *name, int number);

extern char *composeName ( char *baseName, char *ext1, char *ext2 );



/***************************/
/*                         */
/*    ROUND - functions    */
/*                         */
/***************************/

int roundD2I(double f);
long int roundFloat2Long(float f);
int roundFloat2Int(float f);
unsigned char roundFloat2UChar(float f);

/*********************************/
/*                               */
/*    HELP- USAGE - functions    */
/*                               */
/*********************************/
int printUsage(char *usageOf);
/* Searches usage information in a text file "<usageOf>_U.txt".
   path: ./ 
         or the path indicated in the T_HELPDIR constant
   Output goes to stdout.
   Lines beginning with a '%' are skipped.
*/

int printHelp(char *helpon);
/* Same behaviour as printUsage, but for a file "<helpon>_H.txt".
   After the helptext, "<helpon>_U.txt" is given also.
*/

#ifdef __cplusplus
}
#endif

#endif /* TIVOLI_UTIL_H */
