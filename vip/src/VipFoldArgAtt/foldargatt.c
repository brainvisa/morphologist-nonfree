/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipFoldArgAtt.c        * TYPE   : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 10/08/99
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sparc
 ****************************************************************************
 *
 * DESCRIPTION :  compute Fold Arg Attributes
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
#include <vip/talairach.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(skeleton); 
  char *input = NULL;
  char *argname = NULL;
  char output[VIP_NAME_MAXLEN] = "attributed";
  int readlib;
  int loutside = 11;
  int linside = 0;
  FoldArg *arg;
  int talset = VFALSE;
  char write = 'o';
  char point_filename[VIP_NAME_MAXLEN]="";
  char triangulation = 'n';
  int triflag = VFALSE;
  int xAC=-1, yAC=-1, zAC=-1, xPC=-1, yPC=-1, zPC=-1, xIH=-1, yIH=-1, zIH=-1;
  char *lhemname = NULL;
  char *rhemname = NULL;
  VIP_DEC_VOLUME(lhem); 
  VIP_DEC_VOLUME(rhem); 
  VIP_DEC_VOLUME(twohem); 
  char Talairach = 'y';
  VipTalairach tal;

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
      else if (!strncmp (argv[i], "-lhem", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  lhemname = argv[i];
	}
      else if (!strncmp (argv[i], "-rhem", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  rhemname = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-Points", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(point_filename,argv[i]);
	}   
      else if (!strncmp (argv[i], "-xAC", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xAC = atoi(argv[i]);
	  talset = VTRUE;
	}
      else if (!strncmp (argv[i], "-yAC", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  yAC = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zAC", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zAC = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-xPC", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xPC = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-yPC", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  yPC = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zPC", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zPC = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-xIH", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xIH = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-yIH", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  yIH = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zIH", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zIH = atoi(argv[i]);
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
     else if (!strncmp (argv[i], "-triangulation", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    triangulation = argv[i][0];
	    if(triangulation=='y' || triangulation=='o') triflag = VTRUE;
	    else if (triangulation=='n') triflag = VFALSE;
	    else
		{
		    VipPrintfError("-tri: y/n/o flag");
		    return(Usage());
		}
	  }
	} 
     else if (!strncmp (argv[i], "-Talairach", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    Talairach = argv[i][0];
	    if(Talairach!='y' && Talairach!='n')
		{
		    VipPrintfError("-Tal: y/n flag");
		    return(Usage());
		}
	  }
	} 
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipFoldArgAtt");
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
	      VipPrintfExit("(commandline)VipFoldArgAtt");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL || argname==NULL)
    {
      VipPrintfError("input and arg arguments are required by VipFoldArgAtt");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
  if(!strcmp(output,"attributed")) strcpy(output,argname);

  if(Talairach=='y' && triangulation!='o')
      {

	  if( 
	     ( !strcmp(point_filename,"") &&
	       (xAC==-1 || yAC==-1 || zAC==-1 || xPC==-1 || yPC==-1 || zPC==-1 || xIH==-1 || yIH==-1 || zIH==-1 )) ||
	     lhemname==NULL || rhemname==NULL)
	    {
	      VipPrintfError("Missing Arg to compute Talairach transform");
	      fprintf(stderr,"I need 3 points = 9 coordinates and two images (2 hemi)\n");
	      return(Usage());
	    }
	  if (VipTestImageFileExist(lhemname)==PB)
	    {
	      (void)fprintf(stderr,"Can not open this image: %s\n",lhemname);
	      return(VIP_CL_ERROR);
	    }
	  if (VipTestImageFileExist(rhemname)==PB)
	    {
	      (void)fprintf(stderr,"Can not open this image: %s\n",rhemname);
	      return(VIP_CL_ERROR);
	    }
	  
	  printf("--------------------------------\n");
	  printf("Computing Talairach Transform...\n");
	  printf("--------------------------------\n");
	  printf("Reading left hemisphere %s...\n",lhemname);
	  if (readlib == TIVOLI)
	    lhem = VipReadTivoliVolumeWithBorder(lhemname,1);
	  else
	    lhem = VipReadVolumeWithBorder(lhemname,1);	  
	  if(lhem==NULL) return(VIP_CL_ERROR);
	  printf("Reading right hemisphere %s...\n",rhemname);
	  if (readlib == TIVOLI)
	    rhem = VipReadTivoliVolumeWithBorder(rhemname,1);
	  else
	    rhem = VipReadVolumeWithBorder(rhemname,1);	  
	  if(rhem==NULL) return(VIP_CL_ERROR);
	  printf("----------------------------\n");
	  printf("Merging both hemispheres to compute Talairach bounding box\n");
	  if(VipMerge(lhem,rhem,VIP_MERGE_SAME_VALUES,0,0)==PB) return(VIP_CL_ERROR);
	  twohem = lhem;
	  VipFreeVolume(rhem);
	  
	  if(strcmp(point_filename,""))
	    {
	      if(GetCommissureCoordinates(lhem, point_filename, &tal,
					  xAC, yAC, zAC, 
					  xPC, yPC, zPC, 
					  xIH, yIH, zIH, talset)==PB)
		{
		  return(VIP_CL_ERROR);
		}
	    }
	  else
	    {
	      tal.AC.x = xAC;
	      tal.AC.y = yAC;
	      tal.AC.z = zAC;
	      tal.PC.x = xPC;
	      tal.PC.y = yPC;
	      tal.PC.z = zPC;
	      tal.Hemi.x = xIH;
	      tal.Hemi.y = yIH;
	      tal.Hemi.z = zIH;
	    }
	  printf("Processing Talairach transformation...\n");
	  VipComputeTalairachTransformation(twohem, &tal);
	  VipFreeVolume(twohem);
      }
 
  printf("Reading skeleton %s...\n",input);
  if (readlib == TIVOLI)
      skeleton = VipReadTivoliVolumeWithBorder(input,1);
  else
      skeleton = VipReadVolumeWithBorder(input,1);
	  
  if(skeleton==NULL) return(VIP_CL_ERROR);

  printf("Reading fold arg %s...\n",argname);
  arg = VipReadOldFoldArg(argname);
  if(arg==PB) return(VIP_CL_ERROR);

  if(Talairach=='y'&& triangulation!='o')
      {
	  arg->CA.x = tal.AC.x;
	  arg->CA.y = tal.AC.y;
	  arg->CA.z = tal.AC.z;
	  arg->CP.x = tal.PC.x;
	  arg->CP.y = tal.PC.y;
	  arg->CP.z = tal.PC.z;
	  arg->IH.x = tal.Hemi.x;
	  arg->IH.y = tal.Hemi.y;
	  arg->IH.z = tal.Hemi.z;
	  arg->CAfilled = VTRUE;
	  arg->CPfilled = VTRUE;
	  arg->IHfilled = VTRUE;
	  arg->scaleref.x = tal.Scale.x;
	  arg->scaleref.y = tal.Scale.y;
	  arg->scaleref.z = tal.Scale.z;
	  arg->gotoref.r.xx = tal.Rotation[0][0];
	  arg->gotoref.r.xy = tal.Rotation[0][1];
	  arg->gotoref.r.xz = tal.Rotation[0][2];
	  arg->gotoref.r.yx = tal.Rotation[1][0];
	  arg->gotoref.r.yy = tal.Rotation[1][1];
	  arg->gotoref.r.yz = tal.Rotation[1][2];
	  arg->gotoref.r.zx = tal.Rotation[2][0];
	  arg->gotoref.r.zy = tal.Rotation[2][1];
	  arg->gotoref.r.zz = tal.Rotation[2][2];
	  arg->gotoref.t.x = tal.Translation[0];
	  arg->gotoref.t.y = tal.Translation[1];
	  arg->gotoref.t.z = tal.Translation[2];
	  arg->gotoref_filled = VTRUE;
      }

  if(Talairach=='y'&& triangulation!='o')
      {
	  if(VipComputeFoldArgSemanticAttributes(skeleton,arg,
						 linside,loutside,&tal)==PB) return(VIP_CL_ERROR);
      }
  else if(triangulation!='o')
      {
	  if(VipComputeFoldArgSemanticAttributes(skeleton,arg,
						 linside,loutside,NULL)==PB) return(VIP_CL_ERROR);
      }
  if(triflag==VTRUE)
	  {
	      VipComputeTmtkTriangulation( skeleton, arg);
	  }
  	

  if(write=='o')
      VipWriteOldFoldArg( arg, output);
  else if(write=='g')
      VipWriteGenericFoldArg( arg, output);
  else
      VipPrintfError("Write format error"); 

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipFoldArgAtt\n");
  (void)fprintf(stderr,"        -i[nput] {skeleton name}\n");
  (void)fprintf(stderr,"        -a[rg] {fold arg name}\n");
  (void)fprintf(stderr,"        [-o[utput] {arg name (default:\"input arg name\")}]\n");
  (void)fprintf(stderr,"        [-t[riangulation] {char:y/n/o (default:n)}]\n");
  (void)fprintf(stderr,"        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)fprintf(stderr,"        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)fprintf(stderr,"        [-T[alairach] {y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-lh[emi] {left hemisphere image name}]\n");
  (void)fprintf(stderr,"        [-rh[emi] {right hemisphere image name}]\n");
  (void)fprintf(stderr,"        [-P[oints] {AC,PC,IH coord filename (*.APC) (default:not used)}]\n");
  (void)fprintf(stderr,"        [-xA[C] {(int [0-xsize[) AC X coordinate}]\n");
  (void)fprintf(stderr,"        [-yA[C] {(int [0-ysize[) AC Y coordinate}]\n");
  (void)fprintf(stderr,"        [-zA[C] {(int [0-zsize[) AC Z coordinate}]\n");
  (void)fprintf(stderr,"        [-xP[C] {(int [0-xsize[) PC X coordinate}]\n");
  (void)fprintf(stderr,"        [-yP[C] {(int [0-ysize[) PC Y coordinate}]\n");
  (void)fprintf(stderr,"        [-zP[C] {(int [0-zsize[) PC Z coordinate}]\n");
  (void)fprintf(stderr,"        [-xI[nterHem] {(int [0-xsize[) Inter-hemis. point X coord.}]\n");
  (void)fprintf(stderr,"        [-yI[nterHem] {(int [0-ysize[) Inter-hemis. point Y coord.}]\n");
  (void)fprintf(stderr,"        [-zI[nterHem] {(int [0-zsize[) Inter-hemis. point Z coord.}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: o or g (default:o)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Computes various Fold Arg semantic attributes (depth,normal...)");
  (void)printf("\n");
  (void)printf("Usage: VipFoldArgAtt\n");
  (void)printf("        -i[nput] {skeleton name}\n");
  (void)printf("        -a[rg] {fold arg name}\n");
  (void)printf("        [-o[utput] {arg name (default:\"input arg name\")}]\n");
  (void)printf("        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)printf("        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)printf("        [-T[alairach] {y/n (default:y)}]\n");
  (void)printf("        [-t[riangulation] {char:yes/no/only (default:n)}]\n");
  (void)printf("        [-lh[emi] {left hemisphere image name}]\n");
  (void)printf("        [-rh[emi] {right hemisphere image name}]\n");
  (void)printf("        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)printf("Correct format for the commissure coordinate file toto.APC:\n");
  (void)printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");  
  (void)printf("        [-xA[C] {(int [0-xsize[) AC X coordinate}]\n");
  (void)printf("        [-yA[C] {(int [0-ysize[) AC Y coordinate}]\n");
  (void)printf("        [-zA[C] {(int [0-zsize[) AC Z coordinate}]\n");
  (void)printf("        [-xP[C] {(int [0-xsize[) PC X coordinate}]\n");
  (void)printf("        [-yP[C] {(int [0-ysize[) PC Y coordinate}]\n");
  (void)printf("        [-zP[C] {(int [0-zsize[) PC Z coordinate}]\n");
  (void)printf("        [-xI[nterHem] {(int [0-xsize[) Inter-hemis. point X coord.}]\n");
  (void)printf("        [-yI[nterHem] {(int [0-ysize[) Inter-hemis. point Y coord.}]\n");
  (void)printf("        [-zI[nterHem] {(int [0-zsize[) Inter-hemis. point Z coord.}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-w[riteformat] {char: o or g (default:o)}]\n");
  (void)printf("o: old Arg format\n");
  (void)printf("g: Generic Arg format (C++ lib)\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
