/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipCovarianceMatrix.c * TYPE     : Command line
 * AUTHOR      : POUPON F.             * CREATION : 13/03/1997
 * VERSION     : 0.1                   * REVISION :
 * LANGUAGE    : C++                   * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Calcul la matrice de covariance a partir d'une matrice
 *               de donnees
 *
 ****************************************************************************
 *
 * USED MODULES : string.h - vip/volume.h - vip/math.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/volume.h>
#include <vip/math.h>

#include <stdlib.h>
#include <string.h>

static void Usage()
{ (void)fprintf(stderr,"Usage: VipCovarianceMatrix\n");
  (void)fprintf(stderr,"\t\t-i[nput] {ASCII data file}\n");
  (void)fprintf(stderr,"\t\t-f[eature] {number of features}\n");
  (void)fprintf(stderr,"\t\t-p[opulation] {number of individuals in population}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {ASCII result file without extension (default:output on stdout)}]\n");
  (void)fprintf(stderr,"\t\t[-t[type] {n | i (default:n)}]\n");
  (void)fprintf(stderr,"\t\t[-d[iagonal]] {only get the matrix diagonal}\n");
  (void)fprintf(stderr,"\t\t[-m[ean]] {generates an additional ASCII file of means}\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ VipPrintfInfo("Compute the covariance matrix from a data matrix.");
  (void)printf("\n");
  (void)fprintf(stderr,"Usage: VipCovarianceMatrix\n");
  (void)fprintf(stderr,"\t\t-i[nput] {ASCII data file}\n");
  (void)fprintf(stderr,"\t\t-f[eature] {number of features}\n");
  (void)fprintf(stderr,"\t\t-p[opulation] {number of individuals in population}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {ASCII result file without extension (default:output on stdout)}]\n");
  (void)fprintf(stderr,"\t\t[-t[type] {n | i (default:n)}]\n");
  (void)fprintf(stderr,"\t\t\td=NORMAL, i=INVERSE\n");
  (void)fprintf(stderr,"\t\t[-d[iagonal]] {only get the matrix diagonal}\n");
  (void)fprintf(stderr,"\t\t[-m[ean]] {generates an additional ASCII file of means}\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{ char *input=NULL, *output=NULL;
  char covname[VIP_NAME_MAXLEN], meanname[VIP_NAME_MAXLEN];
  int  i, j, dimy=0, dimx=0, type=C_NORMAL, ret_mean=0, diag=C_NORMAL;
  VipMatrix_VDOUBLE *matrix, *covariance, *inverse, *swap;
  VipVector_VDOUBLE *mean=NULL;
  Vip_DOUBLE **covptr;

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
    else if (!strncmp(argv[i], "-feature", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      dimy = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-population", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      dimx = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-mean", 2))  ret_mean = 1;
    else if (!strncmp(argv[i], "-diagonal", 2))  diag = C_VARIANCE; 
    else if (!strncmp(argv[i], "-type", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      if (!strcmp(argv[i], "n"))  type = C_NORMAL;
      else if (!strcmp(argv[i], "i"))  type = C_INVERSE;
      else
      { (void)fprintf(stderr, "Unknown type : %s\n", argv[i]);
        Usage();
      }
    }
    else if (!strncmp(argv[i], "-help", 2))  Help();
    else Usage();
  }

  if (input == NULL)
  { VipPrintfError("input arg is required by VipCovarianceMatrix");
    Usage();
  }

  if (dimx == 0)
  { VipPrintfError("population arg is required by VipCovarianceMatrix");
    Usage();
  }

  if (dimy == 0)
  { VipPrintfError("feature arg is required by VipCovarianceMatrix");
    Usage();
  }

  if (output != NULL)
  { strcpy(covname, output);
    strcat(covname, ".cov");
    strcpy(meanname, output);
    strcat(meanname, ".avg");
  }

  if (ret_mean)
  { mean = VipAllocationVector_VDOUBLE(dimy, NULL);
    if (!mean)
    { VipPrintfError("Error while allocating mean vector");
      VipPrintfExit("(commandline)VipCovarianceMatrix");
      exit(EXIT_FAILURE);
    }
  }

  printf("Reading data file : %s\n", input);
  matrix = VipReadMatrix_VDOUBLE(input, dimx, dimy);
  printf("Computing Covariance matrix...\n");
  covariance = VipCovarianceMatrix_VDOUBLE(matrix, mean);
  
  if (diag == C_VARIANCE)
  { covptr = VipGetMatrixPtr_VDOUBLE(covariance);
    for (j=1; j<dimy; j++)
      for (i=0; i<j; i++)  covptr[i][j] = covptr[j][i] = 0.0;
  }

  if (type == C_INVERSE)
  { inverse = VipMatrixInversion_VDOUBLE(covariance);
    swap = covariance;
    covariance = inverse;
    inverse = swap;
    VipFreeMatrix_VDOUBLE(inverse);
  }

  printf("Writing covariance matrix file : %s\n", (output) ? covname : "stdout\0");
  VipPrintMatrix_VDOUBLE((output) ? covname : NULL, covariance);

  if (ret_mean)
  { printf("Writing mean vector file : %s\n", meanname);
    VipPrintVector_VDOUBLE((output) ? meanname : NULL, mean);
    VipFreeVector_VDOUBLE(mean);
  }

  VipFreeMatrix_VDOUBLE(matrix);
  VipFreeMatrix_VDOUBLE(covariance);

  return(OK);
}


