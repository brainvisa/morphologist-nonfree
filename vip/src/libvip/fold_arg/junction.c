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

/*-------------------------------------------------------------------------*/
static int FillBiJunction( 
			   JunctionPointList *jpl,
			   JunctionSet *jset,
			   SurfaceSimpleList *sslist );
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
static JunctionSet *GetJunctionSet( JunctionPointList *jpl,
				    SurfaceSimpleList *sslist );
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
JunctionSet *VipComputeFoldArgJunctionSet( Volume *vol,
					   SurfaceSimpleList *sslist,
					   int inside,
					   int outside)
/*-------------------------------------------------------------------------*/
/*input: the skeleton, with detection of the ss*/
{
  JunctionSet *jset;
  JunctionPointList *jpl;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("VipComputeFoldArgJunctionSet");
      return(PB);
    }
  if (!sslist)
    {
      VipPrintfError("NULL sslist");
      VipPrintfExit("VipComputeFoldArgJunctionSet");
      return(PB);
    }

  printf("---------------------------------------------------\n");
  printf("Computing topological relations (fold junctions)...\n");
  printf("---------------------------------------------------\n");

  jpl = GetListOfJunctionPoint( vol, inside, outside);
  if(jpl==PB) return(PB);
  /*
 VipWriteVolume(vol,"ssj");
  */
  jset = GetJunctionSet( jpl, sslist);
  if(jset==PB) return(PB);

  if (jpl->n_points!=0) VipFreeJunctionPointList(jpl);

  return(jset); 
}

/*-------------------------------------------------------------------------*/
int VipFreeJunctionPointList( JunctionPointList *jpl)
/*-------------------------------------------------------------------------*/
{
  int i;
 
  if(!jpl) return(PB);

  for(i=0;i<jpl->n_points;i++)
    {
      if(jpl->first[i].label_ss) free(jpl->first[i].label_ss);
    }
  free(jpl);

  return(OK);
  

}

/*-------------------------------------------------------------------------*/
JunctionPointList *GetListOfJunctionPoint(Volume *vol, int inside, int outside)
/* une premiere passe permet de compter les points de junction,
   une seconde de construire leur voisinage,
   enfin on marque leur nature (constantes BIJUNCTION, MULTIJUNCTION3...) */
     /* utilisee pour les jonctions topologiques et corticales*/
/*-------------------------------------------------------------------------*/

