/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_3Dconnex.c       * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 05/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne les composantes connexes 3D
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
#include <vip/alloc.h>

/*----------------------------------------------------------------------------*/
int VipTestCoordBucketConnectedToLabel(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  VipConnectivityStruct *vcs,
  int label)
/*----------------------------------------------------------------------------*/
{
  Vip_S16BIT *ptr;
  VipOffsetStruct *vos;
  Vip3DPoint_S16BIT *ptr3d;
  int i;
  int icon;
  Vip_S16BIT *ptr_neighbor, *the_ptr;
  int *offset;   
  int flagOK;

  if(!buck || !vol || !vcs)
    {
      VipPrintfError("Args in VipTestCoordBucketConnectedToLabel");
      VipPrintfExit("VipTestCoordBucketConnectedToLabel");
      return(PB);
    }

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

  ptr3d = buck->data;
  flagOK = VFALSE;
  for(i=buck->n_points;i--;)
    {
      the_ptr = ptr + ptr3d->x + ptr3d->y * vos->oLine + ptr3d->z * vos->oSlice;
      offset = vcs->offset;
      for(icon=vcs->nb_neighbors ; icon-- ; )
	{
	  ptr_neighbor = the_ptr + *offset++;
	  if(*ptr_neighbor==label)
	    {
	      flagOK=VTRUE;
	      break;
	    }
	}
      if(flagOK==VTRUE) break;
      ptr3d++;
    }
 
  VipFree(vos);

  return(flagOK);
}
/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how)
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/*----------------------------------------------------------------------------*/

{
  Vip3DBucket_S16BIT *cclist, *walker, *killer;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
   {
     VipPrintfExit("(3Dconnex.c)VipGetFiltered3DConnex");
     return(PB);
   }

  
  cclist = VipGet3DConnex( vol, connectivity);

  if(how>=0)
    {
      walker = cclist;
      while(walker!=NULL)
	{
	  if ( walker->n_points < how)
	    {
	      killer = walker;
	      walker = walker->next;
	      cclist = VipUnhook3DBucketFromList_S16BIT ( killer, cclist );
	      VipFree3DBucket_S16BIT( killer);
	    }
	  else walker = walker->next;
	}
    }
  else if (how<0)
    {
      walker = VipGetNthBiggest3DBucket_S16BIT (cclist, -how);
      if(walker==PB) return(PB);
      cclist = VipUnhook3DBucketFromList_S16BIT ( walker, cclist );
      VipFree3DBucketList_S16BIT( cclist);
      cclist = walker;
    }

  return(cclist);

}

/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetDualFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how)
  /* returns the list of connected comp. which are deleted by the corresponding
VipGetFiltered3DConnex*/
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/*----------------------------------------------------------------------------*/

{
  Vip3DBucket_S16BIT *cclist, *walker, *killer, *ccdeadlist, *deadptr;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
   {
     VipPrintfExit("(3Dconnex.c)VipGetDualFiltered3DConnex");
     return(PB);
   }

  cclist = VipGet3DConnex( vol, connectivity);

  ccdeadlist = NULL;
  deadptr = NULL;

  if (how >= 0)
    {
      walker = cclist;
      while(walker!=NULL)
	{
	  if (walker->n_points < how)
	    {
	      killer = walker;
	      walker = walker->next;
	      cclist = VipUnhook3DBucketFromList_S16BIT( killer, cclist );
	      if (ccdeadlist == NULL)
		{
		  ccdeadlist = killer;
		  deadptr = killer;
		}
	      else
		{
		  deadptr->next = killer;
		  deadptr=killer;
		}
	    }
	  else walker = walker->next;
	}
      VipFree3DBucketList_S16BIT( cclist);
    }
  else if (how < 0)
    {
      walker = VipGetNthBiggest3DBucket_S16BIT(cclist, -how);
      if (walker == PB) return(PB);
      ccdeadlist = VipUnhook3DBucketFromList_S16BIT( walker, cclist );
      VipFree3DBucketList_S16BIT(walker);
    }

  return(ccdeadlist);
}

/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetHysteresisFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how,
  float threshold,
  int mode,
  int param)
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/* follows a low thresholding. If mode==HYSTE_NUMBER, all cc with at least
"param" points over threshold will be kept, if mode==HYSTE_PERCENT,
all cc with percent points over threshold will be kept */
/*----------------------------------------------------------------------------*/

