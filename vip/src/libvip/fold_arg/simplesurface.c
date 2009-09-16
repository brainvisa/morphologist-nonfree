/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 02/03/1999
 * VERSION     : 1.4                  * REVISION :
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
#include <vip/topology.h>
#include <vip/util.h>
#include <vip/distmap.h> 
#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/util/file.h>

#ifdef VIP_CARTO_VOLUME_WRAPPING
#include <vip/fold_arg/mesh_aims.h>
#endif

/*-------------------------------------------------------------------------*/
static int  CreateOtherBucket( SurfaceSimpleList *sslist, Volume *vol,
				     int inside, int outside);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
 static int PruneSmallSimpleSurfaces( SurfaceSimpleList *sslist, Volume *vol, int limitsize);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipUnhookSimpleSurfaceFromList(
					  SurfaceSimple *ToUnhook,
					  SurfaceSimpleList *sslist);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static int WriteSSLabelInVol(  Vip3DBucket_S16BIT *bucklist,Volume *vol);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int PutOutsideBorderToHullLabel(Volume *squel, 
				       int inside, int outside, int hull);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int PutJunctionPointToValue(Volume *vol,  Vip3DBucket_S16BIT *bucklist,
			    int inside, int outside);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int NewSSVoronoiAndRecoverSmallOldSS( Vip3DBucket_S16BIT *bucklist, 
Vip3DBucket_S16BIT *bucklist2, Volume *vol, int inside, int outside);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipDilateForbidenJunctionBorderAndCurve( Volume *vol);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static Vip3DBucket_S16BIT *SplitSSAlongPliDePassage(Vip3DBucket_S16BIT *bucklist2,
						    Volume *rootsvoronoi,
						    int **potentialpp, 
						    int *npotentialpp, 
						    int **rootsbassin, 
						    int *nbucket);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static int PutPotentialPPIntoSSList( SurfaceSimpleList *sslist, int *potentialpp,
				     int npotentialpp, int *rootsbassin, int nbucket);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
SurfaceSimpleList *VipComputeFoldArgSSList( Volume *squel , int inside, int outside, int limitsize, char *rootsvoronoi_name)   
/*-------------------------------------------------------------------------*/
  /*input: simple skeleton without topological classification*/
{
 Vip3DBucket_S16BIT *bucklist, *bucklist2, *bucklistpp;
 SurfaceSimpleList *sslist;
 int *potentialpp=NULL;
 int npotentialpp=0;
 int *rootsbassin = 0;
 int nbucket = 0;
 Volume *rootsvoronoi=NULL;
 
 if(squel==PB)
    {
      VipPrintfError("Empty Arg");
      VipPrintfExit("VipComputeFoldArgSSList");
      return(PB);
    }

 if(limitsize > 1000)
     {
	 VipPrintfError("strange limit on ss size");
	 VipPrintfExit("VipComputeFoldArgSSList");
	 return(PB);
     }
 
 printf("----------------------------\n");
 printf("Computing simple surfaces...\n");
 printf("----------------------------\n");
 

 printf("Topological classification of the skeleton...\n");
 if(VipTopologicalClassificationForTwoLabelComplement(squel,inside,outside)==PB) return(PB);

 /*delete all the points 6-connected to the outside domain which will 
   constitute the HULL_SURFACE*/

 printf("Hull detection...\n");

 if(PutOutsideBorderToHullLabel(squel, inside, outside, HULL_SURFACE)==PB) return(PB);

 /*the simple surfaces will be equivalence classes in the skeleton subset:
   TOPO_JUNCTION_SURFACE_CURVES_POINT U TOPO_SURFACE_POINT*/


 if(VipChangeIntLabel(squel,TOPO_JUNCTION_SURFACE_CURVES_POINT,TOPO_SURFACE_POINT)==PB) return(PB);

 printf("First level of topologically simple surfaces (cortical folds)...\n");

 bucklist = GetSurfaceSimple( squel, TOPO_SURFACE_POINT, inside, outside);
 if(bucklist==PB) return(PB);

 /*We try to split large ss at the level of junctions*/
 printf("Second level of topologically simple surfaces (junction points forbidden)...\n");

 if(WriteSSLabelInVol(bucklist, squel)==PB) return(PB);

 if( PutJunctionPointToValue(squel,  bucklist, inside, outside)==PB) return(PB);

 if( VipDilateForbidenJunctionBorderAndCurve( squel)==PB) return(PB);

 bucklist2 = GetSurfaceSimple( squel, TOPO_SURFACE_POINT, inside, outside);
 if(bucklist2==PB) return(PB);

 if(WriteSSLabelInVol(bucklist2, squel)==PB) return(PB);

 NewSSVoronoiAndRecoverSmallOldSS( bucklist, bucklist2, squel, inside, outside);

 VipChangeIntLabel(squel, FORBIDEN_JUNCTION, TOPO_SURFACE_POINT);

 if(rootsvoronoi_name!=NULL)
   {
     rootsvoronoi = VipReadVolume(rootsvoronoi_name);
     if(rootsvoronoi==PB) return(PB);

     printf("Splitting ss according to plis de passage\n");
     bucklistpp = SplitSSAlongPliDePassage(bucklist2, rootsvoronoi,
					   &potentialpp, &npotentialpp, &rootsbassin, &nbucket);
     if(bucklistpp==PB) return(PB);
     
     if(potentialpp==NULL || rootsbassin==NULL)
       {
	 VipPrintfError("Strange split result");
	 VipPrintfExit("VipComputeFoldArgSSList");
	 return(PB);
       }
     
     VipFreeVolume(rootsvoronoi);
     bucklist2 = bucklistpp;
   }

 if(WriteSSLabelInVol(bucklist2, squel)==PB)
   {
     VipPrintfExit("VipComputeFoldArgSSList");
     return(PB);
   }
 sslist = CreateSurfaceSimpleList( bucklist2);
 if(sslist==PB) return(PB);

 if(rootsvoronoi_name!=NULL)
   {
     if(  PutPotentialPPIntoSSList( sslist, potentialpp, npotentialpp, rootsbassin, nbucket )==PB) return(PB);
     free(potentialpp);
     free(rootsbassin);
   }

 printf("Simple surface borders (fold bottoms)... \n");

 if(FillSurfaceSimpleEdgeBucket(sslist, squel, TOPO_BORDER_POINT)==PB) return(PB);

 printf("Pruning small simple surfaces (ss points + edge points) < %d\n",limitsize);
 if(PruneSmallSimpleSurfaces(sslist, squel, limitsize)==PB) return(PB);

 if(rootsvoronoi_name!=NULL)
   nbucket = sslist->n_ss;

 CreateOtherBucket( sslist, squel, inside, outside);

 return(sslist); 
}

/*-------------------------------------------------------------------------*/
static int VipDilateForbidenJunctionBorderAndCurve( Volume *vol)
/*-------------------------------------------------------------------------*/
{
  int i, j, k, loop, *offsetptr, xsize, ysize, zsize;
  Vip_S16BIT *imptr, *voisin;
  VipConnectivityStruct *vcs;
  VipOffsetStruct *vos;
  int n_points = 0;
  int TOUCHED = -432; 

  vos = VipGetOffsetStructure(vol);
  imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);

  vcs = VipGetConnectivityStruct(vol, CONNECTIVITY_26);
 
  for (k=0; k<zsize; k++, imptr+=vos->oLineBetweenSlice)
    for (j=0; j<ysize; j++, imptr+=vos->oPointBetweenLine)
      for (i=0; i<xsize; i++, imptr++)
      { 
	if(*imptr==FORBIDEN_JUNCTION || *imptr==TOPO_BORDER_POINT 
	   || *imptr==TOPO_CURVE_POINT)
	  {
	    offsetptr = vcs->offset;
	    for (loop=0; loop<vcs->nb_neighbors; loop++)
	      {
		voisin = imptr+*offsetptr++;
		if(*voisin==TOPO_SURFACE_POINT)
		  {
		    n_points++;
		    *voisin=TOUCHED;
		  }
	      }
	  }
      }

  VipChangeIntLabel(vol,TOUCHED,FORBIDEN_JUNCTION);
  printf("Junction and border dilations add %d forbidden points\n",n_points);
  return(OK);
}

/*-------------------------------------------------------------------------*/
static int PutPotentialPPIntoSSList( SurfaceSimpleList *sslist, int *potentialpp,
				     int npotentialpp, int *rootsbassin, int nbucket) 
