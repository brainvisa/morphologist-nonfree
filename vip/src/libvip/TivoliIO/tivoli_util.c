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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>


#include <vip/TivoliIO/TivoliIO_util.h>
#include <vip/TivoliIO/TivoliIO_volume.h>
#include <vip/util/file.h>

/***************************/
/*                         */
/*    default modes        */
/*                         */
/***************************/

         int		EXEC	=	ON;
         int		TRACE	=	OFF;	
         int		VERBOSE	=	OFF;	


/***************************/
/*                         */
/*    safe ALLOCATION      */
/*                         */
/***************************/

/*----------------------------------------------------------------------------*/

         void		    *mallocT

/*----------------------------------------------------------------------------*/
 (
         size_t		size
 )
 {
         void		    *ptr;

         ptr = malloc ( size );
         if ( !ptr ) {
                 perror ( NULL );
                 printfexit ( "(util)mallocT" );
         }
	 /*printf("%p (TmallocT)\n",ptr);*/
         return ( ptr );
 }


/*----------------------------------------------------------------------------*/

         void		    *callocT

/*----------------------------------------------------------------------------*/
 (
         size_t		nelem,
         size_t		elsize
 )
 {
         void		    *ptr;

         ptr = calloc ( nelem, elsize );
         if ( ptr == NULL )
         {
                 perror ( NULL );
                 printfexit ( "(util)callocT" );
         }
         return ( ptr );
 }

/*-----------------------------------------------------------------------------*/
void **matAlloc (int lines,
                 int columns,
                 int elem_size )
/*-----------------------------------------------------------------------------*/
/* made by Lars Aurdal */
{
  
  /**** Local variables ****/

  register int   i;
  char  *tmp;
  void  **mat;

  /**** Function for allocation ****/

  tmp = (char *)mallocT(lines * columns * elem_size);
  mat = (void **) mallocT(lines * sizeof(char *)); 

  for (i = lines; --i >= 0;)
  {
    mat[i] = &tmp[i * columns * elem_size];
  }
  return mat;
}

/*-----------------------------------------------------------------------------*/

void matFree (void **mat)

/*-----------------------------------------------------------------------------*/
/* made by Lars Aurdal */
/* you should call the function as: matFree((void **) variableName ) !!
   to prevent warnings w.r.t. type conversion */
{
  if ( mat != NULL )   /* added by Bert Verdonck: 22/02/95 */
  {  
    free ((char *) mat[0]);  /* added by Bert Verdonck: 22/02/95 */
    free ((void **) mat);
    mat = NULL;   /* added by Bert Verdonck: 22/02/95 */
  }
}

/*Bert------------------------------------------------------------------------*/
char *charMalloc(size_t length)
/*----------------------------------------------------------------------------*/
{

  /* length+1 to help not forget the allways added \0 character */
  char *ptr = malloc ( (length+1) * sizeof(char) ); 
  /*printf("%p (Tmalloc)\n",ptr); */
  if ( !ptr ) {
    perror ( NULL );
    printfexit ( "(util)charMalloc" );
  }
  return ( ptr );
}


/***************************/
/*                         */
/*   ERRORS and debugging  */
/*                         */
/***************************/


/* to eliminate: */
/*----------------------------------------------------------------------------*/

        void exitError ( char *errorText )

/*----------------------------------------------------------------------------*/
{
  printf("\n\nError: %s\n\n", errorText);
  exit(PB);
}



