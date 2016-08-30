/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 22/07/1999
 * VERSION     : 1.5                 * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : recuperation du format de graphe relationnel
 *               attribue sur les plis corticaux  de la these de JF Mangin
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/


#include <vip/fold_arg.h>
#include <vip/fold_arg_static.h>
#include <vip/alloc.h>
#include <vip/eigen.h>
#include <vip/util.h>
#include <vip/bucket.h>
#include <vip/volume.h>
#include <vip/distmap.h>

/*-------------------------------------------------------------------------*/
static int VipAddBucketToTalInertia(Vip3DBucket_S16BIT *buck,
                                    VipMatrix_VDOUBLE *covar,
                                    Vip3DPoint_VFLOAT vox,
                                    VipTalairach *tal);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipPutBucketInDepthAndLocation(Vip3DBucket_S16BIT *buck,
                                          float *maxdepth,
                                          Vip3DPoint_S16BIT **loc,
                                          Volume *depth,
                                          VipOffsetStruct *vos); 
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipPutBucketInTwoDepth(Vip3DBucket_S16BIT *buck,
                                  float *mindepth, float *maxdepth,
                                  Volume *depth,
                                  VipOffsetStruct *vos);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int  VipPutBucketInTalBoundingBox(Vip3DBucket_S16BIT *buck,
                                         Vip3DPoint_VFLOAT *boxmin,
                                         Vip3DPoint_VFLOAT *boxmax,
                                         VipTalairach *tal);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int  VipPutBucketInBoundingBox(Vip3DBucket_S16BIT *buck,
                                      Vip3DPoint_S16BIT *boxmin,
                                      Vip3DPoint_S16BIT *boxmax);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int FillArgBoxAndVoxSize(Volume *squel, FoldArg *arg,
                                int inside, int outside);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipAddBucketToGravity(Vip3DBucket_S16BIT *buck,
                                 Vip3DPoint_VFLOAT *g);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipAddBucketToNormal(Vip3DBucket_S16BIT *buck,
                                VipMatrix_VDOUBLE *covar,
                                Vip3DPoint_VFLOAT vox,
                                double G[3]);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
FoldArg *CreateFoldArgFromSquel(char *name, Volume *skeleton,
                                int inside, int outside,
                                int limitsssize, int compute_triang,
                                char *rootsvoronoi_name)
/*-------------------------------------------------------------------------*/
{
  FoldArg *arg;

  if(skeleton==NULL || name==NULL)
    {
      VipPrintfError("Empty Arg");
      VipPrintfExit("CreateFoldArgFromSquel");
      return(PB);
    }

  arg = CreateEmptyFoldArg(name);
  if(arg==PB) return(PB);
  
  VipSetBorderLevel(skeleton, outside);
  
  if(FillArgBoxAndVoxSize(skeleton, arg, inside, outside)==PB) return(PB);

  arg->ss_list = VipComputeFoldArgSSList(skeleton, inside, outside, limitsssize, rootsvoronoi_name);
  if(arg->ss_list == PB) return(PB);

  arg->jset = VipComputeFoldArgJunctionSet(skeleton, arg->ss_list, inside, outside);
  if(arg->jset==PB) return(PB);

  arg->ppset = VipComputeFoldArgPliDePassageSet(arg->ss_list, arg->jset);
  if(arg->ppset==PB) return(PB);
  
  arg->cortex_jset = VipComputeFoldArgCortexRelationSet(skeleton, arg->ss_list);
  if(arg->cortex_jset==PB) return(PB);
  
  if(compute_triang==VTRUE) VipComputeTmtkTriangulation(skeleton, arg);
  
  return(arg);
}

/*-------------------------------------------------------------------------*/
int VipComputeFoldArgSemanticAttributes(Volume *skeleton, FoldArg *arg,
				       int inside, int outside,
					  VipTalairach *tal)