/*-------------------------------------------------------------------------*/
{
  int i;
  int index1, index2;
  int count;

  if(sslist==NULL || potentialpp==NULL || rootsbassin==NULL)
    {
      VipPrintfError("Empty arg");
      VipPrintfExit("PutPotentialPPIntoSSList");
      return(PB);
    }

  if(nbucket!=sslist->n_ss)
    {
      VipPrintfError("incoherence");
      VipPrintfExit("PutPotentialPPIntoSSList");
      return(PB);
    }

  for(i=0;i<npotentialpp;i++)
    {
      index1 = potentialpp[2*i];
      index2 = potentialpp[2*i+1];
      if(index1==index2)
	{
	  VipPrintfError("= indices!");
	  VipPrintfExit("PutPotentialPPIntoSSList");
	  return(PB);
	}
      if(index1>=nbucket || index2>=nbucket || index1<0 || index2<0)
	{
	  VipPrintfError("incoherent index");
	  VipPrintfExit("PutPotentialPPIntoSSList");
	  return(PB);
	}
      sslist->tab[index1]->npotentialpp++; 
      sslist->tab[index2]->npotentialpp++;
    }

  for(i=1;i<=nbucket;i++)
    {
      sslist->tab[i]->pppotentialneighbor_index = (int *)VipCalloc(sslist->tab[i]->npotentialpp,
								   sizeof(int),
								   "PutPotentialPPIntoSSList");
      if(!sslist->tab[i]->pppotentialneighbor_index) return(PB);
      sslist->tab[i]->npotentialpp = 0;
      sslist->tab[i]->rootsbassin = rootsbassin[i];
      sslist->tab[i]->rootsbassin_filled = VTRUE;
    }

  count=0;
  for(i=0;i<npotentialpp;i++)
    {
      index1 = potentialpp[2*i];
      index2 = potentialpp[2*i+1];
      sslist->tab[index1]->pppotentialneighbor_index[sslist->tab[index1]->npotentialpp++]
	= index2;
      sslist->tab[index2]->pppotentialneighbor_index[sslist->tab[index2]->npotentialpp++]
	= index1;
      count += 1;
    }

  return(OK);
}
/*-------------------------------------------------------------------------*/
static Vip3DBucket_S16BIT *SplitSSAlongPliDePassage(Vip3DBucket_S16BIT *bucklist2,
						    Volume *rootsvoronoi,
						    int **potentialpp, 
						    int *npotentialpp, 
						    int **rootsbassin, 
						    int *nbucket)
/*-------------------------------------------------------------------------*/
{
    Vip3DBucket_S16BIT *newbucklist = NULL;
    Vip3DBucket_S16BIT thefirst, *hook, *killer;
    Vip3DBucket_S16BIT **bucktab;
    TouchBassin *tbassin;
    Vip3DBucket_S16BIT *bptr;
    int nbassin;
    int i,j,k;
    VipOffsetStruct *vos;
    Vip_S16BIT *ptr, *first;
    int killed;
    int created;
    int countpotpp;
    int splittedmaxsize;
    int jmax;

    if(bucklist2==PB || rootsvoronoi==PB ||
       potentialpp==PB || npotentialpp==PB || rootsbassin==PB || nbucket==PB)
	{
	    VipPrintfError("Empty arg");
	    VipPrintfExit("SplitSSAlongPliDePassage");
	    return(PB);
	}

    *nbucket=0;
    bptr = bucklist2;
    while(bptr!=NULL)
	{
	    (*nbucket)++;
	    bptr = bptr->next;
	}

    tbassin = (TouchBassin *)VipCalloc(*nbucket,sizeof(TouchBassin),"SplitSSAlongPliDePassage");
    if(!tbassin) return(PB);

    nbassin =  VipGetVolumeMax(rootsvoronoi)+1; /*we use bassin label as index*/
    printf("Dispatching %d ss into catchment bassins\n",*nbucket);

    for(i=0;i<*nbucket;i++)
	{
	    tbassin[i].n_bassins = 0;
	    tbassin[i].lab = (int *)VipCalloc(nbassin,sizeof(int),"SplitSSAlongPliDePassage");
	    if(tbassin[i].lab==NULL) return(PB);
	    tbassin[i].size = (int *)VipCalloc(nbassin,sizeof(int),"SplitSSAlongPliDePassage");
	    if(tbassin[i].size==NULL) return(PB);
	}

    bucktab = ( Vip3DBucket_S16BIT **)VipCalloc(nbassin,sizeof( Vip3DBucket_S16BIT *),"SplitSSAlongPliDePassage");
    if(!bucktab) return(PB);

    vos = VipGetOffsetStructure(rootsvoronoi);
    if(!vos) return(PB);
    first = VipGetDataPtr_S16BIT( rootsvoronoi ) + vos->oFirstPoint;

    bptr = bucklist2;
    for(i=0;i<*nbucket;i++)
	{

	    for(j=0;j<bptr->n_points;j++)
		{
		    ptr = first + bptr->data[j].x + bptr->data[j].y * vos->oLine 
			+ bptr->data[j].z * vos->oSlice; 
		    if(*ptr>0)
			{
			    if(*ptr>nbassin-1)
				{
				    VipPrintfError("Strange value");
				    VipPrintfExit("SplitSSAlongPliDePassage");
				    return(PB);
				}
			    for(k=0;k<tbassin[i].n_bassins;k++)
				{
				    if(tbassin[i].lab[k]==*ptr)
					{
					    tbassin[i].size[k]++; /*already found*/
					    break;
					}
				}
			    if(k==tbassin[i].n_bassins) /*new bassin*/
				{
				    tbassin[i].lab[tbassin[i].n_bassins] = *ptr;
				    tbassin[i].size[(tbassin[i].n_bassins)++] = 1;
				}
			}
		    else
			{
			    printf("lab: %d\n",*ptr);
			    VipPrintfWarning("Skeleton point outside all catchment bassins?");
			}
		}
	    splittedmaxsize = 0;
	    jmax = 0;
	    for(j=0;j<tbassin[i].n_bassins;j++)
		if (tbassin[i].size[j]>splittedmaxsize)
		    {
			splittedmaxsize=tbassin[i].size[j];
			jmax = j;
		    }
	    if(splittedmaxsize < MIN_SIZE_BASSIN_SPLITTED)
		{
		    tbassin[i].n_bassins = 1;
		    tbassin[i].size[0] = bptr->n_points;
		    tbassin[i].lab[0] = tbassin[i].lab[jmax];
		}	    
	    bptr = bptr->next;
	}

    *npotentialpp = 0;
    killed = 0;
    bptr = bucklist2;
    created = 0;
    for(i=0;i<*nbucket;i++)
	{
	    if(tbassin[i].n_bassins>1)
		{
		    /*
		      printf("buck %d: %d bassins, size:%d | ",i,tbassin[i].n_bassins,bptr->n_points);
		      for(k=0;k<tbassin[i].n_bassins;k++)
		      printf("%d:%d ",tbassin[i].lab[k], tbassin[i].size[k]);
		      printf("\n");
		    */
		    killed++;
		    created += tbassin[i].n_bassins;
		    *npotentialpp += tbassin[i].n_bassins * ( tbassin[i].n_bassins-1) /2;   
		}
	    bptr = bptr->next;
	}

    *potentialpp = (int *)VipCalloc(*npotentialpp * 2,sizeof(int),"SplitSSAlongPliDePassage");
    if(!*potentialpp) return(PB); /*future list of paired splitted buckets*/
  
    *rootsbassin = (int *)VipCalloc(*nbucket-killed+created+1,sizeof(int),"SplitSSAlongPliDePassage");
    if(!*rootsbassin) return(PB); /*more often cited bassin in one bucket*/

    hook = &thefirst;
    bptr = bucklist2;
    for(j=0;j<nbassin;j++) bucktab[j]=NULL;
    created = 0;
    countpotpp = 0;
    for(i=0;i<*nbucket;i++)
	{
	    if(tbassin[i].n_bassins>1)
		{
		    for(k=0;k<tbassin[i].n_bassins;k++)
			{
			    bucktab[tbassin[i].lab[k]]=VipAlloc3DBucket_S16BIT( tbassin[i].size[k] );
			    if(!bucktab[tbassin[i].lab[k]]) return(PB);
			    (*rootsbassin)[++created]=tbassin[i].lab[k]; 
			    for(j=0;j<k;j++)
				{
				    /*printf("(%d,%d)=(%d,%d)",2*countpotpp,2*countpotpp+1,created-(k-j),created);*/
				    (*potentialpp)[2*countpotpp] = created-(k-j);
				    (*potentialpp)[2*countpotpp+1] = created;
				    countpotpp++;
				}
			    /* printf("created: %d, countpotpp: %d/%d\n",created, countpotpp,*npotentialpp);*/
			}
		    for(j=0;j<bptr->n_points;j++)
			{
			    ptr = first + bptr->data[j].x + bptr->data[j].y * vos->oLine 
				+ bptr->data[j].z * vos->oSlice; 
			    if(*ptr>0)
				{
				    bucktab[*ptr]->data[bucktab[*ptr]->n_points].x = bptr->data[j].x;
				    bucktab[*ptr]->data[bucktab[*ptr]->n_points].y = bptr->data[j].y;
				    bucktab[*ptr]->data[bucktab[*ptr]->n_points].z = bptr->data[j].z;
				    bucktab[*ptr]->n_points++;
				}
			}
		    for(j=0;j<nbassin;j++) /*create list of splitted bucket*/
			{
			    if(bucktab[j]!=NULL)
				{
				    if(bucktab[j]->size != bucktab[j]->n_points)
					{
					    VipPrintfError("Bad filling");
					    VipPrintfExit("SplitSSAlongPliDePassage");
					    return(PB);
					}
				    hook->next = bucktab[j];
				    hook = hook->next;
				    bucktab[j] = NULL;
				}
			}
		} 
	    bptr = bptr->next;
	}
    if(countpotpp!=*npotentialpp)
	{
	    printf("%d versus %d\n",countpotpp, *npotentialpp);
	    VipPrintfError("Bad filling of potential pli de passage");
	    VipPrintfExit("SplitSSAlongPliDePassage");
	    return(PB);
	}

    bptr = bucklist2; /*destroy old splitted bucket*/
    killed = 0;
    for(i=0;i<*nbucket;i++)
	{
	    if(tbassin[i].n_bassins>1)
		{
		    killer = bptr;
		    killed++;
		}
	    else
		{
		    killer=NULL;
		    (*rootsbassin)[++created] = tbassin[i].lab[0];
		}
	    bptr = bptr->next;
	    if(killer!=NULL)
		{
		    bucklist2 = VipUnhook3DBucketFromList_S16BIT( killer, bucklist2);
		    VipFree3DBucket_S16BIT(killer);
		}
	}

    printf("%d/%d ss have been split into %d new ss\n",killed,*nbucket,(created-*nbucket+killed));

    hook->next = bucklist2; /*concatenation*/

    for(i=0;i<*nbucket;i++)
	{
	    free(tbassin[i].lab);
	    free(tbassin[i].size);
	}
    free(tbassin);
    free(bucktab);

    newbucklist = thefirst.next;
    *nbucket = created;

    return(newbucklist);
}