{

    Vip_S16BIT *ptr, *first;
    int i,j;
    int n_junction_points;
    int center, temp;
    JunctionPointList *jpl;
    JunctionPoint *jpl_ptr;
    int x, y, z;
    int n_ss;
    short label[27];
    int n[28];
    int BORDERLEVEL = outside;
    VipOffsetStruct *vos;
    VipConnectivityStruct *vcs;
    int firstlab;

    if (VipVerifyAll(vol)==PB)
	{
	    VipPrintfExit("GetListOfJunctionPoint");
	    return(PB);
	}

    if(mVipVolBorderWidth(vol)==0)
	{
	    VipPrintfError("no zero border width required by GetListOfJunctionPoint");
	    VipPrintfExit("GetListOfJunctionPoint");
	    return(PB);
	}
    VipSetBorderLevel( vol, BORDERLEVEL );

    vos = VipGetOffsetStructure(vol);
    if(!vos) return(PB);
    vcs = VipGetConnectivityStruct(vol,CONNECTIVITY_26);
    if(!vcs) return(PB);
 
    printf("Computation of the number of junction points...\n");

    firstlab = 0;
    n_junction_points = 0;
    ptr = VipGetDataPtr_S16BIT(vol);
    for(i=vos->oVolume; i>0; i--)
	{
	    center = *ptr;
	    if(center!=inside && center!=outside)
		{
		    n_ss = 0;
		    if(center>=HULL_SURFACE)
			{
			    center -= center%10;
			    n_ss=1;
			    firstlab=center;
			}
		    for(j=0;j<vcs->nb_neighbors;j++)
			{
			    temp = *(ptr+vcs->offset[j]);
			    if(temp>=HULL_SURFACE)
			    {
				temp -= temp%10;
				if(n_ss==0)
				    {
					n_ss=1;
					firstlab=temp;
				    }
				else if (temp!=firstlab)
				    {
					n_junction_points++;				    
					break;
				    }
			    }
			}
		}
	    ptr++;
	}

    printf("%d points\n",n_junction_points);

    jpl = (JunctionPointList *)VipMalloc(sizeof(JunctionPointList),"GetListOfJunctionPoint");
    if (!jpl) return(PB);
    jpl->size = n_junction_points;
    jpl->first = (JunctionPoint *)VipCalloc(n_junction_points,sizeof(JunctionPoint),"GetListOfJunctionPoint");
    if (!jpl->first) return(PB);
    jpl->n_points = 0;

    jpl_ptr = jpl->first;
    ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
    for(z=0;z<mVipVolSizeZ(vol);z++)
	{
	    for(y=0;y<mVipVolSizeY(vol);y++)
		{
		    for(x=0;x<mVipVolSizeX(vol);x++)
			{
			    temp = *ptr;
			    if(temp!=inside && temp!=outside)
			    {
				n_ss = 0;
				if(temp>=HULL_SURFACE)
				    {
					temp -= temp%10;
					label[n_ss++]=temp;
				    }
		
				for(i=0;i<vcs->nb_neighbors;i++)
				    {
					temp = *(ptr+vcs->offset[i]);
					if(temp>=HULL_SURFACE)
					{
					    temp -= temp%10;
					    for(j=0;j<n_ss;j++)
						{
						    if(label[j]==temp) break;
						}
					    if (j==n_ss)
						{
						    label[n_ss++] = temp;
						}
					}
				    }
				if (n_ss>1)
				    {
					jpl_ptr->x = x;
					jpl_ptr->y = y;
					jpl_ptr->z = z;
					jpl_ptr->nb_ss = n_ss;
					jpl_ptr->label_ss = (Vip_S16BIT *)VipCalloc(n_ss,sizeof(Vip_S16BIT),
										    "GetListOfJunctionPoint");
					if (!jpl_ptr->label_ss) return(PB);
					for(j=0;j<n_ss;j++)
					    {
						jpl_ptr->label_ss[j] = label[j];
					    }

					jpl_ptr++;
					jpl->n_points++;
			
				    }
			    }
			    ptr++;
			}
		    ptr+=vos->oPointBetweenLine;
		}
	    ptr += vos->oLineBetweenSlice;
	}

    if(n_junction_points!=jpl->n_points)
	{
	    VipPrintfError("inconsistence!");
	    VipPrintfExit("GetListOfJunctionPoint");
	    return(PB);
	}

    jpl_ptr = jpl->first;
    for(i=0;i<=27;i++) n[i] = 0;
    for(i=jpl->n_points;i--;)
	{
	    n[(jpl_ptr++)->nb_ss]++;
	}
    for(i=10;i<=27;i++) n[9] += n[i];
    printf("Types: 2:%d, 3:%d, 4:%d, 5:%d, 6:%d, 7:%d, 8: %d, more: %d\n",
	   n[2],n[3],n[4],n[5], n[6], n[7], n[8], n[9]);
    jpl_ptr = jpl->first;

    first = VipGetDataPtr_S16BIT(vol)+ vos->oFirstPoint;
    for(i=jpl->n_points;i--;)
	{
	    temp = jpl_ptr->nb_ss;
	    ptr = first + jpl_ptr->x + jpl_ptr->y * vos->oLine + jpl_ptr->z * vos->oSlice;
	    if(temp>=9) *ptr=MULTIJUNCTIONN;
	    else
		{
		    switch(temp)
			{
			case 2: *ptr = BIJUNCTION; break;
			case 3: *ptr = MULTIJUNCTION3; break;
			case 4: *ptr = MULTIJUNCTION4; break;
			case 5: *ptr = MULTIJUNCTION5; break;
			case 6: *ptr = MULTIJUNCTION6; break;
			case 7: *ptr = MULTIJUNCTION7; break;
			case 8: *ptr = MULTIJUNCTION8; break;
			default: VipPrintfError("inconsistence");
			    VipPrintfExit("GetListOfJunctionPoint");
			    return(PB);
			}
		}
	    jpl_ptr++;
	}

    VipFreeConnectivityStruct(vcs);

    return(jpl);										
}