{
  Vip3DBucket_S16BIT *cclist, *walker, *killer;
  int limit=0;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
   {
     VipPrintfExit("(3Dconnex.c)VipGetHysteresisFiltered3DConnex");
     return(PB);
   }

  if((mode!=HYSTE_NUMBER)&&(mode!=HYSTE_PERCENT))
   {
      VipPrintfError("Sorry unknown mode...");
      VipPrintfExit("VipGetHysteresisFiltered3DConnex");
      return(PB);
   }
  if((mode==HYSTE_NUMBER)&&(param<=0))
 {
      VipPrintfError("Negative or zero parameter in HYSTE_NUMBER mode?");
      VipPrintfExit("VipGetHysteresisFiltered3DConnex");
      return(PB);
   }
  if((mode==HYSTE_PERCENT)&&((param<=0)||(param>100)))
 {
      VipPrintfError("Allowed range: [1..100) for HYSTE_PERCENT mode?");
      VipPrintfExit("VipGetHysteresisFiltered3DConnex");
      return(PB);
   }


  cclist = VipGet3DConnex( vol, connectivity);
  if(mode==HYSTE_NUMBER) limit=param;

  if(how>=0)
    {
      walker = cclist;
      while(walker!=NULL)
	{
          if(walker->n_points<=0)
           {
              VipPrintfError("empty bucket?");
              VipFree3DBucketList_S16BIT( cclist);
              VipPrintfExit("VipGetHysteresisFiltered3DConnex");
              return(PB);
           }
          if(mode==HYSTE_PERCENT) limit=(int)((float)param/100.
                                        *walker->n_points + 0.5);
	  if ( (walker->n_points < how)||
               (TestHysteBucket(walker,vol,limit,threshold)==VFALSE))
	    {
	      killer = walker;
	      walker = walker->next;
	      cclist = VipUnhook3DBucketFromList_S16BIT ( killer, cclist );
	      VipFree3DBucket_S16BIT( killer);
	    }
	  else walker = walker->next;
	}
    }
  else if (how<0)
    {
      walker = cclist;
      while(walker!=NULL)
	{
          if(walker->n_points<=0)
           {
              VipPrintfError("empty bucket?");
              VipFree3DBucketList_S16BIT( cclist);
              VipPrintfExit("VipGetHysteresisFiltered3DConnex");
              return(PB);
           }

          if(mode==HYSTE_PERCENT) limit=(int)((float)param/100.
                                        *walker->n_points + 0.5);
	  if ((TestHysteBucket(walker,vol,limit,threshold)==VFALSE))
	    {
	      killer = walker;
	      walker = walker->next;
	      cclist = VipUnhook3DBucketFromList_S16BIT ( killer, cclist );
	      VipFree3DBucket_S16BIT( killer);
	    }
	  else walker = walker->next;
	}

      walker = VipGetNthBiggest3DBucket_S16BIT (cclist, -how);
      if(walker==PB) return(PB);
      cclist = VipUnhook3DBucketFromList_S16BIT ( walker, cclist );
      VipFree3DBucketList_S16BIT( cclist);
      cclist = walker;
    }

  return(cclist);

}

/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetDualHysteresisFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how,
  float threshold,
  int mode,
  int param) 
  /* returns the list of connected comp. which are deleted by the corresponding
VipGetHysteresisFiltered3DConnex*/
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/*----------------------------------------------------------------------------*/