/*-------------------------------------------------------------------------*/
static int NewSSVoronoiAndRecoverSmallOldSS( Vip3DBucket_S16BIT *bucklist, 
					     Vip3DBucket_S16BIT *bucklist2,
					     Volume *vol, int inside, int outside)
/*-------------------------------------------------------------------------*/
{
    Vip_S16BIT *ptr=NULL, *first, *neighbor;
    VipOffsetStruct *vos;
    Vip3DBucket_S16BIT *bptr1, *killer, *bptr2;
    int i;
    int some_in_newss;
    VipConnectivityStruct *vcs;
    Topology26Neighborhood *topo26;
    VipS16BITPtrBucket *comp1, *comp2, *comp1bis, *comp2bis;
    int n;
    int C18;
    int old_small_ss=0;
    int old_n_points, old_n_points2;
    int has_moved[10000]; /*max number of bucket*/
    int current_bucket=0;
    int global_has_moved = VTRUE;
    int n_remaining_point;
    int totalfill;
			  
    if (VipVerifyAll(vol)==PB || bucklist==PB)
	{
	    VipPrintfError("Bad arg");
	    VipPrintfExit("NewSSVoronoiAndRecoverSmallOldSS");
	    return(PB);
	}

    vos = VipGetOffsetStructure(vol);
    if(!vos) return(PB);

    topo26 = VipCreateTopology26Neighborhood( vol);
    if(topo26==PB) return(PB);

    comp1 = VipAllocS16BITPtrBucket(18);
    comp2 = VipAllocS16BITPtrBucket(18);
    comp1bis = VipAllocS16BITPtrBucket(18);
    comp2bis = VipAllocS16BITPtrBucket(18);
    if(comp1==NULL || comp2==NULL || comp1bis==NULL || comp2bis==NULL) return(PB);

    vcs = VipGetConnectivityStruct(vol,CONNECTIVITY_26);
    if(!vcs) return(PB);

    first = VipGetDataPtr_S16BIT(vol) + vos->oFirstPoint;
  
    /*   printf("sizes: ");*/
    bptr1 = bucklist;
    /*boucle sur les old ss, les points marques jonction qui respectent le
      critere de ss-connexite avec une nouvelle ss sont marques: label+5*/
    while(bptr1!=NULL)
	{
	    some_in_newss = VFALSE;
	    for(i=0;i<bptr1->n_points;i++)
		{
		    ptr = first + bptr1->data[i].x + bptr1->data[i].y * vos->oLine 
			+ bptr1->data[i].z * vos->oSlice; 
		    if(*ptr>HULL_SURFACE) /*the point is in the ss set*/
			{
			    some_in_newss = VTRUE;
			    break;
			}
		}
	    if(some_in_newss==VTRUE) /*deletion*/
		{
		    killer = bptr1;
		    bptr1 = bptr1->next;
		    bucklist = VipUnhook3DBucketFromList_S16BIT(killer,bucklist);
		    VipFree3DBucket_S16BIT(killer);
		}
	    else
		{ /*recover*/
		    /*  printf("%d ",bptr1->n_points);*/
		    if(VipWriteCoordBucketInVolume(bptr1, vol, TOPO_SURFACE_POINT)==PB) return(PB);
		    old_small_ss++;
		    bptr1 = bptr1->next;
		}
	}
    /*  printf("\n");*/
    printf("Old small ss deleted by erosion recovered: %d\n",old_small_ss);

    bptr2 = bucklist2; /*increase the bucket sizes*/
    current_bucket=0;
    while(bptr2!=NULL)
	{
	    has_moved[current_bucket++]=VTRUE;
	    old_n_points = bptr2->n_points;
	    bptr2->n_points += 10000;
	    if(VipDwindle3DBucket_S16BIT(bptr2)==PB) return(PB); /*realloue le buffer data*/
	    bptr2->n_points = old_n_points;
	    bptr2 = bptr2->next;
	}

    global_has_moved = VTRUE;
    while(global_has_moved==VTRUE) /*dilation using the ss-connectivity to get a Voronoi*/
	{
	  totalfill = 0;
	    bptr2 = bucklist2;
	    global_has_moved = VFALSE;
	    current_bucket=0;
	    while(bptr2!=NULL) 
		{
		    if(has_moved[current_bucket]==VTRUE)
			{
			    old_n_points = bptr2->n_points;
			    has_moved[current_bucket] = VFALSE;
			    for(i=0;i<old_n_points;i++)
				{
				    ptr = first + bptr2->data[i].x + bptr2->data[i].y * vos->oLine 
					+ bptr2->data[i].z * vos->oSlice;
				    C18 = VipComputeCbarForTwoLabelComplement_S16BIT(topo26, ptr,
										     inside, outside);
				    if (C18!=2)
					{
					    VipPrintfError("This is not a simple surface point (inconsistence)\n");
					    VipPrintfExit("NewSSVoronoiAndRecoverSmallOldSS");
					    return(PB);
					}
				    VipFillSSComponent( comp1, comp2, topo26, ptr);
				    for(n=0;n<vcs->nb_neighbors;n++)
					{
					    neighbor = ptr + vcs->offset[n];
					    if(*neighbor==(FORBIDEN_JUNCTION))
						{
						    C18 = VipComputeCbarForTwoLabelComplement_S16BIT(topo26, neighbor,
												     inside, outside);
						    if (C18!=2)
							{
							    printf("This is not a simple surface point (inconsistence)\n");
							    VipPrintfExit("NewSSVoronoiAndRecoverSmallOldSS");
							    return(PB);
							}				
						    VipFillSSComponent( comp1bis, comp2bis, topo26, neighbor);
						    if(VipSurfaceSimpleEquivalence( comp1, comp2,
										    comp1bis,comp2bis)==VTRUE)
							{
							    has_moved[current_bucket]=VTRUE;
							    global_has_moved = VTRUE;
							    *neighbor = *ptr;
							    if(bptr2->n_points == bptr2->size)
								{
								    VipPrintfWarning("Reallocating bucket");
								    old_n_points2 = bptr2->n_points;
								    bptr2->n_points += 10000;
								    if(VipDwindle3DBucket_S16BIT(bptr2)==PB) return(PB); 
								    bptr2->n_points = old_n_points2;
								}

							    bptr2->data[bptr2->n_points].x = bptr2->data[i].x+vcs->xyz_offset[n].x;
							    bptr2->data[bptr2->n_points].y = bptr2->data[i].y+vcs->xyz_offset[n].y;
							    bptr2->data[bptr2->n_points].z = bptr2->data[i].z+vcs->xyz_offset[n].z; 
							    bptr2->n_points++;
							    totalfill++;
							}
						}
					}
				}
			}
		    bptr2 = bptr2->next;
		    current_bucket++;
		}
	    /* printf("%d old ss points added to new ss\n", totalfill);*/

	}

    bptr2 = bucklist2; /*correct the bucket sizes*/
    while(bptr2!=NULL)
	{
	    if(VipDwindle3DBucket_S16BIT(bptr2)==PB) return(PB); /*realloue le buffer data*/
	    bptr2 = bptr2->next;
	}


    bptr2 = bucklist2;
    while(bptr2->next!=NULL) /*concatenation*/
	{
	    bptr2 = bptr2->next;
	}
    bptr2->next = bucklist;

    n_remaining_point = VipGetNumberLabelPoints(vol, FORBIDEN_JUNCTION);
    /*    printf("%d forbidden have not been touched by the ss-connectivity based dilation\n",n_remaining_point);*/

    VipFreeS16BITPtrBucket(comp1);
    VipFreeS16BITPtrBucket(comp1bis);
    VipFreeS16BITPtrBucket(comp2);
    VipFreeS16BITPtrBucket(comp2bis);
    return(OK);
}