/*-------------------------------------------------------------------------*/
static JunctionSet *GetJunctionSet( 
			    JunctionPointList *jpl,
			    SurfaceSimpleList *sslist )
/*-------------------------------------------------------------------------*/
{
	JunctionSet *jset;

	if (!jpl || !sslist)
	{
	  VipPrintfError("Null args");
	  VipPrintfExit("GetJunctionSet");
	  return(PB);
	}

	jset = (JunctionSet *)VipCalloc(1,sizeof(JunctionSet),"GetJunctionSet");
	if (!jset) return(PB);
	jset->n_bi = 0;
	jset->first_bi = NULL;

	if(jpl->n_points==0) return(jset);
	if ( FillBiJunction( jpl, jset, sslist)==PB) return(PB);
	if ( FillSSListJunctionPtr( sslist, jset )==PB) return(PB);
	return( jset );
}
/*-------------------------------------------------------------------------*/
static int FillBiJunction( 
			   JunctionPointList *jpl,
			   JunctionSet *jset,
			   SurfaceSimpleList *sslist )
/*-------------------------------------------------------------------------*/
     /*convertit la liste des points de jonction en liste de Bijunctions*/
     /*utilise uin tableau temporaire pour une premiere passe
       qui calcules les tailles de ces bijonctions*/
     /*la jonction avec le HULL_BRAIN est geree*/
{
    int size_tab;
    Vip3DBucket_S16BIT ***the_tab, *buck_ptr;
    int **the_count;
    int i,j;
    JunctionPoint *j_ptr;
    int temp1, temp2, temp3, temp4, temp5;
    int n_bijunction, verif;
    Vip3DPoint_S16BIT *p_ptr;
    BiJunction temp, *tail, *creator;

    size_tab = sslist->n_ss + 1; /*HULL_SURFACE*/

    the_tab = (Vip3DBucket_S16BIT ***)VipMalloc(size_tab*sizeof(Vip3DBucket_S16BIT **),
						"FillBiJunction");
    if (!the_tab) return(PB);
    for(i=0;i<size_tab;i++)
	{
	    the_tab[i] = (Vip3DBucket_S16BIT **)VipMalloc(size_tab*sizeof(Vip3DBucket_S16BIT *),
							  "FillBiJunction");
	    if (!(the_tab[i])) return(PB);
	}

    for(i=0;i<size_tab;i++)
	for(j=0;j<size_tab;j++)
	    {
		the_tab[i][j] = NULL;
	    }
	
    the_count = (int **)VipMalloc(size_tab*sizeof(int *),"FillBiJunction");
    if (!the_count) return(PB);
    for(i=0;i<size_tab;i++)
	{
	    the_count[i] = (int *)VipMalloc(size_tab*sizeof(int),"FillBiJunction");
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
		    temp1 = j_ptr->label_ss[0];
		    temp2 = j_ptr->label_ss[1];
		    if (temp1>temp2)
			{
			    temp3 = temp1;
			    temp1 = temp2;
			    temp2 = temp3;
			}
		    if (sslist->labTOindex[temp2]>=size_tab)
			{
			    VipPrintfError("strange too big label\n");
			    VipPrintfExit("FillBiJunction");
			    return(PB);
			}
		    j_ptr->label_ss[0]=temp1;
		    j_ptr->label_ss[1]=temp2;
		    temp1 = sslist->labTOindex[temp1];
		    temp2 = sslist->labTOindex[temp2];
		    the_count[temp1][temp2]++;
		}
	    else if (j_ptr->nb_ss == 3) 
		{
		    temp1 = j_ptr->label_ss[0];
		    temp2 = j_ptr->label_ss[1];
		    temp3 = j_ptr->label_ss[2];
		    if (temp1>temp2)
			{
			    temp4 = temp1;
			    temp1 = temp2;
			    temp2 = temp4;
			}
		    if (temp2>temp3)
			{
			    temp4 = temp2;
			    temp2 = temp3;
			    temp3 = temp4;
			}
		    if (temp1>temp2)
			{
			    temp4 = temp1;
			    temp1 = temp2;
			    temp2 = temp4;
			}
		    if (sslist->labTOindex[temp3]>=size_tab)
			{
			    VipPrintfError("strange too big label\n");
			    VipPrintfExit("FillBiJunction");
			    return(PB);
			}
		    j_ptr->label_ss[0]=temp1;
		    j_ptr->label_ss[1]=temp2;
		    j_ptr->label_ss[2]=temp3;
		    temp1 = sslist->labTOindex[temp1];
		    temp2 = sslist->labTOindex[temp2];
		    temp3 = sslist->labTOindex[temp3];
		    the_count[temp1][temp2]++;
		    the_count[temp1][temp3]++;
		    the_count[temp2][temp3]++;
		    }
	    else if (j_ptr->nb_ss == 4) 
		{
		    temp1 = j_ptr->label_ss[0];
		    temp2 = j_ptr->label_ss[1];
		    temp3 = j_ptr->label_ss[2];
		    temp4 = j_ptr->label_ss[3];
		    if (temp1>temp2) 
			{
			    temp5 = temp1;
			    temp1 = temp2;
			    temp2 = temp5;
			}
		    if (temp2>temp3)
			{
			    temp5 = temp2;
			    temp2 = temp3;
			    temp3 = temp5;
			}
		    if (temp3>temp4)
			{
			    temp5 = temp3;
			    temp3 = temp4;
			    temp4 = temp5;
			}
		    if (temp1>temp2)
			{
			    temp5 = temp1;
			    temp1 = temp2;
			    temp2 = temp5;
			}
		    if (temp2>temp3)
			{
			    temp5 = temp2;
			    temp2 = temp3;
			    temp3 = temp5;
			}
		    if (temp1>temp2)
			{
			    temp5 = temp1;
			    temp1 = temp2;
			    temp2 = temp5;
			}
		    if (sslist->labTOindex[temp4]>=size_tab)
			{
			    VipPrintfError("strange too big label\n");
			    VipPrintfExit("FillBiJunction");
			    return(PB);
			}
		    j_ptr->label_ss[0]=temp1;
		    j_ptr->label_ss[1]=temp2;
		    j_ptr->label_ss[2]=temp3;
		    j_ptr->label_ss[3]=temp4;
		    temp1 = sslist->labTOindex[temp1];
		    temp2 = sslist->labTOindex[temp2];
		    temp3 = sslist->labTOindex[temp3];
		    temp4 = sslist->labTOindex[temp4];
		    the_count[temp1][temp2]++;
		    the_count[temp1][temp3]++;
		    the_count[temp1][temp4]++;
		    the_count[temp2][temp3]++;
		    the_count[temp2][temp4]++;
		    the_count[temp3][temp4]++;
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
			if(i>=j)
			    {
				printf("alloc,%d %d\n",i,j);
				VipPrintfError("Bad label sort");
				VipPrintfExit("FillBiJunction");
				return(PB);
			    }
				
			n_bijunction++;
			the_tab[i][j] = VipAlloc3DBucket_S16BIT( the_count[i][j]);
			if(the_tab[i][j]==PB)
			    {
				VipPrintfExit("FillBiJunction");
				return(PB);
				the_tab[i][j]->n_points = 0;
			    }
		    }
	    }

    for(i=0;i<size_tab;i++) if (the_count[i]!=NULL) free(the_count[i]);
    free (the_count);

    fflush(stdout);
    /*fill les buckets */

    j_ptr = jpl->first;
    for(i=jpl->n_points;i--;)
	{
	    fflush(stdout);
	    if (j_ptr->nb_ss == 2)
		{
		    temp1 = sslist->labTOindex[j_ptr->label_ss[0]];
		    temp2 = sslist->labTOindex[j_ptr->label_ss[1]];
		    if (temp2>=size_tab)
			{
			    VipPrintfError("strange too big label\n");
			    VipPrintfExit("FillBiJunction");
			    return(PB);
			}
		    buck_ptr = the_tab[temp1][temp2];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		}
	    else if (j_ptr->nb_ss == 3)
		{
		    temp1 = sslist->labTOindex[j_ptr->label_ss[0]];
		    temp2 = sslist->labTOindex[j_ptr->label_ss[1]];
		    temp3 = sslist->labTOindex[j_ptr->label_ss[2]];
		    if (temp3>=size_tab)
			{
			    VipPrintfError("strange too big label\n");
			    VipPrintfExit("FillBiJunction");
			    return(PB);
			}
		    buck_ptr = the_tab[temp1][temp2];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		    buck_ptr = the_tab[temp1][temp3];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		    buck_ptr = the_tab[temp2][temp3];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		}
	    else if (j_ptr->nb_ss == 4)
		{
		    temp1 = sslist->labTOindex[j_ptr->label_ss[0]];
		    temp2 = sslist->labTOindex[j_ptr->label_ss[1]];
		    temp3 = sslist->labTOindex[j_ptr->label_ss[2]];
		    temp4 = sslist->labTOindex[j_ptr->label_ss[3]];
		    if (temp4>=size_tab)
			{
			    VipPrintfError("strange too big label\n");
			    VipPrintfExit("FillBiJunction");
			    return(PB);
			}
		    buck_ptr = the_tab[temp1][temp2];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		    buck_ptr = the_tab[temp1][temp3];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		    buck_ptr = the_tab[temp1][temp4];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		    buck_ptr = the_tab[temp2][temp3];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		    buck_ptr = the_tab[temp2][temp4];
		    p_ptr = buck_ptr->data + buck_ptr->n_points++;
		    p_ptr->x = j_ptr->x;
		    p_ptr->y = j_ptr->y;
		    p_ptr->z = j_ptr->z;
		    buck_ptr = the_tab[temp3][temp4];
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
			creator = CreateBiJunction( i, j, the_tab[i][j], sslist);
			tail->next = creator;
			tail = tail->next;
			verif--;
		    }
	    }
    jset->first_bi = temp.next;

    for(i=0;i<size_tab;i++) if (the_tab[i]!=NULL) free(the_tab[i]);
    free (the_tab);

    printf("Bijunction number: %d (including hull junction)\n", n_bijunction);
    if(verif!=0)
	{
	    VipPrintfError("Warning: nb junction traitees anormal\n");
	    VipPrintfExit("FillBiJunction");
	    return(PB);
	}
 
    return(OK);
}	

