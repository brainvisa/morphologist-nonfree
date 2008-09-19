/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 09/08/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : pli de passage
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
static Vip3DBucket_S16BIT *CopyBucket( Vip3DBucket_S16BIT *buck);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
PliDePassageSet *VipComputeFoldArgPliDePassageSet( SurfaceSimpleList *sslist,
						   JunctionSet *jset)
/*-------------------------------------------------------------------------*/
{
  PliDePassageSet *ppset;
  int ss;
  int j,k;
  SurfaceSimple *neighbor;
  int pot;
  BiJunction **bijtabcopy, **bijtabunhook;
  int countcopy, countunhook;
  int ccopy, cunhook;
  PliDePassage temp, *tail, *creator;
  Vip3DBucket_S16BIT *buck;

  temp.next = NULL;

  if (!sslist)
    {
      VipPrintfError("NULL sslist");
      VipPrintfExit("VipComputeFoldArgPliDePassageSet");
      return(PB);
    }
  if (!jset)
    {
      VipPrintfError("NULL jset");
      VipPrintfExit("VipComputeFoldArgPliDePassageSet");
      return(PB);
    }

  printf("---------------------------------------------------\n");
  printf("Computing Plis De Passage (burried gyri)...\n");
  printf("---------------------------------------------------\n");

  ppset = (PliDePassageSet *)VipCalloc(1,sizeof(PliDePassageSet),"VipComputeFoldArgPliDePassageSet");
  if (!ppset) return(PB);
  ppset->n_pp = 0;
  ppset->first_pp = NULL;

  /*count*/
  countcopy = 0;
  countunhook = 0;
  for(ss=1;ss<=sslist->n_ss;ss++)
    {
      /*
      printf("i:%d,ssindex:%d\n",ss,sslist->tab[ss]->index);
      for(pot=0;pot<sslist->tab[ss]->npotentialpp;pot++)
	printf("%d ",sslist->tab[ss]->pppotentialneighbor_index[pot]);
      printf(" |j: ");
      */
      for(j=0;j<sslist->tab[ss]->n_bijunction;j++)
	{
	  neighbor = sslist->tab[ss]->bijunction_neighbor[j];
	  if(sslist->tab[ss]->rootsbassin!=neighbor->rootsbassin)
	  {
	    for(pot=0;pot<neighbor->npotentialpp;pot++)
	      {
		if(neighbor->pppotentialneighbor_index[pot]==ss) 
		  {
		    countunhook++;
		    break;
		  }
	      }
	    if (pot==neighbor->npotentialpp)
	      {
		countcopy++;
	      }
	  }
	}
      /*printf("\n");*/
    }

  printf("%d copy and %d conversion BiJunction -> PliDePassage\n",countcopy/2,countunhook/2);
  if(countcopy%2!=0 || countunhook%2!=0)
    {
      VipPrintfError("incoherence");
      VipPrintfExit("VipComputeFoldArgPliDePassageSet");
      return(PB);
    }
  countcopy /=2;
  countunhook /=2;

  bijtabcopy = (BiJunction **)VipCalloc(countcopy,sizeof(BiJunction *),"VipComputeFoldArgPliDePassageSet");
  if(!bijtabcopy) return(PB);
  bijtabunhook = (BiJunction **)VipCalloc(countunhook,sizeof(BiJunction *),"VipComputeFoldArgPliDePassageSet");
  if(!bijtabunhook) return(PB);

  /*fill tabs*/
  ccopy = 0;
  cunhook = 0;
  for(ss=1;ss<=sslist->n_ss;ss++)
    {
      for(j=0;j<sslist->tab[ss]->n_bijunction;j++)
	{
	  neighbor = sslist->tab[ss]->bijunction_neighbor[j];
	  if(sslist->tab[ss]->rootsbassin!=neighbor->rootsbassin)	
	    {
	      for(pot=0;pot<neighbor->npotentialpp;pot++)
		{
		  if(neighbor->pppotentialneighbor_index[pot]==ss) /*only pp, not junction*/
		    {      /*fill only one time*/
		      if(ss<neighbor->index) bijtabunhook[cunhook++] = sslist->tab[ss]->bijunction[j];
		      break;
		    }
		}
	      if(pot==neighbor->npotentialpp) /*not splitted by voronoi*/
		{
		  if(ss<neighbor->index) bijtabcopy[ccopy++] = sslist->tab[ss]->bijunction[j];
		}
	    }
	}
      
      /*cleaning up junction set*//*the symmetric occur when neighbor will become ss*/
	         
     for(j=0;j<sslist->tab[ss]->n_bijunction;j++)
	{
	  neighbor = sslist->tab[ss]->bijunction_neighbor[j];
	  if(sslist->tab[ss]->rootsbassin!=neighbor->rootsbassin)	
	    {
	      for(pot=0;pot<neighbor->npotentialpp;pot++)
		{
		  if(neighbor->pppotentialneighbor_index[pot]==ss) 
		    {      
		      for(k=j+1;k<sslist->tab[ss]->n_bijunction;k++)
			{
			  sslist->tab[ss]->bijunction[k-1]=sslist->tab[ss]->bijunction[k];
			  sslist->tab[ss]->bijunction_neighbor[k-1]=sslist->tab[ss]->bijunction_neighbor[k];
			}
		      sslist->tab[ss]->bijunction[sslist->tab[ss]->n_bijunction-1]=NULL;
		      sslist->tab[ss]->bijunction_neighbor[sslist->tab[ss]->n_bijunction-1]=NULL;
		      sslist->tab[ss]->n_bijunction -=1;
		      break;
		    } 
		}
	    }
	}
	    
      
    }

  if(countcopy!=ccopy || countunhook!=cunhook)
    {
      printf("copy %d/%d, convert %d/%d\n",ccopy,countcopy,cunhook,countunhook);
      VipPrintfError("incoherence");
      VipPrintfExit("VipComputeFoldArgPliDePassageSet");
      return(PB);      
    }

  ppset->n_pp = countcopy+countunhook;
  tail = &temp;
  for(j=0;j<countcopy;j++)
    {
      buck = CopyBucket(bijtabcopy[j]->points);
      if(buck==PB) return(PB);
      creator = CreatePliDePassage(bijtabcopy[j]->index1,bijtabcopy[j]->index2,
				   buck, sslist);
      tail->next = creator;
      tail = tail->next;
      
    }
  for(j=0;j<countunhook;j++)
    {      
      creator = CreatePliDePassage(bijtabunhook[j]->index1,bijtabunhook[j]->index2,
				   bijtabunhook[j]->points, sslist);
      if(VipUnhookBijunctionFromJset(bijtabunhook[j],jset)==PB) return(PB);
      free(bijtabunhook[j]);
      tail->next = creator;
      tail = tail->next;
      
    }
  ppset->first_pp = temp.next;
  
  if( FillSSListPliDePassagePtr(sslist,ppset) == PB) return(PB);

  free(bijtabunhook);
  free(bijtabcopy);
  return(ppset); 
}

