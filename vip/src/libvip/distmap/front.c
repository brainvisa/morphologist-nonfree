/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_front.c        * TYPE     : Source
 * AUTHOR      : PENGLAOU V.          * CREATION : 07/03/97
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : distance transform using front propagation
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/distmap.h>

#include <vip/distmap_static.h>


static VipIntBucket *retire_bucket(VipIntBucket **chaine);

static int ajoute_bucket(VipIntBucket **chaine,VipIntBucket *buck);


static int Vider_Buckets (VipIntBucket **chaine, Volume *vol,VipIntBucket **Liste_Buckets,
				VipIntBucket **Liste_Buckets_ptr,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS,int limit);

/*---------------------------------------------------------------------------*/
static int Remplissage_Buckets(VipIntBucket **chaine, Volume *vol,DistmapMask *mask,
				VipIntBucket **Liste_Buckets,
				VipIntBucket **Liste_Buckets_ptr,
				int geodesic_mode);

static int Vider_BucketsVoronoi (Volume *vol, Volume *label, VipIntBucket **liste,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS);

static int Remplissage_BucketsVoronoi( Volume *vol, Volume *label, DistmapMask *mask,
				VipIntBucket **Liste_Buckets);

static int Remplissage_BucketsConnectivity( Volume *vol,DistmapMask *mask,
				VipIntBucket **Liste_Buckets);

static int Vider_BucketsConnectivityVoronoi (Volume *vol,Volume *label,VipIntBucket **liste,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS);

static int Remplissage_BucketsConnectivityVoronoi( Volume *vol, Volume *label, DistmapMask *mask,
				VipIntBucket **Liste_Buckets);

/*--------------------------------------------------------------------------*/
static int Vider_BucketsConnectivity (Volume *vol,VipIntBucket **liste,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS,int limit);

/*--------------------------------------------------------------------*/
int FrontPropagation (
   Volume *vol,
   DistmapMask *mask,
   int limit
)
/*--------------------------------------------------------------------*/
{
  int LONGUEUR_LISTE_BUCKETS,x,y,z;
  float vx,vy,vz;
  VipIntBucket **Liste_Buckets;
  VipIntBucket **Liste_Buckets_ptr;
  VipIntBucket *reservoir;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  BackwardSweepingWithBorder is only implemented for S16BIT volume");
      VipPrintfExit("(distmap_sweeping)BackwardSweepingWithBorder");
      return(PB);
    }

  x = mVipVolSizeX(vol);
  y = mVipVolSizeY(vol);
  z = mVipVolSizeZ(vol);
  vx = mVipVolVoxSizeX(vol);
  vy = mVipVolVoxSizeY(vol);
  vz = mVipVolVoxSizeZ(vol);
  reservoir = NULL;
 
  LONGUEUR_LISTE_BUCKETS = (int)(sqrt((double)(z*vz)*(z*vz) + 
			       (double)((x*vx)*(x*vx)+(y*vy)*(y*vy)))*
			       (double)VIP_USED_DISTMAP_MULTFACT); 
 

  
  Liste_Buckets = (VipIntBucket **) VipCalloc(LONGUEUR_LISTE_BUCKETS, sizeof(VipIntBucket *), "allocation liste"); 
  Liste_Buckets_ptr = (VipIntBucket **) VipCalloc(LONGUEUR_LISTE_BUCKETS, sizeof(VipIntBucket *), "allocation liste"); 
 
  if(Liste_Buckets==PB || Liste_Buckets_ptr==PB) return(PB);
  
  if(Remplissage_Buckets(&reservoir,vol,mask,Liste_Buckets,Liste_Buckets_ptr,VFALSE)==PB) return(PB);
  
  if(Vider_Buckets(&reservoir,vol,Liste_Buckets,Liste_Buckets_ptr,mask,LONGUEUR_LISTE_BUCKETS, limit)==PB) return(PB);


  if(reservoir!=NULL) VipFreeIntBucketList(reservoir);
  VipFree(Liste_Buckets);
  VipFree(Liste_Buckets_ptr);
  return(OK);
}