/*----------------------------------------------------------------------------*/

         void			printfexit

 /*----------------------------------------------------------------------------*/
 (
         char		    *fname
 )
 {
         (void) fprintf ( stderr, 
                         "\nExecution aborted in function '%s'.\n", fname );
         exit ( EXIT_FAILURE );
 }





 /*----------------------------------------------------------------------------*/

         void			printftrace

 /*----------------------------------------------------------------------------*/
 (
         int			mode,
         char		    *fmt, ...
 )
 {
         static int	first = VTRUE;
         static int	nTab = -1;
         int			iTab;
         va_list		ap;
         char		    *p,
                             *s;
         int			d;
         unsigned		u;
         long			l;
         unsigned long	lu;
         float		f;
         Volume	    *V;
         FILE		    *stdtrace;

 if ( TRACE )
 {
         if ( first )
         {
                 first = VFALSE;
                 VipUnlink( "trace.txt" );
         }
         stdtrace = fopen ( "trace.txt", "a" );
         if ( stdtrace == NULL )
         {
           printf("\nError opening trace.txt...\n");
           goto abort;
         }
         setbuf ( stdtrace, NULL );
         if ( mode != IN && mode != MSG && mode != OUT )
         {
                 (void) fprintf ( stderr,
                 "Arg #1 of function 'printftrace' has to be IN, MSG or OUT\n" );
                 goto abort;
         }
         if ( mode != OUT )
         {
                 nTab += 1;
                 for ( iTab = 0; iTab < nTab; iTab++ )
                         (void) fprintf ( stdtrace, "\t" );
         }
         va_start ( ap, fmt );
         for ( p = fmt; *p; p++ )
         {
                 if ( *p != '%' && *p != '\\' )
                 {
                         (void) putc ( *p, stdtrace );
                         continue;
                 }
                 if ( *p == '%' )
                 {
                         switch ( *++p )
                         {
                         case 'u' :
                                 u = va_arg ( ap, unsigned );
                                 (void) fprintf ( stdtrace, "%u", u );
                                 break;
                         case 'd' :
                         case 'i' :
                                 d = va_arg ( ap, int );
                                 (void) fprintf ( stdtrace, "%d", d );
                                 break;
                         case 'l' :
                                 switch ( *++p )
                                 {
                                 case 'u' :
                                         lu = va_arg ( ap, unsigned long );
                                         (void) fprintf ( stdtrace, "%lu", lu );
                                         break;
                                 case 'd' :
                                 case 'i' :
                                         l = va_arg ( ap, long );
                                         (void) fprintf ( stdtrace, "%ld", l );
                                         break;
                                 }
                                 break;
                         case 'f' :
                                 f = (float)va_arg ( ap, double );
                                 (void) fprintf ( stdtrace, "%.2f", f );
                                 break;
                         case 's' :
                                 for ( s = va_arg(ap,char*); *s; s++ )
                                         (void) putc ( *s, stdtrace );
                                 break;
                         case 'V' :
               V = va_arg ( ap, Volume* );
               (void) fprintf ( stdtrace,
                               "volume '%s' - id %ld\n", 
                               V->name, V->id );
               for ( iTab = 0; iTab < nTab; iTab++ )
                   (void) fprintf ( stdtrace, "\t" );
               (void) fprintf ( stdtrace,
                               "\t%s - %s\n - border %d", 
                               typeName(V->type), stateName(V->state),
                               V->borderWidth );
               for ( iTab = 0; iTab < nTab; iTab++ )
                   (void) fprintf ( stdtrace, "\t" );
               (void) fprintf ( stdtrace,
                         "\tsize %dx%dx%dx%d - voxel %.3fx%.3fx%.3fx%.3f\n",
                               V->size.x, V->size.y, V->size.z, V->size.t,
                               V->voxelSize.x, V->voxelSize.y, V->voxelSize.z, 
                               V->voxelSize.t );
               for ( iTab = 0; iTab < nTab; iTab++ )
                   (void) fprintf ( stdtrace, "\t" );
               (void) fprintf ( stdtrace,
                               "\tfrom '%s' at %dx%dx%dx%d",
                               V->ref.name, 
                               V->ref.x, V->ref.x, V->ref.z, V->ref.x );
               for ( iTab = 0; iTab < nTab; iTab++ )
                   (void) fprintf ( stdtrace, "\t" );
               (void) fprintf ( stdtrace, "\t%d bytes allocated", V->size3Db );
                                 
                                 break;
                         default :
                                 (void) fprintf ( stdtrace, "%c", *p );
                                 break;
                         }
                 }
                 else
                 {
                         switch ( *++p )
                         {
                         case 'n' :
                                 for ( iTab = 0; iTab < nTab; iTab++ )
                                         (void) fprintf ( stdtrace, "\t" );
                                 break;
                         case 't' :
                                 (void) fprintf ( stdtrace, "\t" );
                                 break;
                         default :
                                 (void) putc ( *p, stdtrace );
                                 break;
                         }
                 }
         }
         va_end ( ap );
         if ( mode != OUT )
                 fprintf ( stdtrace, "\n" );
         if ( mode != IN )
                 nTab -= 1;
         fclose ( stdtrace );
 }
         return;
 abort :
         printfexit ( "(util)printftrace" );
 }