/*-------------------------------------------------------------------------*/
static int PutJunctionPointToValue(Volume *vol,  Vip3DBucket_S16BIT *bucklist,
			    int inside, int outside)
/* touch junction points (forbidden) for a second step of SS detection*/
/*-------------------------------------------------------------------------*/

{
  Vip_S16BIT *ptr;
  int i,j;
  int n_junction_points;
  int center, temp;
  int n_ss=0;
  VipOffsetStruct *vos;
  VipConnectivityStruct *vcs;
  int firstlab;

  if (VipVerifyAll(vol)==PB || bucklist==PB)
    {
      VipPrintfError("Bad arg");
      VipPrintfExit("PutJunctionPointToValue");
      return(PB);
    }
    
  if(mVipVolBorderWidth(vol)==0)
    {
      VipPrintfError("no zero border width required by GetListOfJunctionPoint");
      VipPrintfExit("PutJunctionPointToValue");
      return(PB);
    }
    
  vos = VipGetOffsetStructure(vol);
  if(!vos) return(PB);
  vcs = VipGetConnectivityStruct(vol,CONNECTIVITY_26);
  if(!vcs) return(PB);
 
  printf("Forbid junction points...\n");

  firstlab = 0;
  n_junction_points = 0;
  ptr = VipGetDataPtr_S16BIT(vol);
  for(i=vos->oVolume; i>0; i--)
    {
      center = *ptr;
      if(center!=inside && center!=outside)
	{
	  n_ss = 0;
	  if(center>HULL_SURFACE) /*We do not put HULL_SURFACE point to FORBIDEN_JUNCTION*/
	    {
	      if(center%10!=0)
		{
		  VipPrintfWarning("Border should not be marked there?");
		}
	      center -= center%10;
	      n_ss=1;
	      firstlab=center;

	      for(j=0;j<vcs->nb_neighbors;j++)
		{
		  temp = *(ptr+vcs->offset[j]);
		  if(temp>=HULL_SURFACE)
		    {
		      if(temp%10!=0)
			{
			  VipPrintfWarning("Border should not be marked there?");
			}
		      temp -= temp%10;
		      if(n_ss==0)
			{
			  n_ss=1;
			  firstlab=temp;
			}
		      else if (temp!=firstlab)
			{
			  n_ss++;
			  n_junction_points++;				    
			  break;
			}
		    }
		}	
	      if(n_ss==2) *ptr = FORBIDEN_JUNCTION;
	      else if(*ptr!=HULL_SURFACE) *ptr = TOPO_SURFACE_POINT;
	    }
	}
      ptr++;
    }

  printf("%d points\n",n_junction_points);

  return(OK);
    
}
/*-------------------------------------------------------------------------*/
int VipComputeTmtkTriangulation( Volume *skeleton, FoldArg *arg)
/*-------------------------------------------------------------------------*/
{
    Volume *fortriang;
    int index=0;
    int value = 255;
    int n;
    SurfaceSimple **tab; /*WARNING, tab is of size n_ss + 1, 0 = HULL*/
    char filename[VIP_NAME_MAXLEN];
    char tempimagename[VIP_NAME_MAXLEN];
    char shortname[VIP_NAME_MAXLEN];
    char tmpdir[VIP_NAME_MAXLEN];
    char *root1, *root2, *tmpdir2;
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif

    if(skeleton==NULL || arg==NULL)
      {
        VipPrintfError("Empty Arg");
        VipPrintfExit("VipComputeTmtkTriangulation");
        return(PB);
      }

    /* retreive a valid temp directory */
    tmpdir2 = getenv( "TEMP" );
    if( tmpdir2 == NULL )
      strcpy( tmpdir, VipTmpDirectory() ); 
    else
      strcpy( tmpdir, tmpdir2 );

    root2 = arg->name;
    root1 = strrchr( root2, '/' );
#ifdef _WIN32
    root2 = strrchr( root2, '\\' );
    if( root2 != NULL && (root1 == NULL || root1 < root2 ) )
      root1 = root2;
#endif
    if( root1 != NULL )
      ++root1;
    else
      root1 = arg->name;
    strcpy(shortname,root1);   

    printf("----------------------------------------------\n");
    printf("Computing triangulations using AIMS library...\n");
    printf("----------------------------------------------\n");

    sprintf(filename,"%s.data",arg->name);
    VipMkdir(filename);

    sprintf(filename,"%s.data%cTmtk",arg->name, sep);
    VipMkdir(filename);

    fortriang = VipDuplicateVolumeStructure(skeleton,"fortriang");
    if(VipAllocateVolumeData(fortriang)==PB) return(PB);

    printf("remaining ss %3d",arg->ss_list->n_ss-index+1);


    tab = arg->ss_list->tab;
    for(index=1;index<=arg->ss_list->n_ss;index++)
	{
	  /*	    printf("-----------------------------\n");
	    printf("still %d ss for triangulation\n",arg->ss_list->n_ss-index+1);
	    printf("-----------------------------\n");
	  */
	  printf("\b\b\b%3d",arg->ss_list->n_ss-index+1);

	    fflush(stdout);
	    VipSetVolumeLevel(fortriang,0); /*negligeable devant l'ecriture...*/
	    if(VipWriteCoordBucketInVolume(tab[index]->surface_points,
					   fortriang, value)==PB) return(PB);
	    if(VipWriteCoordBucketInVolume(tab[index]->edge_points,
					   fortriang, value)==PB) return(PB);
	    if(tab[index]->other_points!=NULL)
		if(VipWriteCoordBucketInVolume(tab[index]->other_points,
					       fortriang, value)==PB) return(PB);
	    if(tab[index]->hull_bijunction!=NULL)
		if(VipWriteCoordBucketInVolume(tab[index]->hull_bijunction->points,
						fortriang, value)==PB) return(PB);

	    for(n=0;n<tab[index]->n_bijunction;n++)
		{
		    if(VipWriteCoordBucketInVolume(tab[index]->bijunction[n]->points,
						fortriang, value)==PB) return(PB);
		}
	    for(n=0;n<tab[index]->n_plidepassage;n++)
		{
		    if(VipWriteCoordBucketInVolume(tab[index]->plidepassage[n]->points,
						fortriang, value)==PB) return(PB);
		}
	    if(VipConnexVolumeFilter(fortriang,CONNECTIVITY_18,-1,CONNEX_BINARY)==PB)
	       return(PB);

#ifdef VIP_CARTO_VOLUME_WRAPPING
            sprintf( tempimagename, "%s.data%cTmtk%c%s-tmt%d", arg->name, sep,
                     sep, shortname, index );
            meshAims( fortriang, tempimagename );

#else

	    sprintf(tempimagename,"%s%c%s-mesh%d",tmpdir,sep,shortname,index);
	    VipWriteTivoliVolume(fortriang,tempimagename);
#ifdef _WIN32
	    sprintf(command,"AimsMeshBrain -i \"%s.ima\" -o " 
                    "\"%s.data%cTmtk%c%s-tmt%d\"",
		    tempimagename,arg->name,sep,sep,shortname,index);
#else
	    sprintf(command,"AimsMeshBrain -i \"%s.ima\" -o " 
                    "\"%s.data/Tmtk/%s-tmt%d\" >/dev/null",
		    tempimagename,arg->name,shortname,index);
#endif
	    system(command);

            sprintf( command, "%s.ima", tempimagename );
            VipUnlink( command );
            sprintf( command, "%s.dim", tempimagename );
            VipUnlink( command );
#endif
	}
    printf("\n");

    VipFreeVolume(fortriang);

    return(OK);

}

