/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipPca.c             * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 28/06/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Analyse en Composantes Principales
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_pca.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <string.h>

#include <vip/pca.h>

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipPca\n");
  (void)fprintf(stderr,"\t\t-i[nput] {data ASCII file}\n");
  (void)fprintf(stderr,"\t\t[-p[rincipal] {principal axis data file (default:stdout output)}]\n");
  (void)fprintf(stderr,"\t\t[-c[ircle] {correlation circle data file (default:stdout output)}]\n");
  (void)fprintf(stderr,"\t\t-v[ariables] {number of variables}\n");
  (void)fprintf(stderr,"\t\t-t[ests] {number of tests}\n");
  (void)fprintf(stderr,"\t\t[-a[nalyse] {c | v | s (default:c)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ 
  VipPrintfInfo("Principal Components Analysis.");
  (void)printf("\n");
  (void)fprintf(stderr,"Usage: VipPca\n");
  (void)fprintf(stderr,"\t\t-i[nput] {data ASCII file}\n");
  (void)fprintf(stderr,"\t\t[-p[rincipal] {principal axis data file (default:stdout output)}]\n");
  (void)fprintf(stderr,"\t\t\tset the file extension to .plot to have a specific\n");
  (void)fprintf(stderr,"\t\t\tASCII file structure for IDL/Matlab plot.\n");
  (void)fprintf(stderr,"\t\t[-c[ircle] {correlation circle data file (default:stdout output)}]\n");
  (void)fprintf(stderr,"\t\t\tset the file extension to .plot to have a specific\n");
  (void)fprintf(stderr,"\t\t\tASCII file structure for IDL/Matlab plot.\n");
  (void)fprintf(stderr,"\t\t-v[ariables] {number of variables (e.g. number of moments)}\n");
  (void)fprintf(stderr,"\t\t-t[ests] {number of tests (e.g. number of objects)}\n");
  (void)fprintf(stderr,"\t\t[-a[nalyse] {c | v | s (default:c)}]\n");
  (void)fprintf(stderr,"\t\t\tc=Correlation analysis, v=Variance/covariance analysis\n");
  (void)fprintf(stderr,"\t\t\ts=SSCP analysis\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{ 
  char *input=NULL, *principal=NULL, *circle=NULL;
  int i, nb_var=0, nb_tests=0, analyse=PCA_CORRELATION;
  Pca_VDOUBLE *pca;

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
    { 
      if (!strncmp(argv[i], "-input", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  input = argv[i];
	}
      else if (!strncmp(argv[i], "-principal", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  principal = argv[i];
	}
      else if (!strncmp(argv[i], "-circle", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  circle = argv[i];
	}
      else if (!strncmp(argv[i], "-variables", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  nb_var = atoi(argv[i]);
	}
      else if (!strncmp(argv[i], "-tests", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  nb_tests = atoi(argv[i]);
	}
      else if (!strncmp(argv[i], "-analyse", 2))
	{ 
	  if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  if (!strcmp(argv[i], "c"))  analyse = PCA_CORRELATION;
	  else if (!strcmp(argv[i], "v"))  analyse = PCA_COVARIANCE;
	  else if (!strcmp(argv[i], "s"))  analyse = PCA_SSCP;
	  else
	    { 
	      (void)fprintf(stderr, "Unknown type : %s\n", argv[i]);
	      Usage();
	    }
	}
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if (input == NULL)
    { 
      VipPrintfError("input arg is required by VipPca");
      Usage();
    }

  if (nb_var == 0)
    { 
      VipPrintfError("variables arg is required by VipPca");
      Usage();
    }

  if (nb_tests == 0)
    { 
      VipPrintfError("tests arg is required by VipPca");
      Usage();
    }

  printf("Reading %s...\n", input);
  pca = VipReadPCADataFile_VDOUBLE(input, nb_tests, nb_var);  
  if (pca == NULL)
    { 
      VipPrintfError("Error while reading file.");
      VipPrintfExit("(commandline)VipPca.");
      exit(EXIT_FAILURE);
    }
  printf("Processing PCA analysis...\n");
  VipPCAAnalysis_VDOUBLE(pca, analyse);
  VipPrintPCA_VDOUBLE(pca, principal, circle, analyse);

  VipFreePCA_VDOUBLE(pca);

  return 0;
}
