/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_distmap_sweeping.c* TYPE     : Source
 * AUTHOR      : MANGIN J.F.           * CREATION : 10/10/1996
 * VERSION     : 0.1                   * REVISION :
 * LANGUAGE    : C                     * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne les balayages de la transformation
 *               du chanfrein
 *               jfmangin@cea.fr
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


/*--------------------------------------------------------------------*/
int ForwardSweepingWithBorder (
   Volume *vol,
   DistmapMask *mask,
   int borderlevel
)
/*--------------------------------------------------------------------*/
{
  int xcourant, l;
  Vip_S16BIT *volptr;
  int ycourant, zcourant;
  DistmapMask_point *maskptr, *mask_first_point;
  int maxval, newval;
  int xcubesize, ycubesize, zcubesize;
  int xsize, ysize, zsize;
  int xmaxtest, ymaxtest, zmaxtest;
  int mask_length;
  int CORRECT_BORDERWIDTH = VFALSE;
  VipOffsetStruct *vos;
  int neighbor;
  int nb_change = 0;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  ForwardSweepingWithBorder is only implemented for S16BIT volume");
      VipPrintfExit("(distmap_sweeping)ForwardSweepingWithBorder");
      return(PB);
    }


  if(borderlevel!=VIP_OUTSIDE_DOMAIN)
    {
      VipPrintfWarning("strange borderlevel in ForwardSweepingWithBorder");
      VipSetBorderLevel(vol, VIP_OUTSIDE_DOMAIN);
      borderlevel = VIP_OUTSIDE_DOMAIN;
    }

  if( (mVipVolBorderWidth(vol)>=mask->xcubesize)
      &&(mVipVolBorderWidth(vol)>=mask->ycubesize)
      &&(mVipVolBorderWidth(vol)>=mask->zcubesize)
      )
    CORRECT_BORDERWIDTH = VTRUE;
  else
    {
      CORRECT_BORDERWIDTH = VFALSE;
    }
  vos = VipGetOffsetStructure(vol);
  volptr = VipGetDataPtr_S16BIT( vol )+ vos->oFirstPoint;

  mask_first_point = mask->first_point;
  mask_length = mask->length;
  xcubesize = mask->xcubesize;
  ycubesize = mask->ycubesize;
  zcubesize = mask->zcubesize;
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  xmaxtest = xsize - xcubesize;
  ymaxtest = ysize - ycubesize;
  zmaxtest = zsize - zcubesize;
  xcourant = 1;
  ycourant = 1;
  zcourant = 1;


  (void)printf("Slice:   ");


  for( zcourant = 1; zcourant<=zsize; zcourant++)
    {
      (void)printf("\b\b\b%3d",zcourant);
      (void)fflush(stdout);

      for( ycourant = 1; ycourant<=ysize; ycourant++)
	{
	  if(CORRECT_BORDERWIDTH==VFALSE)
	    for( xcourant = 1 ; xcourant<=xsize; xcourant++)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    if( (xcourant<=xcubesize) || (xcourant>=xmaxtest) ||
			(ycourant<=ycubesize) || (ycourant>=ymaxtest) ||
			(zcourant<=zcubesize) || (zcourant>=zmaxtest) )
		      {
			maskptr = mask_first_point;
			for(l=mask_length;l--;)
			  {
			    if(  ( (xcourant-1) >=  - maskptr->x  )  &&
				 ( (xsize-xcourant) >= maskptr->x )  &&
				 ( (ycourant-1) >=  - maskptr->y  )  &&
				 ( (ysize-ycourant) >= maskptr->y )  &&
				 ( (zcourant-1) >=  - maskptr->z )  &&
				 ( (zsize-zcourant) >= maskptr->z ) )
			      {
				newval =   *(volptr + maskptr->offset) +(maskptr->dist);
				if (newval<maxval) maxval = newval;
			      }
			    maskptr++;
			  }
		      }
		    else
		      {
			maskptr = mask_first_point;
			for(l=mask_length; l--; )
			  {
			    newval =   *(volptr + maskptr->offset) + (maskptr->dist);
			    if (newval<maxval) maxval = newval;
			    maskptr++;
			  }
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
		      }
		  }
		volptr++;
	      }
	  else
	    for( xcourant = 1 ; xcourant<=xsize; xcourant++)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    maskptr = mask_first_point;
		    for(l=mask_length; l--; )
		      {
			neighbor = *(volptr + maskptr->offset);
			if (neighbor!=borderlevel)
			  {
			    newval = neighbor + (maskptr->dist);
			    if (newval<maxval) maxval = newval;
			  }
			maskptr++;
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
		      }
		  }
		volptr++;
	      }
	  volptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      volptr += vos->oLineBetweenSlice; /*skip border lines*/ 
    }
  printf("\n");

  VipFree(vos);

  return(nb_change);
}

