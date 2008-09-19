/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipGetSlice.c        * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 12/02/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Extraction d'une coupe dans un volume
 *
 ****************************************************************************
 *
 * USED MODULES : stdlib.h - string.h - vip/slice.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *            04/03/97| POUPON F.    | Modifications pour permettre la
 *                    |              | specification du format d'image
 *                    |              | (VIDA ou TIVOLI)
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <vip/util.h>

static void Usage()
{ (void)fprintf(stderr,"Usage: VipGetSlice\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-s[lice] {slice to extract}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"slice\")}]\n");
  (void)fprintf(stderr,"\t\t[-t[ype] {a | c | s (default:a)}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ VipPrintfInfo("Extract a slice from a volume");
  (void)printf("\n");
  (void)fprintf(stderr,"Usage: VipGetSlice\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-s[lice] {slice to extract}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"slice\")}]\n");
  (void)fprintf(stderr,"\t\t[-t[ype] {a | c | s (default:a)}]\n");
  (void)fprintf(stderr,"\t\t\ta=AXIAL, c=CORONAL, s=SAGITTAL\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t\tv=VIDA, t=TIVOLI\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(volout);
  char output[VIP_NAME_MAXLEN]="slice";
  char *input=NULL;
  int i, slice=-1, type=VIP_AXIAL_SLICE, readlib=VIDA, writelib=VIDA;

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
  { if (!strncmp(argv[i], "-input", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      input = argv[i];
    }
    else if (!strncmp(argv[i], "-output", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      strcpy(output, argv[i]);
    }
    else if (!strncmp(argv[i], "-slice", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      slice = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-type", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      if (!strcmp(argv[i], "a"))  type = VIP_AXIAL_SLICE;
      else if (!strcmp(argv[i], "c"))  type = VIP_CORONAL_SLICE;
      else if (!strcmp(argv[i], "s"))  type = VIP_SAGITTAL_SLICE;
      else
      { (void)fprintf(stderr, "Unknown type : %s\n", argv[i]);
        Usage();
      }
    }
    else if (!strncmp(argv[i], "-readformat", 2)) 
    { if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
      if (argv[i][0] == 't')  readlib = TIVOLI;
      else if (argv[i][0] == 'v')  readlib = VIDA;
      else
      { VipPrintfError("This format is not implemented for reading");
        VipPrintfExit("(commandline)VipGetSlice");
        Usage();
      }
    }
    else if (!strncmp(argv[i], "-writeformat", 2)) 
    { if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
      if (argv[i][0] == 't')  writelib = TIVOLI;
      else if (argv[i][0] == 'v')  writelib = VIDA;
      else
      { VipPrintfError("This format is not implemented for writing");
        VipPrintfExit("(commandline)VipGetSlice");
        Usage();
      }
    }
    else if (!strncmp(argv[i], "-help", 2))  Help();
    else Usage();
  }

  if (input == NULL)
  { VipPrintfError("input arg is required by VipGetSlice");
    Usage();
  }

  if (slice == -1)
  { VipPrintfError("slice arg is required by VipGetSlice");
    Usage();
  }

  /***** supprime les extensions des noms si elles existent  *****/
  if (strstr(input, ".vimg") != NULL)  *strstr(input, ".vimg") = '\0';
  if (strstr(input, ".ima") != NULL)  *strstr(input, ".ima") = '\0';
  if (strstr(output, ".vimg") != NULL)  *strstr(output, ".vimg") = '\0';
  if (strstr(output, ".ima") != NULL)  *strstr(output, ".ima") = '\0';

  printf("\nReading initial volume : %s ...\n", input);
  if ((vol = (readlib == VIDA) ? VipReadVolume(input) : VipReadTivoliVolume(input)) == (Volume *)PB)
  { VipPrintfError("Error while reading volume.");
    VipPrintfExit("(commandline)VipGetSlice.");
    exit(EXIT_FAILURE);
  }

  printf("Getting slice...\n");
  volout = VipGetSlice(vol, slice, type);
  if (!volout)
  { VipPrintfError("Unable to get slice.");
    VipPrintfExit("(commandline)VipGetSlice.");
    VipFreeVolume(vol);
    exit(EXIT_FAILURE);
  }

  VipSetVolumeName(volout, output);
  printf("Saving slice : %s ...\n", output);
  if (writelib == VIDA)  VipWriteVolume(volout, output);
  else VipWriteTivoliVolume(volout, output);
  VipFreeVolume(vol);
  VipFreeVolume(volout);

  return 0;
}