/*-------------------------------------------------------------------------*/
int VipUnhookBijunctionFromJset(
					  BiJunction *ToUnhook,
					  JunctionSet *jset)
/*-------------------------------------------------------------------------*/
{
  BiJunction *ptr, *list;

  if(jset==NULL || jset->first_bi==NULL)
    {
      VipPrintfWarning("Empty list in VipUnhookBijunctionFromJset");
      return(PB);
    }
  if(ToUnhook==NULL)
    {
      VipPrintfWarning("Nobody to be deleted in VipUnhookBijunctionFromJset");
      VipPrintfExit("VipUnhookBijunctionFromJset");
      return(PB);
    }

  list = jset->first_bi;
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
	  VipPrintfError("Can not find this bijunction in List");
	  VipPrintfExit("VipUnhookBijunctionFromJset");
	  return(PB);
	}
      ptr->next = ptr->next->next;
    }
  ToUnhook->next = NULL;

  jset->first_bi = list;
  jset->n_bi--;
  return(OK);  
}
/*-------------------------------------------------------------------------*/
BiJunction *CreateBiJunction(
			     int index1,
			     int index2,
			     Vip3DBucket_S16BIT *buck,
			     SurfaceSimpleList *sslist)
/*-------------------------------------------------------------------------*/
{
  BiJunction *new;

  if(!buck)
    {
      VipPrintfError("Null bucket");
      VipPrintfExit("CreateBiJunction");
      return(PB);
    }
  new = (BiJunction *)VipCalloc(1,sizeof(BiJunction),"CreateBiJunction");
  if (!new) return(PB);
  new->index1 = index1;
  new->index2 = index2;
  new->points = buck;
  if(sslist!=NULL && sslist->tab!=NULL)
    {
      new->s1 = sslist->tab[index1]; /*When HULL, tab[0]=NULL*/
      new->s2 = sslist->tab[index2];
    }
  new->next = NULL;
  new->size_filled = VFALSE;
  new->size = 0.;
  new->direction_filled = VFALSE;
  new->maxdepth_filled = VFALSE;
  new->mindepth_filled = VFALSE;
  new->extremity1_filled = VFALSE;
  new->extremity2_filled = VFALSE;
  return(new);
}