/*--------------------------------------------------------------------*/
int FrontPropagationConnectivity (
   Volume *vol,
   DistmapMask *mask,
   int limit
)
/*--------------------------------------------------------------------*/
{
  int LONGUEUR_LISTE_BUCKETS,x,y,z;
  float vx,vy,vz;
  VipIntBucket **Liste_Buckets;
 

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  BackwardSweepingWithBorder is only implemented for S16BIT volume");
      VipPrintfExit("(distmap_sweeping)BackwardSweepingWithBorder");
      return(PB);
    }

  x = mVipVolSizeX(vol);
  y = mVipVolSizeY(vol);
  z = mVipVolSizeZ(vol);
  vx = mVipVolVoxSizeX(vol);
  vy = mVipVolVoxSizeY(vol);
  vz = mVipVolVoxSizeZ(vol);
 
  LONGUEUR_LISTE_BUCKETS = (int)(sqrt((double)(z*vz)*(z*vz) + 
			       (double)((x*vx)*(x*vx)+(y*vy)*(y*vy)))*
			       (double)VIP_USED_DISTMAP_MULTFACT); 
 

  
  Liste_Buckets = (VipIntBucket **) VipCalloc(LONGUEUR_LISTE_BUCKETS, sizeof(VipIntBucket *), "allocation liste"); 
 
  
  
  if(Remplissage_BucketsConnectivity(vol,mask,Liste_Buckets)==PB) return(PB);
  
  if(Vider_BucketsConnectivity(vol,Liste_Buckets,mask,LONGUEUR_LISTE_BUCKETS, limit)==PB) return(PB);

  return(OK);
}

/*--------------------------------------------------------------------*/
int FrontPropagationVoronoi (
   Volume *vol,
   Volume *label,
   DistmapMask *mask
)
/*--------------------------------------------------------------------*/
{
  int LONGUEUR_LISTE_BUCKETS,x,y,z;
  float vx,vy,vz;
  VipIntBucket **Liste_Buckets;
 

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  BackwardSweepingWithBorder is only implemented for S16BIT volume");
      VipPrintfExit("(distmap_sweeping)BackwardSweepingWithBorder");
      return(PB);
    }

  x = mVipVolSizeX(vol);
  y = mVipVolSizeY(vol);
  z = mVipVolSizeZ(vol);
  vx = mVipVolVoxSizeX(vol);
  vy = mVipVolVoxSizeY(vol);
  vz = mVipVolVoxSizeZ(vol);
 
  LONGUEUR_LISTE_BUCKETS = (int)(sqrt((double)(z*vz)*(z*vz) + 
			       (double)((x*vx)*(x*vx)+(y*vy)*(y*vy)))*
			       (double)VIP_USED_DISTMAP_MULTFACT); 
 

  
  Liste_Buckets = (VipIntBucket **) VipCalloc(LONGUEUR_LISTE_BUCKETS, sizeof(VipIntBucket *), "allocation liste"); 
 
  
  
  if(Remplissage_BucketsVoronoi(vol,label,mask,Liste_Buckets)==PB) return(PB);
  
  if(Vider_BucketsVoronoi(vol,label,Liste_Buckets,mask,LONGUEUR_LISTE_BUCKETS)==PB) return(PB);

  return(OK);
}

/*--------------------------------------------------------------------*/
int FrontPropagationConnectivityVoronoi (
   Volume *vol,
   Volume *label,
   DistmapMask *mask
)
/*--------------------------------------------------------------------*/
{
  int LONGUEUR_LISTE_BUCKETS,x,y,z;
  float vx,vy,vz;
  VipIntBucket **Liste_Buckets;
 

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  FrontPropagationConnectivityVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(distmap_sweeping)FrontPropagationConnectivityVoronoi");
      return(PB);
    }

  x = mVipVolSizeX(vol);
  y = mVipVolSizeY(vol);
  z = mVipVolSizeZ(vol);
  vx = mVipVolVoxSizeX(vol);
  vy = mVipVolVoxSizeY(vol);
  vz = mVipVolVoxSizeZ(vol);
 
  LONGUEUR_LISTE_BUCKETS = (int)(sqrt((double)(z*vz)*(z*vz) + 
			       (double)((x*vx)*(x*vx)+(y*vy)*(y*vy)))*
			       (double)VIP_USED_DISTMAP_MULTFACT); 
 

  
  Liste_Buckets = (VipIntBucket **) VipCalloc(LONGUEUR_LISTE_BUCKETS, sizeof(VipIntBucket *), "allocation liste"); 
 
  
  
  if(Remplissage_BucketsConnectivityVoronoi(vol,label,mask,Liste_Buckets)==PB) return(PB);
  
  if(Vider_BucketsConnectivityVoronoi(vol,label,Liste_Buckets,mask,LONGUEUR_LISTE_BUCKETS)==PB) return(PB);

  return(OK);
}

