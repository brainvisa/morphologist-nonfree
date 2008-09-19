/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 02/06/1999
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
#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/distmap.h>

/*-------------------------------------------------------------------------*/
static int FillCortexBiRelation( 
			   JunctionPointList *jpl,
			   CortexRelationSet *jset,
			   SurfaceSimpleList *sslist );
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
static Volume *CreateFirstVoronoi(
			SurfaceSimpleList *ss_list, 
			Volume *skeleton,
			int forbiden);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static CortexRelationSet *GetCortexRelationSet( 
			    JunctionPointList *jpl,
			    SurfaceSimpleList *sslist );
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
CortexRelationSet *VipComputeFoldArgCortexRelationSet( Volume *vol,
					   SurfaceSimpleList *sslist )
/*-------------------------------------------------------------------------*/
/*input: the skeleton, with detection of the ss*/
     /* La creation des relations corticales s'appuie sur des routines
equivalentes a celles utilisees pour les jonctions. Celles ci sont utilisees
apres calcul prealable d'un diagramme de voronoi conditionnel a HULL_BRAIN
et dont les graines sont les jonctions ss avec HULL_BRAIN, les
relations corticales correspondent alors a des adjacences entre
zones d'influence equivalentes a des jonctions entre SS*/
     /*un second diagramme de Voronoi peut permettre de calculer
l'attribut surface. Il est conditionnel a HULL_BRAIN prive
des jonctions ss/HULL_BRAIN, les graines sont les adjacences
precedentes (liees a un decoupage du squelette par zone d'influence
sous-jacent)*/
{
  CortexRelationSet *jset;
  JunctionPointList *jpl;
  Volume *voronoi;
  int forbiden = 0;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("VipComputeFoldArgCortexRelationSet");
      return(PB);
    }
  if (!sslist)
    {
      VipPrintfError("NULL sslist");
      VipPrintfExit("VipComputeFoldArgCortexRelationSet");
      return(PB);
    }

  printf("-------------------------------------------------\n");
  printf("Computing cortical relations (gyral junctions)...\n");
  printf("-------------------------------------------------\n");

  printf("First voronoi (domain = hull, seeds = hull/ss junctions)...\n");
  voronoi = CreateFirstVoronoi( sslist, vol, forbiden);
  if(!voronoi)
    {
      VipPrintfExit("VipComputeFoldArgCortexRelationSet");
      return(PB);
    }
  /*
  VipWriteVolume(voronoi,"voronoi");
  */

  printf("Detection of voronoi junctions in the influence zone skeleton...\n");

  jpl = GetListOfJunctionPoint( voronoi, forbiden, forbiden);
  if(jpl==PB) return(PB);

  VipFreeVolume(voronoi);

  jset = GetCortexRelationSet( jpl, sslist);
  if(jset==PB) return(PB);

  if (jpl->n_points!=0) VipFreeJunctionPointList(jpl);

  return(jset); 
}

/*-------------------------------------------------------------------------*/
static CortexRelationSet *GetCortexRelationSet( 
			    JunctionPointList *jpl,
			    SurfaceSimpleList *sslist )
/*-------------------------------------------------------------------------*/
{
	CortexRelationSet *jset;

	if (!jpl || !sslist)
	{
	  VipPrintfError("Null args");
	  VipPrintfExit("GetCortexRelationSet");
	  return(PB);
	}

	jset = (CortexRelationSet *)VipCalloc(1,sizeof(CortexRelationSet),
					      "GetCortexRelationSet");
	if (!jset) return(PB);
	jset->n_bi = 0;
	jset->first_bi = NULL;

	if(jpl->n_points==0) return(jset);
	if ( FillCortexBiRelation( jpl, jset, sslist)==PB) return(PB);
	if ( FillSSListCortexBirelationPtr( sslist, jset )==PB) return(PB);
	return( jset );
}

/*-------------------------------------------------------------------------*/
static Volume *CreateFirstVoronoi(
			SurfaceSimpleList *ss_list, 
			Volume *skeleton,
			int forbiden)