/*-------------------------------------------------------------------------*/
{
    Volume *depth;
    int forbidden = -111;
    VipOffsetStruct *vos;
    int ix, iy, iz;
    int xsize, ysize, zsize;
    Vip_S16BIT *ptr;
    float coord[3], refcoord[3];

    if(skeleton==NULL || arg==NULL)
    {
      VipPrintfError("Empty Arg");
      VipPrintfExit("VipComputeFoldArgSemanticAttributes");
      return(PB);
    }

    if(tal!=NULL) 
	{
	    printf("Filling %s  global bounding box in reference frame\n",skeleton->name);
	    arg->min_ref.x = 1000.;
	    arg->min_ref.y = 1000.;
	    arg->min_ref.z = 1000.;
	    arg->max_ref.x = -1000.;
	    arg->max_ref.y = -1000.;
	    arg->max_ref.z = -1000.;
	    vos = VipGetOffsetStructure(skeleton);
	    ptr = VipGetDataPtr_S16BIT( skeleton ) + vos->oFirstPoint;
	    xsize = mVipVolSizeX(skeleton);
	    ysize = mVipVolSizeY(skeleton);
	    zsize = mVipVolSizeZ(skeleton);

	    for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
		{
		    for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
			{
			    for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
				{
				    if(*ptr!=inside && *ptr!=outside)
					{
					    coord[0] = mVipVolVoxSizeX(skeleton)*ix;
					    coord[1] = mVipVolVoxSizeY(skeleton)*iy;
					    coord[2] = mVipVolVoxSizeZ(skeleton)*iz;
					    VipTalairachTransformation(coord,tal,refcoord);
					    coord[0] = refcoord[0]*tal->Scale.x;
					    coord[1] = refcoord[1]*tal->Scale.y;
					    coord[2] = refcoord[2]*tal->Scale.z;
					    if(coord[0]<arg->min_ref.x) arg->min_ref.x=coord[0];
					    if(coord[1]<arg->min_ref.y) arg->min_ref.y=coord[1];
					    if(coord[2]<arg->min_ref.z) arg->min_ref.z=coord[2];
					    if(coord[0]>arg->max_ref.x) arg->max_ref.x=coord[0];
					    if(coord[1]>arg->max_ref.y) arg->max_ref.y=coord[1];
					    if(coord[2]>arg->max_ref.z) arg->max_ref.z=coord[2];
					}
					ptr++;
				}
			    ptr += vos->oPointBetweenLine;  /*skip border points*/
			}
		    ptr += vos->oLineBetweenSlice; /*skip border lines*/
		}
	}
 
    printf("Computing skeleton geodesic depth (distance to outside)...\n"); 
    depth = VipCreateDoubleThresholdedVolume(skeleton,NOT_EQUAL_TO,inside,outside,
					     BINARY_RESULT);

    if(depth==PB) return(PB);
    VipMerge(depth,skeleton,VIP_MERGE_ONE_TO_ONE,inside,forbidden);
    
    
    if( VipComputeFrontPropagationChamferDistanceMap (depth, 
						      255, forbidden,
						      VIP_NO_LIMIT_IN_PROPAGATION
						      , 0)==PB)
	return(PB);

    
    if(arg->ss_list!=NULL)
	if(VipComputeSSAttributes(depth, arg, tal)==PB)
	return(PB);
    
    if(arg->jset!=NULL)
	if(VipComputeJunctionAttributes(depth, arg, tal)==PB) return(PB);
    
    if(arg->ppset!=NULL)
	if(VipComputePliDePassageAttributes(depth, arg, tal)==PB) return(PB);
    
    if(arg->cortex_jset!=NULL)
	if(VipComputeCortexRelationAttributes(depth, arg, tal)==PB) return(PB);
    

    VipFreeVolume(depth);

    return(OK);
}

/*-------------------------------------------------------------------------*/
int VipComputeCortexRelationAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal)
/*-------------------------------------------------------------------------*/
{
    CortexBiRelation *rel;
    VipOffsetStruct *vos;
    double dist2, mindist2, temp, xvox2, yvox2, zvox2;
    Vip3DPoint_S16BIT *ss1n=NULL, *ss2n=NULL;
    int i, j;
    BiJunction *hj1, *hj2;
    float coord[3], refcoord[3];
    int test_dist0 = VFALSE;

   if(depth==NULL || arg==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipComputeCortexRelationAttributes");
	    return(PB);
	}

   vos = VipGetOffsetStructure(depth);
   if(!vos) return(PB);

   xvox2 = arg->vox.x * arg->vox.x;
   yvox2 = arg->vox.y * arg->vox.y;
   zvox2 = arg->vox.z * arg->vox.z;

   rel = arg->cortex_jset->first_bi;

   while(rel!=NULL)
       {
	    /*size*/
	   rel->size = rel->skiz->n_points * arg->vox.x * arg->vox.y * arg->vox.z;
	   rel->size_filled = VTRUE;

	   /*dist*/
	   if(rel->s1==NULL || rel->s2==NULL)
	       {
		   VipPrintfError("cortical relation without ss");
		   VipPrintfExit("VipComputeCortexRelationAttributes");
		   return(PB);
	       }
	   if(rel->s1->hull_bijunction==NULL || rel->s2->hull_bijunction==NULL) 
	       {
		   VipPrintfError("ss without hull junction");
		   VipPrintfExit("VipComputeCortexRelationAttributes");
		   return(PB);
	       }
	   hj1 = rel->s1->hull_bijunction;
	   hj2 = rel->s2->hull_bijunction;
	   mindist2 = 10000000.;
	   test_dist0 = VFALSE;
	   for(i=0;i<hj1->points->n_points;i++)
	       for(j=0;j<hj2->points->n_points;j++)
		   {
		       dist2 = 0.;
		       temp = hj1->points->data[i].x - hj2->points->data[j].x;
		       dist2 += temp*temp*xvox2;
		       temp = hj1->points->data[i].y - hj2->points->data[j].y;
		       dist2 += temp*temp*yvox2;
		       temp = hj1->points->data[i].z - hj2->points->data[j].z;
		       dist2 += temp*temp*zvox2;
		       if(dist2>0.0001 && dist2<mindist2) 
			   /*trijunction points are discarded*/
			   {
			       mindist2 = dist2;
			       ss1n = hj1->points->data + i;
			       ss2n = hj2->points->data + j;
			   }
		       if(dist2<0.0001) test_dist0 = VTRUE;
		   }
	   if(test_dist0==VTRUE) mindist2 = 0.;
	   if(ss1n==NULL || ss2n==NULL)
	       { 
		   VipPrintfWarning("no nearest point between hull_junction");
		   printf("hull junction sizes: %d, %d\n",
			  hj1->points->n_points,
			  hj2->points->n_points);
	       }
	   else
	       {
		   rel->hulljunctiondist = (float)sqrt(mindist2);
		   rel->hulljunctiondist_filled = VTRUE;
		   rel->SS1nearest.x = ss1n->x;
		   rel->SS1nearest.y = ss1n->y;
		   rel->SS1nearest.z = ss1n->z;
		   rel->SS2nearest.x = ss2n->x;
		   rel->SS2nearest.y = ss2n->y;
		   rel->SS2nearest.z = ss2n->z;
		   rel->SS1nearest_filled = VTRUE;
		   rel->SS2nearest_filled = VTRUE;
	       }
	    if(tal!=NULL)
		{
		    /* I know, it is very heavy, a simple conversion procedure
		       and life is easier, existing ones are so dumbheaded...*/
		    coord[0] = tal->VoxelGeometry.x*ss1n->x;
		    coord[1] = tal->VoxelGeometry.y*ss1n->y;
		    coord[2] = tal->VoxelGeometry.z*ss1n->z;
		    VipTalairachTransformation(coord,tal,refcoord);
		    rel->refSS1nearest.x = refcoord[0]*tal->Scale.x;
		    rel->refSS1nearest.y = refcoord[1]*tal->Scale.y;
		    rel->refSS1nearest.z = refcoord[2]*tal->Scale.z;    
		    rel->refSS1nearest_filled = VTRUE;
		    coord[0] = tal->VoxelGeometry.x*ss2n->x;
		    coord[1] = tal->VoxelGeometry.y*ss2n->y;
		    coord[2] = tal->VoxelGeometry.z*ss2n->z;
		    VipTalairachTransformation(coord,tal,refcoord);
		    rel->refSS2nearest.x = refcoord[0]*tal->Scale.x;
		    rel->refSS2nearest.y = refcoord[1]*tal->Scale.y;
		    rel->refSS2nearest.z = refcoord[2]*tal->Scale.z;    
		    rel->refSS2nearest_filled = VTRUE;
		}
	    rel = rel->next;
       }

   return(OK);
}