{
  Vip3DBucket_S16BIT *cclist, *walker, *killer, *ccdeadlist, *deadptr;
  int limit=0;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
   {
     VipPrintfExit("(3Dconnex.c)VipGetDualHysteresisFiltered3DConnex");
     return(PB);
   }

  

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
   {
     VipPrintfExit("(3Dconnex.c)VipGetDualHysteresisFiltered3DConnex");
     return(PB);
   }

  if((mode!=HYSTE_NUMBER)&&(mode!=HYSTE_PERCENT))
   {
      VipPrintfError("Sorry unknown mode...");
      VipPrintfExit("VipGetDualHysteresisFiltered3DConnex");
      return(PB);
   }
  if((mode==HYSTE_NUMBER)&&(param<=0))
 {
      VipPrintfError("Negative or zero parameter in HYSTE_NUMBER mode?");
      VipPrintfExit("VipGetDualHysteresisFiltered3DConnex");
      return(PB);
   }
  if((mode==HYSTE_PERCENT)&&((param<=0)||(param>100)))
 {
      VipPrintfError("Allowed range: [1..100) for HYSTE_PERCENT mode?");
      VipPrintfExit("VipGetDualHysteresisFiltered3DConnex");
      return(PB);
   }


  cclist = VipGet3DConnex( vol, connectivity);
  if(mode==HYSTE_NUMBER) limit=param;

  ccdeadlist = NULL;
  deadptr = NULL;
  if(how>=0)
    {
      walker = cclist;
      while(walker!=NULL)
	{
           if(walker->n_points<=0)
           {
              VipPrintfError("empty bucket?");
              VipFree3DBucketList_S16BIT( cclist);
              VipPrintfExit("VipGetDualHysteresisFiltered3DConnex");
              return(PB);
           }

          if(mode==HYSTE_PERCENT) limit=(int)((float)param/100.
                                        *walker->n_points + 0.5);
	  if ( (walker->n_points < how)||
               (TestHysteBucket(walker,vol,limit,threshold)==VFALSE))
	    {
	      killer = walker;
	      walker = walker->next;
	      cclist = VipUnhook3DBucketFromList_S16BIT ( killer, cclist );
	      if(ccdeadlist==NULL)
		{
		  ccdeadlist = killer;
		  deadptr = killer;
		}
	      else
		{
		  deadptr->next = killer;
		  deadptr=killer;
		}
	    }
	  else walker = walker->next;
	}
      VipFree3DBucketList_S16BIT( cclist);
    }
  else if (how<0)
    {
      walker = cclist;
      while(walker!=NULL)
	{
          if(walker->n_points<=0)
           {
              VipPrintfError("empty bucket?");
              VipFree3DBucketList_S16BIT( cclist);
              VipPrintfExit("VipGetDualHysteresisFiltered3DConnex");
              return(PB);
           }

          if(mode==HYSTE_PERCENT) limit=(int)((float)param/100.
                                        *walker->n_points + 0.5);
	  if ((TestHysteBucket(walker,vol,limit,threshold)==VFALSE))
	    {
	      killer = walker;
	      walker = walker->next;
	      cclist = VipUnhook3DBucketFromList_S16BIT ( killer, cclist );
              if(ccdeadlist==NULL)
		{
		  ccdeadlist = killer;
		  deadptr = killer;
		}
	      else
		{
		  deadptr->next = killer;
		  deadptr=killer;
		}
	    }
	  else walker = walker->next;
	}

      walker = VipGetNthBiggest3DBucket_S16BIT (cclist, -how);
      if(walker==PB) return(PB);

      if(ccdeadlist==NULL)
	{
	  ccdeadlist = VipUnhook3DBucketFromList_S16BIT ( walker, cclist );
	}
      else
	{
	  deadptr->next = VipUnhook3DBucketFromList_S16BIT ( walker, cclist );
        }

      VipFree3DBucket_S16BIT( walker);
    }

  return(ccdeadlist);

}



/*----------------------------------------------------------------------------*/
int VipConnexVolumeFilter(
  Volume *vol,
  int connectivity,
  int how,
  int mode)
  
/*----------------------------------------------------------------------------*/

{
  Vip3DBucket_S16BIT *cclist;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(3Dconnex.c)VipConnexVolumeFilter");
      return(PB);
    }
  if((mode !=CONNEX_BINARY) && (mode !=CONNEX_LABEL)
     && (mode!=CONNEX_ORDER) && (mode!=CONNEX_GREYLEVEL))
    {
      VipPrintfError("Sorry unknown mode...");
      VipPrintfExit("VipConnexVolumeFilter");
      return(PB);
    }

  if(mode!=CONNEX_GREYLEVEL)
    {
      cclist = VipGetFiltered3DConnex( vol, connectivity, how);
      if(cclist==PB)
	{
	  VipPrintfWarning("Nothing is left in VipConnexVolumeFilter");
	  VipSetImageLevel(vol,0);
	  return(OK);
	}
    }
  else
    {
      cclist = VipGetDualFiltered3DConnex( vol, connectivity, how);
    }

  switch( mode)
    {
    case CONNEX_BINARY:
      VipSetImageLevel(vol,0);
      VipWriteCoordBucketListInVolume(cclist,vol,255,0);
      break;
    case CONNEX_GREYLEVEL:
      VipWriteCoordBucketListInVolume(cclist,vol,0,0);
      break;
    case CONNEX_LABEL:
      VipSetImageLevel(vol,0);
      VipWriteCoordBucketListInVolume(cclist,vol,1,1);
      break;
    case CONNEX_ORDER:
      VipPrintfError("Sorry CONNEX_ORDER not implemented yet...");
      VipPrintfExit("VipConnexVolumeFilter");
      return(PB);
    default:
      VipPrintfError("Sorry unknown mode...");
      VipPrintfExit("VipConnexVolumeFilter");
      return(PB);
    }

  VipFree3DBucketList_S16BIT(cclist);

  return(OK);
}