/*--------------------------------------------------------------------*/
int BackwardSweepingWithBorder (
   Volume *vol,
   DistmapMask *mask,
   int borderlevel
)
/*--------------------------------------------------------------------*/
{
  int xcourant, l;
  Vip_S16BIT *volptr;
  int ycourant, zcourant;
  DistmapMask_point *maskptr, *mask_first_point;
  int maxval, newval;
  int xcubesize, ycubesize, zcubesize;
  int xsize, ysize, zsize;
  int xmaxtest, ymaxtest, zmaxtest;
  int mask_length;
  int CORRECT_BORDERWIDTH = VFALSE;
  VipOffsetStruct *vos;
  int neighbor;
  int nb_change = 0;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  BackwardSweepingWithBorder is only implemented for S16BIT volume");
      VipPrintfExit("(distmap_sweeping)BackwardSweepingWithBorder");
      return(PB);
    }

  if(borderlevel!=VIP_OUTSIDE_DOMAIN)
    {
      VipPrintfWarning("strange borderlevel in BackwardSweepingWithBorder");
      VipSetBorderLevel(vol, VIP_OUTSIDE_DOMAIN);
      borderlevel = VIP_OUTSIDE_DOMAIN;
    }

  if( (mVipVolBorderWidth(vol)>=mask->xcubesize)
      &&(mVipVolBorderWidth(vol)>=mask->ycubesize)
      &&(mVipVolBorderWidth(vol)>=mask->zcubesize)
      )
    CORRECT_BORDERWIDTH = VTRUE;
  else
    {
      CORRECT_BORDERWIDTH = VFALSE;
    }
  vos = VipGetOffsetStructure(vol);
  volptr = VipGetDataPtr_S16BIT( vol )+ vos->oLastPoint;

  mask_first_point = mask->first_point;
  mask_length = mask->length;
  xcubesize = mask->xcubesize;
  ycubesize = mask->ycubesize;
  zcubesize = mask->zcubesize;
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  xmaxtest = xsize - xcubesize;
  ymaxtest = ysize - ycubesize;
  zmaxtest = zsize - zcubesize;

  (void)printf("Slice:   ");

  for( zcourant = zsize; zcourant>0; zcourant--)
    {
      (void)printf("\b\b\b%3d",zcourant);
      (void)fflush(stdout);

      for( ycourant = ysize; ycourant>0; ycourant--)
	{
	  if(CORRECT_BORDERWIDTH==VFALSE)
	    for( xcourant = xsize ; xcourant--;)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    if( (xcourant<=xcubesize) || (xcourant>=xmaxtest) ||
			(ycourant<=ycubesize) || (ycourant>=ymaxtest) ||
			(zcourant<=zcubesize) || (zcourant>=zmaxtest) )
		      {
			maskptr = mask_first_point;
			for(l=mask_length;l--;)
			  {
			    if(  ( (xcourant-1) >=  - maskptr->x  )  &&
				 ( (xsize-xcourant) >= maskptr->x )  &&
				 ( (ycourant-1) >=  - maskptr->y  )  &&
				 ( (ysize-ycourant) >= maskptr->y )  &&
				 ( (zcourant-1) >=  - maskptr->z )  &&
				 ( (zsize-zcourant) >= maskptr->z ) )
			      {
				newval =   *(volptr + maskptr->offset) +(maskptr->dist);
				if (newval<maxval) maxval = newval;
			      }
			    maskptr++;
			  }
		      }
		    else
		      {
			maskptr = mask_first_point;
			for(l=mask_length; l--; )
			  {
			    newval =   *(volptr + maskptr->offset) + (maskptr->dist);
			    if (newval<maxval) maxval = newval;
			    maskptr++;
			  }
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
		      }
		  }
		volptr--;
	      }
	  else
	    for( xcourant = xsize ; xcourant--;)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    maskptr = mask_first_point;
		    for(l=mask_length; l--; )
		      {
			neighbor = *(volptr + maskptr->offset);
			if (neighbor!=borderlevel)
			  {
			    newval = neighbor + (maskptr->dist);
			    if (newval<maxval) maxval = newval;
			  }
			maskptr++;
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
		      }
		  }
		volptr--;
	      }
	  volptr -= vos->oPointBetweenLine;  /*skip border points*/
	}
      volptr -= vos->oLineBetweenSlice; /*skip border lines*/ 
    }
  printf("\n");

  VipFree(vos);

  return(nb_change);
}