/*-------------------------------------------------------------------------*/
int VipComputePliDePassageAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal)
/*-------------------------------------------------------------------------*/
{
    PliDePassage *pp;
    float maxdepth;
    VipOffsetStruct *vos;
    float coord[3], refcoord[3];
    Vip3DPoint_S16BIT *loc = 0;

   if(depth==NULL || arg==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipComputePliDePassageAttributes");
	    return(PB);
	}

    vos = VipGetOffsetStructure(depth);
    if(!vos) return(PB);

    pp = arg->ppset->first_pp;
    while(pp!=NULL)
	{
	    /*size*/
	    pp->size = pp->points->n_points * arg->vox.x * arg->vox.y * arg->vox.z;
	    pp->size_filled = VTRUE;

	    /*depth and location*/

	    maxdepth = 0.;
	    VipPutBucketInDepthAndLocation(pp->points,&maxdepth,&loc,depth,vos);
	    pp->depth = maxdepth;
	    pp->depth_filled = VTRUE;
	    pp->location.x = loc->x;
	    pp->location.y = loc->y;
	    pp->location.z = loc->z;
	    pp->location_filled = VTRUE;
	    if(tal!=NULL)
		{
		    /* I know, it is very heavy, a simple conversion procedure
		       and life is easier, existing ones are so dumbheaded...*/
		    coord[0] = tal->VoxelGeometry.x*loc->x;
		    coord[1] = tal->VoxelGeometry.y*loc->y;
		    coord[2] = tal->VoxelGeometry.z*loc->z;
		    VipTalairachTransformation(coord,tal,refcoord);
		    pp->reflocation.x = refcoord[0]*tal->Scale.x;
		    pp->reflocation.y = refcoord[1]*tal->Scale.y;
		    pp->reflocation.z = refcoord[2]*tal->Scale.z;    
		    pp->reflocation_filled = VTRUE;
		}
	    pp = pp->next;
	}
    return(OK);
}
/*-------------------------------------------------------------------------*/
int VipComputeJunctionAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal)
/*-------------------------------------------------------------------------*/
{
    int i, j;
    BiJunction *bij;
    double G[3], n[3], norme;
    float maxdepth, mindepth;
    VipOffsetStruct *vos;
    VipMatrix_VDOUBLE *covar;
    VipVector_VDOUBLE *intermediate;
    VipVector_VDOUBLE *eigenvalue;
    double maxeigenvalue;
    int imaxeigenvalue;
    float coord[3], refcoord[3];
    Vip3DPoint_VFLOAT g;
    float temp, dist, maxdist;
    Vip3DPoint_S16BIT *ptr1, *ptr2, *e1, *e2;

    covar = VipAllocationMatrix_VDOUBLE(3,3,"covariance matrix");
    if(covar==PB) return(PB);
    eigenvalue = VipAllocationVector_VDOUBLE(3, "VipComputeJunctionAttributes");
    if(eigenvalue==PB) return(PB);
    intermediate = VipAllocationVector_VDOUBLE(3, "VipComputeJunctionAttributes");
    if(intermediate==PB) return(PB);

    if(depth==NULL || arg==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipComputeJunctionAttributes");
	    return(PB);
	}

    vos = VipGetOffsetStructure(depth);
    if(!vos) return(PB);

    bij = arg->jset->first_bi;
    while(bij!=NULL)
	{
	    /*size*/
	    bij->size = bij->points->n_points * arg->vox.x * arg->vox.y * arg->vox.z;
	    bij->size_filled = VTRUE;

	    
	    /*direction*/
	    if(bij->points->n_points>10)
		{
		    g.x = 0.;
		    g.y = 0.;
		    g.z = 0.;
		    VipAddBucketToGravity(bij->points,&g);
		    g.x /= bij->points->n_points;
		    g.y /= bij->points->n_points;
		    g.z /= bij->points->n_points;
		    G[0] = g.x * arg->vox.x;
		    G[1] = g.y * arg->vox.y;
		    G[2] = g.z * arg->vox.z;
		    for(i=0;i<3;i++)
			{
			    intermediate->data[i] = 0;
			    eigenvalue->data[i] = 0;
			    for(j=0;j<3;j++)
				{
				    covar->data[i][j] = 0.;
				}
			}
		    VipAddBucketToNormal(bij->points,covar,arg->vox,G);
		    covar->data[1][0] = covar->data[0][1];
		    covar->data[2][0] = covar->data[0][2];
		    covar->data[2][1] = covar->data[1][2];
		    /*sorry the following is a big mess...*/
		    VipTRED2_VDOUBLE(covar, eigenvalue, intermediate);
		    if(VipTQLI_VDOUBLE(eigenvalue, intermediate, covar) == PB)
			{
			    VipPrintfWarning("error during normal computation");
			    continue;
			}
		    maxeigenvalue = -10000000.;
		    imaxeigenvalue = 2;
		    for(i=0;i<3;i++)
			if(maxeigenvalue<eigenvalue->data[i])
			   {
			      maxeigenvalue=eigenvalue->data[i]; 
			      imaxeigenvalue = i;
			   }
		    n[0] = covar->data[0][imaxeigenvalue];
		    n[1] = covar->data[1][imaxeigenvalue];
		    n[2] = covar->data[2][imaxeigenvalue];
		    norme = n[0]*n[0]+n[1]*n[1]+n[2]*n[2];
		    norme = sqrt(norme);
		    if(norme<0.0000001)
			{
			    VipPrintfError("error during direction computation");
			    VipPrintfExit("(VipComputeJunctionAttributes)");
			    return(PB);
			}
		    bij->direction.x = (float)n[0];
		    bij->direction.y = (float)n[1];
		    bij->direction.z = (float)n[2];
		    bij->direction_filled = VTRUE;
		    if(tal!=NULL)
			{
			    bij->refdirection.x = tal->Rotation[0][0] * n[0]
				+ tal->Rotation[0][1] * n[1]
				+ tal->Rotation[0][2] * n[2];
			    bij->refdirection.y = tal->Rotation[1][0] * n[0]
				+ tal->Rotation[1][1] * n[1]
				+ tal->Rotation[1][2] * n[2];
			    bij->refdirection.z = tal->Rotation[2][0] * n[0]
				+ tal->Rotation[2][1] * n[1]
				+ tal->Rotation[2][2] * n[2];
			    bij->refdirection_filled = VTRUE;
			}
		}

	
	    /*depth*/
	    maxdepth = 0.;
	    mindepth = 10000;
	    VipPutBucketInTwoDepth(bij->points,&mindepth,&maxdepth,depth,vos);
	    bij->maxdepth = maxdepth;
	    bij->maxdepth_filled = VTRUE;
	    bij->mindepth = mindepth;
	    bij->mindepth_filled = VTRUE;

	    /*extremity*/
	    maxdist = -100.;
	    e1 = NULL;
	    e2 = NULL;
	    for(i=0;i<bij->points->n_points;i++)
		{
		    ptr1 = bij->points->data + i;
		    for(j=0;j<bij->points->n_points;j++)
			{
			    ptr2 = bij->points->data + j;
			    dist = (ptr1->x - ptr2->x)* arg->vox.x;
			    dist *= dist;
			    temp = (ptr1->y - ptr2->y)* arg->vox.y;
			    temp *= temp;
			    dist += dist;
			    temp = (ptr1->z - ptr2->z)* arg->vox.z;
			    temp *= temp;
			    dist += dist;
			    if(dist>maxdist)
				{
				    maxdist = dist;
				    e1 = ptr1;
				    e2 = ptr2;			    
				}
			}
		}
	    if(e1==NULL || e2==NULL)
		{
		    VipPrintfError("computing extremity");
		    VipPrintfExit("(VipComputeJunctionAttributes)");
		    return(PB);
		}
	    bij->extremity1.x = e1->x;
	    bij->extremity1.y = e1->y;
	    bij->extremity1.z = e1->z;
	    bij->extremity1_filled = VTRUE;
	    bij->extremity2.x = e2->x;
	    bij->extremity2.y = e2->y;
	    bij->extremity2.z = e2->z;
	    bij->extremity2_filled = VTRUE;
	    if(tal!=NULL)
		{
		    /* I know, it is very heavy, a simple conversion procedure
		       and life is easier, existing ones are so dumbheaded...*/
		    coord[0] = tal->VoxelGeometry.x*e1->x;
		    coord[1] = tal->VoxelGeometry.y*e1->y;
		    coord[2] = tal->VoxelGeometry.z*e1->z;
		    VipTalairachTransformation(coord,tal,refcoord);
		    bij->refextremity1.x = refcoord[0]*tal->Scale.x;
		    bij->refextremity1.y = refcoord[1]*tal->Scale.y;
		    bij->refextremity1.z = refcoord[2]*tal->Scale.z;    
		    bij->refextremity1_filled = VTRUE;
		    coord[0] = tal->VoxelGeometry.x*e2->x;
		    coord[1] = tal->VoxelGeometry.y*e2->y;
		    coord[2] = tal->VoxelGeometry.z*e2->z;
		    VipTalairachTransformation(coord,tal,refcoord);
		    bij->refextremity2.x = refcoord[0]*tal->Scale.x;
		    bij->refextremity2.y = refcoord[1]*tal->Scale.y;
		    bij->refextremity2.z = refcoord[2]*tal->Scale.z;    
		    bij->refextremity2_filled = VTRUE;
		}

	    bij = bij->next;

	}
    return(OK);
}