/***************************/
/*                         */
/*    USER I/O- functions  */
/*                         */
/***************************/

/*****************************************************************************/
/* Ask for a float */
/**********************/

float ask_float(float *def, float *mini, float *maxi, char *comment)
/*****************************************************************************/
{
	float           f;
	double          d;
	char            input[128];

	printf("\n%s ", comment);
	if (def != NULL)
		printf("[def=%f] ", *def);
	if (mini != NULL)
		printf("[mini=%f] ", *mini);
	if (maxi != NULL)
		printf("[maxi=%f] ", *maxi);
	printf(": ");
	setbuf(stdin, NULL);

	scanf("%127s", input);
	if (strlen(input) == 0)
		if (def == NULL)
		{
			printf("You have to answer me something !  Try again !");
			return ( ask_float(def, mini, maxi, comment) );
		} else
			f = *def;
	else
	{
		d = strtod(input, NULL);

		if (errno == ERANGE)
      {
			if (d == HUGE_VAL)
			{
				printf("\nOverflow, try again !!");
				return ( ask_float(def, mini, maxi, comment) );
			} else
			{
				printf("\nUnderflow, try again !!");
				return ( ask_float(def, mini, maxi, comment) );
			}
      }

		f = (float) d;
		if ((maxi != NULL) && (f > *maxi))
		{
			printf("\nGreater than maximum (%f), try again !!", *maxi);
			return (ask_float(def, mini, maxi, comment) );
		}
		if ((mini != NULL) && (f < *mini))
		{
			printf("\nSmaller than minimum (%f), try again !!", *mini);
			return (ask_float(def, mini, maxi, comment) );
		}
	}
	printf("\n***Thank you: %f\n", f);
	return (f);

}

/***********************************************************************/

int getMyInt( 
int liminf, int limsup, int show)
{
	int result;

        printftrace(IN,"getMyInt");
        if(show) (void)printf("%d..%d: ",liminf, limsup);
	else (void)printf(": ");
        while(1)
        {
          if( !scanf("%d", &result)) (void)getchar();
          else if((result>=liminf) && (result<=limsup)) break;
          (void)printf("Pardon???\n");
        }
        printftrace(OUT,"");
	return(result);
      }
/***************************************************************/
float getMyFloat(  float liminf, float limsup,
                 int show)
{
  float result;
  
  printftrace(IN,"getMyFloat");
  if(show) (void)printf("%3f..%3f: ",liminf, limsup);
  else (void)printf(": ");
  while(1)
  {
    if( !scanf("%f", &result)) (void)getchar();
    else if((result>=liminf) && (result<=limsup)) break;
    (void)printf("Pardon???\n");
  }
  printftrace(OUT,"");
  return(result);
}

/***************************************************************/
char *getMyName(
                char *question,
                int maxsize)
{
  char *rep;
  int flag, l;
  
  printftrace(IN,"getMyName");
  if(maxsize<=0)
  {
    printf("taille incoherente dans GetMyName\n");
    exit(EXIT_FAILURE);
  }
  printf("%s\n", question);
  
  rep = (char *)mallocT(maxsize);
  
  flag = VTRUE;
  while(flag)
  {
    printf(":");
    scanf("%s", rep);
    l = strlen(rep);
    if(l<=0 || l>maxsize) printf("???\n");
    else flag = VFALSE;
  }
  printftrace(OUT,"");
  return(rep);
}

/********************************************************************/

int getManicheanRep(
                    char *question)
{
  char rep[200];
  
  printftrace(IN,"getManicheanRep");
  printf("\n%s (o/n)\n\n", question);
  (void)scanf("%s",rep);
  printftrace(OUT,"");
  if( (*rep=='o') || (*rep=='O') || 
     (*rep=='y') || (*rep=='Y') ) return(VTRUE);
  else return(VFALSE);
}


/***************************/
/*                         */
/*    DISK I/O- functions  */
/*                         */
/***************************/

/*----------------------------------------------------------------------------*/
void matPrint(char     name[],
              int        rows,
              int        cols,
              double  **array)
/*----------------------------------------------------------------------------*/

/* Function for printing data arrays in MATLAB format. */

