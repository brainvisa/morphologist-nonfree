/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/activations         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 21/10/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sparc
 ****************************************************************************
 *
 * DESCRIPTION : convertion d'un contrast SPM au format generique de graphe
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/bucket.h>
#include <vip/connex.h>
#include <vip/depla.h>
#include <vip/activations.h> 
#include <vip/util/file.h>

/*-------------------------------------------------------------------------*/
int  VipWriteClusterArg( Volume *vol,
int size,   int connectivity,
char *name, char *normaname, char meshflag)
/*-------------------------------------------------------------------------*/
{
    char buckfilename[VIP_NAME_MAXLEN];
    FILE *fp, *buckf;
    char filename[VIP_NAME_MAXLEN];
    Vip3DBucket_S16BIT *cclist=NULL, *buckptr;
    int nlabel;
    int i,j;
    Vip3DPoint_S16BIT *pptr;
    VipDeplacement *norma = NULL;
    Vip3DPoint_VFLOAT G,talG;
    float lx = 0.;
    float ly = 0.;
    float lz = 0.; /*because of yy and zz flip between SPM and vida format...*/

    if(vol==NULL)
	{
	    VipPrintfError("No cluster volume as input");
	    VipPrintfExit("VipWriteClusterArg");
	    return(PB);
	}

    if(name==NULL)
	{
	    VipPrintfError("No name for Cluster arg as input");
	    VipPrintfExit("VipWriteClusterArg");
	    return(PB);
	}

    if(normaname!=NULL)
      {
	norma = VipReadDeplacement(normaname);
	if(norma==PB) return(PB);
      }

    printf("Computing clusters...\n");
    
    lx = (mVipVolSizeX(vol)-1) * mVipVolVoxSizeX(vol);
    ly = (mVipVolSizeY(vol)-1) * mVipVolVoxSizeY(vol);
    lz = (mVipVolSizeZ(vol)-1) * mVipVolVoxSizeZ(vol);

    cclist = VipGetFiltered3DConnex( vol, connectivity, size);
    VipSetImageLevel(vol,0);
    if(cclist==PB)
	{
	    VipPrintfWarning("No cluster alive with these parameters");
	}
    else
	{
	    VipWriteCoordBucketListInVolume(cclist,vol,1,1);
	}
    nlabel = VipGetVolumeMax(vol);
    printf("%d clusters to process...\n",nlabel);

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
	    VipPrintfExit("VipWriteGenericARGtoRef");
	    return(PB);
	}

	
    fprintf(fp,"# graph 1.0\n\n");
    fprintf(fp,"*BEGIN GRAPH ClusterArg\n\n");
    fprintf(fp,"ClusterArg_VERSION 1.0\n");
    fprintf(fp,"voxel_size ");
    fprintf(fp, "%f %f %f\n", mVipVolVoxSizeX(vol),
	    mVipVolVoxSizeY(vol),
	    mVipVolVoxSizeZ(vol));
    fprintf(fp,"nbclusters %d\n",nlabel);
    fprintf(fp, "filename_base %s.data\n\n",name);
    fprintf(fp,"#filenames semantic\n");

    if(meshflag=='y')
      {
	fprintf(fp,"type.tri cluster.tri\n");
	fprintf(fp,"cluster.tri cluster Tmtktri_filename\n");
      }
    fprintf(fp,"type.bck cluster.bck\n");
    fprintf(fp,"cluster.bck cluster bucket_filename\n");
    if(meshflag=='y')
      {
	fprintf(fp,"Tmtktri_filename 0 255 0\n");
      }
    fprintf(fp,"bucket_filename 0 255 0\n");
    fprintf(fp,"boundingbox_min 0 0 0\n");
    fprintf(fp,"boundingbox_max %d %d %d\n\n",
	    mVipVolSizeX(vol),mVipVolSizeY(vol),mVipVolSizeZ(vol));
    buckptr = cclist;

    for(i=1;i<=nlabel;i++)
	{
	    fprintf(fp,"*BEGIN NODE cluster %d\n",i);
	    fprintf(fp,"index %d\n", i);
	    pptr = buckptr->data;
	    G.x = 0.;
	    G.y = 0.;
	    G.z = 0.;
	    for(j=buckptr->n_points;j--;)
		{
		  G.x += pptr->x;
		  G.y += pptr->y;
		  G.z += pptr->z;
		  pptr++;
		}
	    G.x *= mVipVolVoxSizeX(vol) / (float)buckptr->n_points;
	    G.y *= mVipVolVoxSizeY(vol) / (float)buckptr->n_points;
	    G.z *= mVipVolVoxSizeZ(vol) / (float)buckptr->n_points;

	    if(norma!=NULL)
	      {
		VipApplyDeplacement(norma,&G,&talG);
		talG.x = (156 - talG.x)-79;
		talG.y = (188 - talG.y)-113;
		talG.z = (136 - talG.z)-51;
	      }
	    else
	      {
		/*flip vida/SPM*/
		if(vol->shfj->spm_normalized==VTRUE)
		  {
		    G.x = lx - G.x;
		    G.y = ly - G.y;
		    G.z = lz - G.z;

		    talG.x = G.x-vol->shfj->center.x;
		    talG.y = G.y-vol->shfj->center.y;
		    talG.z = G.z-vol->shfj->center.z;
		  }
	      }
	    if(norma!=NULL || vol->shfj->spm_normalized==VTRUE)
	      {
		fprintf(fp,"Talairach_(mm) %d %d %d\n",(int)(talG.x+0.5),
			(int)(talG.y+0.5), (int)(talG.z+0.5));
		fprintf(fp,"label %d %d %d\n",(int)(talG.x+0.5),
			(int)(talG.y+0.5), (int)(talG.z+0.5));
	      }
	    fprintf(fp,"point_number %d\n",buckptr->n_points);
	    fprintf(fp,"size %.3f\n",buckptr->n_points*mVipVolVoxSizeX(vol)*
		    mVipVolVoxSizeX(vol)*mVipVolVoxSizeZ(vol));
	    fprintf(fp,"name %s\n", name);
	    fprintf(fp,"bucket_filename bucket/%s-c%d.bck\n",name,i);
	    if(meshflag=='y')
	      {
		fprintf(fp,"Tmtktri_filename Tmtk/%s-tmt%d.mesh\n",name,i);
	      }
	    sprintf(buckfilename,"%s.data/bucket/%s-c%d.bck",name,name,i);
	    fflush(stdout);
	    buckf = fopen(buckfilename,"w");
	    if (!buckf)
		{
		    fprintf(stderr,"Can not open file %s\n",buckfilename);
		    VipPrintfExit("VipWriteGenericSSfile");
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
	    fprintf(buckf,"-dim %d\n",buckptr->n_points);
	    pptr = buckptr->data;
	    for(j=buckptr->n_points;j--;)
		{
		    fprintf(buckf,"(%d,%d,%d)\n",pptr->x,pptr->y,pptr->z);
		    pptr++;
		}
	    fclose(buckf);

	    fprintf(fp,"*END\n\n");
	    buckptr = buckptr->next;
	}

    fprintf(fp,"\n*END\n"); 
    fclose(fp);
	
    VipFree3DBucketList_S16BIT(cclist);

    return(OK);
}


