/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipMatching.c        * TYPE     : Command line
 * AUTHOR      : MANGIN J.-F.         * CREATION : 
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : 
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            03/01/99| POUPON F.    | Passage en ANSI pour VIP
 *--------------------|--------------|----------------------------------------
 *            13/12/99| POUPON F.    | Utilisation de la fonction VipMatching
 *--------------------|--------------|----------------------------------------
 *                    |              |  
 *****************************************************************************/

#include <string.h>

#include <vip/matching.h>

static void Usage()
{
  (void)fprintf(stderr,"Usage: VipMatching\n");
  (void)fprintf(stderr,"        -r[efsurf] {image name}\n");
  (void)fprintf(stderr,"        -m[obilesurf] {image name}\n");
  (void)fprintf(stderr,"        [-nr[efsurf] {reference image name for copy of deplacement file}]\n");
  (void)fprintf(stderr,"        [-nm[obilesurf] {mobile image name for copy of deplacement file}]\n");
  (void)fprintf(stderr,"        [-gf[ile]] {image name} (default:same as mobile surface)\n");
  (void)fprintf(stderr,"        [-gd[im]] <2/3> (default:2)\n");
  (void)fprintf(stderr,"        [-dil[ation]] char: y/n (default:\"y\")\n");
  (void)fprintf(stderr,"        [-dis[tmap] {image name (default:\"not used\")}]\n");
  (void)fprintf(stderr,"        [-i[nit]] <dep file> (default:\"not used\")\n");
  (void)fprintf(stderr,"        [-dir[ecttransfo]] {file name (default:\"<mobsurf>_TO_<refsurf>\")}\n");
  (void)fprintf(stderr,"        [-inv[ersetransfo]] {file name (default:\"<refsurf>_TO_<mobsurf>\")}\n");
  (void)fprintf(stderr,"        [-u[ndersampling]] <int> ([1..100] (default:100)\n"); 
  (void)fprintf(stderr,"        [-i1[angle]] <float> (Minimization 1, init angle step(default:5.deg)\n");  
  (void)fprintf(stderr,"        [-f1[angle]] <float> (Minimization 1, final angle step(default:0.5deg)\n");  
  (void)fprintf(stderr,"        [-i2[angle]] <float> (Minimization 2, init angle step(default:1.deg)\n");  
  (void)fprintf(stderr,"        [-f2[angle]] <float> (Minimization 2, final angle step(default:0.2deg)\n");
  (void)fprintf(stderr,"        [-i3[angle]] <float> (Minimization 3, init angle step(default:1.deg)\n");  
  (void)fprintf(stderr,"        [-f3[angle]] <float> (Minimization 3, final angle step(default:0.05deg)\n");
  (void)fprintf(stderr,"        [-t[hreshold]] <float> (outlier) (default:1.7 * mean dist)\n");
  (void)fprintf(stderr,"        [-x[masksize]] <int> ((default:5)\n");
  (void)fprintf(stderr,"        [-y[masksize]] <int> ((default:5)\n"); 
  (void)fprintf(stderr,"        [-z[masksize]] <int> ((default:3)\n");  
  (void)fprintf(stderr,"        [-2[D]] <y/n> (default:\"n\")\n");  
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  exit(-1);
}

