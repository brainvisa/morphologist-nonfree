/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipClusterArg        * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 22/10/99
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
#include <vip/volume.h>
#include <vip/util.h>
#include <vip/util/file.h>
#include <vip/connex.h>
#include <vip/distmap.h>
#include <vip/morpho.h>
#include <vip/depla.h>
#include <vip/activations.h>
#include <vip/splineresamp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <math.h>

#ifdef VIP_CARTO_VOLUME_WRAPPING
#include <vip/fold_arg/mesh_aims.h>
#endif

/*-------------------------------------------------------------------------*/
static int  VipWriteLabelArg( Volume *vol,
char *name, char mode, char triangulation);
/*-------------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "label";
  int readlib;
  char mode = 'l';
  char triangulation = 'y';

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
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  mode = argv[i][0];
	}
      else if (!strncmp (argv[i], "-triangulation", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  triangulation = argv[i][0];
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipLabelArg");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input  argument is required by VipLabelArg");
      return(Usage());
    }
  if ( VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
  if (mode!='l' && mode!='n')
    {
      (void)fprintf(stderr,"Unknown mode\n");
      return(Usage());
    }

  printf("Reading label image %s...\n",input);
  if (readlib == TIVOLI)
      vol = VipReadTivoliVolumeWithBorder(input,0);
  else
      vol = VipReadVolumeWithBorder(input,0);
  
  if(vol==NULL) return(VIP_CL_ERROR);

  VipWriteLabelArg( vol, output, mode, triangulation);


  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipLabelArg\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-m[ode] {n/l (nucleus/label) default:l}]\n");
  (void)fprintf(stderr,"        [-t[riangulation] {y/n default:y}]\n");
  (void)fprintf(stderr,"        [-o[utput] {arg name (default:\"input\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Create a nucleus label based attributed relational graph");
  (void)printf("\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-m[ode] {n/l (nucleus/label) default:l}]\n");
  (void)printf("nucleus mode supposed you use the standard label set\n");
  (void)printf("In that case, nucleus names will be given to the VOIs\n");
  (void)printf("Hence, you can use nucleus hierarchy in Anatomist\n");
  (void)printf("        [-t[riangulation] {y/n default:y}]\n");
  (void)printf("        [-o[utput] {arg name (default:\"input\")}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/

/*-------------------------------------------------------------------------*/
int  VipWriteLabelArg( Volume *vol,
char *name, char mode, char triangulation)
/*-------------------------------------------------------------------------*/
{
    char buckfilename[VIP_NAME_MAXLEN];
    FILE *fp, *buckf;
    char filename[VIP_NAME_MAXLEN];
    int nlabel;
    int i;
    int n;
    Volume *fortriang;
    char tempimagename[VIP_NAME_MAXLEN];
    char nucleusname[VIP_NAME_MAXLEN];
    char *shortname=NULL, *cptr;


    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_S16BIT *ptr;
    int label[100000];

    cptr = name;
    shortname = name;
    while(cptr!=NULL)
      {
	cptr = strstr(cptr,"/");
	if(cptr!=NULL)
	  {
	    cptr++;
	    shortname = cptr;
	  }
      }
	
    for(i=0;i<100000;i++)
      label[i] = 0;

    vos = VipGetOffsetStructure(vol);
    ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
      {
	for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	  {
	    for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	      {
		  if(*ptr) label[*ptr]++;
		  ptr++;
	      }
	    ptr += vos->oPointBetweenLine;  /*skip border points*/
	  }
	ptr += vos->oLineBetweenSlice; /*skip border lines*/
      }

    nlabel = 0;
    printf("Existing labels: ");
    for(i=0;i<256;i++) if(label[i]>0)
      {
	printf("%d ", i);
	nlabel++;
      }
    printf("\n");

    if(vol==NULL)
	{
	    VipPrintfError("No Label volume as input");
	    VipPrintfExit("VipWriteLabelArg");
	    return(PB);
	}

    if(name==NULL)
	{
	    VipPrintfError("No name for Label arg as input");
	    VipPrintfExit("VipWriteLabelArg");
	    return(PB);
	}


    printf("%d labels to process...\n",nlabel);

    printf("------------------------------------\n");
    printf("Writing C++ generic library graph format\n");
    printf("------------------------------------\n");

    sprintf(filename,"%s.data",name);
    VipMkdir(filename);

    sprintf(filename,"%s.data/bucket",name);
    VipMkdir(filename);

    sprintf(filename,"%s.data/Tmtk",name);
    VipMkdir(filename);

    strcpy(filename,name);
    strcat(filename,".arg");

    fp = fopen(filename,"w");
    if (!fp)
	{
	    printf("Can not open file %s\n",filename);
	    VipPrintfExit("VipWriteLabelArg");
	    return(PB);
	}

	
    fprintf(fp,"# graph 1.0\n\n");
    fprintf(fp,"*BEGIN GRAPH RoiArg\n\n");
    fprintf(fp,"RoiArg_VERSION 1.0\n");
    fprintf(fp,"voxel_size ");
    fprintf(fp, "%f %f %f\n", mVipVolVoxSizeX(vol),
	    mVipVolVoxSizeY(vol),
	    mVipVolVoxSizeZ(vol));
    fprintf(fp, "filename_base %s.data\n\n",shortname);
    fprintf(fp,"#filenames semantic\n");
    fprintf(fp,"type.tri roi.tri\n");
    fprintf(fp,"roi.tri roi Tmtktri_filename\n");
    fprintf(fp,"type.bck roi.bck\n");
    fprintf(fp,"roi.bck roi bucket_filename\n");
    fprintf(fp,"Tmtktri_filename 0 255 0\n");
    fprintf(fp,"bucket_filename 0 255 0\n");
    fprintf(fp,"boundingbox_min 60 5 20\n");
    fprintf(fp,"boundingbox_max 90 160 140\n\n");


    n = nlabel;
    for(i=0;i<100000;i++)
	{
	  if(label[i]>0)
	    {

	      strcpy(nucleusname,"other");
	      if (mode=='n')
		{
		  switch (i)
		    {
		    case 165: strcpy(nucleusname,"Llat_vent");
		      break;
		    case 205: strcpy(nucleusname,"Rlat_vent");
		      break;
		    case 145: strcpy(nucleusname,"Lcaudate");
		      break;
		    case 225: strcpy(nucleusname,"Rcaudate");
		      break;
		    case 155: strcpy(nucleusname,"Laccumbens");
		      break;
		    case 215: strcpy(nucleusname,"Raccumbens");
		      break;
		    case 175: strcpy(nucleusname,"Lputamen");
		      break;
		    case 235: strcpy(nucleusname,"Rputamen");
		      break;
		    case 185: strcpy(nucleusname,"Lpallidum");
		      break;
		    case 245: strcpy(nucleusname,"Rpallidum");
		      break;
		    case 195: strcpy(nucleusname,"Lthalamus");
		      break;
		    case 250: strcpy(nucleusname,"Rthalamus");
		      break;
		    default: 
		      VipPrintfWarning("Unknown nucleus label");
		    }
		}

	      printf("-----------------------------\n");
	      printf("still %d/%d labels \n",n--, nlabel);
	      printf("-----------------------------\n");
	      

	      fflush(stdout);
	      fortriang = VipCreateSingleThresholdedVolume(vol,
							   EQUAL_TO,i,BINARY_RESULT);
	      if(fortriang==PB) return(PB);
	      
	      fprintf(fp,"*BEGIN NODE roi %d\n",i);
	      fprintf(fp,"label %d\n", i);
	      fprintf(fp,"point_number %d\n",label[i]);
	      fprintf(fp,"size %.3f\n",label[i]*mVipVolVoxSizeX(vol)*
		      mVipVolVoxSizeX(vol)*mVipVolVoxSizeZ(vol));

	      if(!strcmp(nucleusname,"other")) 
		{
		  sprintf(nucleusname,"label%d", i);
		}

	      fprintf(fp,"name %s\n",nucleusname);
	      fprintf(fp,"bucket_filename bucket/%s-%s.bck\n",shortname,nucleusname);
	      if(triangulation=='y') fprintf(fp,"Tmtktri_filename Tmtk/%s-%s.mesh\n",shortname,nucleusname);
	      
	      sprintf(buckfilename,"%s.data/bucket/%s-%s.bck",name,shortname,nucleusname);
	      fflush(stdout);
	      buckf = fopen(buckfilename,"w");
	      if (!buckf)
		{
		  fprintf(stderr,"Can not open file %s\n",buckfilename);
		  VipPrintfExit("VipWriteLabelArg");
		  return(PB);
		}
	      
	      fprintf(buckf,"ascii\n");
	      fprintf(buckf,"-type VOID\n");
	      fprintf(buckf,"-dx %f\n",mVipVolVoxSizeX(vol));
	      fprintf(buckf,"-dy %f\n",mVipVolVoxSizeY(vol));
	      fprintf(buckf,"-dz %f\n",mVipVolVoxSizeZ(vol));
	      fprintf(buckf,"-dt 1.\n");
	      fprintf(buckf,"-dimt 1\n");
	      fprintf(buckf,"-time 0\n");
	      fprintf(buckf,"-dim %d\n",label[i]);

	      ptr = VipGetDataPtr_S16BIT( fortriang ) + vos->oFirstPoint;
	      
	      for ( iz = 0; iz < mVipVolSizeZ(fortriang); iz++ )               /* loop on slices */
		{
		  for ( iy = 0; iy < mVipVolSizeY(fortriang); iy++ )            /* loop on lines */
		    {
		      for ( ix = 0; ix <  mVipVolSizeX(fortriang); ix++ )/* loop on points */
			{
			  if(*ptr!=0)
			    fprintf(buckf,"(%d,%d,%d)\n",ix,iy,iz);
			  ptr++;
			}
		      ptr += vos->oPointBetweenLine;  /*skip border points*/
		    }
		  ptr += vos->oLineBetweenSlice; /*skip border lines*/
		}
	      
	      fclose(buckf);
	      
	      fprintf(fp,"*END\n\n");

	      if(triangulation=='y') 
		{
#ifdef VIP_CARTO_VOLUME_WRAPPING
		  sprintf(tempimagename, "%s.data%cTmtk%c%s-%s", 
                          name, VipFileSeparator(), 
                          VipFileSeparator(), shortname, nucleusname );
                  meshAims( fortriang, tempimagename );
#else
		  sprintf(tempimagename,"%s%c%s-mesh%d",VipTmpDirectory(), 
                          VipFileSeparator(), shortname,i);
		  printf("Writing temporary image...\n");
		  VipWriteTivoliVolume(fortriang,tempimagename);
		  sprintf(command,
                          "AimsMeshBrain -i \"%s\" -o " 
                          "\"%s.data%cTmtk%c%s-%s\"",
			  tempimagename,name, VipFileSeparator(), 
                          VipFileSeparator(), shortname,nucleusname);
		  system(command);
                  sprintf( command, "%s.ima", tempimagename );
                  VipUnlink( command );
                  sprintf( command, "%s.dim", tempimagename );
                  VipUnlink( command );
                  sprintf( command, "%s.ima.minf", tempimagename );
                  VipUnlink( command );
#endif
		}
	      VipFreeVolume(fortriang);

	    }
	}
    printf("\n");
    
    fprintf(fp,"\n*END\n"); 
    fclose(fp);
	
    return(OK);
}