/*-------------------------------------------------------------------------*/
int FillSSListJunctionPtr(
				 SurfaceSimpleList *ss_list,
				 JunctionSet *jset)
/*-------------------------------------------------------------------------*/
{
  BiJunction *walker;
  SurfaceSimple *fill;

  /*compte le nombre de jonctions pour chaque SS*/
  walker = jset->first_bi;
  while(walker!=NULL)
    {
      if (walker->s1!=NULL) 
	{
	  if(walker->s2!=NULL) (walker->s1->n_bijunction)++;
	}
      if (walker->s2!=NULL) 
	{
	  if(walker->s1!=NULL) (walker->s2->n_bijunction)++;
	}
      else 
	  {
	      VipPrintfWarning("strange HULL junction in FillSSListJunctionPtr (bad sort)");
	  }
      walker = walker->next;
    }

  /*alloue le buffers*/
  
  fill = ss_list->first_ss;
  while(fill!=NULL)
    {
      if(fill->n_bijunction!=0)
	{
	  fill->bijunction =
	    (BiJunction **)VipCalloc(fill->n_bijunction, sizeof(BiJunction *),
				     "FillSSListJunctionPtr");
	  if (!fill->bijunction) return(PB);
	  fill->bijunction_neighbor=
	    (SurfaceSimple **)VipCalloc(fill->n_bijunction,sizeof(SurfaceSimple *),
					"FillSSListJunctionPtr");
	  if (!fill->bijunction_neighbor) return(PB);
	  fill->n_bijunction = 0;
	}
      fill = fill->next;
    }

  /*remplie les buffers*/
  walker = jset->first_bi;
  while(walker!=NULL)
    {
      if (walker->s1!=NULL)
	{
	  if(walker->s2!=NULL)
	    {
	      walker->s1->bijunction[(walker->s1->n_bijunction)]
		= walker;
	      walker->s1->bijunction_neighbor[(walker->s1->n_bijunction)]
		= walker->s2;
	      walker->s1->n_bijunction++;
	    }
	}
      if (walker->s2!=NULL)
	{
	  if(walker->s1==NULL)
	    /*junction with HULL*/
	    {
	      walker->s2->hull_bijunction = walker;
	    }
	  else
	    {
	      walker->s2->bijunction[(walker->s2->n_bijunction)]
		= walker;
	      walker->s2->bijunction_neighbor[(walker->s2->n_bijunction)]
		= walker->s1;
	      walker->s2->n_bijunction++;
	    }
	}
      walker = walker->next;
    }

  return(OK);
}