/*-------------------------------------------------------------------------*/
int VipComputeSSAttributes(Volume *depth, FoldArg *arg,
			   VipTalairach *tal)
/*-------------------------------------------------------------------------*/
{
    int i, j, ssindex;
    SurfaceSimple *ss;
    int total;
    double G[3], n[3], norme;
    float maxdepth, mindepth;
    VipOffsetStruct *vos;
    VipMatrix_VDOUBLE *covar;
    VipVector_VDOUBLE *intermediate;
    VipVector_VDOUBLE *eigenvalue;
    double mineigenvalue;
    int imineigenvalue;
    float coord[3], refcoord[3];


    covar = VipAllocationMatrix_VDOUBLE(3,3,"covariance matrix");
    if(covar==PB) return(PB);
    eigenvalue = VipAllocationVector_VDOUBLE(3, "VipComputeSSAttributes");
    if(eigenvalue==PB) return(PB);
    intermediate = VipAllocationVector_VDOUBLE(3, "VipComputeSSAttributes");
    if(intermediate==PB) return(PB);

    if(depth==NULL || arg==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipComputeSSAttributes");
	    return(PB);
	}

    vos = VipGetOffsetStructure(depth);
    if(!vos) return(PB);

    for(ssindex=1;ssindex<=arg->ss_list->n_ss;ssindex++)
	{
	    ss = arg->ss_list->tab[ssindex];

	    /*size*/
	    ss->size = ss->n_points * arg->vox.x * arg->vox.y * arg->vox.z;
	    ss->size_filled = VTRUE;

	    /*boundingbox*/
	    ss->boxmin.x = 10000;
	    ss->boxmin.y = 10000;
	    ss->boxmin.z = 10000;
	    ss->boxmax.x = -10000;
	    ss->boxmax.y = -10000;
	    ss->boxmax.z = -10000;
	    VipPutBucketInBoundingBox(ss->surface_points,&(ss->boxmin),&(ss->boxmax));
	    VipPutBucketInBoundingBox(ss->edge_points,&(ss->boxmin),&(ss->boxmax));
	    if(ss->other_points!=NULL)
		VipPutBucketInBoundingBox(ss->other_points,&(ss->boxmin),&(ss->boxmax));
	    ss->box_filled = VTRUE;

	    if(tal!=NULL)
		{
		    ss->refboxmin.x = 10000;
		    ss->refboxmin.y = 10000;
		    ss->refboxmin.z = 10000;
		    ss->refboxmax.x = -10000;
		    ss->refboxmax.y = -10000;
		    ss->refboxmax.z = -10000;
		    VipPutBucketInTalBoundingBox(ss->surface_points,&(ss->refboxmin),&(ss->refboxmax),tal);
		    VipPutBucketInTalBoundingBox(ss->edge_points,&(ss->refboxmin),&(ss->refboxmax),tal);
		    if(ss->other_points!=NULL)
			VipPutBucketInTalBoundingBox(ss->other_points,&(ss->refboxmin),&(ss->refboxmax),tal);
		    ss->refbox_filled = VTRUE;
		}


	    /*gravity*/
	    ss->g.x = 0.;
	    ss->g.y = 0.;
	    ss->g.z = 0.;
	    VipAddBucketToGravity(ss->surface_points,&(ss->g));
	    VipAddBucketToGravity(ss->edge_points,&(ss->g));
	    if(ss->other_points!=NULL)
		VipAddBucketToGravity(ss->other_points,&(ss->g));
	    total = ss->surface_points->n_points + ss->edge_points->n_points;
	    if(ss->other_points!=NULL)
		total +=  ss->other_points->n_points;
	    if(total!=ss->n_points)
		{
		    VipPrintfWarning("ss point numbers seem strange (VipComputeSSAttributes)");
		}
	    if(total==0)
		{
		    VipPrintfError("empty ss ");
		    VipPrintfExit("(VipComputeSSAttributes)");
		    return(PB);
		}
	    /*NB: g coordinates do not take into account voxel sizes*/
	    ss->g.x /= (float)total;
	    ss->g.y /= (float)total;
	    ss->g.z /= (float)total;
	    ss->g_filled = VTRUE;
	    G[0] = ss->g.x * arg->vox.x;
	    G[1] = ss->g.y * arg->vox.y;
	    G[2] = ss->g.z * arg->vox.z;
	    if(tal!=NULL)
		{
		    coord[0] = G[0];
		    coord[1] = G[1];
		    coord[2] = G[2];
		    VipTalairachTransformation(coord,tal,refcoord);
		    ss->refg.x = refcoord[0]*tal->Scale.x;
		    ss->refg.y = refcoord[1]*tal->Scale.y;
		    ss->refg.z = refcoord[2]*tal->Scale.z;    
		    ss->refg_filled = VTRUE;
		}
	    if(tal!=NULL) /*matrice d'innertie dans Talairach*/
		{
		    for(i=0;i<3;i++)
			{
			    for(j=0;j<3;j++)
				{
				    covar->data[i][j] = 0.;
				}
			}
		    VipAddBucketToTalInertia(ss->surface_points,covar,arg->vox,tal);
		    VipAddBucketToTalInertia(ss->edge_points,covar,arg->vox,tal);
		    if(ss->other_points!=NULL)
			VipAddBucketToTalInertia(ss->other_points,covar,arg->vox,tal);
		    covar->data[1][0] = covar->data[0][1];
		    covar->data[2][0] = covar->data[0][2];
		    covar->data[2][1] = covar->data[1][2];
		    for(i=0;i<3;i++)
			{
			    for(j=0;j<3;j++)
				{
				    ss->talcovar[i][j] = covar->data[i][j];
				}
			}
		    ss->talcovar_filled = VTRUE;
		}
	    /*normal*/
	    if(total>=10)
		{
		    for(i=0;i<3;i++)
			{
			    intermediate->data[i] = 0;
			    eigenvalue->data[i] = 0;
			    for(j=0;j<3;j++)
				{
				    covar->data[i][j] = 0.;
				}
			}
		    VipAddBucketToNormal(ss->surface_points,covar,arg->vox,G);
		    VipAddBucketToNormal(ss->edge_points,covar,arg->vox,G);
		    if(ss->other_points!=NULL)
			VipAddBucketToNormal(ss->other_points,covar,arg->vox,G);
		    covar->data[1][0] = covar->data[0][1];
		    covar->data[2][0] = covar->data[0][2];
		    covar->data[2][1] = covar->data[1][2];
		    /*sorry the following is a big mess...*/
		    VipTRED2_VDOUBLE(covar, eigenvalue, intermediate);
		    if(VipTQLI_VDOUBLE(eigenvalue, intermediate, covar) == PB)
			{
			    VipPrintfWarning("error during normal computation");
			    continue;
			}
		    mineigenvalue = 10000000.;
		    imineigenvalue = 2;
		    for(i=0;i<3;i++)
			if(mineigenvalue>eigenvalue->data[i])
			   {
			      mineigenvalue=eigenvalue->data[i]; 
			      imineigenvalue = i;
			   }
		    n[0] = covar->data[0][imineigenvalue];
		    n[1] = covar->data[1][imineigenvalue];
		    n[2] = covar->data[2][imineigenvalue];
		    norme = n[0]*n[0]+n[1]*n[1]+n[2]*n[2];
		    norme = sqrt(norme);
		    if(norme<0.0000001)
			{
			    VipPrintfError("error during normal computation");
			    VipPrintfExit("(VipComputeSSAttributes)");
			    return(PB);
			}
		    ss->normale.x = (float)n[0];
		    ss->normale.y = (float)n[1];
		    ss->normale.z = (float)n[2];
		    ss->normale_filled = VTRUE;	

		    if(tal!=NULL)
			{
			    ss->refnormale.x = tal->Rotation[0][0] * n[0]
				+ tal->Rotation[0][1] * n[1]
				+ tal->Rotation[0][2] * n[2];
			    ss->refnormale.y = tal->Rotation[1][0] * n[0]
				+ tal->Rotation[1][1] * n[1]
				+ tal->Rotation[1][2] * n[2];
			    ss->refnormale.z = tal->Rotation[2][0] * n[0]
				+ tal->Rotation[2][1] * n[1]
				+ tal->Rotation[2][2] * n[2];
			    ss->refnormale_filled = VTRUE;
			}
		}
	    /*depth*/
	    maxdepth = 0.;
	    mindepth = 100000.;
	    VipPutBucketInTwoDepth(ss->surface_points,&mindepth,&maxdepth,depth,vos);
	    VipPutBucketInTwoDepth(ss->edge_points,&mindepth,&maxdepth,depth,vos);
	    if(ss->other_points!=NULL)
		VipPutBucketInTwoDepth(ss->other_points,&mindepth,&maxdepth,depth,vos);
	    if(ss->hull_bijunction!=NULL)
		if(ss->hull_bijunction->points!=NULL)
		    VipPutBucketInTwoDepth(ss->hull_bijunction->points,
					   &mindepth,&maxdepth,depth,vos);
	    if((mindepth-0.01)<=arg->vox.x || 
	       (mindepth-0.01)<=arg->vox.y ||
	       (mindepth-0.01)<=arg->vox.z)
		{
		    mindepth=0;
		}
	    ss->depth = maxdepth;
	    ss->depth_filled = VTRUE;
	    ss->mindepth = mindepth;
	    ss->mindepth_filled = VTRUE;
	}
    return(OK);
}