/*--------------------------------------------------------------------*/
int ForwardSweepingVoronoiWithBorder (
   Volume *vol,
   Volume *label,
   DistmapMask *mask,
   int borderlevel
)
/*--------------------------------------------------------------------*/
{
  int xcourant, l;
  Vip_S16BIT *volptr, *labelptr;
  int ycourant, zcourant;
  DistmapMask_point *maskptr, *mask_first_point;
  int maxval, newval;
  int xcubesize, ycubesize, zcubesize;
  int xsize, ysize, zsize;
  int xmaxtest, ymaxtest, zmaxtest;
  int mask_length;
  int CORRECT_BORDERWIDTH = VFALSE;
  VipOffsetStruct *vos, *voslabel;
  int neighbor;
  int nb_change = 0;
  int maxlabel=0;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,ForwardSweepingVoronoiWithBorder is only implemented for S16BIT distance volume");
      VipPrintfExit("(distmap_sweeping)ForwardSweepingVoronoiWithBorder");
      return(PB);
    }

 if (VipTestType(label,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  ForwardSweepingVoronoiWithBorder is only implemented for S16BIT label volume");
      VipPrintfExit("(distmap_sweeping)ForwardSweepingVoronoiWithBorder");
      return(PB);
    }


  if(borderlevel!=VIP_OUTSIDE_DOMAIN)
    {
      VipPrintfWarning("strange borderlevel in ForwardSweepingVoronoiWithBorder");
      VipSetBorderLevel(vol, VIP_OUTSIDE_DOMAIN);
      borderlevel = VIP_OUTSIDE_DOMAIN;
    }

  if( (mVipVolBorderWidth(vol)>=mask->xcubesize)
      &&(mVipVolBorderWidth(vol)>=mask->ycubesize)
      &&(mVipVolBorderWidth(vol)>=mask->zcubesize)
      )
    CORRECT_BORDERWIDTH = VTRUE;
  else
    {
      CORRECT_BORDERWIDTH = VFALSE;
    }

  vos = VipGetOffsetStructure(vol);
  voslabel = VipGetOffsetStructure(label);

 
  volptr = VipGetDataPtr_S16BIT( vol )+ vos->oFirstPoint;
  labelptr = VipGetDataPtr_S16BIT( label )+ voslabel->oFirstPoint;


  mask_first_point = mask->first_point;
  mask_length = mask->length;
  xcubesize = mask->xcubesize;
  ycubesize = mask->ycubesize;
  zcubesize = mask->zcubesize;
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  xmaxtest = xsize - xcubesize;
  ymaxtest = ysize - ycubesize;
  zmaxtest = zsize - zcubesize;
  xcourant = 1;
  ycourant = 1;
  zcourant = 1;


  (void)printf("Slice:   ");


  for( zcourant = 1; zcourant<=zsize; zcourant++)
    {
      (void)printf("\b\b\b%3d",zcourant);

      (void)fflush(stdout);

      for( ycourant = 1; ycourant<=ysize; ycourant++)
	{
	  if(CORRECT_BORDERWIDTH==VFALSE)
	    for( xcourant = 1 ; xcourant<=xsize; xcourant++)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    if( (xcourant<=xcubesize) || (xcourant>=xmaxtest) ||
			(ycourant<=ycubesize) || (ycourant>=ymaxtest) ||
			(zcourant<=zcubesize) || (zcourant>=zmaxtest) )
		      {
			maskptr = mask_first_point;
			for(l=mask_length;l--;)
			  {
			    if(  ( (xcourant-1) >=  - maskptr->x  )  &&
				 ( (xsize-xcourant) >= maskptr->x )  &&
				 ( (ycourant-1) >=  - maskptr->y  )  &&
				 ( (ysize-ycourant) >= maskptr->y )  &&
				 ( (zcourant-1) >=  - maskptr->z )  &&
				 ( (zsize-zcourant) >= maskptr->z ) )
			      {
				newval =   *(volptr + maskptr->offset) +(maskptr->dist);
				if (newval<maxval)
				  {
				    maxval = newval;
				    maxlabel = *(labelptr+maskptr->offset);
				  }
			      }
			    maskptr++;
			  }
		      }
		    else
		      {
			maskptr = mask_first_point;
			for(l=mask_length; l--; )
			  {
			    newval =   *(volptr + maskptr->offset) + (maskptr->dist);
			    if(newval<maxval)
			      {
				maxval = newval;
				maxlabel = *(labelptr+maskptr->offset);
			      }
			    maskptr++;
			  }
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
			*labelptr = maxlabel;
		      }
		  }		
		volptr++;
		labelptr++;	
	      }
	  else
	    for( xcourant = xsize ; xcourant--;)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    maskptr = mask_first_point;
		    for(l=mask_length; l--; )
		      {
			neighbor = *(volptr + maskptr->offset);
			if (neighbor!=borderlevel)
			  {
			    newval = neighbor + (maskptr->dist);
			    if (newval<maxval)
			    {
			      maxval = newval;
			      maxlabel = *(labelptr+maskptr->offset);
			    }
			  }
			maskptr++;
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
			*labelptr = maxlabel;
		      }
		  }	
		volptr++;
		labelptr++;		
	      }	
	  volptr += vos->oPointBetweenLine;  /*skip border points*/
	  labelptr += voslabel->oPointBetweenLine;  /*skip border points*/	   
	}      
      volptr += vos->oLineBetweenSlice; /*skip border lines*/ 
      labelptr += voslabel->oLineBetweenSlice; /*skip border lines*/
    }
  printf("\n");

  VipFree(vos);

  return(nb_change);
}