/*---------------------------------------------------------------------------*/
static int Remplissage_Buckets(VipIntBucket **chaine, Volume *vol,DistmapMask *mask,
				VipIntBucket **Liste_Buckets,
				VipIntBucket **Liste_Buckets_ptr,
				int geodesic_mode)
{ 
 
  Vip_S16BIT *ptr, *voisin;
  int i, l, NbTotalPts, neighbor, newval;
  DistmapMask_point *maskptr, *maskafter6;
  VipIntBucket *buckptr;
  int nuntil_6con_end;
  int background_6neighbor;

 
  /* during this step (filling bucket 0), the only points which can be
     seed are necessarily 6-connected to background (not formally proven yet...)*/
  /*Unfortunately, this is nor true for geodesic therefore we need
    a switch :geodesic_mode*/
  /* Therefore if one point has no 6-neighbor in background, we do not
     try to propagate more distances */

  /*a second important improvement (jeff sept 97):
    to preserve memory, we use chained lists of fixed size buckets
    and we propagate distances from the smallest to the largest
    in order to assure that one point will be put in only one bucket */
      


  nuntil_6con_end = 0;
  maskptr = mask->first_point;
  for(i=0;i<mask->length;i++)
    {
      if((abs((int)(maskptr->x))+abs((int)(maskptr->y))+abs((int)(maskptr->z)))==1)
	{
	  nuntil_6con_end = i+1;
	}
      maskptr++;
    }
  /*
  printf("The last 6-connectivity point has position %d\n",nuntil_6con_end);
  */
  printf("dist: %6.1f",0.);
  fflush(stdout);


  NbTotalPts = VipOffsetVolume(vol);

  /*6-neighbors*/
  
  ptr = VipGetDataPtr_S16BIT(vol);
  for ( i=0; i<NbTotalPts; i++ )
    {
      if (!*ptr)            
	{	  
	  maskptr = mask->first_point;
	  background_6neighbor = VFALSE;
	  for ( l=0;l<nuntil_6con_end;l++)
	    {
	      voisin = ptr + maskptr->offset;
	      neighbor = *voisin;
	      
	      if(neighbor && neighbor!=VIP_OUTSIDE_DOMAIN)
		{
		  newval = maskptr->dist; 
		  background_6neighbor = VTRUE;
		  if (newval < neighbor)
		    {
		      buckptr = Liste_Buckets_ptr[newval];
		      if(buckptr==NULL)
			{			  
			  Liste_Buckets[newval] = retire_bucket(chaine);
			  if(Liste_Buckets[newval]==PB) return(PB);
			  Liste_Buckets_ptr[newval] = Liste_Buckets[newval];
			  buckptr = Liste_Buckets_ptr[newval];
			}		       
		      if(buckptr->n_points==buckptr->size)
			{
			  buckptr->next = retire_bucket(chaine);
			  Liste_Buckets_ptr[newval] = buckptr->next;
			  buckptr = Liste_Buckets_ptr[newval];
			}
		      buckptr->data[buckptr->n_points++] = i + maskptr->offset;		      
		      *voisin = newval;		      
		    }
		}
	      maskptr++;
	    } 

	  if((geodesic_mode==VFALSE)&&(background_6neighbor==VTRUE))
	    {
	      buckptr = Liste_Buckets_ptr[0];
	      if(buckptr==NULL)
		{			  
		  Liste_Buckets[0] = retire_bucket(chaine);
		  if(Liste_Buckets[0]==PB) return(PB);
		  Liste_Buckets_ptr[0] = Liste_Buckets[0];
		  buckptr = Liste_Buckets_ptr[0];
		}
	      if(buckptr->n_points==buckptr->size)
		{
		  buckptr->next = retire_bucket(chaine);
		  Liste_Buckets_ptr[0] = buckptr->next;
		  buckptr = Liste_Buckets_ptr[0];
		}
	      buckptr->data[buckptr->n_points++] = i;	
	      
	    }                   
	}
      ptr++;
    }

  if(geodesic_mode==VTRUE)
    {
      maskafter6 = mask->first_point + 6;
      ptr = VipGetDataPtr_S16BIT(vol);
      for ( i=0; i<NbTotalPts; i++ )
	{
	  if (!*ptr)            
	    {	  
	      maskptr = maskafter6;
	      for (l=nuntil_6con_end;l<mask->length;l++)
		{
		  voisin = ptr + maskptr->offset;
		  neighbor = *voisin;
	      
		  if(neighbor && neighbor!=VIP_OUTSIDE_DOMAIN)
		    {
		      newval = maskptr->dist; 
		      if (newval < neighbor)
			{
			  buckptr = Liste_Buckets_ptr[newval];
			  if(buckptr==NULL)
			    {			  
			      Liste_Buckets[newval] = retire_bucket(chaine);
			      if(Liste_Buckets[newval]==PB) return(PB);
			      Liste_Buckets_ptr[newval] = Liste_Buckets[newval];
			      buckptr = Liste_Buckets_ptr[newval];
			    }		       
			  if(buckptr->n_points==buckptr->size)
			    {
			      buckptr->next = retire_bucket(chaine);
			      Liste_Buckets_ptr[newval] = buckptr->next;
			      buckptr = Liste_Buckets_ptr[newval];
			    }
			  buckptr->data[buckptr->n_points++] = i + maskptr->offset;		      
			  *voisin = newval;		      
			}
		    }
		  maskptr++;
		} 
	    }
	  ptr++;
	}
    }
  return(OK);
}