/*-------------------------------------------------------------------------*/
static int  CreateOtherBucket( SurfaceSimpleList *sslist, Volume *squel,
				     int inside, int outside)
/*-------------------------------------------------------------------------*/
{
    Volume *voronoi, *domain;
    int domainvalue = 100;

    printf("--------------------------------------------------------------\n");
    printf("Computing voronoi to dispatch small ss (visualisation purpose)\n");
    printf("--------------------------------------------------------------\n");
    printf("Initialization...\n");
	   
    domain = VipCopyVolume(squel, "domain");
    if(!domain) return(PB);
    VipChangeIntLabel(domain, inside, 0);
    VipChangeIntLabel(domain, outside, 0);
    VipChangeIntLabel(domain, HULL_SURFACE, 0);
    VipChangeIntRange(domain, VIP_BETWEEN, 0, HULL_SURFACE, domainvalue);

    voronoi = VipComputeFrontPropagationGeodesicVoronoi(domain,domainvalue,0);
    if(voronoi==PB) return(PB);
    VipFreeVolume(domain);
    
    if(FillSurfaceSimpleOtherBucket(sslist, squel, voronoi)==PB) return(PB);

    VipFreeVolume(voronoi);

    return(OK);
}

/*-------------------------------------------------------------------------*/
int FillSurfaceSimpleOtherBucket(
SurfaceSimpleList *ss_list,
Volume *squel,
Volume *voronoi)
/*-------------------------------------------------------------------------*/
     /*remplit les buckets other en fonction du voronoi*/
{
    int i;
    Vip_S16BIT *squel_ptr, *voronoi_ptr;
    SurfaceSimple *walker;
    VipOffsetStruct *vos, *vvos;
    int *sizebuf;
    int temp;
    int BORNESUP;
    int x, y, z;
    Vip3DBucket_S16BIT *current;

    if(!squel || !voronoi || !ss_list)
	{
	    VipPrintfError("Empty args");
	    VipPrintfExit("FillSurfaceSimpleOtherBucket");
	    return(PB);
	}

    if(mVipVolBorderWidth(squel)==0)
	{
	    VipPrintfError("no zero border width required by FillSurfaceSimpleOtherBucket");
	    VipPrintfExit("FillSurfaceSimpleOtherBucket");
	    return(PB);
	}

    vos = VipGetOffsetStructure(squel);
    if(!vos) return(PB);
    vvos = VipGetOffsetStructure(voronoi);
    if(!vvos) return(PB);

    /*Determine taille des OtherBuffer*/

    sizebuf = (int *)VipCalloc(ss_list->n_ss+1,sizeof(int),"FillSurfaceSimpleOtherBucket");
    if(!sizebuf) return(PB);

    for(i=0;i<=ss_list->n_ss;i++) sizebuf[i] = 0;

    BORNESUP = HULL_SURFACE + 10*(ss_list->n_ss+1);

    squel_ptr = VipGetDataPtr_S16BIT(squel);
    voronoi_ptr = VipGetDataPtr_S16BIT(voronoi);

    for( z=0; z<squel->size.z; ++z )
      for( y=0; y<squel->size.y; ++y )
        {
          squel_ptr = VipGetDataPtr_S16BIT(squel) + vos->oFirstPoint + vos->oLine * y 
            + vos->oSlice * z;
          voronoi_ptr = VipGetDataPtr_S16BIT(voronoi) + vvos->oFirstPoint + vvos->oLine * y 
            + vvos->oSlice * z;
          for( x=0; x<squel->size.x; ++x )
	      {
	        if(*voronoi_ptr>HULL_SURFACE && *squel_ptr<HULL_SURFACE)
		    {
		        temp = *voronoi_ptr;
		        sizebuf[ss_list->labTOindex[temp]]++;
		        *squel_ptr = temp - temp%10 + 2;
                }
	        squel_ptr++;
	        voronoi_ptr++;
            }
        }

    /*alloue les other buffers */

    walker = ss_list->first_ss;
    while(walker!=NULL)
	{
	    walker->other_points = VipAlloc3DBucket_S16BIT(sizebuf[walker->index]);
	    if(!walker->other_points) return(PB);
	    walker->n_points += sizebuf[walker->index];
	    walker = walker->next;
	}

    /*remplie les other buffers*/

    squel_ptr = VipGetDataPtr_S16BIT(squel)+vos->oFirstPoint;

    for(z=0;z<mVipVolSizeZ(squel);z++)
	{
	    for(y=0;y<mVipVolSizeY(squel);y++)
		{
		    for(x=0;x<mVipVolSizeX(squel);x++)
			{
			    if (*squel_ptr>HULL_SURFACE)
				{
				    temp = *squel_ptr;
				    if (((temp-HULL_SURFACE)%10)==2)
					{
					    current = ss_list->tab[ss_list->labTOindex[temp]]->other_points;
					    if(current->n_points>current->size)
						{
						    VipPrintfError("Two many other points, strange");
						    VipPrintfExit("FillSurfaceSimpleOtherBucket");
						    return(PB);
						}
					    current->data[current->n_points].x = x;
					    current->data[current->n_points].y = y;
					    current->data[current->n_points].z = z;
					    current->n_points++;
					}
				}
			    squel_ptr++;
			}
		    squel_ptr+=vos->oPointBetweenLine;
		}
	    squel_ptr += vos->oLineBetweenSlice;
	}

    free(sizebuf);

    /*test*/
    for(i=1;i<=ss_list->n_ss;i++)
	{
	    if(ss_list->tab[i]->other_points->size != ss_list->tab[i]->other_points->n_points)
		{
		    VipPrintfError("Something strange has occured!");
		    printf("i:%d, ss size: %d, %d, %d\n",i,ss_list->tab[i]->surface_points->n_points,
			   ss_list->tab[i]->edge_points->size, ss_list->tab[i]->other_points->n_points);
		    VipPrintfExit("FillSurfaceSimpleOtherBucket");
		    return(PB);
		}
	}

    return(OK);
}

/*-------------------------------------------------------------------------*/
static int PruneSmallSimpleSurfaces( 
				     SurfaceSimpleList *sslist,
				     Volume *vol, 
				     int limitsize)