/*-------------------------------------------------------------------------*/
/*Compute le voronoi conditionel au hull_brain pour les jonctions externe*/
{
  Volume *domain, *voronoi;
  SurfaceSimple *ss_ptr;
  Vip_S16BIT *sptr, *dptr;
  int i, j;
  Vip3DPoint_S16BIT *pptr;
  VipOffsetStruct *vos;
  int FORBIDENval;
  int DOMAINval = 8; 
  int nb_hull_junction = 0;

  if(ss_list==NULL || !skeleton)
    {
      VipPrintfError("Null args");
      return(PB);
    }

  FORBIDENval = forbiden;

  domain = VipDuplicateVolumeStructure(skeleton, "voronoi_domain");
  if (domain==PB) return(PB);
  if( VipAllocateVolumeData( domain )==PB) return(PB);

  vos = VipGetOffsetStructure(skeleton);
  if(!vos) return(PB);

  sptr = VipGetDataPtr_S16BIT(skeleton);
  dptr = VipGetDataPtr_S16BIT(domain);

  /*intialisation du domaine*/
  for(i=vos->oVolume; i>0; i--)
    {
      if (*sptr!=HULL_SURFACE) *dptr = FORBIDENval;
      else *dptr = DOMAINval;
      dptr++;
      sptr++;
    }

  /*seed creation*/
  ss_ptr = ss_list->first_ss;
  dptr = VipGetDataPtr_S16BIT(domain)+vos->oFirstPoint;
  while(ss_ptr!=NULL)
    {
      if(ss_ptr->hull_bijunction != NULL)
	{
	  nb_hull_junction++;
	  pptr = ss_ptr->hull_bijunction->points->data;
	  for(j = ss_ptr->hull_bijunction->points->n_points;j--;)
	    {
	      *(dptr + pptr->x + pptr->y * vos->oLine +
		pptr->z * vos->oSlice) = ss_ptr->label;
	      pptr++;
	    }
	}
      ss_ptr = ss_ptr->next;
    }

  printf("Number of hull/ss junctions: %d\n", nb_hull_junction);

  if(nb_hull_junction>1)
    {
      voronoi = VipComputeFrontPropagationGeodesicVoronoi( domain, DOMAINval, FORBIDENval);
      
      if(voronoi==PB)
	{
	  VipPrintfExit("ComputeFirstVoronoi");
	  return(PB);
	}
      
      VipFreeVolume(domain);

    }
  else
    {
      VipPrintfWarning("Zero or one hull junction, Voronoi computation is skipped!");
      voronoi = domain;
    }
  return(voronoi);
}

/*-------------------------------------------------------------------------*/
static int FillCortexBiRelation( 
			   JunctionPointList *jpl,
			   CortexRelationSet *jset,
			   SurfaceSimpleList *sslist )
/*-------------------------------------------------------------------------*/

{
  int size_tab;
  Vip3DBucket_S16BIT ***the_tab, *buck_ptr;
  int **the_count;
  int i,j;
  JunctionPoint *j_ptr;
  int temp1, temp2, temp3;
  int n_bijunction, verif;
  Vip3DPoint_S16BIT *p_ptr;
  CortexBiRelation temp, *tail, *creator;

  size_tab = sslist->n_ss + 1; /*HULL_SURFACE*/

  the_tab = (Vip3DBucket_S16BIT ***)VipMalloc(size_tab*sizeof(Vip3DBucket_S16BIT **),
					      "FillCortexBiRelation");
  if (!the_tab) return(PB);
  for(i=0;i<size_tab;i++)
    {
      the_tab[i] = (Vip3DBucket_S16BIT **)VipMalloc(size_tab*sizeof(Vip3DBucket_S16BIT *),
						    "FillCortexBiRelation");
      if (!(the_tab[i])) return(PB);
    }

  for(i=0;i<size_tab;i++)
    for(j=0;j<size_tab;j++)
      {
	the_tab[i][j] = NULL;
      }
	
  the_count = (int **)VipMalloc(size_tab*sizeof(int *),"FillCortexBiRelation");
  if (!the_count) return(PB);
  for(i=0;i<size_tab;i++)
    {
      the_count[i] = (int *)VipMalloc(size_tab*sizeof(int),"FillCortexBiRelation");
      if (!(the_count[i])) return(PB);
    }

  for(i=0;i<size_tab;i++)
    for(j=0;j<size_tab;j++)
      {
	the_count[i][j] = 0;
      }

  j_ptr = jpl->first;
  for(i=jpl->n_points;i--;)
    {
      if (j_ptr->nb_ss == 2)
	{
	  temp1 = sslist->labTOindex[j_ptr->label_ss[0]];
	  temp2 = sslist->labTOindex[j_ptr->label_ss[1]];
	  if (temp1>temp2)
	    {
	      temp3 = temp1;
	      temp1 = temp2;
	      temp2 = temp3;
	    }
	  if (temp2>=size_tab)
	    {
	      VipPrintfError("strange too big label\n");
	      VipPrintfExit("FillCortexBiRelation");
	      return(PB);
	    }
	  the_count[temp1][temp2]++;
	}
      j_ptr++;
    }
		
  /* alloue les buckets */
  n_bijunction = 0;
  for(i=0;i<size_tab;i++)
    for(j=0;j<size_tab;j++)
      {
	if (the_count[i][j])
	  {
	    n_bijunction++;
	    the_tab[i][j] = VipAlloc3DBucket_S16BIT( the_count[i][j]);
	    if(the_tab[i][j]==PB)
	      {
		VipPrintfExit("FillCortexBiRelation");
		return(PB);
		the_tab[i][j]->n_points = 0;
	      }
	  }
      }

  for(i=0;i<size_tab;i++) if (the_count[i]!=NULL) free(the_count[i]);
  free (the_count);

  /*fill les buckets */

  j_ptr = jpl->first;
  for(i=jpl->n_points;i--;)
    {
      if (j_ptr->nb_ss == 2)
	{
	  temp1 = sslist->labTOindex[j_ptr->label_ss[0]];
	  temp2 = sslist->labTOindex[j_ptr->label_ss[1]];
	  if (temp1>temp2)
	    {
	      temp3 = temp1;
	      temp1 = temp2;
	      temp2 = temp3;
	    }
	  if (temp2>=size_tab)
	    {
	      VipPrintfError("strange too big label\n");
	      VipPrintfExit("FillCortexBiRelation");
	      return(PB);
	    }
	  buck_ptr = the_tab[temp1][temp2];
	  p_ptr = buck_ptr->data + buck_ptr->n_points++;
	  p_ptr->x = j_ptr->x;
	  p_ptr->y = j_ptr->y;
	  p_ptr->z = j_ptr->z;
	}
      j_ptr++;
    }

  jset->n_bi = n_bijunction;

  tail = &temp;
  verif = n_bijunction;
  for(i=0;i<size_tab;i++)
    for(j=0;j<size_tab;j++)
      {
	if (the_tab[i][j]!=NULL)
	  {
	    creator = CreateCortexBiRelation( i, j, the_tab[i][j], sslist);
	    tail->next = creator;
	    tail = tail->next;
	    verif--;
	  }
      }
  jset->first_bi = temp.next;

  for(i=0;i<size_tab;i++) if (the_tab[i]!=NULL) free(the_tab[i]);
  free (the_tab);


  printf("CortexBiRelation number: %d\n", n_bijunction);
  if(verif!=0)
    {
      VipPrintfError("Warning: strange number of cortex birelation\n");
      VipPrintfExit("FillCortexBiRelation");
      return(PB);
    }
 
  return(OK);
}	