/*----------------------------------------------------------------------------*/
int VipHysteresisPercentThresholding(
  Volume *vol,
  int connectivity,
  int how,
  int connex_mode,
  int low_percent,
  int high_percent,
  int mode,
  int param)
/*VipHysteresisThresholding with percent of max*/
/*----------------------------------------------------------------------------*/
{
   float lemax;
   float tl, th;

   if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
   {
     VipPrintfExit("(3Dconnex.c)VipHysteresisPercentThresholding");
     return(PB);
   }
   if((low_percent<0)||(high_percent>100)||(low_percent>high_percent))
   {
      VipPrintfError("Bad percent values");
      VipPrintfExit("VipHysteresisPercentThresholding");
      return(PB);
   }

   lemax = VipGetVolumeMax(vol);
   if((lemax<0))
   {
      VipPrintfError("Negative maximum value? (not implemented...)");
      VipPrintfExit("VipHysteresisPercentThresholding");
      return(PB);
   }


   tl = (float)low_percent/(float)(100.)*lemax;
   th = (float)high_percent/(float)(100.)*lemax;

   (void)printf("Hysteresis values: %f %f\n",tl,th);
   return(VipHysteresisThresholding(vol,connectivity,how,connex_mode,
                                    tl,th,mode,param));
}

/*----------------------------------------------------------------------------*/
int VipHysteresisThresholding(
  Volume *vol,
  int connectivity,
  int how,
  int connex_mode,
  float low_threshold,
  float high_threshold,
  int mode,
  int param)
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/*  If mode==HYSTE_NUMBER, all cc with at least
"param" points over high_threshold will be kept, if mode==HYSTE_PERCENT,
all cc with percent points over high_threshold will be kept */  
/*----------------------------------------------------------------------------*/

{
  Vip3DBucket_S16BIT *cclist;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(3Dconnex.c)VipHysteresisThresholding");
      return(PB);
    }
  if((connex_mode !=CONNEX_BINARY) && (connex_mode !=CONNEX_LABEL)
     && (connex_mode!=CONNEX_ORDER) && (connex_mode!=CONNEX_GREYLEVEL))
    {
      VipPrintfError("Sorry unknown connex mode...");
      VipPrintfExit("VipHysteresisThresholding");
      return(PB);
    }
  if((mode!=HYSTE_NUMBER)&&(mode!=HYSTE_PERCENT))
    {
      VipPrintfError("Sorry unknown mode...");
      VipPrintfExit("VipHysteresisThresholding");
      return(PB);
    }
  if((mode==HYSTE_NUMBER)&&(param<=0))
    {
      VipPrintfError("Negative or zero parameter in HYSTE_NUMBER mode?");
      VipPrintfExit("VipHysteresisThresholding");
      return(PB);
    }
  if((mode==HYSTE_PERCENT)&&((param<=0)||(param>100)))
    {
      VipPrintfError("Allowed range: [1..100) for HYSTE_PERCENT mode?");
      VipPrintfExit("VipHysteresisThresholding");
      return(PB);
    }

  VipSingleFloatThreshold(vol,GREATER_THAN,low_threshold,GREYLEVEL_RESULT);

  if(connex_mode!=CONNEX_GREYLEVEL)
    {
      cclist = VipGetHysteresisFiltered3DConnex( vol, connectivity, how,
						 high_threshold, mode, param);
      if(cclist==PB)
	{
	  VipPrintfWarning("Nothing is left in VipHysteresisThresholding");
	  VipSetImageLevel(vol,0);
	  return(OK);
	}
    }
  else
    {
      cclist = VipGetDualHysteresisFiltered3DConnex( vol, connectivity, how,
						 high_threshold, mode, param);
    }

  switch( connex_mode)
    {
    case CONNEX_BINARY:
      VipSetImageLevel(vol,0);
      VipWriteCoordBucketListInVolume(cclist,vol,255,0);
      break;
    case CONNEX_GREYLEVEL:
      VipWriteCoordBucketListInVolume(cclist,vol,0,0);
      break;
    case CONNEX_LABEL:
      VipSetImageLevel(vol,0);
      VipWriteCoordBucketListInVolume(cclist,vol,1,1);
      break;
    case CONNEX_ORDER:
      VipPrintfError("Sorry CONNEX_ORDER not implemented yet...");
      VipPrintfExit("VipHysteresisThresholding");
      return(PB);
    default:
      VipPrintfError("Sorry unknown connex mode...");
      VipPrintfExit("VipHysteresisThresholding");
      return(PB);
    }

  VipFree3DBucketList_S16BIT( cclist);

  return(OK);
}