/*-------------------------------------------------------------------------*/
static int VipAddBucketToTalInertia(Vip3DBucket_S16BIT *buck,
				VipMatrix_VDOUBLE *covar,
				Vip3DPoint_VFLOAT vox,
				VipTalairach *tal)
/*-------------------------------------------------------------------------*/
{
    int i;
    float x,y,z;
    float coord[3], refcoord[3];


    if(buck==NULL || covar==NULL || tal==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipAddBucketToTalInertia");
	    return(PB);
	}
    for(i=0;i<buck->n_points;i++)
	{
	    coord[0] = buck->data[i].x * vox.x;
	    coord[1] = buck->data[i].y * vox.y;
	    coord[2] = buck->data[i].z * vox.z;
	    VipTalairachTransformation(coord,tal,refcoord);
	    x = refcoord[0]*tal->Scale.x;
	    y = refcoord[1]*tal->Scale.y;
	    z = refcoord[2]*tal->Scale.z;    
	    covar->data[0][0] += x*x;
	    covar->data[1][1] += y*y;
	    covar->data[2][2] += z*z;
	    covar->data[0][1] += x*y;
	    covar->data[0][2] += x*z;
	    covar->data[1][2] += y*z;
	}
    return(OK);
}

/*-------------------------------------------------------------------------*/
static int VipAddBucketToNormal(Vip3DBucket_S16BIT *buck,
				VipMatrix_VDOUBLE *covar,
				Vip3DPoint_VFLOAT vox,
				double G[3])
