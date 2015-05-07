/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_gmorpho.c          * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 17/01/1998
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : filtrages morphologiques en niveaux de gris
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
#include <vip/morpho.h>	
#include <vip/connex.h>	


/*finally not used but could be interested for other stuff */
/* it's a sophisticated gray level morphological closing
involving the combination of several directional closing */
/*---------------------------------------------------------------------------*/
Volume *VipIntersectionDirectionalGrayLevelClosing(Volume *vol)
/*---------------------------------------------------------------------------*/
{
  Volume *out;
  VipConnectivityStruct *vcs;
  int icon, current;
  Vip_S16BIT *ptr, *outptr;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int borderval = -123;
  int dirmax, fullmin, theval, val1, val2, offset, *offsetptr;

   if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("VipIntersectionDirectionalGrayLevelClosing");
      return(PB);
    }

   if(mVipVolBorderWidth(vol)<=0)
     {
       VipPrintfError("non zero borderwidth required");
       VipPrintfExit("VipIntersectionDirectionalGrayLevelClosing");
       return(PB);
     }
   if(VipSetBorderLevel(vol,borderval)==PB) return(PB);

   out = VipDuplicateVolumeStructure( vol, "copy");
   if(out==PB) return(PB);
   if(VipAllocateVolumeData(out)==PB) return(PB);

   vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
   if(vcs==PB) return(PB);

   current = 0;
   for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
      {
	if(vcs->offset[icon]>0)
	  {
	    vcs->offset[current] = vcs->offset[icon];
	    vcs->xyz_offset[current].x = vcs->xyz_offset[icon].x;
	    vcs->xyz_offset[current].y = vcs->xyz_offset[icon].y;
	    vcs->xyz_offset[current].z = vcs->xyz_offset[icon].z;
	    current++;
	  }
      }
   if(current!=13)
     {
       VipPrintfError("problem with oriented connectivity");
       VipPrintfExit("VipIntersectionDirectionalGrayLevelClosing");
       return(PB);
     }
   vcs->nb_neighbors = current;

   vos = VipGetOffsetStructure(vol);

   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
   outptr = VipGetDataPtr_S16BIT( out ) + vos->oFirstPoint;

   (void)printf("Slice:   ");

   for ( iz = 0; iz < mVipVolSizeZ(vol); iz++ )               /* loop on slices */
   {
      (void)printf("\b\b\b%3d",iz+1);
      fflush(stdout);
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            theval = *ptr;
	    fullmin = 100000;
	    offsetptr = vcs->offset;
	    for(icon=vcs->nb_neighbors ; icon--;)
	      {
		offset = *offsetptr++;
		val1 = *(ptr+offset);
		val2 = *(ptr-offset);
		dirmax = mVipMax(val1,val2);
		dirmax = mVipMax(theval,dirmax);
		if(dirmax<fullmin) fullmin=dirmax;
	      }
	    *outptr++ = fullmin;
	    ptr++;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
         outptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      outptr += vos->oLineBetweenSlice; /*skip border lines*/
   }

   printf("\n");

   VipFreeConnectivityStruct(vcs);

   return(out);
 
}


/*---------------------------------------------------------------------------*/
Volume *VipGrayLevelPlanDilation(Volume *vol, int connectivity, int forbidden)
/*---------------------------------------------------------------------------*/
{
  Volume *out;
  VipConnectivityStruct *vcs;
  int icon;
  Vip_S16BIT *ptr, *outptr;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int offset, *offsetptr;
  int themax;
  int nval;

   if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("VipGrayLevelPlanDilation");
      return(PB);
    }

   if(mVipVolBorderWidth(vol)<=0)
     {
       VipPrintfError("non zero borderwidth required");
       VipPrintfExit("VipGrayLevelPlanDilation");
       return(PB);
     }
   if(VipSetBorderLevel(vol,forbidden)==PB) return(PB);

   out = VipDuplicateVolumeStructure( vol, "copy");
   if(out==PB) return(PB);
   if(VipAllocateVolumeData(out)==PB) return(PB);

   vcs = VipGetConnectivityStruct( vol, connectivity );
   if(vcs==PB) return(PB);

   vos = VipGetOffsetStructure(vol);

   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
   outptr = VipGetDataPtr_S16BIT( out ) + vos->oFirstPoint;

   (void)printf("Slice:   ");

   for ( iz = 0; iz < mVipVolSizeZ(vol); iz++ )               /* loop on slices */
   {
      (void)printf("\b\b\b%3d",iz+1);
      fflush(stdout);
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
	    themax = *ptr;
	    if(themax!=forbidden)
		{
		    offsetptr = vcs->offset;
		    for(icon=vcs->nb_neighbors ; icon--;)
			{
			    offset = *offsetptr++;
			    nval = *(ptr+offset);
			    if(nval!=forbidden) themax = mVipMax(themax,nval);				
			}
		}
	    *outptr++ = themax;
	    ptr++;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
         outptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      outptr += vos->oLineBetweenSlice; /*skip border lines*/
   }

   printf("\n");

   VipFreeConnectivityStruct(vcs);

   return(out);
 
}