/*-------------------------------------------------------------------------*/
CortexBiRelation *CreateCortexBiRelation(
			     int index1,
			     int index2,
			     Vip3DBucket_S16BIT *buck,
			     SurfaceSimpleList *sslist)
/*-------------------------------------------------------------------------*/
{
  CortexBiRelation *new;

  if(!buck)
    {
      VipPrintfError("Null bucket");
      VipPrintfExit("CreateCortexBiRelation");
      return(PB);
    }
  new = (CortexBiRelation *)VipCalloc(1,sizeof(CortexBiRelation),
				      "CreateCortexBiRelation");
  if (!new) return(PB);
  new->index1 = index1;
  new->index2 = index2;
  new->skiz = buck;
  if(sslist!=NULL && sslist->tab!=NULL)
    {
      new->s1 = sslist->tab[index1];
      new->s2 = sslist->tab[index2];
    }
  new->next = NULL;
  new->size_filled = VFALSE;
  new->size = 0.;
  new->surface_filled = VFALSE;
  new->surface = 0.;
  new->hulljunctiondist = 0.;
  new->hulljunctiondist_filled = VFALSE;
  new->SS1nearest_filled = VFALSE;
  new->SS2nearest_filled = VFALSE;
  new->refSS1nearest_filled = VFALSE;
  new->refSS2nearest_filled = VFALSE;

  return(new);
}

/*-------------------------------------------------------------------------*/
int FillSSListCortexBirelationPtr(
				 SurfaceSimpleList *ss_list,
				 CortexRelationSet *jset)
/*-------------------------------------------------------------------------*/
{
  CortexBiRelation *walker;
  SurfaceSimple *fill;

  /*compte le nombre de jonctions pour chaque SS*/
  walker = jset->first_bi;
  while(walker!=NULL)
    {
      if (walker->s1!=NULL && walker->s2!=NULL) 
	{
	  (walker->s1->n_cortex_birel)++;
	  (walker->s2->n_cortex_birel)++;
	}
      else VipPrintfWarning("strange CortexBirelation in FillSSListCortexBirelationPtr");
      walker = walker->next;
    }

  /*alloue le buffers*/
  
  fill = ss_list->first_ss;
  while(fill!=NULL)
    {
      if(fill->n_cortex_birel!=0)
	{
	  fill->cortex_birel =
	    (CortexBiRelation **)VipCalloc(fill->n_cortex_birel, sizeof(CortexBiRelation *),
				     "FillSSListJunctionPtr");
	  if (!fill->cortex_birel) return(PB);
	  fill->cortex_neighbor=
	    (SurfaceSimple **)VipCalloc(fill->n_cortex_birel,sizeof(SurfaceSimple *),
					"FillSSListJunctionPtr");
	  if (!fill->cortex_neighbor) return(PB);
	  fill->n_cortex_birel = 0;
	}
      fill = fill->next;
    }

  /*remplie les buffers*/
  walker = jset->first_bi;
  while(walker!=NULL)
    {
      if (walker->s1!=NULL && walker->s2!=NULL)
	{
	  walker->s1->cortex_birel[(walker->s1->n_cortex_birel)]
	    = walker;
	  walker->s1->cortex_neighbor[(walker->s1->n_cortex_birel)]
	    = walker->s2;
	  walker->s1->n_cortex_birel++;

	  walker->s2->cortex_birel[(walker->s2->n_cortex_birel)]
	    = walker;
	  walker->s2->cortex_neighbor[(walker->s2->n_cortex_birel)]
	    = walker->s1;
	  walker->s2->n_cortex_birel++;
	}
      walker = walker->next;
    }

  return(OK);
}