/*--------------------------------------------------------------------------*/
static int Vider_Buckets (VipIntBucket **chaine, Volume *vol,VipIntBucket **Liste_Buckets,
				VipIntBucket **Liste_Buckets_ptr,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS,int limit)
{ 
  DistmapMask_point *maskptr, *maskfastlim;
  Vip_S16BIT *ptr, *central, *voisin;
  int lim[10000], fastlength, current, nlim, n, total, debutbuck0;
  VipIntBucket *buckptr, *walker, *kill;
  int *pointptr;
  int newskip;
  int i, j, l, neighbor, newval, aux2;
  int nuntil_6con_end;


  ptr = VipGetDataPtr_S16BIT(vol);

  /*the distances are propagate from the smallest to the largest,
    then, the points should go in only one buffer*/


  nuntil_6con_end = 0;
  maskptr = mask->first_point;
  current = mask->first_point->dist;
  nlim = 0;
  lim[nlim] = 0;
  for(i=0;i<mask->length;i++)
    {
      if((abs((int)(maskptr->x))+abs((int)(maskptr->y))+abs((int)(maskptr->z)))==1)
	{
	  nuntil_6con_end = i+1;
	}
      if(maskptr->dist!=current)
	{
	  current = maskptr->dist;
	  nlim++;
	  lim[nlim] = 1;
	}
      else lim[nlim]++;
      maskptr++;
    }

  total = 0;
  debutbuck0 = 0;
  for(i=0;i<nlim;i++)
    {
      total+=lim[i];
      if(total==nuntil_6con_end)
	{
	  debutbuck0 = i+1;
	  break;
	}
    }
  if(debutbuck0==0)
    {
      VipPrintfError("Unconsistence in bucket 0 optimization stuff\n");
      VipPrintfExit("Vider_Buckets");
      return(PB);
    }

  limit = mVipMin(limit,LONGUEUR_LISTE_BUCKETS);
  

  for ( i=0; i<limit; i++ )
    {
      if (Liste_Buckets[i]!=NULL)
	{
	  printf("\b\b\b\b\b\b%6.1f",i/VIP_USED_DISTMAP_MULTFACT);
	  fflush(stdout);

	  maskfastlim = mask->first_point;
	  for(n=0;n<=nlim;n++)
	    {
	      fastlength = lim[n];
	      if(i==0 && n<debutbuck0)
		{
		  maskfastlim += fastlength;
		  continue; /*already done in remplissage...*/
		}
	      walker = Liste_Buckets[i];
	      while(walker!=NULL)
		{
		  pointptr = walker->data;
		  for ( j=walker->n_points; j--; )
		    { 	     
		      central  = ptr + *pointptr;
		      aux2 = *central;	 
		      if(aux2>=i)
			{
			  if(aux2>i)
			    {
			      VipPrintfWarning("bad bucket filling");
			    }
			  maskptr = maskfastlim;
			  for ( l=fastlength;l--;)
			    {
			      newskip = *pointptr + maskptr->offset;
			      voisin = ptr + newskip;
			      neighbor = *voisin;
			      
			      if((neighbor>i) &&(neighbor!=VIP_OUTSIDE_DOMAIN))
				{
				  newval = aux2 + maskptr->dist;
				  
				  if (newval < neighbor)
				    {
				      if(newval<limit)
					{
					  buckptr = Liste_Buckets_ptr[newval];
					  if(buckptr==NULL)
					    {			  
					      Liste_Buckets[newval] = retire_bucket(chaine);
					      if(Liste_Buckets[newval]==PB) return(PB);
					      Liste_Buckets_ptr[newval] = Liste_Buckets[newval];
					      buckptr = Liste_Buckets_ptr[newval];
					    }		       
					  if(buckptr->n_points==buckptr->size)
					    {
					      buckptr->next = retire_bucket(chaine);
					      Liste_Buckets_ptr[newval] = buckptr->next;
					      buckptr = Liste_Buckets_ptr[newval];
					    }
					  buckptr->data[buckptr->n_points++] = newskip;
					}
				      *voisin = newval;		
				    }			     
				}
			      maskptr++;
			    }
			}
		      pointptr++;
		    }
		  walker = walker->next;
		}
	      maskfastlim += fastlength;
	    }
	  walker = Liste_Buckets[i];
	  total = 0;
	  while(walker!=NULL)
	    {
	      kill = walker;
	      walker=walker->next;
	      ajoute_bucket(chaine,kill);
	      total++;
	    }
	  /*
	  printf("a la dist %d, il y avait %d bucket\n",i,total);
	  */
	  Liste_Buckets[i] = NULL;
	  Liste_Buckets_ptr[i] = NULL;
	}
   
    }
  printf("\n");

  for(i=0;i<LONGUEUR_LISTE_BUCKETS;i++)
    {
      walker = Liste_Buckets[i];
      total = 0;     
      if(walker!=NULL)
	{
	  /*
	  VipPrintfWarning("strange");
	  printf("a la dist %d, il y avait %d bucket\n",i,total);
	  */
	  while(walker!=NULL)
	    {
	      kill = walker;
	      walker=walker->next;
	      ajoute_bucket(chaine,kill);
	      total++;
	    }
	}
    }
  
  return(OK);
}

