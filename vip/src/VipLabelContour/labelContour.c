/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipLabelContour.c    * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 14/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Recupere le(s) contour(s) a partir d'un label ou d'une
 *               liste de labels
 *
 ****************************************************************************
 *
 * USED MODULES : stdio.h - stdlib.h - string.h - vip/label_contour.h
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

#include <vip/label.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void Usage()
{
  (void)fprintf(stderr,"Usage: VipLabelContour\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-l[abel {(int,...) = list of labels to extract (max:256 labels)}\n");
  (void)fprintf(stderr,"\t\t-v[alue {(int,...) = list of corresponding label filling colors (max:256 colors)}\n");
  (void)fprintf(stderr,"\t\t[-c[connexity] {(int) = 6, 18 or 26 (default:6)}}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"contour\")}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{
  VipPrintfInfo("Get contour(s) in a label volume from a label or a list of labels");
  (void)printf("The input and output are label volume file.\n");
  (void)printf("\n");
  (void)fprintf(stderr,"Usage: VipLabelContour\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-l[abel {(int,...) = list of labels to extract (max:256 labels)}\n");
  (void)fprintf(stderr,"\t\t\texample : -l 60,250,16,128\n"); 
  (void)fprintf(stderr,"\t\t-v[alue {(int,...) = list of corresponding label filling colors (max:256 colors)}\n");
  (void)fprintf(stderr,"\t\t\texample : -v 15,245,253,75\n"); 
  (void)fprintf(stderr,"\t\t\tCaution : Number of labels must be equal to number of colors\n");
  (void)fprintf(stderr,"\t\t[-c[connexity] {(int) = 6, 18 or 26 (default:6)}}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"contour\")}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t\tv=VIDA, t=TIVOLI\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(volres);
  char  output[VIP_NAME_MAXLEN]="contour";
  char *input=NULL, *temp=NULL;
  int   i, j=0, k=0, connexite=6, *label=NULL, *label_col=NULL, readlib=VIDA, writelib=VIDA; 
  
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
    else if (!strncmp(argv[i], "-connexity", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      connexite = atoi(argv[i]);
      if ((connexite != 6) && (connexite != 18) && (connexite != 26))  connexite = 6;
    }
    else if (!strncmp(argv[i], "-label", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      label = (int *)VipCalloc(256, sizeof(int), NULL); 
      if (!label)  
      { VipPrintfError("Error while allocating label buffer.");
        VipPrintfExit("(commandline)VipGetContour");
        exit(EXIT_FAILURE);
      }
      label[j++] = atoi(strtok(argv[i], ","));
      while (((temp = strtok(NULL, ",")) != NULL) && (j < 256))  label[j++] = atoi(temp);
    }
    else if (!strncmp(argv[i], "-value", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      label_col = (int *)VipCalloc(256, sizeof(int), NULL); 
      if (!label_col)  
      { VipPrintfError("Error while allocating label color buffer.");
        VipPrintfExit("(commandline)VipGetContour");
        exit(EXIT_FAILURE);
      }
      label_col[k++] = atoi(strtok(argv[i], ","));
      while (((temp = strtok(NULL, ",")) != NULL) && (k < 256))  label_col[k++] = atoi(temp);
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
  { VipPrintfError("input arg is required by VipLabelContour");
    Usage();
  }

  if (label == NULL)
  { VipPrintfError("label arg is required by VipLabelContour");
    Usage();
  }

  if (label_col == NULL)
  { VipPrintfError("value arg is required by VipLabelContour");
    Usage();
  }

  if (k != j)
  { VipPrintfError("Number of labels must be equal to number of colors");
    Usage();
  }

  /***** supprime les extensions des noms si elles existent  *****/
  if (strstr(input, ".vimg") != NULL)  *strstr(input, ".vimg") = '\0';
  if (strstr(input, ".ima") != NULL)  *strstr(input, ".ima") = '\0';
  if (strstr(output, ".vimg") != NULL)  *strstr(output, ".vimg") = '\0';
  if (strstr(output, ".ima") != NULL)  *strstr(output, ".ima") = '\0';

  printf("\nReading initial volume : %s file...\n", input);
  if ((vol = (readlib == VIDA) ? VipReadVolumeWithBorder(input, 1) 
                               : VipReadTivoliVolumeWithBorder(input, 1)) == (Volume *)PB)
  { VipPrintfError("Error while reading volume.");
    VipPrintfExit("(commandline)VipLabelContour");
    exit(EXIT_FAILURE); 
  }
  if (!VipSetBorderLevel(vol, -1))
  { VipPrintfError("Error while filling border.");
    VipPrintfExit("(commandline)VipLabelContour");
    exit(EXIT_FAILURE);
  }

  printf("Getting %s ", (j == 1) ? "contour\0" : "contours\0");
  volres = VipGetMultipleLabelContourVolume(vol, label, label_col, j, connexite);
  if (volres == PB)
  { sprintf(temp, "Unable to create %s volume", (j == 1) ? "contour\0" : "contours\0");
    VipPrintfError(temp);
    VipPrintfExit("(commandline)VipLabelContour");
    exit(EXIT_FAILURE);
  }
  VipFreeVolume(vol);

  VipSetVolumeName(volres, output);
  printf("Writing volume : %s file...\n", output);
  if (writelib == VIDA)  VipWriteVolume(volres, output);
  else VipWriteTivoliVolume(volres, output);
  VipFreeVolume(volres);
  VipFree(label);

  return(OK);
}


