/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipVFilter.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 30/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Application d'un V-Filtre sur un volume - fpoupon@cea.fr
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            04/03/97| POUPON F.    | Modifications pour permettre la
 *                    |              | specification du format d'image
 *                    |              | (VIDA ou TIVOLI)
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 *****************************************************************************/

#include <vip/volume.h>
#include <vip/vfilter.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipVFilter\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"vfilter\")}]\n");
  (void)fprintf(stderr,"\t\t[-m[masksize] {int (default:2)}]\n");
  (void)fprintf(stderr,"\t\t[-t[ype] {char: o or n (default:o)}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ 
  VipPrintfInfo("Apply a V-Filter to an image");
  (void)printf("\n");
  (void)printf("Usage: VipVFilter\n");
  (void)printf("\t\t-i[nput] {image name}\n");
  (void)printf("\t\t[-o[utput] {image name (default:\"vfilter\")}]\n");
  (void)printf("\t\t[-m[masksize] {int (default:2)}]\n");
  (void)printf("\t\t[-t[ype] {char: o or n (default:o)}]\n");
  (void)printf("\t\t\to=OPTIMIZED, n=NON OPTIMIZED\n");
  (void)printf("\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("\t\t\tv=VIDA, s=SPM, t=TIVOLI\n");
  (void)printf("\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{ 
  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(volout); 
  char  output[VIP_NAME_MAXLEN]="vfilter";
  char *input=NULL;
  int   i, mask=2, readlib=VIDA, writelib=VIDA, type=OPTIMIZED_VFILTER; 

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
      else if (!strncmp(argv[i], "-masksize", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  mask = atoi(argv[i]);
	  if (mask < 2)  mask = 2;
	}
      else if (!strncmp(argv[i], "-type", 2)) 
	{ 
	  if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
	  if (argv[i][0] == 'o')  type = OPTIMIZED_VFILTER;
	  else if (argv[i][0] == 'n')  type = NON_OPTIMIZED_VFILTER;
	  else
	    { 
	      VipPrintfError("Unknown processing type.");
	      VipPrintfExit("(commandline)VipVFilter");
	      Usage();
	    }
	}
      else if (!strncmp(argv[i], "-readformat", 2)) 
	{ 
	  if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
	  if (argv[i][0] == 't')  readlib = TIVOLI;
	  else if (argv[i][0] == 's')  readlib = SPM;
	  else if (argv[i][0] == 'v')  readlib = VIDA;
	  else
	    { 
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipVFilter");
	      Usage();
	    }
	}
      else if (!strncmp(argv[i], "-writeformat", 2)) 
	{ 
	  if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
	  if (argv[i][0] == 't')  writelib = TIVOLI;
	  else if (argv[i][0] == 's')  writelib = SPM;
	  else if (argv[i][0] == 'v')  writelib = VIDA;
	  else
	    { 
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipVFilter");
	      Usage();
	    }
	}
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if (input == NULL)
    { 
      VipPrintfError("input arg is required by VipVFilter");
      Usage();
    }

  if (VipTestImageFileExist(input)==PB)
    { 
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(PB);
    }

  /***** supprime les extensions des noms si elles existent  *****/
  if (strstr(input, ".vimg") != NULL)  *strstr(input, ".vimg") = '\0';
  if (strstr(input, ".ima") != NULL)  *strstr(input, ".ima") = '\0';
  if (strstr(input, ".img") != NULL)  *strstr(input, ".img") = '\0';
  if (strstr(output, ".vimg") != NULL)  *strstr(output, ".vimg") = '\0';
  if (strstr(output, ".ima") != NULL)  *strstr(output, ".ima") = '\0';
  if (strstr(output, ".img") != NULL)  *strstr(output, ".img") = '\0';

  printf("\nReading initial volume : %s ...\n", input);
  switch(readlib)
    {
    case VIDA:
      {
	vol = VipReadVolumeWithBorder(input, 1);
	break;
      }
    case TIVOLI:
      {
	vol = VipReadTivoliVolumeWithBorder(input, 1);
	break;
      }
    case SPM:
      {
	vol = VipReadSPMVolumeWithBorder(input, 1);
	break;
      }
    }

  if (!vol)
    { 
      VipPrintfError("Error while reading volume.");
      VipPrintfExit("(commandline)VipVFilter");
      exit(EXIT_FAILURE); 
    }

  printf("Processing V-Filter...\n");
  switch(type)
    {
    case OPTIMIZED_VFILTER:
      {
	volout = VipVFilter(vol, mask);
	break;
      }
    case NON_OPTIMIZED_VFILTER:
      {
	volout = VipNonOptimizedVFilter(vol, mask);
	break;
      }
    }

  if (!volout)
    { 
      VipPrintfError("unable to create V-Filtered volume");
      VipPrintfExit("(commandline)VipVFilter");  
      exit(EXIT_FAILURE);
    }

  printf("Writing volume : %s ...\n", output);
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
    }

  VipFreeVolume(vol);
  VipFree(vol);
  VipFreeVolume(volout);
  VipFree(volout);

  return(EXIT_SUCCESS);
}