/*-------------------------------------------------------------------------*/
{
    int i;
    float x,y,z;

    if(buck==NULL || covar==NULL || G==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipAddBucketToGravity");
	    return(PB);
	}
    for(i=0;i<buck->n_points;i++)
	{
	    x = buck->data[i].x * vox.x - G[0];
	    y = buck->data[i].y * vox.y - G[1];
	    z = buck->data[i].z * vox.z - G[2];
	    covar->data[0][0] += x*x;
	    covar->data[1][1] += y*y;
	    covar->data[2][2] += z*z;
	    covar->data[0][1] += x*y;
	    covar->data[0][2] += x*z;
	    covar->data[1][2] += y*z;
	}
    return(OK);
}

/*-------------------------------------------------------------------------*/
static int VipPutBucketInDepthAndLocation(Vip3DBucket_S16BIT *buck,
			       float *maxdepth, Vip3DPoint_S16BIT **loc, Volume *depth,
			       VipOffsetStruct *vos)
/*-------------------------------------------------------------------------*/
{
    int i;
    Vip_S16BIT *ptr, *first;

    if(buck==NULL || maxdepth==NULL || depth==NULL || vos==NULL || loc==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipPutBucketInDepthAndLoc");
	    return(PB);
	}

    first = VipGetDataPtr_S16BIT( depth ) + vos->oFirstPoint;
    *loc = buck->data;
    for(i=0;i<buck->n_points;i++)
	{
	    ptr = first + buck->data[i].x 
		+ buck->data[i].y * vos->oLine
		+ buck->data[i].z * vos->oSlice;
	    if((float)*ptr/VIP_USUAL_DISTMAP_MULTFACT > *maxdepth)
		{
		    *maxdepth = (float)*ptr/VIP_USUAL_DISTMAP_MULTFACT;
		    *loc = buck->data + i;
		}
	}
    return(OK);
}