static void Help()
{
  (void)printf("Surface matching tool\n");
  (void)printf("(the program yields several 3D transformations)\n");
  (void)printf("      - <refsurf>_TO_<mobsurf>: alignment of the reference image along the mobile one\n");
  (void)printf("      - <mobsurf>_TO_<refsurf>: alignment of the mobile image along the reference one\n\n");
  (void)printf("Usage: VipMatching\n");
  (void)printf("        -r[efsurf] {image name}\n");
  (void)printf("The reference edge image from which is computed the distance map\n");
  (void)printf("        -m[obsurf] {image name}\n");
  (void)printf("        [-nr[efsurf] {reference image name for deplacement file}]\n");
  (void)printf("        [-nm[obilesurf] {mobile image name for deplacement file}]\n");
  (void)printf("The mobile surface which is moved through the distance map\n");
  (void)printf("        [-gf[ile]] {image name} (default:same as mobile surface)\n");
  (void)printf("The mobile surface gradient computed from VipDeriche\n");
  (void)printf("        [-gd[im]] <2/3> (default:2)\n");
  (void)printf("The gradient dimension (2D or 3D) resulting from VipDeriche\n");
  (void)printf("        [-dil[ation]] char: y/n (default:\"y\")\n");
  (void)printf("y : The mobile surface will be inflated to correct for edge localization differences\n");
  (void)printf("        [-dis[tmap] {image name (default:\"not used\")}]\n");
  (void)printf("A distance map computed from the reference surface\n");
  (void)printf("        [-i[nit]] <deplacement file>\n");
  (void)printf("This deplacement (rotation affine) initialized the minimization\n"); 
  (void)printf("        [-dir[ecttransfo]] {file name (default:\"<mobsurf>_TO_<refsurf>\")}\n");
  (void)printf("        [-inv[ersetransfo]] {file name (default:\"<refsurf>_TO_<mobsurf>\")}\n");
  (void)printf("        [-u[ndersampling]] <int> ([1..100] (default:100)\n"); 
  (void)printf("Undersamples (percentage) the mobile surface which speeds up the process\n"); 
  (void)printf("Angle steps:\n"); 
  (void)printf(" First Minimization:\n"); 
  (void)printf("        [-i1[angle]] <float> (Minimization 1, init angle step(default:5.deg)\n");  
  (void)printf("        [-f1[angle]] <float> (Minimization 1, final angle step(default:0.5deg)\n");  
  (void)printf(" Second Minimization (including optimal dilation):\n"); 
  (void)printf("        [-i2[angle]] <float> (Minimization 2, init angle step(default:1.deg)\n");  
  (void)printf("        [-f2[angle]] <float> (Minimization 2, final angle step(default:0.2deg)\n");
  (void)printf(" Third Minimization (including optimal dilation and outlier elimination):\n");  
  (void)printf("        [-i3[angle]] <float> (Minimization 3, init angle step(default:1.deg)\n");  
  (void)printf("        [-f3[angle]] <float> (Minimization 3, final angle step(default:0.05deg)\n");
  (void)printf("        [-t[hreshold]] <float> (outlier) (default:1.7 * mean dist)\n");
  (void)printf(" Chamfer mask sizes:\n"); 
  (void)printf("        [-x[masksize]] <int> ((default:5)\n");
  (void)printf("        [-y[masksize]] <int> ((default:5)\n"); 
  (void)printf("        [-z[masksize]] <int> ((default:3)\n");  
  (void)printf("        [-2[D]] <y/n> (default:\"n\")\n");
  (void)printf("Only translations and rotation around Z axis are allowed\n");   
  (void)printf("        [-R[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  /* Noms des images */
  char *mri=NULL, *trans=NULL;
  char *nmri=NULL, *ntrans=NULL;
  char *dep_name=NULL, *gradFile=NULL;
  char *trans2mri_file_name=NULL;
  char *mri2trans_file_name=NULL;

  int readlib=ANY_FORMAT;
  int rotation_2D=VFALSE;
  int i;
  int gradDim=GRAD_2D;
  VIP_DEC_VOLUME(volmri);
  VIP_DEC_VOLUME(voltrans);
  VipDeplacement *dep=NULL, *initDep=NULL;
  float angle;
  float threshold=1.7;
  VipDeplacement invdep;
  VIP_DEC_VOLUME(first_dmap);
  int dep_init=VFALSE;
  char *dmap=NULL;
  int dmap_activ = VFALSE;
  int undersampling = 100;
  float M1_init_angle=5.0, M1_final_angle=0.5;
  float M2_init_angle=1.0, M2_final_angle=0.2;
  float M3_init_angle=1.0, M3_final_angle=0.05;
  char dodilation = 'y';
  int dodilat = VTRUE;
  int x_mask_size = X_MASK_SIZE;
  int y_mask_size = Y_MASK_SIZE;
  int z_mask_size = Z_MASK_SIZE;
  int dirTransf=VFALSE;

  int invTransf=VFALSE;
 
  /* Check usage */
  if (argc == 1) Usage();

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-nrefsurf", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  nmri = argv[i];
	}
      else if (!strncmp (argv[i], "-nmobsurf", 3))
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  ntrans = argv[i];
	}	
      else if (!strncmp (argv[i], "-refsurf", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  mri = argv[i];
	}
      else if (!strncmp (argv[i], "-mobsurf", 2))
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  trans = argv[i];
	}	
      else if (!strncmp (argv[i], "-gfile", 3))
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  gradFile = argv[i];
	}	
      else if (!strncmp (argv[i], "-gdim", 3))
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  gradDim = atoi(argv[i]);
	  if ((gradDim != GRAD_2D) && (gradDim != GRAD_3D)) 
	    gradDim = GRAD_2D;
	}	
      else if (!strncmp (argv[i], "-dilation", 4))
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  dodilation = argv[i][0];
	}	
      else if (!strncmp (argv[i], "-distmap", 4))
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  dmap = argv[i];
	  dmap_activ = VTRUE;
	}	
      else if (!strncmp (argv[i], "-2D", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  if(argv[i][0]=='y') rotation_2D = VTRUE;
	  else if (argv[i][0]=='n') rotation_2D = VFALSE;
	  else
	    {
	      VipPrintfError("Bad mod for 2D flag");
	      Help();
	    }
	}	
      else if (!strncmp (argv[i], "-threshold", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  threshold = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-i1angle", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  M1_init_angle = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-f1angle", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  M1_final_angle = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-i2angle", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  M2_init_angle = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-f2angle", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  M2_final_angle = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-i3angle", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  M3_init_angle = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-f3angle", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  M3_final_angle = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-undersampling", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  undersampling = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-xmasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  x_mask_size = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-ymasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  y_mask_size = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zmasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  z_mask_size = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-init", 5)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  dep_name = argv[i];
	  dep_init = VTRUE;
	}		
      else if (!strncmp (argv[i], "-directtransfo", 4)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  trans2mri_file_name = argv[i];
	  dirTransf = VTRUE;
	}		
      else if (!strncmp (argv[i], "-inversetransfo", 4)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  mri2trans_file_name = argv[i];
	  invTransf = VTRUE;
	}		
      else if (!strncmp (argv[i], "-Readformat", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
          if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='a') readlib = ANY_FORMAT;
          else if(argv[i][0]=='v') readlib = VIDA;
          else if(argv[i][0]=='s') readlib = SPM;
          else
            {
              VipPrintfError("This format is not implemented for reading\n");
              Usage();
            }
        }
      else if (!strncmp(argv[i], "-help",2)) Help();
      else Usage();
    }

  if((mri==NULL) || (trans==NULL))
    {
      VipPrintfError(" Surfaces are required by VipMatching");
      Usage();
    }

  if (VipTestImageFileExist(mri)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",mri);
      return(PB);
    }

  if (VipTestImageFileExist(trans)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",trans);
      return(PB);
    }

  if (gradFile == NULL) gradFile = trans;

  if((M1_final_angle<=0)||(M2_final_angle<=0))
    {
      VipPrintfError("bad final angle step");
      Usage();
    }

  if (undersampling<1 || undersampling>100)
    {
      VipPrintfError("Bad undersampling value, range: [1..100]");
      Help();
    }

  if(threshold<=0)
    {
      VipPrintfError("Bad threshold");
      Help();
    }

  if (nmri==NULL) nmri = mri;
  if (ntrans==NULL) ntrans = trans;

  /* surface de reference */
  printf("Reading reference surface...\n");
  if (readlib == TIVOLI)
    volmri = VipReadTivoliVolumeWithBorder( mri, 1 );
  else if (readlib == SPM)
    volmri = VipReadSPMVolumeWithBorder( mri, 1 );
  else
    volmri = VipReadVolumeWithBorder( mri, 1 );

  if (!volmri)
    {
      VipPrintfError("Error while reading reference surface");
      VipPrintfExit("(commandline)VipReadVolume");
      exit(EXIT_FAILURE);
    }

  /* surface mobile */
  printf("Reading mobile surface...\n");
  if (readlib == TIVOLI)
    voltrans = VipReadTivoliVolumeWithBorder( trans, 1 );
  else if (readlib == SPM)
    voltrans = VipReadSPMVolumeWithBorder( trans, 1 );
  else
    voltrans = VipReadVolumeWithBorder( trans, 1 );

  if (!voltrans)
    {
      VipPrintfError("Error while reading mobile surface");
      VipPrintfExit("(commandline)VipReadVolume");
      if (volmri) VipFreeVolume(volmri);
      exit(EXIT_FAILURE);
    }

  /*centres de gravite*/
  printf("Computing initial optimal translation...\n");
  if (dep_init==VTRUE)
    {
      fprintf(stderr,"Using initialization file\n");
      initDep = VipReadDeplacement( dep_name );
      if (!initDep)
	{
	  VipPrintfError("Error while reading deplacement file\n");
	  VipPrintfExit("(commandline)VipReadDeplacement");
	  if (volmri) VipFreeVolume(volmri);
	  if (voltrans) VipFreeVolume(voltrans);
	  exit(EXIT_FAILURE);
	}
    }

  /* carte de distance */
  if (dmap_activ==VTRUE)
    {
      printf("Reading distmap...\n");
      if (readlib == TIVOLI)
	first_dmap = VipReadTivoliVolumeWithBorder(dmap, 1);
      else if (readlib == SPM)
	first_dmap = VipReadSPMVolumeWithBorder(dmap, 1);
      else
	first_dmap = VipReadVolumeWithBorder(dmap, 1);
      if (!first_dmap)
	{
	  VipPrintfError("Error while reading distance map file");
	  VipPrintfExit("(commandline)VipReadVolume");
	  if (volmri) VipFreeVolume(volmri);
	  if (voltrans) VipFreeVolume(voltrans);
	  if (initDep) VipFree(initDep);
	  exit(EXIT_FAILURE);
	}
    }

  if ( dodilation == 'y' ) dodilat = VTRUE;
  else dodilat = VFALSE;

  /* recalage */
  dep = VipMatching( volmri, voltrans, first_dmap, gradFile, gradDim, 
                     undersampling, initDep, x_mask_size, y_mask_size, 
                     z_mask_size, dodilat, rotation_2D, threshold,
                     M1_init_angle, M1_final_angle, M2_init_angle, 
                     M2_final_angle, M3_init_angle, M3_final_angle );
  if (!dep)
    {
      VipPrintfError("Error while processing registration");
      VipPrintfExit("(commandline)VipMatching");
      if (first_dmap) VipFreeVolume(first_dmap);
      if (initDep) VipFree(initDep);
      exit(EXIT_FAILURE);
    }

  /* deplacement inverse */
  if ((VipInverseDeplacement( dep, &invdep)) == PB)
    {
      VipPrintfError("Error while inversing deplacement");
      VipPrintfExit("(commandline)VipInverseDeplacement");
      if (first_dmap) VipFreeVolume(first_dmap);
      if (initDep) VipFree(initDep);
      if (dep) VipFree(dep);
      exit(EXIT_FAILURE);
    }

  angle = VipGetRotationAngle(dep);
  printf("Rotation angle of the optimal transformation: %.2f deg\n", angle);

  /* sauvegarde des resultats */
  if (dirTransf == VFALSE)
    {
      trans2mri_file_name = VipGetTrans2MriFileName( mri, trans);
      if (!trans2mri_file_name)
	{
	  VipPrintfError("Not enough memory for file name allocation");
	  VipPrintfExit("(commandline)VipGetTrans2MriFileName");
	  if (first_dmap) VipFreeVolume(first_dmap);
          if (initDep) VipFree(initDep);
	  if (dep) VipFree(dep);
	  exit(EXIT_FAILURE);
	}
    }

  if (invTransf == VFALSE)
    {
      mri2trans_file_name = VipGetMri2TransFileName( mri, trans);
      if (!mri2trans_file_name)
	{
	  VipPrintfError("Not enough memory for file name allocation");
	  VipPrintfExit("(commandline)VipGetMri2TransFileName");
	  if (first_dmap) VipFreeVolume(first_dmap);
          if (initDep) VipFree(initDep);
	  if (dep) VipFree(dep);
	  exit(EXIT_FAILURE);
	}
    }

  if ((VipWriteDeplacement(dep, trans2mri_file_name)) == PB)
    {
      VipPrintfError("Error while writting direct transformation file");
      VipPrintfExit("(commandline)VipWriteDeplacement");
      if (first_dmap) VipFreeVolume(first_dmap);
      if (initDep) VipFree(initDep);
      if (dep) VipFree(dep);
      exit(EXIT_FAILURE);
    }

  if ((VipWriteDeplacement(&invdep, mri2trans_file_name)) == PB)
    {
      VipPrintfError("Error while writting inverse transformation file");
      VipPrintfExit("(commandline)VipWriteDeplacement");
      if (first_dmap) VipFreeVolume(first_dmap);
      if (initDep) VipFree(initDep);
      if (dep) VipFree(dep);
      exit(EXIT_FAILURE);
    }

  if (nmri!=mri || ntrans!=trans)
    {
      trans2mri_file_name = VipGetTrans2MriFileName( nmri, ntrans);
      mri2trans_file_name = VipGetMri2TransFileName( nmri, ntrans);
      VipWriteDeplacement(dep, trans2mri_file_name);
      VipWriteDeplacement(&invdep, mri2trans_file_name);
    }
/*
  VipFree(trans2mri_file_name);
  VipFree(mri2trans_file_name);
*/
  VipFree(volmri);
  VipFree(voltrans);

  if (first_dmap) VipFreeVolume(first_dmap);
  if (initDep) VipFree(initDep);
  VipFree(dep);

  return 0;
}