/*-------------------------------------------------------------------------*/
static Vip3DBucket_S16BIT *CopyBucket( Vip3DBucket_S16BIT *buck)
/*-------------------------------------------------------------------------*/
{
  Vip3DBucket_S16BIT *copy;
  int i;

  if(!buck)
    {
      VipPrintfError("empty arg");
      VipPrintfExit("CopyBucket");
      return(PB);
    }

  copy = VipAlloc3DBucket_S16BIT( buck->n_points);
  if(!copy) return(PB);

  for(i=0;i<copy->size;i++)
    {
      copy->data[i].x = buck->data[i].x;
      copy->data[i].y = buck->data[i].y;
      copy->data[i].z = buck->data[i].z;
    }
  copy->n_points = copy->size;

  return(copy);
}

/*-------------------------------------------------------------------------*/
PliDePassage *CreatePliDePassage(
			     int index1,
			     int index2,
			     Vip3DBucket_S16BIT *buck,
			     SurfaceSimpleList *sslist)
/*-------------------------------------------------------------------------*/
{
  PliDePassage *new;

  if(!buck)
    {
      VipPrintfError("Null bucket");
      VipPrintfExit("CreatePliDePassage");
      return(PB);
    }
  new = (PliDePassage *)VipCalloc(1,sizeof(PliDePassage),"CreatePliDePassage");
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
  new->depth_filled = VFALSE;
  new->location_filled = VFALSE;
  new->reflocation_filled = VFALSE;
  new->size = 0.;

  
  return(new);
}

/*-------------------------------------------------------------------------*/
int FillSSListPliDePassagePtr(
				 SurfaceSimpleList *ss_list,
				 PliDePassageSet *ppset)
/*-------------------------------------------------------------------------*/
{
  PliDePassage *walker;
  SurfaceSimple *fill;

  /*compte le nombre de jonctions pour chaque SS*/
  walker = ppset->first_pp;
  while(walker!=NULL)
    {
      if (walker->s1!=NULL) 
	{
	  if(walker->s2!=NULL) (walker->s1->n_plidepassage)++;
	}
      if (walker->s2!=NULL) 
	{
	  if(walker->s1!=NULL) (walker->s2->n_plidepassage)++;
	}
      if(walker->s1==NULL || walker->s2==NULL) 
	  {
	      VipPrintfWarning("strange HULL pli de passage in FillSSListJunctionPtr (bad sort)");
	  }
      walker = walker->next;
    }

  /*alloue le buffers*/
  
  fill = ss_list->first_ss;
  while(fill!=NULL)
    {
      if(fill->n_plidepassage!=0)
	{
	  fill->plidepassage =
	    (PliDePassage **)VipCalloc(fill->n_plidepassage, sizeof(PliDePassage *),
				     "FillSSListJunctionPtr");
	  if (!fill->plidepassage) return(PB);
	  fill->plidepassage_neighbor=
	    (SurfaceSimple **)VipCalloc(fill->n_plidepassage,sizeof(SurfaceSimple *),
					"FillSSListJunctionPtr");
	  if (!fill->plidepassage_neighbor) return(PB);
	  fill->n_plidepassage = 0;
	}
      fill = fill->next;
    }

  /*remplie les buffers*/
  walker = ppset->first_pp;
  while(walker!=NULL)
    {
      if (walker->s1!=NULL)
	{
	  if(walker->s2!=NULL)
	    {
	      walker->s1->plidepassage[(walker->s1->n_plidepassage)]
		= walker;
	      walker->s1->plidepassage_neighbor[(walker->s1->n_plidepassage)]
		= walker->s2;
	      walker->s1->n_plidepassage++;
	    }
	}
      if (walker->s2!=NULL)
	{
	  if(walker->s1==NULL)
	    /*junction with HULL*/
	    {
	      VipPrintfWarning("strange HULL pli de passage in FillSSListJunctionPtr (bad sort)");
	    }
	  else
	    {
	      walker->s2->plidepassage[(walker->s2->n_plidepassage)]
		= walker;
	      walker->s2->plidepassage_neighbor[(walker->s2->n_plidepassage)]
		= walker->s1;
	      walker->s2->n_plidepassage++;
	    }
	}
      walker = walker->next;
    }

  return(OK);
}
