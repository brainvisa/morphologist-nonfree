/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : swapEndian.c         * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 15/10/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Inverse les octets forts et faibles d'une image
 *
 ****************************************************************************
 *
 * USED MODULES : stdlib.h - stdio.h - string.h - Vip_volume.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/alloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(__osf__)
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#elif defined(__sun) || defined(__sgi)
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#define OTHER_FORMAT 12321

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipSwapEndian\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"swapped\")}]\n");
  (void)fprintf(stderr,"\t\t[-n[bytes] {integer: 2 or 4 (default:2)}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v, s, t or o (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v, s, t or o (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{
  VipPrintfInfo("Swap high and low bytes of a VIDA image");
  (void)printf("\n");
  (void)printf("Usage: VipSwapEndian\n");
  (void)printf("\t\t-i[nput] {image name}\n");
  (void)printf("\t\t[-o[utput] {image name (default:\"swapped\")}]\n");
  (void)printf("\t\t[-n[bytes] number of bytes involved for swap {integer: 2 or 4 (default:2)}]\n");
  (void)printf("\t\t\t2: AB -> BA;  4, ABCD -> DCBA    (only used with the '-r o' option)\n");
  (void)printf("\t\t[-r[eadformat] {char: v, s, t or o (default:v)}]\n");
  (void)printf("\t\t[-w[riteformat] {char: v, s, t or o (default:v)}]\n");
  (void)printf("\t\t\tv=VIDA, s=SPM, t=TIVOLI, o=OTHER (single binary file)\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{ 
  int i, j, k, l, sx, sy, sz, st, slice, line, plane;
  int nbytes=2, readlib=VIDA, writelib=VIDA;
  char output[VIP_NAME_MAXLEN]="swapped";
  char theInput[VIP_NAME_MAXLEN];
  char *input=NULL;
  int16_t *vol2=NULL, *v2ptr=NULL;
  int32_t *vol4=NULL, *v4ptr=NULL;
  Volume *volin=NULL, *volout=NULL;
  Vip_S16BIT *inptr=NULL, *outptr=NULL;
  VipOffsetStruct *vos=NULL;
  FILE *fic;
  int lo=0, b=0;

  struct stat theStat;

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
    { 
      if (!strncmp(argv[i], "-input", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  input = argv[i];
	}
      else if (!strncmp(argv[i], "-output", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  strcpy(output, argv[i]);
	}
      else if (!strncmp(argv[i], "-nbytes", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  nbytes = atoi( argv[i] );
	}
      else if (!strncmp(argv[i], "-readformat", 2)) 
        { 
          if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
          if (argv[i][0] == 't')  readlib = TIVOLI;
          else if (argv[i][0] == 's')  readlib = SPM;
          else if (argv[i][0] == 'v')  readlib = VIDA;
	  else if (argv[i][0] == 'o')  readlib = OTHER_FORMAT;
          else
            { 
              VipPrintfError("This format is not implemented for reading");
              VipPrintfExit("(commandline)VipSwapEndian");
              Usage();
            }
        }
      else if (!strncmp(argv[i], "-writeformat", 2)) 
        { 
          if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
          if (argv[i][0] == 't')  writelib = TIVOLI;
          else if (argv[i][0] == 's')  writelib = SPM;
          else if (argv[i][0] == 'v')  writelib = VIDA;
	  else if (argv[i][0] == 'o')  writelib = OTHER_FORMAT;
          else
            { 
              VipPrintfError("This format is not implemented for writting");
              VipPrintfExit("(commandline)VipSwapEndian");
              Usage();
            }
        }
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if (input == NULL)
    { 
      VipPrintfError("input arg is required by VipSwapEndian");
      Usage();
    }

  if ( nbytes != 2 && nbytes != 4 )
    {
      VipPrintfError( "Incompatibles nbytes value. Must be 2 or 4." );
      Usage();
    }

  if ( readlib == OTHER_FORMAT )  writelib = OTHER_FORMAT;

  strcpy(theInput, input);

  /***** ajoute les extensions des noms si elles n'existent pas  *****/
  switch( readlib )
    {
    case VIDA:
      if (strstr(theInput, ".vimg") == NULL)  strcat(theInput, ".vimg");
      break;
    case TIVOLI:
      if (strstr(theInput, ".ima") == NULL)  strcat(theInput, ".ima");
      break;
    case SPM:
      if (strstr(theInput, ".img") == NULL)  strcat(theInput, ".img");
    }

  if (stat(theInput, &theStat) == -1)
    {
      VipPrintfError("Invalid file name.");
      VipPrintfExit("(commandline)VipSwapEndian");
      exit(EXIT_FAILURE);
    }

  /*** test si le modulo par nbytes est nul ou non ***/
  if (theStat.st_size % nbytes)
    {
      VipPrintfError("Invalid file length.");
      VipPrintfExit("(commandline)VipSwapEndian");
      exit(EXIT_FAILURE);
    }

  printf("Loading file : %s...\n", input);
  switch(readlib)
    {
    case VIDA:
      {
        volin = VipReadVolume(input);
        break;
      }
    case TIVOLI:
      {
        volin = VipReadTivoliVolume(input);
        break;
      }
    case SPM:
      {
        volin = VipReadSPMVolume(input);
        break;
      }
    case OTHER_FORMAT:
      {
	lo = (int)theStat.st_size / (int)nbytes;

	switch( nbytes )
	  {
	  case 2:
	    {
	      vol2 = (int16_t *)VipCalloc( (size_t)lo, sizeof(int16_t), "" );
	      fic = fopen( input, VIP_READ_BIN );
	      if (!fic)
		{
		  VipPrintfError( "Cannot open file." );
		  exit( EXIT_FAILURE );
		}
	      if ( fread( vol2, sizeof(int16_t), (size_t)lo, fic ) != (size_t)lo )
        {
          VipPrintfError( "Error reading file." );
          fclose( fic );
		      exit( EXIT_FAILURE );
        }
	      fclose( fic );
	      break;
	    }
	  case 4:
	    {
	      vol4 = (int32_t *)VipCalloc( (size_t)lo, sizeof(int32_t), "" );
	      fic = fopen( input, VIP_READ_BIN );
	      if (!fic)
		{
		  VipPrintfError( "Cannot open file." );
		  exit( EXIT_FAILURE );
		}
	      if ( fread( vol4, sizeof(int32_t), (size_t)lo, fic ) != (size_t)lo )
        {
          VipPrintfError( "Error reading file." );
          fclose( fic );
		      exit( EXIT_FAILURE );
        }
	      fclose( fic );
	      break;
	    }
	  }
	break;
      }
    }

  printf("File length : %d bytes\nProcessing : %4d%%", (int)theStat.st_size, 0);

  switch( readlib )
    {
    case VIDA:
    case TIVOLI:
    case SPM:
      {
	volout = VipDuplicateVolumeStructure(volin, "");
	VipAllocateVolumeData(volout);

	vos = VipGetOffsetStructure(volin);
	inptr = VipGetDataPtr_S16BIT(volin)+vos->oFirstPoint;
	outptr = VipGetDataPtr_S16BIT(volout)+vos->oFirstPoint;

	sx = mVipVolSizeX(volin);
	sy = mVipVolSizeY(volin);
	sz = mVipVolSizeZ(volin);
	st = mVipVolSizeT(volin);
	line = vos->oPointBetweenLine;
	slice = vos->oLineBetweenSlice;
	plane = vos->oSliceBetweenVolume;

	for (l=st; l--; inptr+=plane, outptr+=plane)
	  for (k=sz; k--; inptr+=slice, outptr+=slice)
	    {
	      for (j=sy; j--; inptr+=line, outptr+=line)
		for (i=sx;i--; inptr++)
		  *outptr++ = ((*inptr & 0xFF00) >> 8) | ((*inptr & 0x00FF) << 8);
	      printf("\b\b\b\b\b%4d%%", (int)((float)(sz*st-k-l*sz)/(float)(sz*st)*100.0+0.5));
	      fflush(stdout);
	    }

	break;
      }
    case OTHER_FORMAT:
      {
	switch( nbytes )
	  {
	  case 2:
	    {
	      v2ptr = vol2;
	      for ( b=lo; b--; )
		{
		  *v2ptr = ((*v2ptr & 0xFF00) >> 8) | ((*v2ptr & 0x00FF) << 8);
                  ++v2ptr;
		  if (!(b % (lo/100)))
		    {
		      printf("\b\b\b\b\b%4d%%", (int)((float)(lo-b)/(float)(lo)*100.0+0.5));
		      fflush(stdout);
		    }
		}
	      break;
	    }
	  case 4:
	    {
	      v4ptr = vol4;
	      for ( b=lo; b--; )
		{
		  *v4ptr = ((*v4ptr & 0xFF000000) >> 24) 
                    | ((*v4ptr & 0x00FF0000) >> 8) |
                    ((*v4ptr & 0x0000FF00) << 8) 
                    | ((*v4ptr & 0x000000FF) << 24);
                  ++v4ptr;
		  if (!(b % (lo/100)))
		    {
		      printf("\b\b\b\b\b%4d%%", (int)((float)(lo-b)/(float)(lo)*100.0+0.5));
		      fflush(stdout);
		    }
		}
	      break;
	    }
	  }

	printf("\b\b\b\b\b%4d%%", (int)((float)(lo-b)/(float)(lo)*100.0+0.5));
	fflush(stdout);

	break;
      }
    }

  printf("\nWritting %s...\n", output);
  switch(writelib)
    {
    case VIDA:
      {
        VipWriteVolume(volout, output);
        break;
      }
    case TIVOLI:
      {
        VipWriteTivoliVolume(volout, output);
        break;
      }
    case SPM:
      {
        VipWriteSPMVolume(volout, output);
        break;
      }
    case OTHER_FORMAT:
      {
	switch( nbytes )
	  {
	  case 2:
	    {
	      fic = fopen( output, VIP_WRITE_BIN );
	      if (!fic)
		{
		  VipPrintfError( "Cannot open file." );
		  exit( EXIT_FAILURE );
		}
	      fwrite( vol2, sizeof(int16_t), (size_t)lo, fic );
	      fclose( fic );
	      break;
	    }
	  case 4:
	    {
	      fic = fopen( output, VIP_WRITE_BIN );
	      if (!fic)
		{
		  VipPrintfError( "Cannot open file." );
		  exit( EXIT_FAILURE );
		}
	      fwrite( vol4, sizeof(int32_t), (size_t)lo, fic );
	      fclose( fic );
	      break;
	    }
	  }
	break;
      }
    }

  printf("Done...\n");

  switch( readlib )
    {
    case VIDA:
    case TIVOLI:
    case SPM:
      {
	VipFreeVolume(volout);
	VipFree(volout);
	VipFreeVolume(volin);
	VipFree(volout);
	VipFree(vos);
	break;
      }
    case OTHER_FORMAT:
      {
	switch( nbytes )
	  {
	  case 2:
	    VipFree( vol2 );
	    break;
	  case 4:
	    VipFree( vol4 );
	    break;
	  }
	break;
      }
    }

  return(EXIT_SUCCESS);
}
