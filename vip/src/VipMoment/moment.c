/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipMoment.c          * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 26/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Calcul les moments d'une forme
 *
 ****************************************************************************
 *
 * USED MODULES : string.h - vip/volume.h - vip/moment.h
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

#include <vip/volume.h>
#include <vip/moment.h>

#include <string.h>

static void Usage()
{ (void)fprintf(stderr,"Usage: VipMoment\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-l[abel] {pattern label in image}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {ASCII result file (default:output on stdout)}]\n");
  (void)fprintf(stderr,"\t\t[-t[ype] {n | l (default:n)}]\n");
  (void)fprintf(stderr,"\t\t[-d[imension] {2 | 3 (default:3)}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ VipPrintfInfo("Compute moment and moment invariant of a pattern from a label image.");
  (void)printf("\n");
  (void)fprintf(stderr,"Usage: VipMoment\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-l[abel] {pattern label in image}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {ASCII result file (default:output on stdout)}]\n");
  (void)fprintf(stderr,"\t\t\tset the file extension to .pca to have a specific\n");
  (void)fprintf(stderr,"\t\t\tASCII file structure for Principal Component Analysis.\n");
  (void)fprintf(stderr,"\t\t\tset the file extension to .inv to have a specific\n");
  (void)fprintf(stderr,"\t\t\tASCII file structure for Covariance matrix computation.\n");
  (void)fprintf(stderr,"\t\t[-t[ype] {n | l (default:n)}]\n");
  (void)fprintf(stderr,"\t\t\tn=NORMAL, l=LEGENDRE\n");
  (void)fprintf(stderr,"\t\t[-d[imension] {2 | 3 (default:3)}]\n");
  (void)fprintf(stderr,"\t\t\t2=2D moment, 3=3D moment\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t\tv=VIDA, t=TIVOLI\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{ VIP_DEC_VOLUME(vol); 
  char *input=NULL, *output=NULL, message[256];
  int  label=-1, dim=M_3D, i, type=M_NORMAL, readlib=VIDA;
  Moment *moment;
  Legendre *legendre;
  Moment2D *moment2D;
  Legendre2D *legendre2D;

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
  { if (!strncmp(argv[i], "-input", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      input = argv[i];
    }
    else if (!strncmp(argv[i], "-output", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      output = argv[i];
    }
    else if (!strncmp(argv[i], "-label", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      label = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-type", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      if (!strcmp(argv[i], "n"))  type = M_NORMAL;
      else if (!strcmp(argv[i], "l"))  type = M_LEGENDRE;
      else
      { (void)fprintf(stderr, "Unknown type : %s\n", argv[i]);
        Usage();
      }
    }
    else if (!strncmp(argv[i], "-dimension", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      dim = atoi(argv[i]);
      if ((dim != M_2D) && (dim != M_3D))
      { (void)fprintf(stderr, "Wrong dimension : %d\n", dim);
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
    else if (!strncmp(argv[i], "-help", 2))  Help();
    else Usage();
  }

  if (input == NULL)
  { VipPrintfError("input arg is required by VipMoment");
    Usage();
  }
  if (label == -1)
  { VipPrintfError("label arg is required by VipMoment");
    Usage();
  }

  if (strstr(input, ".vimg") != NULL)  *strstr(input, ".vimg") = '\0';
  if (strstr(input, ".ima") != NULL)  *strstr(input, ".ima") = '\0';

  printf("Reading %s...\n", input);
  if ((vol = (readlib == VIDA) ? VipReadVolume(input) : VipReadTivoliVolume(input)) == (Volume *)PB)
  { VipPrintfError("Error while reading volume.");
    VipPrintfExit("(commandline)VipMoment.");
    exit(EXIT_FAILURE);
  }
  printf("%dD moment computation...\n", dim);
  
  switch(dim)
  { case M_2D :
      if (mVipVolSizeZ(vol) != 1)
      { sprintf(message, "%s is a volume!", input);
        VipPrintfError(message);
        VipPrintfExit("(commandline)VipMoment");
        VipFreeVolume(vol);
        exit(EXIT_FAILURE);
      }

      switch(type)
      { case M_NORMAL : moment2D = VipCreateAndComputeMomentAndInvariant2D(vol, 
                                                       (Vip_U16BIT)label, M_ADD);
                        if (moment2D == NULL)  
                        { VipPrintfExit("(commandline)VipMoment");
                          VipFreeVolume(vol);
                          exit(EXIT_FAILURE);
                        }
                        if (VipPrintMoment2D(moment2D, input, output) == PB)
                        { VipPrintfExit("(commandline)VipMoment");
                          VipFreeMoment2D(moment2D);
                          VipFreeVolume(vol);
                          exit(EXIT_FAILURE);
                        }
                        VipFreeMoment2D(moment2D);
                        break;
        case M_LEGENDRE : legendre2D = VipCreateAndComputeMomentAndLegendreMoment2D(vol, 
                                                                (Vip_U16BIT)label, M_ADD);
                          if (legendre2D == NULL)  
                          { VipPrintfExit("(commandline)VipMoment");
                            VipFreeVolume(vol);
                            exit(EXIT_FAILURE);
                          }
                          if (VipPrintMoment2D(legendre2D->moment, input, output) == PB)
                          { VipPrintfExit("(commandline)VipMoment");
                            VipFreeLegendreMoment2D(legendre2D);
                            VipFreeVolume(vol);
                            exit(EXIT_FAILURE);
                          }
                          if (VipPrintLegendreMoment2D(legendre2D, input, output) == PB)
                          { VipPrintfExit("(commandline)VipMoment");
                            VipFreeLegendreMoment2D(legendre2D);
                            VipFreeVolume(vol);
                            exit(EXIT_FAILURE);
                          }
                          VipFreeLegendreMoment2D(legendre2D);
                          break;
      }
      break;
    
    case M_3D :
      switch(type)
      { case M_NORMAL : moment = VipCreateAndComputeMomentAndInvariant(vol, 
                                                   (Vip_U16BIT)label, M_ADD);
                        if (moment == NULL)  
                        { VipPrintfExit("(commandline)VipMoment");
                          VipFreeVolume(vol);
                          exit(EXIT_FAILURE);
                        }
                        if (VipPrintMoment(moment, input, output) == PB)
                        { VipPrintfExit("(commandline)VipMoment");
                          VipFreeMoment(moment);
                          VipFreeVolume(vol);
                          exit(EXIT_FAILURE);
                        }
                        VipFreeMoment(moment);
                        break;
        case M_LEGENDRE : legendre = VipCreateAndComputeMomentAndLegendreMoment(vol, 
                                                            (Vip_U16BIT)label, M_ADD);
                          if (legendre == NULL)  
                          { VipPrintfExit("(commandline)VipMoment");
                            VipFreeVolume(vol);
                            exit(EXIT_FAILURE);
                          }
                          if (VipPrintMoment(legendre->moment, input, output) == PB)
                          { VipPrintfExit("(commandline)VipMoment");
                            VipFreeLegendreMoment(legendre);
                            VipFreeVolume(vol);
                            exit(EXIT_FAILURE);
                          }
                          if (VipPrintLegendreMoment(legendre, input, output) == PB)
                          { VipPrintfExit("(commandline)VipMoment");
                            VipFreeLegendreMoment(legendre);
                            VipFreeVolume(vol);
                            exit(EXIT_FAILURE);
                          }
                          VipFreeLegendreMoment(legendre);
                          break;
      }
      break;
  }

  VipFreeVolume(vol);

  return(0);
}