/*---------------------------------------------------------------------------*/
static int Remplissage_BucketsConnectivity( Volume *vol,DistmapMask *mask,
				VipIntBucket **Liste_Buckets)
{ 
 
  Vip_S16BIT *ptr, *voisin;
  int i, l, NbTotalPts, neighbor;
  DistmapMask_point *maskptr;
  VipIntBucket *buckptr;
 


  printf("dist: %6d",1);
  fflush(stdout);

  ptr = VipGetDataPtr_S16BIT(vol);

  NbTotalPts = VipOffsetVolume(vol);
 
  Liste_Buckets[1] = VipMallocIntBucket(LONG_BUCKET);


  for ( i=0; i<NbTotalPts; i++ )
    {
      if (!*ptr)            
	{
	  maskptr = mask->first_point;
	  
	  for ( l=mask->length;l--;)
	    {
	      voisin = ptr + maskptr->offset;
	      neighbor = *voisin;
	      
	      if(neighbor==CHAMFER_DOMAIN)
		{		  		     
		  buckptr = Liste_Buckets[1];
		  if(buckptr->n_points==buckptr->size)
		    {
		      if(VipIncreaseIntBucket(buckptr,INCREMENT_BUCKET)==PB) return(PB);
		    }
		  buckptr->data[buckptr->n_points++] = i + maskptr->offset;
		      
		  *voisin = 1;		      
		    
		}
	      maskptr++;

	    } 
	  
	}
      ptr++;
    }
  return(OK);
}

