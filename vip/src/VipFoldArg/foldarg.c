/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipFoldArg.c        * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 22/07/99
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sparc
 ****************************************************************************
 *
 * DESCRIPTION :  
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/




/*##############################################################################

	I N C L U S I O N S

##############################################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <math.h>

#include <vip/volume.h>
#include <vip/fold_arg.h>
#include <vip/util/file.h>
#include <vip/util/shelltools.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(skeleton); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "someone";
  int readlib;
  int loutside = 11;
  int linside = 0;
  int size = 15;
  FoldArg *arg;
  char write = 'o';
  char *voronoi = NULL;
  char *argname = NULL;
  int triflag = VFALSE;
  char buffer[10000];
  char buffer2[10000];
 
  int i;

  readlib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-arg", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  argname = argv[i];
	}
      else if (!strncmp (argv[i], "-voronoi", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  voronoi = argv[i];
	}    
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-size", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  size = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-linside", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  linside = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-loutside", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  loutside = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipFoldArg");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='o') write = 'o';
	  else if(argv[i][0]=='g') write = 'g';
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipFoldArg");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL && argname==NULL)
    {
      VipPrintfError("input or arg arguments is required by VipFoldArg");
      return(Usage());
    }
  if ( argname==NULL && VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
	  
  if(argname==NULL)
      {
	  printf("Reading skeleton %s...\n",input);
	  if (readlib == TIVOLI)
	      skeleton = VipReadTivoliVolumeWithBorder(input,1);
	  else
	      skeleton = VipReadVolumeWithBorder(input,1);
	  
	  if(skeleton==NULL) return(VIP_CL_ERROR);

	  arg = CreateFoldArgFromSquel( output, skeleton,
					linside, loutside, size, triflag,
					voronoi);
	  if(arg==PB) return(VIP_CL_ERROR);
      }
  else 
      {
	  printf("Reading fold arg %s...\n",argname);
	  arg = VipReadOldFoldArg(argname);
	  if(arg==PB) return(VIP_CL_ERROR);
      }

  if(write=='o')
      VipWriteOldFoldArg( arg, output);
  else if(write=='g')
    {
      if( argname != NULL && strcmp(output,argname))
	{
	  sprintf(buffer,"%s.data",output);
	  VipMkdir(buffer);
	  sprintf( buffer, "%s.data/Tmtk", argname );
	  sprintf( buffer2, "%s.data/Tmtk", output );
          VipCp( buffer, buffer2, VipRecursive );
	}
      VipWriteGenericFoldArg( arg, output);
    }
  else
      VipPrintfError("Write format error");

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipFoldArg\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-a[rg] {graphe name for conversion}]\n");
  (void)fprintf(stderr,"        [-o[utput] {arg name (default:\"someone\")}]\n");
  (void)fprintf(stderr,"        [-v[oronoi] {roots voronoi image name}]\n");
  (void)fprintf(stderr,"        [-s[ize] {int: min ss size (default:15)}]\n");
  (void)fprintf(stderr,"        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)fprintf(stderr,"        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: o or g (default:o)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Create a fold based attributed relational graph");
  (void)printf("\n");
  (void)printf("Usage: VipFoldArg\n");
  (void)printf("        -i[nput] {image name} : depends on the algorithm\n");
  (void)printf("        [-o[utput] {arg name (default:\"someone\")}]\n");
  (void)printf("        [-a[rg] {arg name for conversion}]\n");
  (void)printf("        [-v[oronoi] {roots voronoi image name}]\n");
  (void)printf("This voronoi stem from VipSkeleton and represent watershed catchment bassins\n");
  (void)printf("of sulcal roots. It is used to split topologically simple surfaces\n");
  (void)printf("if this image name is not given, such split will not occur...\n");
  (void)printf("        [-s[ize] {int: min ss size (default:15)}]\n");
  (void)printf("ss size coorresponds to (ss points + edge points)\n");
  (void)printf("        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)printf("        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-w[riteformat] {char: o or g (default:o)}]\n");
  (void)printf("o: old Arg format\n");
  (void)printf("g: Generic Arg format (C++ lib)\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
