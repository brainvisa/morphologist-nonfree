/***************************************************************************
 * PROJECT     : 
 * MODULE NAME : VipTopoClassification.c * TYPE     : Function
 * AUTHOR      : POUPON F.               * CREATION : 11/01/2000
 * VERSION     : 0.1                     * REVISION :
 * LANGUAGE    : C++                     * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ***************************************************************************
 *
 * DESCRIPTION : 
 *
 ***************************************************************************
 *
 * USED MODULES : 
 *
 ***************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|--------------------------------------
 *              /  /  |              |
 ***************************************************************************/

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/topology.h>

#include <stdlib.h>
#include <string.h>

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipTopoClassification\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"classified\")}]\n");
  (void)fprintf(stderr,"\t\t[-l[abel] {label to classify (default:255)}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ 
  VipPrintfInfo("Topological classification of a given label");
  (void)printf("\n");
  (void)printf("Usage: VipTopoClassification\n");
  (void)printf("\t\t-i[nput] {image name}\n");
  (void)printf("\t\t[-o[utput] {image name (default:\"classified\")}]\n");
  (void)printf("\t\t[-l[abel] {label to classify (default:255)}]\n");
  (void)printf("\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("\t\t\tv=VIDA, s=SPM, t=TIVOLI\n");
  (void)printf("\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  VIP_DEC_VOLUME(vol); 
  char output[VIP_NAME_MAXLEN]="classified\0";
  char *input=NULL;
  int i, lab=255, readlib=VIDA, writelib=VIDA; 

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
          strcpy( output, argv[i] );
        }
      else if (!strncmp(argv[i], "-label", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          lab = atoi( argv[i] );
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
              VipPrintfExit("(commandline)VipTopoClassification");
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
              VipPrintfExit("(commandline)VipTopoClassification");
              Usage();
            }
        }
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if (input == NULL)
    { 
      VipPrintfError("input arg is required by VipTopoClassification");
      Usage();
    }

  /***** Lecture de l'image *****/
  printf("Reading : %s ...\n", input);
  switch( readlib )
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

  if (VipTopologicalClassificationForLabel( vol, lab ) == PB)
    {
      VipPrintfError( "Error while classifying" );
      exit( EXIT_FAILURE );
    }

  /***** sauvegarde du resultat *****/
  printf("Writing : %s ...\n", output);
  switch(writelib)
    {
    case VIDA:
      {
        VipWriteVolume(vol, output);
        break;
      }
    case TIVOLI:
      {
        VipWriteTivoliVolume(vol, output);
        break;
      }
    case SPM:
      {
        VipWriteSPMVolume(vol, output);
        break;
      }
    }

  VipFreeVolume(vol);
  VipFree(vol);

  return 0;
}