/*--------------------------------------------------------------------------*/
static int Vider_BucketsConnectivity (Volume *vol,VipIntBucket **liste,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS,int limit)
{ 
  DistmapMask_point *maskptr;
  Vip_S16BIT *ptr, *central, *voisin;
  VipIntBucket *chaine, *buckptr;
  int *pointptr;
  int newskip;
  int i, j, l, neighbor, aux2;
  int iplus1;


  ptr = VipGetDataPtr_S16BIT(vol);

  chaine = NULL;
  

  buckptr = VipMallocIntBucket(liste[1]->n_points + INCREMENT_BUCKET);
  ajoute_bucket(&chaine,buckptr);

  limit = mVipMin(limit,LONGUEUR_LISTE_BUCKETS);
  

  for ( i=1; i<limit; i++ )
    {
      if ((*(liste+i)!=NULL) && (liste[i]->n_points>0))
	{
	  printf("\b\b\b\b\b\b%6d",(int)(i/VIP_USED_DISTMAP_MULTFACT + 1));
	  fflush(stdout);

	  iplus1 = i+1;
	  liste[iplus1]=retire_bucket(&chaine);
	  buckptr = liste[iplus1];

	  pointptr = liste[i]->data;
	  for ( j=liste[i]->n_points; j--; )
	    { 
	     
	      central  = ptr + *pointptr;

	      aux2 = *central;
	     
	      maskptr = mask->first_point;
	      
	      for ( l=mask->length;l--;)
		{
		  newskip = *pointptr + maskptr->offset;
		  voisin = ptr + newskip;
		  neighbor = *voisin;
		     
		  if(neighbor==CHAMFER_DOMAIN)
		    {			 			 
		      if(buckptr->n_points==buckptr->size)
			{
			  if(VipIncreaseIntBucket(buckptr,INCREMENT_BUCKET)==PB) return(PB);
			}
		      buckptr->data[buckptr->n_points++] = newskip;	     
		      *voisin = iplus1;			     		      			 
		    }
		  maskptr++;
		}
	      pointptr++;
	    }
	  liste[i]->n_points = 0;
	  ajoute_bucket(&chaine,liste[i]);
	  liste[i] = NULL;
	}
   
    }
  printf("\n");
  for ( --i;i<LONGUEUR_LISTE_BUCKETS; i++ )
    {
      if (*(liste+i)!=NULL)
	{
	  ajoute_bucket(&chaine,liste[i]);
	  liste[i]=NULL;
	}
    }
   if(chaine!=NULL) VipFreeIntBucketList(chaine);
  VipFree(liste);
  
  return(OK);
}

/*---------------------------------------------------------------------------*/
static int Remplissage_BucketsConnectivityVoronoi( Volume *vol, Volume *label, DistmapMask *mask,
				VipIntBucket **Liste_Buckets)
{ 
 
  Vip_S16BIT *ptr, *voisin, *labelptr;
  int i, l, NbTotalPts, neighbor;
  DistmapMask_point *maskptr;
  VipIntBucket *buckptr;
 


  printf("dist: %6d",0);
  fflush(stdout);

  ptr = VipGetDataPtr_S16BIT(vol);
  labelptr = VipGetDataPtr_S16BIT(label);

  NbTotalPts = VipOffsetVolume(vol);
 
  Liste_Buckets[1] = VipMallocIntBucket(LONG_BUCKET);


  for ( i=0; i<NbTotalPts; i++ )
    {
      if (!*ptr)            
	{
	  maskptr = mask->first_point;
	  
	  for ( l=mask->length;l--;)
	    {
	      voisin = ptr + maskptr->offset;
	      neighbor = *voisin;
	      
	      if(neighbor==CHAMFER_DOMAIN)
		{		  		     
		  buckptr = Liste_Buckets[1];
		  if(buckptr->n_points==buckptr->size)
		    {
		      if(VipIncreaseIntBucket(buckptr,INCREMENT_BUCKET)==PB) return(PB);
		    }
		  buckptr->data[buckptr->n_points++] = i + maskptr->offset;
		      
		  *voisin = 1;	
		  *(labelptr+maskptr->offset) = *labelptr;
		    
		}
	      maskptr++;

	    } 
	  
	}
      ptr++;
      labelptr++;
    }
  return(OK);
}