/*-------------------------------------------------------------------------*/
{
  int i;
  int *tokill;
  int nkilled;
  int j;
  int currentindex;
  SurfaceSimple *neighbor;
  int p,d;
  int oldindex;
  int nppkilled = 0;


  tokill = (int *)VipCalloc(sslist->n_ss+1, sizeof(int), "PruneSmallSimpleSurfaces");
  if(!tokill) return(PB);

  nkilled = 0;
  for(i=1;i<=sslist->n_ss;i++)
    {
	if(sslist->tab[i]->npotentialpp>0)
	    {   
		if(sslist->tab[i]->surface_points->n_points<MIN_SIZE_BASSIN_SPLITTED) 
/*pli de passage should
  not create to small pieces*/
		    {
			tokill[i] = VTRUE;
			nppkilled++;
			nkilled++;
		    }
		else if(sslist->tab[i]->n_points<limitsize) /*limitsize can be >20*/
		    {
			tokill[i] = VTRUE;
			nkilled++;
		    }		
	    }
	else if(sslist->tab[i]->n_points<limitsize)
	    {
		tokill[i] = VTRUE;
		nkilled++;
	    }
	else tokill[i] = VFALSE;
    }

  printf("Killing %d little simple surfaces (included %d plidepassage related)...\n",nkilled,
	 nppkilled);

  /*
    printf("Preserving ss:");
  */
  currentindex = 1;
  for(i=1;i<=sslist->n_ss;i++)
    {
      if(tokill[i]==VTRUE)
	{
	  if(VipWriteCoordBucketInVolume(sslist->tab[i]->surface_points,
					 vol, TOPO_SURFACE_POINT)==PB) return(PB);
	  if(VipWriteCoordBucketInVolume(sslist->tab[i]->edge_points,
					 vol, TOPO_BORDER_POINT)==PB) return(PB);
	  /*mise a jour du tableau d'index de plidepassage...*/
	  for(j=0;j<sslist->tab[i]->npotentialpp;j++)
	    {
	      neighbor = sslist->tab[sslist->tab[i]->pppotentialneighbor_index[j]];
	      for(p=0;p<neighbor->npotentialpp;p++)
		{
		  if (neighbor->pppotentialneighbor_index[p]==i)
		    {
		      for(d=p+1;d<neighbor->npotentialpp;d++)
			neighbor->pppotentialneighbor_index[d-1]=neighbor->pppotentialneighbor_index[d];
		      break;
		    }
		}
	      if (p==neighbor->npotentialpp)
		{
		  VipPrintfError("Incoherence in plidepassage index tab killer");
		  VipPrintfExit("PruneSmallSimpleSurfaces");
		  return(PB);
		}
	      neighbor->npotentialpp--;
	    }
	  if(VipUnhookSimpleSurfaceFromList(sslist->tab[i],sslist)==PB) 
	    return(PB);
	  if(FreeSurfaceSimple(sslist->tab[i])==PB) return(PB);
	}
      else
	{
	  /* printf(" (%d+%d)",sslist->tab[i]->surface_points->n_points, 
	     sslist->tab[i]->edge_points->n_points);*/
	  oldindex = sslist->tab[i]->index;
	  if(oldindex!=currentindex)
	    {
	      sslist->tab[i]->index = currentindex;
	      sslist->tab[i]->label = HULL_SURFACE + 10*currentindex;
	      sslist->tab[currentindex] = sslist->tab[i];
	      for(j=0;j<10;j++)
		sslist->labTOindex[sslist->tab[currentindex]->label+j] = currentindex;
	      /*mise a jour du tableau d'index de plidepassage...*/
	      for(j=0;j<sslist->tab[currentindex]->npotentialpp;j++)
		{
		  neighbor = sslist->tab[sslist->tab[currentindex]->pppotentialneighbor_index[j]];
		  for(p=0;p<neighbor->npotentialpp;p++)
		    {
		      if (neighbor->pppotentialneighbor_index[p]==oldindex)
			{
			  neighbor->pppotentialneighbor_index[p]=currentindex;
			  break;
			}
		    }
		  if (p==neighbor->npotentialpp)
		    {
		      VipPrintfError("Incoherence in plidepassage index tab");
		      VipPrintfExit("PruneSmallSimpleSurfaces");
		      return(PB);
		    }
		}
	    }
			  
	  if(VipWriteCoordBucketInVolume(sslist->tab[currentindex]->surface_points,
					 vol, sslist->tab[currentindex]->label)==PB) return(PB);
	  if(VipWriteCoordBucketInVolume(sslist->tab[currentindex]->edge_points,
					 vol, sslist->tab[currentindex]->label+1)==PB) return(PB);
	  currentindex++;
	}
    }

  /*   printf("\n");*/

  if((currentindex-1)!=(sslist->n_ss - nkilled))
    {
      VipPrintfError("Problem");
      VipPrintfExit("PruneSmallSimpleSurfaces");
      return(PB);
    }

  sslist->n_ss = sslist->n_ss - nkilled;
  printf("%d simple surfaces stay alive\n",sslist->n_ss);

  free(tokill);

  return(OK);
}

/*-------------------------------------------------------------------------*/
static int VipUnhookSimpleSurfaceFromList(
					  SurfaceSimple *ToUnhook,
					  SurfaceSimpleList *sslist)
/*-------------------------------------------------------------------------*/
{
  SurfaceSimple *ptr, *list;

  if(sslist==NULL || sslist->first_ss==NULL)
    {
      VipPrintfWarning("Empty list in VipUnhookSimpleSurfaceFromList");
      return(PB);
    }
  if(ToUnhook==NULL)
    {
      VipPrintfWarning("Nobody to be deleted in VipUnhookSimpleSurfaceFromList");
      VipPrintfExit("VipUnhookSimpleSurfaceFromList");
      return(PB);
    }

  list = sslist->first_ss;
  if(ToUnhook == list) list = list->next;
  else
    {
      ptr = list;
      while((ptr)&&(ptr->next != ToUnhook))
	{
	  ptr = ptr->next;
	}
      if (ptr==NULL)
	{
	  VipPrintfError("Can not find this ss in List");
	  VipPrintfExit("VipUnhookSimpleSurfaceFromList");
	  return(PB);
	}
      ptr->next = ptr->next->next;
    }
  ToUnhook->next = NULL;

  sslist->first_ss = list;

  return(OK);  
}



/*-------------------------------------------------------------------------*/
static int WriteSSLabelInVol(  Vip3DBucket_S16BIT *bucklist,Volume *vol)
/*-------------------------------------------------------------------------*/
{
  int label;
  Vip3DBucket_S16BIT *writer;

  label = HULL_SURFACE;
  writer = bucklist;
  while(writer!=NULL)
    {
      label += 10;
      if(VipWriteCoordBucketInVolume(writer, vol, 
				     label)==PB) return(PB);
      writer = writer->next;
    }

  return(OK);
}

/*-------------------------------------------------------------------------*/
static int PutOutsideBorderToHullLabel(Volume *squel, 
				       int inside, int outside, int hull)
/*-------------------------------------------------------------------------*/
{
  VipOffsetStruct *vos;
  VipConnectivityStruct *vcs;
  Vip_S16BIT *squel_ptr;
  int x,y,z,i;

  if(inside==outside)
    {
      VipPrintfWarning("inside lab and outside lab are equal in PutOutsideBorderToHullLabel");
    }

  vos = VipGetOffsetStructure(squel);
  if(!vos) return(PB);
  vcs = VipGetConnectivityStruct(squel,CONNECTIVITY_6);
  if(!vcs) return(PB);

  squel_ptr = VipGetDataPtr_S16BIT(squel)+vos->oFirstPoint;
  
  for(z=0;z<mVipVolSizeZ(squel);z++)
    {
      for(y=0;y<mVipVolSizeY(squel);y++)
	{
	  for(x=0;x<mVipVolSizeX(squel);x++)
	    {
	      if(*squel_ptr!=inside && *squel_ptr!=outside)
		{
		  for(i=0;i<vcs->nb_neighbors;i++)
		    {
		      if(*(squel_ptr+vcs->offset[i])==outside) break;
		    }
		  if (i!=vcs->nb_neighbors) *squel_ptr = hull;
		}
	      squel_ptr++;
	    }
	  squel_ptr+=vos->oPointBetweenLine;
	}
      squel_ptr += vos->oLineBetweenSlice;
    }

  return(OK);
}

/*-------------------------------------------------------------------------*/
int FillSurfaceSimpleEdgeBucket(
SurfaceSimpleList *ss_list,
Volume *vol,
int labelFRONTIERE)
/*-------------------------------------------------------------------------*/
     /*ajoute le bucket des points de type POINT SIMPLE (ou FRONTIERE)
       dans chacune des surfaces simples*/
{
  int i, j;
  Vip_S16BIT *squel_ptr, *voisin_ptr;
  SurfaceSimple *walker;
  VipOffsetStruct *vos;
  VipConnectivityStruct *vcs;
  int *sizebuf;
  int temp;
  int BORNESUP;
  int x, y, z;
  Vip3DBucket_S16BIT *current;
  int BORDERLEVEL = -123;

  if(!vol || !ss_list)
    {
      VipPrintfError("Empty args");
      VipPrintfExit("FillSurfaceSimpleEdgeBucket");
      return(PB);
    }

  if(mVipVolBorderWidth(vol)==0)
    {
      VipPrintfError("no zero border width required by FillSurfaceSimpleEdgeBucket");
      VipPrintfExit("GetSurfaceSimple");
      return(PB);
    }
  VipSetBorderLevel( vol, BORDERLEVEL ); 

  vos = VipGetOffsetStructure(vol);
  if(!vos) return(PB);
  vcs = VipGetConnectivityStruct(vol,CONNECTIVITY_26);
  if(!vcs) return(PB);

  /*Determine taille des edgeBuffer*/

  sizebuf = (int *)VipCalloc(ss_list->n_ss+1,sizeof(int),"FillSurfaceSimpleEdges");
  if(!sizebuf) return(PB);

  for(i=0;i<=ss_list->n_ss;i++) sizebuf[i] = 0;

  BORNESUP = HULL_SURFACE + 10*(ss_list->n_ss+1);

  squel_ptr = VipGetDataPtr_S16BIT(vol);

  for(i=vos->oVolume; i>0; i--)
    {
      if(*squel_ptr==labelFRONTIERE)
	{
	  for(j=0;j<vcs->nb_neighbors;j++)
	    {
	      voisin_ptr = squel_ptr+vcs->offset[j];
	      if (*voisin_ptr>HULL_SURFACE)
		{
		  temp = *voisin_ptr;
		  if(((temp-HULL_SURFACE)%10)==1) continue;
		  if ((((temp-HULL_SURFACE)%10)!=0) || (temp>BORNESUP))
		    {
		      fprintf(stderr,"label: %d\n",temp);
		      VipPrintfError("Strange label\n");
		      VipPrintfExit("FillSurfaceSimpleEdges");
		      return(PB);
		    }
		  sizebuf[ss_list->labTOindex[temp]]++;
		  *squel_ptr = temp + 1;
		  break;
		}
	    }
	}
      squel_ptr++;
    }

  /*alloue les edge buffers */

  walker = ss_list->first_ss;
  while(walker!=NULL)
    {
      walker->edge_points = VipAlloc3DBucket_S16BIT(sizebuf[walker->index]);
      if(!walker->edge_points) return(PB);
      walker->n_points += sizebuf[walker->index];
      walker = walker->next;
    }

  /*remplie les edge buffers*/

  squel_ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  for(z=0;z<mVipVolSizeZ(vol);z++)
    {
    for(y=0;y<mVipVolSizeY(vol);y++)
      {
      for(x=0;x<mVipVolSizeX(vol);x++)
	{
	  if (*squel_ptr>HULL_SURFACE)
	    {
	      temp = *squel_ptr;
	      if (((temp-HULL_SURFACE)%10)==1)
		{
		  current = ss_list->tab[ss_list->labTOindex[temp]]->edge_points;
		  current->data[current->n_points].x = x;
		  current->data[current->n_points].y = y;
		  current->data[current->n_points].z = z;
		  current->n_points++;
		}
	    }
	  squel_ptr++;
	}
      squel_ptr+=vos->oPointBetweenLine;
      }
    squel_ptr += vos->oLineBetweenSlice;
    }

  free(sizebuf);

  /*test*/
  for(i=1;i<=ss_list->n_ss;i++)
      {
	  if(ss_list->tab[i]->edge_points->size != ss_list->tab[i]->edge_points->n_points)
	      {
		  VipPrintfError("Something strange has occured!");
		  printf("i:%d, ss size: %d, %d, %d\n",i,ss_list->tab[i]->surface_points->n_points,
			 ss_list->tab[i]->edge_points->size, ss_list->tab[i]->edge_points->n_points);
		  VipPrintfExit("FillSurfaceSimpleEdgeBucket");
		  return(PB);
	      }
      }

  return(OK);
}