{
  
  /**** Local declarations ****/
  
  int    i;
  int    j;
  FILE *fp;

  /**** Body of function ****/

  fp = fopen(name,"w");
  if(fp == NULL)
  {
    printf("\nCannot open file for array: %s\n",name);
    exit(EXIT_FAILURE);
  }
  for(i = 0;i < rows;i++)
  {
    for(j = 0;j < cols;j++)
    {
      fprintf(fp,"%32f ",array[i][j]);
    }
    fprintf(fp,"\n");
  }
  fclose(fp);
}

/*-----------------------------------------------------------------*/

/*****************************************************************************/
/* open file to read from */
int file_open_read(FILE **fp, const char *name)
/*****************************************************************************/
{
  if ((*fp = fopen(name,"r"))==NULL)
  {	
    perror( name );
    exit(EXIT_FAILURE);
    return(PB);
  }
  printftrace(MSG, "$file_open_read$: I opened file %s to read:", name);
  return(OK);
}

/*****************************************************************************/
/* open file to write to */
int file_open_write(FILE **fp, const char *name)
/*****************************************************************************/
{
  if ((*fp = fopen(name,"w"))==NULL)
  {	
    perror( name );
    exit(EXIT_FAILURE);
    return(PB);
  }
  printftrace(MSG, "$file_open_write$: I opened file %s to write:", name);
  return(OK);
}

/*****************************************************************************/
/* open file to append to */
int file_open_append(FILE **fp, const char *name)
/*****************************************************************************/
{
  if ((*fp = fopen(name,"a"))==NULL)
  {	
    perror( name );
    exit(EXIT_FAILURE);
    return(PB);
  }
  return(OK);
}

/*****************************************************************************/
/* close file */
int file_close(FILE **fp)
/*****************************************************************************/
{
  if(fclose(*fp))
  {
    perror(NULL);
    exit(EXIT_FAILURE);
    return(PB);
  }
  printftrace(MSG, "$file_close$: I closed a file");
  return(OK);
}


/*****************************************************************************/
int file_write(void *ptr, size_t size, size_t length, FILE *fp)
/*****************************************************************************/
{
  if (fwrite(ptr, size, length, fp) != length)
  {
    perror(NULL);
    exit(EXIT_FAILURE);
    return(PB);
  }
  return(OK);
}


/*****************************************************************************/
int m_filename(char *base, char *name, int number)
/*****************************************************************************/
{
  if ( number<0 && number>999 )
    {	
      printf("\n@@ error in m_filename: number = %d\n", number);
      exit(EXIT_FAILURE);
    }
  else
    if (number<10)
      sprintf(name,"%s_00%1d",base,number);
    else if (number<100)
      sprintf(name,"%s_0%2d",base,number);
    else
      sprintf(name,"%s_%3d",base,number);
  
/*  printftrace(MSG, "(m_filename): Actual filename: %s", name);
*/
  return (OK);
}

/*****************************************************************************/
int m_point_filename(char *base, char *name, int number)
/*****************************************************************************/
{
  if ( number<0 && number>999 )
    {	
      printf("\n@@ error in m_point_filename: number = %d\n", number);
      exit(EXIT_FAILURE);
    }
  else
    if (number<10)
      sprintf(name,"%s.00%1d",base,number);
    else if (number<100)
      sprintf(name,"%s.0%2d",base,number);
    else
      sprintf(name,"%s.%3d",base,number);
  
/*  printftrace(MSG, "(m_point_filename): Actual filename: %s", name);
*/
  return (OK);
}


/*****************************************************************************/
int m_zero_filename(char *base, char *name, int number)
/*****************************************************************************/
{
  if ( number<0 && number>999 )
    {	
      printf("\n@@ error in m_filename: number = %d\n", number);
      exit(EXIT_FAILURE);
    }
  else
    if (number<10)
      sprintf(name,"%s00%1d",base,number);
    else if (number<100)
      sprintf(name,"%s0%2d",base,number);
    else
      sprintf(name,"%s%3d",base,number);
  
/*  printftrace(MSG, "(m_zero_filename): Actual filename: %s", name);
*/
  return (OK);
}


/*----------------------------------------------------------------------------*/

      char *composeName ( char *baseName, char *ext1, char *ext2 )