/*--------------------------------------------------------------------------*/
static int Vider_BucketsConnectivityVoronoi (Volume *vol,Volume *label,VipIntBucket **liste,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS)
{ 
  DistmapMask_point *maskptr;
  Vip_S16BIT *ptr, *central, *voisin, *labelptr, *centrallabelptr;
  VipIntBucket *chaine, *buckptr;
  int labelcentral;
  int *pointptr;
  int newskip;
  int i, j, l, neighbor, aux2;
  int iplus1;


  ptr = VipGetDataPtr_S16BIT(vol);
  labelptr = VipGetDataPtr_S16BIT(label);

  chaine = NULL;
  

  buckptr = VipMallocIntBucket(liste[1]->n_points + INCREMENT_BUCKET);
  ajoute_bucket(&chaine,buckptr);
  

  for ( i=1; i<LONGUEUR_LISTE_BUCKETS; i++ )
    {
      if ((*(liste+i)!=NULL) && (liste[i]->n_points>0))
	{
	  printf("\b\b\b\b\b\b%6d",(int)(i/VIP_USED_DISTMAP_MULTFACT));
	  fflush(stdout);

	  iplus1 = i+1;
	  liste[iplus1]=retire_bucket(&chaine);
	  buckptr = liste[iplus1];

	  pointptr = liste[i]->data;
	  for ( j=liste[i]->n_points; j--; )
	    { 
	     
	      central  = ptr + *pointptr;

	      aux2 = *central;
	      centrallabelptr = labelptr + *pointptr; 
	      labelcentral = *centrallabelptr;

	      maskptr = mask->first_point;
	      
	      for ( l=mask->length;l--;)
		{
		  newskip = *pointptr + maskptr->offset;
		  voisin = ptr + newskip;
		  neighbor = *voisin;
		     
		  if(neighbor==CHAMFER_DOMAIN)
		    {			 			 
		      if(buckptr->n_points==buckptr->size)
			{
			  if(VipIncreaseIntBucket(buckptr,INCREMENT_BUCKET)==PB) return(PB);
			}
		      buckptr->data[buckptr->n_points++] = newskip;	     
		      *voisin = iplus1;	
		      *(centrallabelptr+maskptr->offset) = labelcentral;
		    }
		  maskptr++;
		}
	      pointptr++;
	    }
	  liste[i]->n_points = 0;
	  ajoute_bucket(&chaine,liste[i]);
	  liste[i] = NULL;
	}
   
    }
  printf("\n");

   if(chaine!=NULL) VipFreeIntBucketList(chaine);
  VipFree(liste);
  
  return(OK);
}
/*---------------------------------------------------------------------------*/
static int Remplissage_BucketsVoronoi( Volume *vol, Volume *label, DistmapMask *mask,
				VipIntBucket **Liste_Buckets)
{ 
 
  Vip_S16BIT *ptr, *voisin, *ptrlabel;
  int i, l, NbTotalPts, neighbor, newval;
  DistmapMask_point *maskptr;
  VipIntBucket *buckptr;
 


  printf("dist: %6d",0);
  fflush(stdout);

  ptr = VipGetDataPtr_S16BIT(vol);
  ptrlabel = VipGetDataPtr_S16BIT(label);


  NbTotalPts = VipOffsetVolume(vol);
 
  

  for ( i=0; i<NbTotalPts; i++ )
    {
      if (!*ptr)            
	{
	  maskptr = mask->first_point;
	  
	  for ( l=mask->length;l--;)
	    {
	      voisin = ptr + maskptr->offset;
	      neighbor = *voisin;
	      
	      if(neighbor && neighbor!=VIP_OUTSIDE_DOMAIN)
		{
		  newval = maskptr->dist; /*centralval=0*/
		  
		  if (newval < neighbor)
		    {
		      
		      if(Liste_Buckets[newval]==NULL)
			{
			  Liste_Buckets[newval] = VipMallocIntBucket(LONG_BUCKET);
			  if(Liste_Buckets[newval]==PB) return(PB);
			}
		      buckptr = Liste_Buckets[newval];
		      if(buckptr->n_points==buckptr->size)
			{
			  if(VipIncreaseIntBucket(buckptr,INCREMENT_BUCKET)==PB) return(PB);
			}
		      buckptr->data[buckptr->n_points++] = i + maskptr->offset;
		      
		      *voisin = newval;
		      *(ptrlabel+maskptr->offset) = *ptrlabel;

		    }
		}
	      maskptr++;

	    } 
	  
	}
                  /*label "closed"*/  
      ptr++;
      ptrlabel++;
    }
  return(OK);
}

