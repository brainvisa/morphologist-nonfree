/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : fileDump.c           * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 11/10/1999
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Dump hexadecimal d'un fichier binaire
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipDumpFile\n");
  (void)fprintf(stderr,"\t\t-i[nput] {binary file name}\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{
  VipPrintfInfo("Binary file dump.");
  (void)printf("\n");
  (void)printf("Usage: VipDumpFile\n");
  (void)printf("\t\t-i[nput] {binary file name}\n");
  (void)printf("\t\t[-h[elp]]\n");
  (void)printf("\n");
  (void)printf("For large file, use: VipDumpFile -i binaryFile | more\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  char *input=NULL;
  int i, nl, nc;
  off_t size;
  size_t reste, addr=0;
  FILE *theFile;
  unsigned char *fileptr, *dptr;
	
  struct stat fileStat;

  // Gestion des arguments
  for (i=1; i<argc; i++)
    {
      if (!strncmp(argv[i], "-input", 2))
	{
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1)) Usage();
	  input = argv[i];
	}
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if (input == NULL)
    {
      printf("input argument is required by VipDumpFile\n");
      Usage();
    }

  if (!(theFile = fopen(input, "rb")))
    {
      printf("Cannot open file...\n");
      exit(EXIT_FAILURE);
    }
		
  fstat(fileno(theFile), &fileStat);
	
  size = fileStat.st_size;
  /* ## does not support large files! */
  fileptr = (unsigned char *)calloc((size_t)size, sizeof(unsigned char));
  fread((void *)fileptr, sizeof(unsigned char), (size_t)size, theFile);
  fclose(theFile);
	
  reste = (size_t)size%16;

  dptr = fileptr;

  /***** pour tout le fichier, sauf la derniere ligne qui
   ***** n'est pas toujours pleine 
   *****/
  for (nl=size/16; nl--;)
    {
      printf("0x%08x   ", (int)addr);
      for (nc=8; nc--; dptr++) printf("%02x ", *dptr);
      printf(" ");
      for (nc=8; nc--; dptr++) printf("%02x ", *dptr);
      printf("   ");
      dptr -= 16;
      for (nc=8; nc--; dptr++) 
	{
	  if ((*dptr < 32) || (*dptr > 122))
	    printf(". ");
	  else 
	    printf("%c ", *dptr);
	}
      printf(" ");
      for (nc=8; nc--; dptr++)
	{
	  if ((*dptr < 32) || (*dptr > 122))
	    printf(". ");
	  else 
	    printf("%c ", *dptr);
	}
      printf("\n");
      addr += 16;
    }

  /***** pour la derniere ligne *****/
  if (reste < 8)
    {
      printf("0x%08x   ", (int)addr);
      for (nc=reste; nc--; dptr++) printf("%02x ", *dptr);
      for (nc=8-reste; nc--;) printf("   ");
      printf(" ");
      for (nc=8; nc--;) printf("   ");
      printf("   ");
      dptr -= reste;
      for (nc=reste; nc--; dptr++)
	{
	  if ((*dptr < 32) || (*dptr > 122))
	    printf(". ");
	  else 
	    printf("%c ", *dptr);
	}
      for (nc=8-reste; nc--;) printf("   ");
      printf(" ");
      for (nc=8; nc--;) printf("   ");
    }
  else
    {
      printf("0x%08x   ", (int)addr);
      for (nc=8; nc--; dptr++) printf("%02x ", *dptr);
      printf(" ");
      for (nc=reste-8; nc--; dptr++) printf("%02x ", *dptr);
      for (nc=16-reste; nc--;) printf("   ");
      printf("   ");
      dptr -= reste;
      for (nc=8; nc--; dptr++)
	{
	  if ((*dptr < 32) || (*dptr > 122))
	    printf(". ");
	  else 
	    printf("%c ", *dptr);
	}
      printf(" ");
      for (nc=reste-8; nc--; dptr++)
	{
	  if ((*dptr < 32) || (*dptr > 122))
	    printf(". ");
	  else 
	    printf("%c ", *dptr);
	}
      for (nc=16-reste; nc--;) printf("   ");
    }
		
  printf("\n");
		
  free(fileptr);

  return(EXIT_SUCCESS);
}