/*-------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *GetSurfaceSimple(
	Volume *vol,
	int labelSS,
	int inside,
	int outside)
/*-------------------------------------------------------------------------*/
     /* Retourne la liste des Buckets correspondant aux
	surfaces simples correspondant a un label precis = point de SS */
     /* deux labels designent le complementaire: complement */
{
  Topology26Neighborhood *topo26;
  int xsize, ysize, zsize;
  Vip_S16BIT *in;
  int flag_first_bucket;
  Vip3DPoint_S16BIT *temp_last, *courant_last;
  int x, y, z;
  int i;
  unsigned char *flag_volume, *buf_flag_volume;
  Vip3DBucket_S16BIT *temp, *courant;
  Vip3DBucket_S16BIT *bucket_list, *last_bucket;
  int jump;
  VipS16BITPtrBucket *comp1, *comp2, *comp1bis, *comp2bis;
  int C18;
  VipOffsetStruct *vos;
  VipConnectivityStruct *vcs;
  int bucketsizemax;
  int BORDERLEVEL = -123;
  int count = 0;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("GetSurfaceSimple");
      return(PB);
    }

  if(mVipVolBorderWidth(vol)==0)
    {
      VipPrintfError("no zero border width required by GetSurfaceSimple");
      VipPrintfExit("GetSurfaceSimple");
      return(PB);
    }
  VipSetBorderLevel( vol, BORDERLEVEL ); 

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  comp1 = VipAllocS16BITPtrBucket(18);
  comp2 = VipAllocS16BITPtrBucket(18);
  comp1bis = VipAllocS16BITPtrBucket(18);
  comp2bis = VipAllocS16BITPtrBucket(18);
  if(comp1==NULL || comp2==NULL || comp1bis==NULL || comp2bis==NULL) return(PB);

  vos = VipGetOffsetStructure(vol);
  if(!vos) return(PB);
  vcs = VipGetConnectivityStruct(vol,CONNECTIVITY_26);
  if(!vcs) return(PB);

  in = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);

  bucketsizemax = VipGetNumberLabelPoints( vol, labelSS );
  if( bucketsizemax == 0 )
    {
      VipPrintfError("The volume is empty\n");
      VipPrintfExit("GetSurfaceSimple");
      return(PB);   
    }

  bucket_list = last_bucket = NULL;
  temp = VipAlloc3DBucket_S16BIT( bucketsizemax );
  if(!temp) return(PB);
  /* En supposant que les composantes 3d
     ne seront pas plus grandes  */
		
  buf_flag_volume = (unsigned char*)VipCalloc( (xsize+2*mVipVolBorderWidth(vol))
					   *(ysize+2*mVipVolBorderWidth(vol))
					   *(zsize+2*mVipVolBorderWidth(vol)),
					   sizeof(unsigned char),"GetSurfaceSimple");
  /* Ce flag est vrai si le point a deja ete considere */
  if(!buf_flag_volume) return(PB); 
  flag_volume = buf_flag_volume + vos->oFirstPoint;

  flag_first_bucket = VTRUE;

  /* printf("Simple surface sizes: ");*/
  /* Balayage du volume avec determination des composantes par propagation */
  for( z=0; z<zsize; z++ )
    for( y=0; y<ysize; y++ )
      for( x=0; x<xsize; x++ )
	{
	  jump = x+y*vos->oLine+z*vos->oSlice;
	  if( !(*(flag_volume+jump)) && (*(in+jump) == labelSS) )
	    {
	      count++;
	      *(flag_volume+jump)=VTRUE;
	      courant = VipAlloc3DBucket_S16BIT( bucketsizemax );
	      if(!courant) return(PB);
	      courant_last = courant->data;
	      temp_last=temp->data;
	      temp_last->x = x;		
	      temp_last->y = y;
	      temp_last->z = z;
	      (temp->n_points)++;
	      do
		{
		  courant_last->x = temp_last->x;
		  courant_last->y = temp_last->y;
		  courant_last->z = temp_last->z;
		  jump = courant_last->x + (courant_last->y)*vos->oLine
		    + (courant_last->z)*vos->oSlice;
		  (temp->n_points)--;
		  (temp_last)--;
		  if(courant->n_points>=courant->size)
		    {
		      VipPrintfError("Too big simple surface, it is not consistent");
		      VipPrintfExit("GetSurfaceSimple");
		      return(PB);
		    }
		  (courant->n_points)++;

		  C18 = VipComputeCbarForTwoLabelComplement_S16BIT(topo26, in+jump, inside, outside);
		  if (C18!=2)
		    {
		      VipPrintfError("This is not a simple surface point (inconsistence)\n");
		      VipPrintfExit("GetSurfaceSimple");
		      return(PB);
		    }
		  VipFillSSComponent( comp1, comp2, topo26, in+jump);

		  for( i=0; i<vcs->nb_neighbors; i++)
		    {
	
		      /* Si le voisin(non nul) est bien dans l'image,
			 qu'il fait partie de la meme classe d'equivalence et s'il
			 n'a pas encore ete traite, il est mis dans
			 la liste des points a examiner */
										
		      if( !(*(flag_volume+jump+vcs->offset[i])) )
			{
			  if((*(in+jump+vcs->offset[i])==labelSS))
			    {
			      C18 = VipComputeCbarForTwoLabelComplement_S16BIT(topo26, in+jump+vcs->offset[i], inside, outside);
			      if (C18!=2)
				{
				  printf("This is not a simple surface point (inconsistence)\n");
				  VipPrintfExit("GetSurfaceSimple");
				  return(PB);
				}				
			      VipFillSSComponent( comp1bis, comp2bis, topo26,
						  in+jump+vcs->offset[i]);
		
			      if (VipSurfaceSimpleEquivalence( comp1, comp2, comp1bis,comp2bis) )
				{
				  (temp_last)++;
				  temp_last->x = courant_last->x + vcs->xyz_offset[i].x;
				  temp_last->y = courant_last->y + vcs->xyz_offset[i].y;
				  temp_last->z = courant_last->z + vcs->xyz_offset[i].z;

				  (temp->n_points)++;
				  if(temp->n_points>=temp->size)
				    {
				      VipPrintfError("Too big simple surface, it is not consistent");
				      VipPrintfExit("GetSurfaceSimple");
				      return(PB);
				    }	
				  *(flag_volume+jump+vcs->offset[i]) = VTRUE;
				}
			    }
			  else *(flag_volume+jump+vcs->offset[i]) = VTRUE;
			}
		    }				
		  /*
		    fprintf(stderr,"temp: %d, courant: %d, x: %d, y:%d, z:%d\n",temp->n_points, 
                    courant->n_points, courant->last->x, courant->last->y,
		    courant->last->z);
		  */
		  (courant_last)++;

		} while( temp->n_points );
	      VipDwindle3DBucket_S16BIT( courant );
	      if(flag_first_bucket)
		{
		  bucket_list = last_bucket = courant;
		  flag_first_bucket = VFALSE;
		}
	      else
		{
		  last_bucket->next=courant;
		  last_bucket=courant;
		}

	      /*   printf(" %d",courant->n_points);
		   fflush( stdout);*/
	    }
	  else *(flag_volume+jump)=VTRUE;
	}
  /*printf("\n");*/

  free(buf_flag_volume);
  VipFree3DBucket_S16BIT(temp);

  VipFreeS16BITPtrBucket(comp1);
  VipFreeS16BITPtrBucket(comp1bis);
  VipFreeS16BITPtrBucket(comp2);
  VipFreeS16BITPtrBucket(comp2bis);

  printf("%d simple surfaces\n",count);

  return(bucket_list);
}	