/*---------------------------------------------------------------------------*/
Volume *VipGrayLevelPlanErosionDilation(Volume *vol, int connectivity, int forbidden)
/*---------------------------------------------------------------------------*/
{
    Volume *temp, *result;
    
   if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("VipGrayLevelPlanErosionDilation");
      return(PB);
    }

   temp = VipGrayLevelPlanErosion( vol, connectivity, forbidden);
   if(temp==PB) return(PB);

   result = VipGrayLevelPlanDilation( temp, connectivity, forbidden);
   if(result==PB) return(PB);

   VipFreeVolume(temp);

   return(result);
}

/*---------------------------------------------------------------------------*/
Volume *VipGrayLevelPlanDilationErosion(Volume *vol, int connectivity, int forbidden)
/*---------------------------------------------------------------------------*/
{
    Volume *temp, *result;
    
   if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("VipGrayLevelPlanDilationErosion");
      return(PB);
    }

   temp = VipGrayLevelPlanDilation( vol, connectivity, forbidden);
   if(temp==PB) return(PB);

   result = VipGrayLevelPlanErosion( temp, connectivity, forbidden);
   if(result==PB) return(PB);

   VipFreeVolume(temp);

   return(result);
}

/*---------------------------------------------------------------------------*/
Volume *VipGrayLevelPlanErosion(Volume *vol, int connectivity, int forbidden)
/*---------------------------------------------------------------------------*/
{
  Volume *out;
  VipConnectivityStruct *vcs;
  int icon;
  Vip_S16BIT *ptr, *outptr;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int offset, *offsetptr;
  int themin;
  int nval;


   if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("VipGrayLevelPlanErosion");
      return(PB);
    }

   if(mVipVolBorderWidth(vol)<=0)
     {
       VipPrintfError("non zero borderwidth required");
       VipPrintfExit("VipGrayLevelPlanErosion");
       return(PB);
     }
   if(VipSetBorderLevel(vol,forbidden)==PB) return(PB);

   out = VipDuplicateVolumeStructure( vol, "copy");
   if(out==PB) return(PB);
   if(VipAllocateVolumeData(out)==PB) return(PB);

   vcs = VipGetConnectivityStruct( vol, connectivity );
   if(vcs==PB) return(PB);

   vos = VipGetOffsetStructure(vol);

   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
   outptr = VipGetDataPtr_S16BIT( out ) + vos->oFirstPoint;

   (void)printf("Slice:   ");

   for ( iz = 0; iz < mVipVolSizeZ(vol); iz++ )               /* loop on slices */
   {
      (void)printf("\b\b\b%3d",iz+1);
      fflush(stdout);
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
	    themin = *ptr;
	    if(themin!=forbidden)
		{
		    offsetptr = vcs->offset;
		    for(icon=vcs->nb_neighbors ; icon--;)
			{
			    offset = *offsetptr++;
			    nval = *(ptr+offset);
			    if(nval!=forbidden) themin = mVipMin(themin,nval);				
			}
		}
	    *outptr++ = themin;
	    ptr++;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
         outptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      outptr += vos->oLineBetweenSlice; /*skip border lines*/
   }

   printf("\n");

   VipFreeConnectivityStruct(vcs);

   return(out);
 
}
