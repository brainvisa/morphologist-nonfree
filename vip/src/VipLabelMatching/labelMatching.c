/***************************************************************************
 * PROJECT     : 
 * MODULE NAME : VipLabelMatching.c   * TYPE     : Function
 * AUTHOR      : POUPON F.            * CREATION : 11/01/2000
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
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
#include <vip/deriche.h>
#include <vip/matching.h>
#include <vip/splineresamp.h>

#include <stdlib.h>
#include <string.h>

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipLabelMatching\n");
  (void)fprintf(stderr,"\t\t-r[eference] {reference image name}\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-a[ctivation] {reference statistical map name (default:not used)}]\n");
  (void)fprintf(stderr,"\t\t[-f[unctionnal] {statistical map name (default:not used)}]\n");
  (void)fprintf(stderr,"\t\t[-l[abel] {label to match (default:use all label found)}]\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"resampled\")}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ 
  VipPrintfInfo("Matching from labelled MRI and statistical map image resampling");
  (void)printf("\n");
  (void)printf("Usage: VipLabelMatching\n");
  (void)printf("\t\t-r[eference] {reference image name}\n");
  (void)printf("\t\t-i[nput] {image name}\n");
  (void)printf("\t\t[-a[ctivation] {reference statistical map name (default:not used)}]\n");
  (void)printf("\t\t[-f[unctionnal] {statistical map name (default:not used)}]\n");
  (void)printf("\t\t[-l[abel] {label to match (default:use all label found)}]\n");
  (void)printf("\t\t[-o[utput] {image name (default:\"resampled\")}]\n");
  (void)printf("\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("\t\t\tv=VIDA, s=SPM, t=TIVOLI\n");
  (void)printf("\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(copyVol); 
  VIP_DEC_VOLUME(vtmp); 
  VIP_DEC_VOLUME(refer);
  VIP_DEC_VOLUME(copyRef); 
  VIP_DEC_VOLUME(funct);
  VIP_DEC_VOLUME(funcRef);
  VIP_DEC_VOLUME(copyFunc);
  VIP_DEC_VOLUME(resamp1);
  VIP_DEC_VOLUME(resamp2);
  VIP_DEC_VOLUME(resana);
  VIP_DEC_VOLUME(resfunc);
  char output[VIP_NAME_MAXLEN]="resampled_anat\0";
  char output2[VIP_NAME_MAXLEN]="resampled_func\0";
  char frefname[VIP_NAME_MAXLEN];
  char dirfl[VIP_NAME_MAXLEN];
  char invfl[VIP_NAME_MAXLEN];
  char racine[VIP_NAME_MAXLEN];
  char *input=NULL;
  char *ref=NULL;
  char *func=NULL;
  char *fref=NULL;
  char *dirf=NULL, *invf=NULL;
  int i, writelib=VIDA; 
  VipDeplacement *dep, invdep, tmpl;
  int lablst[256], nbLab=0, lab2[256], nbLab2=0, isok=0;
  int x, y, z, dx, dy, dz, dx2, dy2, dz2, k;
  float sx, sy, sz, sx2, sy2, sz2;
  long offfp, offlbs, offpbl, ofp, olbs, opbl;
  Vip_S16BIT *imptr, *outptr;
  int tLab=-1;

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
    { 
      if (!strncmp(argv[i], "-input", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          input = argv[i];
        }
      else if (!strncmp(argv[i], "-activation", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          fref = argv[i];
          strcpy( frefname, argv[i] );
          strcat( frefname, "_resamp\0" );
        }
      else if (!strncmp(argv[i], "-reference", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          ref = argv[i];
        }
      else if (!strncmp(argv[i], "-functionnal", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          func = argv[i];
        }
      else if (!strncmp(argv[i], "-label", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          tLab = atoi( argv[i] );
        }
      else if (!strncmp(argv[i], "-output", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          strcpy( output, argv[i] );
          strcat( output, "_anat\0" );
          strcpy( output2, argv[i] );
          strcat( output2, "_func\0" );
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
              VipPrintfExit("(commandline)VipLabelMatching");
              Usage();
            }
        }
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if (input == NULL)
    { 
      VipPrintfError("input arg is required by VipLabelMatching");
      Usage();
    }

  if (ref == NULL)
    { 
      VipPrintfError("reference arg is required by VipLabelMatching");
      Usage();
    }

  tmpl.t.x = tmpl.t.y = tmpl.t.z = 0.0;
  tmpl.r.xx = tmpl.r.yy = tmpl.r.zz = 1.0;
  tmpl.r.xy = tmpl.r.xz = tmpl.r.yx = 0.0;
  tmpl.r.yz = tmpl.r.zx = tmpl.r.zy = 0.0;

  /***** Lecture des images *****/
  printf("Reading reference : %s ...\n", ref);
  refer = VipReadVolume( ref );

  if (fref != NULL)
    {
      printf("Reading reference statistical map : %s ...\n", fref);
      funcRef = VipReadVolume( fref );
    }

  printf("Reading image : %s ...\n", input);
  vol = VipReadVolume( input );

  if (func != NULL)
    {
      printf("Reading functionnal image : %s ...\n", func);
      resamp2 = VipReadVolume( func );
    }

  strcpy( racine, input );
  if ( strrchr( racine, '/' ) )  *(strrchr( racine, '/' ) + 1) = '\0';
  else strcpy( racine, "./\0");

  /***** Recherche des etiquettes *****/
  offfp = VipOffsetFirstPoint( refer );
  offlbs = VipOffsetLineBetweenSlice( refer );
  offpbl = VipOffsetPointBetweenLine( refer );
  VipGet3DSize( refer, &dx, &dy, &dz );
  VipGet3DVoxelSize( refer, &sx, &sy, &sz );
  imptr = VipGetDataPtr_S16BIT( refer ) + offfp;

  printf( "Looking for labels in reference image...\n" );
  for ( z=dz; z--; imptr+=offlbs )
    for ( y=dy; y--; imptr+=offpbl )
      for ( x=dx; x--; imptr++ )
        if ( *imptr )
          {
            k = 0;
            while( k<nbLab && lablst[k]!=(int)*imptr )  k++;
            if ( k == nbLab )  lablst[nbLab++] = *imptr;
          }

  printf( "Labels found (reference image) :\n\t" );
  for ( k=0; k<nbLab; k++ ) 
    printf( "%d, ", lablst[k] );
  printf("\n");

  /***** Recherche des etiquettes de l'image a traiter *****/
  ofp = VipOffsetFirstPoint( vol );
  olbs = VipOffsetLineBetweenSlice( vol );
  opbl = VipOffsetPointBetweenLine( vol );
  VipGet3DSize( vol, &dx2, &dy2, &dz2 );
  VipGet3DVoxelSize( vol, &sx2, &sy2, &sz2 );
  imptr = VipGetDataPtr_S16BIT( vol ) + ofp;

  printf( "Looking for labels...\n" );
  for ( z=dz2; z--; imptr+=olbs )
    for ( y=dy2; y--; imptr+=opbl )
      for ( x=dx2; x--; imptr++ )
        if ( *imptr )
          {
            k = 0;
            while( k<nbLab2 && lab2[k]!=(int)*imptr )  k++;
            if ( k == nbLab2 )  lab2[nbLab2++] = *imptr;
          }

  printf( "Labels found :\n\t" );
  for ( k=0; k<nbLab2; k++ ) 
    printf( "%d, ", lab2[k] );
  printf("\n");

  /***** Verifie si toutes les etiquettes sont presentes *****/
  if ( tLab == -1 )
    {
      for ( x=0; x<nbLab; x++ )
        {
          k = 0;
          while( k<nbLab2 && lab2[k]!=lablst[x] )  k++;
          if ( k != nbLab2 ) isok++;
        }

      if ( isok != nbLab )
        {
          VipPrintfError("Some label are missing in the image to process.");
          exit(EXIT_FAILURE);
        }
    }
  else
    {
      k = 0;
      while( k<nbLab && lablst[k]!=tLab )  k++;
      if ( k != nbLab )  isok++;

      k = 0;
      while( k<nbLab2 && lab2[k]!=tLab )  k++;
      if ( k != nbLab2 )  isok++;

      if ( isok != 2 )
        {
          VipPrintfError("Label not found in both images..." );
          exit(EXIT_FAILURE);
        }

      nbLab = 1;
      lablst[0] = tLab;
    }

  printf("Ok, all labels found...\n");

  /***** Reechantillonnage de la carte statistique de reference *****/
  if (fref != NULL)
    {
      printf( "Resampling reference statistical map...\n" );
      resamp1 = VipSplineResamp( funcRef, refer, &tmpl, 1 );
      VipFreeVolume(funcRef);
      VipFree(funcRef);

      printf( "Writing resampled volume : %s...\n", frefname );
      switch(writelib)
	{
	case VIDA:
	  {
            if ( VipTestType(resamp1, S16BIT) == PB )
              {
                funcRef = VipCopyVolume( resamp1, "" );
                VipFreeVolume(resamp1);
                VipFree(resamp1);
                resamp1 = VipTypeConversionToS16BIT( funcRef, 
                                                     RAW_TYPE_CONVERSION );
                VipFreeVolume(funcRef);
                VipFree(funcRef);
              }
	    VipWriteVolume(resamp1, frefname);
	    break;
	  }
	case TIVOLI:
	  {
	    VipWriteTivoliVolume(resamp1, frefname);
	    break;
	  }
	case SPM:
	  {
	    VipWriteSPMVolume(resamp1, frefname);
	    break;
	  }
	}

      VipFreeVolume(resamp1);
      VipFree(resamp1);
    }

  if (func != NULL)
    {
      printf( "Resampling statistical map...\n" );
      tmpl.t.x = tmpl.t.y = tmpl.t.z = 0.0;
      tmpl.r.xx = tmpl.r.yy = tmpl.r.zz = 1.0;
      tmpl.r.xy = tmpl.r.xz = tmpl.r.yx = 0.0;
      tmpl.r.yz = tmpl.r.zx = tmpl.r.zy = 0.0;
      funct = VipSplineResamp( resamp2, vol, &tmpl, 1 );

      VipFreeVolume(resamp2);
      VipFree(resamp2);
    }

  /***** Traitements *****/
  copyRef = VipCopyVolume( refer, "" );
  copyVol = VipCopyVolume( vol, "" );
  resana = VipCreate3DVolume( dx, dy, dz, sx, sy, sz, S16BIT, "", 0 );

  if (func != NULL) 
    { 
      copyFunc = VipCopyVolume( funct, "" );
      resfunc = VipCreate3DVolume( dx, dy, dz, sx, sy, sz, S16BIT, "", 0 );
    }

  for ( k=0; k<nbLab; k++ )
    {
      printf( "Processing label %d:\n", lablst[k] );

      printf( "\tThresholding...\n" );
      VipSingleThreshold( refer, EQUAL_TO, lablst[k], GREYLEVEL_RESULT );
      VipSingleThreshold( vol, EQUAL_TO, lablst[k], GREYLEVEL_RESULT );

      vtmp = VipCreate3DVolume( dx2, dy2, dz2, sx2, sy2, sz2, S16BIT, "", 0 );
      imptr = VipGetDataPtr_S16BIT( vol ) + ofp;
      outptr = VipGetDataPtr_S16BIT( vtmp ) + ofp;
      for ( z=dz2; z--; imptr+=olbs )
	for ( y=dy2; y--; imptr+=opbl )
	  for ( x=dx2; x--; )
	    *outptr++ = *imptr++;

      if (func != NULL)
	{
	  printf( "\tMasking statistical map...\n" );
	  VipMaskVolume( funct, vol );
	}

      printf( "\tExternal edge extraction...\n" );
      VipExtedge( refer, EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM, SAME_VOLUME );
      VipExtedge( vol, EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM, SAME_VOLUME );

      printf("\tMatching...\n");
      dep = VipMatching( refer, vol, NULL, NULL, GRAD_3D, 100, NULL, 
                         X_MASK_SIZE, Y_MASK_SIZE, Z_MASK_SIZE, VTRUE, VFALSE, 
                         1.7, 5.0, 0.5, 1.0, 0.2, 1.0, 0.05 );
      VipInverseDeplacement( dep, &invdep );

      printf("\tWritting deplacement files...\n");
      dirf = VipGetTrans2MriFileName( input, ref );
      sprintf( dirfl, "%s%s_%d", racine, dirf, lablst[k] );
      VipWriteDeplacement( dep, dirfl );
      invf = VipGetMri2TransFileName( input, ref );
      sprintf( invfl, "%s%s_%d", racine, invf, lablst[k] );
      VipWriteDeplacement( &invdep, invfl );

      printf("\tSpline resampling...\n");
      resamp1 = VipSplineResamp( vtmp, refer, dep, 0 );
      if (func != NULL) resamp2 = VipSplineResamp( funct, refer, dep, 1 );

      printf( "\tMerging...\n" );
      VipMerge( resana, resamp1, VIP_MERGE_SAME_VALUES, 0, 0 );
      if (func != NULL) VipMerge( resfunc, resamp2, VIP_MERGE_SAME_VALUES, 0, 0 );

      VipFreeVolume(resamp1);
      VipFree(resamp1);
      VipFreeVolume(vtmp);
      VipFree(vtmp);
      VipFree(dep);

      imptr = VipGetDataPtr_S16BIT( copyRef ) + offfp;
      outptr = VipGetDataPtr_S16BIT( refer ) + offfp;
      for ( z=dz; z--; imptr+=offlbs )
	for ( y=dy; y--; imptr+=offpbl )
	  for ( x=dx; x--; )
	    *outptr++ = *imptr++;

      imptr = VipGetDataPtr_S16BIT( copyVol ) + ofp;
      outptr = VipGetDataPtr_S16BIT( vol ) + ofp;
      for ( z=dz2; z--; imptr+=olbs )
	for ( y=dy2; y--; imptr+=opbl )
	  for ( x=dx2; x--; )
	    *outptr++ = *imptr++;

      if (func != NULL)
	{
	  VipFreeVolume(resamp2);
	  VipFree(resamp2);

	  imptr = VipGetDataPtr_S16BIT( copyFunc ) + ofp;
	  outptr = VipGetDataPtr_S16BIT( funct ) + ofp;
	  for ( z=dz2; z--; imptr+=olbs )
	    for ( y=dy2; y--; imptr+=opbl )
	      for ( x=dx2; x--; )
		*outptr++ = *imptr++;
	}
    }

  /***** sauvegarde du resultat *****/
  printf("Writing resampled volumes : %s and %s ...\n", output, output2);
  switch(writelib)
    {
    case VIDA:
      {
        VipWriteVolume(resana, output);
        if (func != NULL) VipWriteVolume(resfunc, output2);
        break;
      }
    case TIVOLI:
      {
        VipWriteTivoliVolume(resana, output);
        if (func != NULL) VipWriteTivoliVolume(resfunc, output2);
        break;
      }
    case SPM:
      {
        VipWriteSPMVolume(resana, output);
        if (func != NULL) VipWriteSPMVolume(resfunc, output2);
        break;
      }
    }

  VipFreeVolume(resana);
  VipFree(resana);
  VipFreeVolume(vol);
  VipFree(vol);
  VipFreeVolume(refer);
  VipFree(refer);
  VipFreeVolume(copyRef);
  VipFree(copyRef);
  VipFreeVolume(copyVol);
  VipFree(copyVol);
  VipFree(dirf);
  VipFree(invf);

  if (func != NULL)
    {
      VipFreeVolume(resfunc);
      VipFree(resfunc);
      VipFreeVolume(copyFunc);
      VipFree(copyFunc);
      VipFreeVolume(funct);
      VipFree(funct);
    }

  return(EXIT_SUCCESS);
}