/*-------------------------------------------------------------------------*/
SurfaceSimpleList *CreateSurfaceSimpleList(
Vip3DBucket_S16BIT *sp_list)
/*-------------------------------------------------------------------------*/
     /*convertit la liste des buckets en liste chainee de surfaces simples*/
     /*cree egalement dans la sslist un acces au ss sous forme de tableau
       de pointeurs. Le premier pointeur NULL represente le HULL_BRAIN*/
     /*cree une look up table entre les labels attribues a la ss et leur
       index de 1 a nss (ces labels vont de 10 en 10, pour pouvoir
       distinguer plusieurs type de points dans une ss)*/
{
  SurfaceSimple *ss_list_first, *new, *tail;
  Vip3DBucket_S16BIT *sp_list_ptr;
  int label_courant, nb_surface;
  SurfaceSimpleList *ss_list;
  int nss;
  int i;
  int lookup_size;

  if(!sp_list)
    {
      VipPrintfError("No Bucket list of SS points");
      VipPrintfExit("CreateSurfaceSimpleList");
      return(PB);
    }

  ss_list = (SurfaceSimpleList *)VipCalloc(1,sizeof(SurfaceSimpleList),"CreateSurfaceSimpleList");
  if(!ss_list) return(PB);

  nss = 1; /*=HULL_SURFACE*/
  sp_list_ptr = sp_list;
  while(sp_list_ptr!=NULL)
    {
      nss++;
      sp_list_ptr = sp_list_ptr->next;
    }
  ss_list->tab = (SurfaceSimple **)VipCalloc(nss,sizeof(SurfaceSimple *),"CreateSurfaceSimpleList");
  if(ss_list->tab==PB) return(PB);

  label_courant = HULL_SURFACE+10;

  lookup_size = HULL_SURFACE + nss*10; /*SS = label + range of length 9*/ 
  ss_list->labTOindex = (int *)VipCalloc(lookup_size,sizeof(int),"CreateSurfaceSimpleList");
  if(ss_list->labTOindex==PB) return(PB);
  for(i=0;i<lookup_size;i++) ss_list->labTOindex[i] = HULL_SURFACE_INDEX;
  
  ss_list->tab[0] = NULL; /*HULL_SURFACE*/

  ss_list_first = CreateEmptySurfaceSimple();
  nb_surface = 1;
  ss_list_first->label = label_courant;
  ss_list_first->index = nb_surface;
  ss_list->tab[nb_surface] = ss_list_first;
  for(i=0;i<10;i++)
    ss_list->labTOindex[label_courant+i] = nb_surface;
  label_courant += 10;
  ss_list_first->surface_points = sp_list;
  ss_list_first->n_points = sp_list->n_points;
  tail = ss_list_first;

  sp_list_ptr = sp_list->next;
	
  while(sp_list_ptr != NULL)
    {
      nb_surface++;
      new = CreateEmptySurfaceSimple();
      new->label = label_courant;
      new->index = nb_surface;
      ss_list->tab[new->index] = new;
      for(i=0;i<10;i++)
	ss_list->labTOindex[new->label+i] = new->index;
      label_courant += 10;
      new->surface_points = sp_list_ptr;
      new->n_points = sp_list_ptr->n_points;
      sp_list_ptr = sp_list_ptr->next;
      new->surface_points->next=NULL;
      tail->next = new;
      tail = new;
    }

  if(nb_surface!=(nss-1))
    {
      VipPrintfError("Inconsistence in CreateSurfaceSimpleList");
      return(PB);
    }

  printf("Number of simple surfaces: %d\n", nb_surface);
  ss_list->first_ss = ss_list_first;
  ss_list->n_ss = nb_surface;

  return(ss_list);
}		

/*-------------------------------------------------------------------------*/
int FreeSurfaceSimple( SurfaceSimple *dead )
/*-------------------------------------------------------------------------*/
{
  if (dead==NULL)
    {
      VipPrintfError("No Simple surface to kill");
      VipPrintfExit("FreeSurfaceSimple");
      return(PB);;
    }
  if (dead->surface_points==NULL)
    {
      VipPrintfWarning("Freeing empty Simple Surface (no surface points)");
      return(OK);
    }
  VipFree3DBucket_S16BIT(dead->surface_points);
  if( dead->edge_points) VipFree3DBucket_S16BIT(dead->edge_points);
  free(dead);

  return(OK);
}

/*-------------------------------------------------------------------------*/
int FreeSurfaceSimpleList(SurfaceSimpleList *thedeadlist)
/*-------------------------------------------------------------------------*/
{
  SurfaceSimple *deadlist,*killer;

  if (thedeadlist == NULL)
    {
      VipPrintfError("No Simple surface list to kill");
      VipPrintfExit("FreeSurfaceSimpleList");
      return(PB);
    }
  deadlist = thedeadlist->first_ss;
  while(deadlist!=NULL)
    {
      killer=deadlist;
      deadlist=deadlist->next;
      if(FreeSurfaceSimple(killer)==PB) return(PB);
    }
  if(thedeadlist->tab!=NULL) free(thedeadlist->tab);
  if(thedeadlist->labTOindex!=NULL) free(thedeadlist->labTOindex);
  free(thedeadlist);
  return(OK);
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
SurfaceSimple *CreateEmptySurfaceSimple( )
/*-------------------------------------------------------------------------*/

	/* retourne la structure d'une surfacesimple */
{
  SurfaceSimple *new;

  new = (SurfaceSimple *)VipMalloc(sizeof(SurfaceSimple),"CreateEmptySurfaceSimple");
  if(!new) return(PB);;
  new->label = 0;
  new->index = 0;
  new->the_label = 0;
  strcpy(new->the_name,"unknown");
  new->confiance = 0.;
  new->dist_moy = 0.;

  new->n_points = 0;
  new->surface_points = NULL;
  new->edge_points = NULL;
  new->other_points = NULL;
  new->next = NULL;

  new->hull_bijunction = NULL;

  new->n_bijunction = 0;
  new->bijunction = NULL;
  new->bijunction_neighbor = NULL;

  new->n_cortex_birel = 0;
  new->cortex_birel = NULL;
  new->cortex_neighbor = NULL;

  new->box_filled = VFALSE;
  new->boxmin.x = 0;
  new->boxmin.y = 0;
  new->boxmin.z = 0;
  new->boxmax.x = 0;
  new->boxmax.y = 0;
  new->boxmax.z = 0;

  new->refbox_filled = VFALSE;
  new->refboxmin.x = 0.;
  new->refboxmin.y = 0.;
  new->refboxmin.z = 0.;
  new->refboxmax.x = 0.;
  new->refboxmax.y = 0.;
  new->refboxmax.z = 0.;

  new->size = 0.;
  new->size_filled = VFALSE;
  new->g.x = 0.;
  new->g.y = 0.;
  new->g.z = 0.;
  new->g_filled = VFALSE;
  new->normale.x = 0.;
  new->normale.y = 0.;
  new->normale.z = 0.;
  new->normale_filled = VFALSE;
  new->refg.x = 0.;
  new->refg.y = 0.;
  new->refg.z = 0.;
  new->refg_filled = VFALSE;
  new->refnormale.x = 0.;
  new->refnormale.y = 0.;
  new->refnormale.z = 0.;
  new->refnormale_filled = VFALSE;
  new->depth = 0.;
  new->depth_filled = VFALSE;
  new->mindepth = 0.;
  new->mindepth_filled = VFALSE;
  new->rootsbassin_filled = VFALSE;
  new->rootsbassin=0;
  new->npotentialpp = 0;
  new->pppotentialneighbor_index = NULL;
  new->n_plidepassage = 0;
  new->plidepassage = NULL;
  new->plidepassage_neighbor = NULL;

  return(new);
}
/*-------------------------------------------------------------------------*/