/*----------------------------------------------------------------------------*/
{
  char *name;
  char *s0, *s1, *s2;
  int len;

  if (baseName == NULL)
    {
    printf("Warning: 'baseName' is empty in funcion 'composeName'");
    s0 = charMalloc(9);
    len = 9;
    strcpy(s0, "emptyName");
    }
  else
    {
    s0 = baseName;
    len = strlen(baseName);
    }

  if (ext1 == NULL)
    {
    s1 = charMalloc(0);
    strcpy(s1, "");
    }
  else
    {
    s1 = ext1;
    len += strlen(ext1);
    }
  if (ext2 == NULL)
    {
    s2 = charMalloc(0);
    strcpy(s2, "");
    }
  else
    {
    s2 = ext2;
    len += strlen(ext2);
    }

  name = charMalloc(len);
  sprintf(name, "%s%s%s", s0, s1, s2);

  return(name);
}




/***************************/
/*                         */
/*    ROUND - functions    */
/*                         */
/***************************/



/*----------------------------------------------------------------------------*/
int roundD2I(double f)
/*----------------------------------------------------------------------------*/
{
  if (f<=0)
      return (int) ( (f-floor(f) < 0.5) ? floor(f) : ceil(f) );
  else
      return (int) ( (floor(f)-f <= 0.5) ? floor(f) : ceil(f) );
}


/*****************************************************************************/
/******************************roundFloat2Long********************************/
/*****************************************************************************/
long int
roundFloat2Long(float f)
{
	return (long int) (((f - floor(f)) < 0.5) ? floor(f) : ceil(f));
}

/*****************************************************************************/
/******************************roundFloat2Int********************************/
/*****************************************************************************/
int 
roundFloat2Int(float f)
{
	return (int) (((f - floor(f)) < 0.5) ? floor(f) : ceil(f));
}

/*****************************************************************************/
/******************************roundFloat2UChar*******************************/
/*****************************************************************************/
unsigned char
roundFloat2UChar(float f)
{
	return (unsigned char) (((f - floor(f)) < 0.5) ? floor(f) : ceil(f));
}

/*********************************/
/*                               */
/*    HELP- USAGE - functions    */
/*                               */
/*********************************/

/*----------------------------------------------------------------------------*/
int printUsage(char *usageOf)
/*----------------------------------------------------------------------------*/
{
  FILE *fp=NULL;
  char *filename;
  char c, *charPtr;
  
  filename=charMalloc(150);
      
  /* search current directory first */
  sprintf(filename, "%s_U.txt", usageOf);
  if ((fp=fopen(filename, "r"))!=NULL)
      ;
    /* else: search in directory given by T_HELPDIR environment-variable*/
  else if ((charPtr=getenv(T_HELPDIR))!=NULL)
  {
    sprintf(filename, "%s%s_U.txt", charPtr, usageOf);
    fp=fopen(filename, "r");
  }
  
  if (fp == NULL)
  {
    printf("\nI could not find the usage of '%s'\n", usageOf);
    return(PB);
  }
  else
  {
    while ((c=getc(fp))!= EOF)
        if (c!='%')
            printf("%c", c);
        else
            while ( (c=getc(fp))!= EOF && c!='\n' )
                ;
    
    file_close(&fp);
  }
  
  return(OK);
}


/*----------------------------------------------------------------------------*/
int printHelp(char *helpOn)
/*----------------------------------------------------------------------------*/
{

  FILE *fp=NULL;
  char *filename;
  char c, *charPtr;
  
  filename=charMalloc(150);

  /* search current directory first */
  sprintf(filename, "%s_H.txt", helpOn);
  if ((fp=fopen(filename, "r"))!=NULL)
      ;
  /* else: search in directory given by T_HELPDIR environment-variable*/
  else if ((charPtr=getenv(T_HELPDIR))!=NULL)
  {
    sprintf(filename, "%s%s_H.txt", charPtr, helpOn);
    fp=fopen(filename, "r");
  }
  
  if (fp == NULL)
  {
    printf("\nI could not find help for '%s'\n", helpOn);
    printUsage(helpOn);
    return(PB);
  }
  else
  {
    while ((c=getc(fp))!= EOF)
        if (c!='%')
            printf("%c", c);
        else
            while ( (c=getc(fp))!= EOF && c!='\n' )
                ;
    
    file_close(&fp);
  }
  
  printUsage(helpOn);
  return(OK);
}






/* The end */