/*-------------------------------------------------------------------------*/
static int VipPutBucketInTwoDepth(Vip3DBucket_S16BIT *buck,
			       float *mindepth, float *maxdepth, Volume *depth,
			       VipOffsetStruct *vos)
/*-------------------------------------------------------------------------*/
{
    int i;
    Vip_S16BIT *ptr, *first;

    if(buck==NULL || maxdepth==NULL || depth==NULL || vos==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipAddBucketToGravity");
	    return(PB);
	}

    first = VipGetDataPtr_S16BIT( depth ) + vos->oFirstPoint;
    for(i=0;i<buck->n_points;i++)
	{
	    ptr = first + buck->data[i].x 
		+ buck->data[i].y * vos->oLine
		+ buck->data[i].z * vos->oSlice;
	    if((float)*ptr/VIP_USUAL_DISTMAP_MULTFACT > *maxdepth)
		*maxdepth = (float)*ptr/VIP_USUAL_DISTMAP_MULTFACT;
	    if((float)*ptr/VIP_USUAL_DISTMAP_MULTFACT < *mindepth)
		*mindepth = (float)*ptr/VIP_USUAL_DISTMAP_MULTFACT;
	}
    return(OK);
}

/*-------------------------------------------------------------------------*/
static int VipAddBucketToGravity(Vip3DBucket_S16BIT *buck,Vip3DPoint_VFLOAT *g)
/*-------------------------------------------------------------------------*/
{
    int i;

    if(buck==NULL || g==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipAddBucketToGravity");
	    return(PB);
	}
    for(i=0;i<buck->n_points;i++)
	{
	    g->x += buck->data[i].x;
	    g->y += buck->data[i].y;
	    g->z += buck->data[i].z;
	}
    return(OK);
}
/*-------------------------------------------------------------------------*/
static int  VipPutBucketInBoundingBox(    Vip3DBucket_S16BIT *buck,
    Vip3DPoint_S16BIT *boxmin, Vip3DPoint_S16BIT *boxmax)
/*-------------------------------------------------------------------------*/
{
    int x,y,z;
    int i;

    if(buck==NULL || boxmin==NULL || boxmax==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipPutBucketInBoundingBox");
	    return(PB);
	}
    for(i=0;i<buck->n_points;i++)
	{
	    x = buck->data[i].x;
	    y = buck->data[i].y;
	    z = buck->data[i].z;
	    if(x<boxmin->x) boxmin->x = x;
	    if(y<boxmin->y) boxmin->y = y;
	    if(z<boxmin->z) boxmin->z = z;
	    if(x>boxmax->x) boxmax->x = x;
	    if(y>boxmax->y) boxmax->y = y;
	    if(z>boxmax->z) boxmax->z = z;
	}

    return(OK);

}