/*--------------------------------------------------------------------*/
int BackwardSweepingVoronoiWithBorder (
   Volume *vol,
   Volume *label,
   DistmapMask *mask,
   int borderlevel
)
/*--------------------------------------------------------------------*/
{
  int xcourant, l;
  Vip_S16BIT *volptr, *labelptr;
  int ycourant, zcourant;
  DistmapMask_point *maskptr, *mask_first_point;
  int maxval, newval;
  int xcubesize, ycubesize, zcubesize;
  int xsize, ysize, zsize;
  int xmaxtest, ymaxtest, zmaxtest;
  int mask_length;
  int CORRECT_BORDERWIDTH = VFALSE;
  VipOffsetStruct *vos, *voslabel;
  int neighbor;
  int nb_change = 0;
  int maxlabel=0;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,BackwardSweepingVoronoiWithBorder is only implemented for S16BIT distance volume");
      VipPrintfExit("(distmap_sweeping)BackwardSweepingVoronoiWithBorder");
      return(PB);
    }

 if (VipTestType(label,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  BackwardSweepingVoronoiWithBorder is only implemented for S16BIT label volume");
      VipPrintfExit("(distmap_sweeping)BackwardSweepingVoronoiWithBorder");
      return(PB);
   }


  if(borderlevel!=VIP_OUTSIDE_DOMAIN)
    {
      VipPrintfWarning("strange borderlevel in BackwardSweepingVoronoiWithBorder");
      VipSetBorderLevel(vol, VIP_OUTSIDE_DOMAIN);
      borderlevel = VIP_OUTSIDE_DOMAIN;
    }

  if( (mVipVolBorderWidth(vol)>=mask->xcubesize)
      &&(mVipVolBorderWidth(vol)>=mask->ycubesize)
      &&(mVipVolBorderWidth(vol)>=mask->zcubesize)
      )
    CORRECT_BORDERWIDTH = VTRUE;
  else
    {
      CORRECT_BORDERWIDTH = VFALSE;
    }

  vos = VipGetOffsetStructure(vol);
  voslabel = VipGetOffsetStructure(label);

  
  volptr = VipGetDataPtr_S16BIT( vol )+ vos->oLastPoint;
  labelptr = VipGetDataPtr_S16BIT( label )+ voslabel->oLastPoint;
   

  mask_first_point = mask->first_point;
  mask_length = mask->length;
  xcubesize = mask->xcubesize;
  ycubesize = mask->ycubesize;
  zcubesize = mask->zcubesize;
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  xmaxtest = xsize - xcubesize;
  ymaxtest = ysize - ycubesize;
  zmaxtest = zsize - zcubesize;
  xcourant = 1;
  ycourant = 1;
  zcourant = 1;


  (void)printf("Slice:   ");


  for( zcourant = zsize; zcourant--;)
    {
      (void)printf("\b\b\b%3d",zcourant);

      (void)fflush(stdout);

      for( ycourant = ysize; ycourant--;)
	{
	  if(CORRECT_BORDERWIDTH==VFALSE)
	    for( xcourant = xsize ; xcourant--;)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    if( (xcourant<=xcubesize) || (xcourant>=xmaxtest) ||
			(ycourant<=ycubesize) || (ycourant>=ymaxtest) ||
			(zcourant<=zcubesize) || (zcourant>=zmaxtest) )
		      {
			maskptr = mask_first_point;
			for(l=mask_length;l--;)
			  {
			    if(  ( (xcourant-1) >=  - maskptr->x  )  &&
				 ( (xsize-xcourant) >= maskptr->x )  &&
				 ( (ycourant-1) >=  - maskptr->y  )  &&
				 ( (ysize-ycourant) >= maskptr->y )  &&
				 ( (zcourant-1) >=  - maskptr->z )  &&
				 ( (zsize-zcourant) >= maskptr->z ) )
			      {
				newval =   *(volptr + maskptr->offset) +(maskptr->dist);
				if (newval<maxval)
				  {
				    maxval = newval;
				    maxlabel = *(labelptr+maskptr->offset);
				  }
			      }
			    maskptr++;
			  }
		      }
		    else
		      {
			maskptr = mask_first_point;
			for(l=mask_length; l--; )
			  {
			    newval =   *(volptr + maskptr->offset) + (maskptr->dist);
			    if(newval<maxval)
			      {
				maxval = newval;
				maxlabel = *(labelptr+maskptr->offset);
			      }
			    maskptr++;
			  }
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
			*labelptr = maxlabel;
		      }
		  }		
		volptr++;
		labelptr++;		 
	      }
	  else
	    for( xcourant = xsize ; xcourant--;)
	      {
		maxval =  *volptr;	
		if (maxval!=VIP_OUTSIDE_DOMAIN) 
		  {
		    maskptr = mask_first_point;
		    for(l=mask_length; l--; )
		      {
			neighbor = *(volptr + maskptr->offset);
			if (neighbor!=borderlevel)
			  {
			    newval = neighbor + (maskptr->dist);
			    if (newval<maxval)
			    {
			      maxval = newval;
			      maxlabel = *(labelptr+maskptr->offset);
			    }
			  }
			maskptr++;
		      }
		    if( *volptr != maxval )
		      {
			nb_change++;
			*volptr = (Vip_S16BIT) maxval;
			*labelptr = maxlabel;
		      }
		  }	
		volptr--;
		labelptr--;		 
	      }	  
	  volptr -= vos->oPointBetweenLine;  /*skip border points*/
	  labelptr -= voslabel->oPointBetweenLine;  /*skip border points*/	  
	}     
      volptr -= vos->oLineBetweenSlice; /*skip border lines*/ 
      labelptr -= voslabel->oLineBetweenSlice; /*skip border lines*/
    }
  printf("\n");

  VipFree(vos);

  return(nb_change);
}