/*--------------------------------------------------------------------------*/
static int Vider_BucketsVoronoi (Volume *vol, Volume *label, VipIntBucket **liste,
		    DistmapMask *mask,int LONGUEUR_LISTE_BUCKETS)
{ 
  DistmapMask_point *maskptr;
  Vip_S16BIT *ptr, *central, *voisin, *ptrlabel, *centrallabelptr;
  VipIntBucket *chaine, *buckptr;
  int *pointptr;
  int newskip, centrallabelvalue;
  int i, j, l, neighbor, newval, aux2;


  ptr = VipGetDataPtr_S16BIT(vol);
  ptrlabel = VipGetDataPtr_S16BIT(label);


  chaine = NULL;
  
  for ( i=1; i<LONGUEUR_LISTE_BUCKETS; i++ )
    {
      if (*(liste+i)!=NULL)
	{
	  printf("\b\b\b\b\b\b%6d",(int)(i/VIP_USED_DISTMAP_MULTFACT));
	  fflush(stdout);
	  pointptr = liste[i]->data;
	  for ( j=liste[i]->n_points; j--; )
	    { 
	     
	      central  = ptr + *pointptr;
	     
	      aux2 = *central;
	     
	      if(aux2>=i)
		{
		  centrallabelptr = ptrlabel + *pointptr;
		  centrallabelvalue = *centrallabelptr;
		  maskptr = mask->first_point;

		  for ( l=mask->length;l--;)
		    {
		      newskip = *pointptr + maskptr->offset;
		      voisin = ptr + newskip;
		      neighbor = *voisin;
		     
		      if((neighbor>i) &&(neighbor!=VIP_OUTSIDE_DOMAIN))
			{
			  newval = aux2 + maskptr->dist;
			 
			  if (newval < neighbor)
			    {
			      if (liste[newval]==NULL)
				{
				  liste[newval] = retire_bucket(&chaine);
				  if(liste[newval]==PB) return(PB);
				}
			      buckptr = liste[newval];

			      if(buckptr->n_points==buckptr->size)
				{
				  if(VipIncreaseIntBucket(buckptr,INCREMENT_BUCKET)==PB) return(PB);
				}
			      buckptr->data[buckptr->n_points++] = newskip;	     
			      *voisin = newval;
			      *(centrallabelptr+maskptr->offset) = centrallabelvalue;
			     
			    }
			 
			}
		      maskptr++;

		    }
		}
	      pointptr++;
	    }
	  liste[i]->n_points = 0;
	  ajoute_bucket(&chaine,liste[i]);
	  liste[i] = NULL;
	}
   
    }
  printf("\n");

   if(chaine!=NULL) VipFreeIntBucketList(chaine);
  VipFree(liste);
  
  return(OK);
}



/*-----------------------------------------------------------------------*/
static int ajoute_bucket(VipIntBucket **chaine,VipIntBucket *buck)
{
  VipIntBucket *aux;
  
  buck->n_points = 0;


  if(*chaine == NULL)
    {
      *chaine = buck;
      buck->next = NULL;
    }
  else
    {
      aux = *chaine;
      *chaine = buck;
      buck->next = aux;
    }
  return(OK);
}

/*------------------------------------------------------------------------*/
static VipIntBucket *retire_bucket(VipIntBucket **chaine)
{
  VipIntBucket *aux;
  static int n=0;

  if(*chaine == NULL)
    {
      n++;
      /*
      printf("nbuckets created (%d points): %d\n",LONG_BUCKET,n);
      */
      fflush(stdout);
      aux = VipAllocIntBucket(LONG_BUCKET);
      if(aux==PB) return(PB);
    }

  else
    {
      aux = *chaine;
      *chaine = (*chaine)->next;
      aux->next = NULL;
    }

  return (aux);
  
}