/*-------------------------------------------------------------------------*/
static int  VipPutBucketInTalBoundingBox(    Vip3DBucket_S16BIT *buck,
    Vip3DPoint_VFLOAT *boxmin, Vip3DPoint_VFLOAT *boxmax, VipTalairach *tal)
/*-------------------------------------------------------------------------*/
{
    int x,y,z;
    float rx, ry,rz;
    int i;
    float coord[3], refcoord[3];

    if(buck==NULL || boxmin==NULL || boxmax==NULL || tal==NULL)
	{
	    VipPrintfError("empty args");
	    VipPrintfExit("VipPutBucketInTalBoundingBox");
	    return(PB);
	}
    for(i=0;i<buck->n_points;i++)
	{
	    x = buck->data[i].x;
	    y = buck->data[i].y;
	    z = buck->data[i].z;
	    coord[0] = tal->VoxelGeometry.x*x;
	    coord[1] = tal->VoxelGeometry.y*y;
	    coord[2] = tal->VoxelGeometry.z*z;
	    VipTalairachTransformation(coord,tal,refcoord);
	    rx = refcoord[0]*tal->Scale.x;
	    ry = refcoord[1]*tal->Scale.y;
	    rz = refcoord[2]*tal->Scale.z;
	    if(rx<boxmin->x) boxmin->x = rx;
	    if(ry<boxmin->y) boxmin->y = ry;
	    if(rz<boxmin->z) boxmin->z = rz;
	    if(rx>boxmax->x) boxmax->x = rx;
	    if(ry>boxmax->y) boxmax->y = ry;
	    if(rz>boxmax->z) boxmax->z = rz;
	}

    return(OK);

}
/*-------------------------------------------------------------------------*/
static int FillArgBoxAndVoxSize(Volume *skeleton, FoldArg *arg,
				       int inside, int outside)
/*-------------------------------------------------------------------------*/
{
  VipOffsetStruct *vos;
  Vip_S16BIT *skeleton_ptr;
  int x,y,z;

  if(skeleton==NULL || arg==NULL)
    {
      VipPrintfError("Empty Arg");
      VipPrintfExit("FillArgBoxAndVoxSize");
      return(PB);
    }

  arg->vox.x = mVipVolVoxSizeX(skeleton);
  arg->vox.y = mVipVolVoxSizeY(skeleton);
  arg->vox.z = mVipVolVoxSizeZ(skeleton);

  arg->min_ima.x = 10000;
  arg->min_ima.y = 10000;
  arg->min_ima.z = 10000;
  arg->max_ima.x = -10000;
  arg->max_ima.y = -10000;
  arg->max_ima.z = -10000;

  vos = VipGetOffsetStructure(skeleton);
  if(!vos) return(PB);

  skeleton_ptr = VipGetDataPtr_S16BIT(skeleton)+vos->oFirstPoint;
  
  for(z=0;z<mVipVolSizeZ(skeleton);z++)
    {
      for(y=0;y<mVipVolSizeY(skeleton);y++)
	{
	  for(x=0;x<mVipVolSizeX(skeleton);x++)
	    {
	      if(*skeleton_ptr!=inside && *skeleton_ptr!=outside)
		{
		  if(x<arg->min_ima.x) arg->min_ima.x = x;
		  if(y<arg->min_ima.y) arg->min_ima.y = y;
		  if(z<arg->min_ima.z) arg->min_ima.z = z;
		  if(x>arg->max_ima.x) arg->max_ima.x = x;
		  if(y>arg->max_ima.y) arg->max_ima.y = y;
		  if(z>arg->max_ima.z) arg->max_ima.z = z;
		}
	      skeleton_ptr++;
	    }
	  skeleton_ptr+=vos->oPointBetweenLine;
	}
      skeleton_ptr += vos->oLineBetweenSlice;
    }

  return(OK);
}

/*-------------------------------------------------------------------------*/
FoldArg *CreateEmptyFoldArg(char *name)
/*-------------------------------------------------------------------------*/
{
  FoldArg *arg;

  arg = (FoldArg *)VipCalloc(1,sizeof(FoldArg),"CreateEmptyFoldArg");

  if (arg==PB) return(PB);

  strcpy(arg->name,name);
  arg->ss_list = NULL;
  arg->jset = NULL;
  arg->cortex_jset = NULL;
  arg->gotoref_filled = VFALSE;
  arg->gonetoref = VFALSE;
  arg->CAfilled = VFALSE;
  arg->CPfilled = VFALSE;
  arg->IHfilled = VFALSE;
  
  arg->min_ima.x=0;
  arg->min_ima.y=0;
  arg->min_ima.z=0;
  arg->max_ima.x=0;
  arg->max_ima.y=0;
  arg->max_ima.z=0;

  arg->CA.x=0;
  arg->CA.y=0;
  arg->CA.z=0;
  arg->CP.y=0;
  arg->CP.y=0;
  arg->CP.z=0;
  arg->IH.y=0;
  arg->IH.y=0;
  arg->IH.z=0;

  return(arg);
}