/*-------------------------------------------------------------------------*/
int VipComputeLabelledClusterTmtkTriangulation( Volume *labeling, char *name)
/*-------------------------------------------------------------------------*/
{
    int nlabel;
    Volume *fortriang=NULL;
    int index;
    char command[VIP_NAME_MAXLEN];
    char filename[VIP_NAME_MAXLEN];
    char tempimagename[VIP_NAME_MAXLEN];

    if(labeling==NULL || name==NULL)
	{
	    VipPrintfError("Empty Arg");
	    VipPrintfExit("VipComputeLabelledClusterTmtkTriangulation");
	    return(PB);
	}
    nlabel = VipGetVolumeMax(labeling);

    if(nlabel==0)
	{
	    VipPrintfWarning("no cluster");
	    VipPrintfExit("VipComputeLabelledClusterTmtkTriangulation");
	    return(PB);
	}
    
    printf("----------------------------------------------\n");
    printf("Computing triangulations using Tmtk library...\n");
    printf("----------------------------------------------\n");

    sprintf(filename,"%s.data",name);
    VipMkdir(filename);

    sprintf(filename,"%s.data/Tmtk",name);
    VipMkdir(filename);

    for(index=1;index<=nlabel;index++)
	{
	    printf("-----------------------------\n");
	    printf("still %d clusters for triangulation\n",nlabel -index+1);
	    printf("-----------------------------\n");

	    fflush(stdout);
	    fortriang = VipCreateSingleThresholdedVolume(labeling,
							 EQUAL_TO,index,BINARY_RESULT);
	    if(fortriang==PB) return(PB);

	    sprintf(tempimagename,"%s%c%s-mesh%d", VipTmpDirectory(), 
                    VipFileSeparator(), name, index);
	    VipWriteTivoliVolume(fortriang,tempimagename);
	    VipFreeVolume(fortriang);
	    sprintf(command,"AimsMeshBrain -i \"%s\" -o " 
                    "\"%s.data/Tmtk/%s-tmt%d\"",
		    tempimagename,name,name,index);
	    system(command);
            sprintf( command, "%s.ima", tempimagename );
            VipUnlink( command );
            sprintf( command, "%s.dim", tempimagename );
            VipUnlink( command );
	}
    printf("\n");

    return(OK);

}
