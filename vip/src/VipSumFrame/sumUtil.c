/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : sumUtil.c              * TYPE     : Function
 * AUTHOR      : POUPON F.              * CREATION : 04/03/1999
 * VERSION     : 1.1                    * REVISION :
 * LANGUAGE    : C                      * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Somme des frames. DEVRAIT ETRE MIS EN LIBRARY
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_subvolume.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/volume.h>
#include <vip/alloc.h>


Volume *VipSumFrame_VFLOAT(Volume *vol, int from , int to)
{
  VipOffsetStruct              *vos, *vis;
  int                          ix, iy, iz, it;
  Vip_FLOAT                   *iptr;
  Vip_FLOAT                    *optr;
  VIP_DEC_VOLUME(           volout);
  int bidon; /*compilation warning (mais ca doit etre un oubli, je prefere pas toucher*/
  bidon = from;
  bidon = to;
  bidon = 0;
 
  volout = VipCreate3DVolume (
			      mVipVolSizeX(vol),
			      mVipVolSizeY(vol),
			      mVipVolSizeZ(vol),
			      mVipVolVoxSizeX(vol),
			      mVipVolVoxSizeY(vol),
			      mVipVolVoxSizeZ(vol),
			      VFLOAT,
			      "framesummed",
			      0);
  if (volout == NULL)
    {
      VipPrintfError("volume(sumUtils.c).");
      return((Volume *) NULL);
    }
   VipSetShfjUnite(volout,mVipVolShfjUnite(vol));


   vis = VipGetOffsetStructure(vol);
   iptr = VipGetDataPtr_VFLOAT( vol ) + vis->oFirstPoint;
   vos = VipGetOffsetStructure(volout);
   optr = VipGetDataPtr_VFLOAT( volout ) + vos->oFirstPoint;


   for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
   {
     for ( iz = mVipVolSizeZ(vol); iz-- ; )             /* loop on slices */
      {
        for ( iy = mVipVolSizeY(vol); iy-- ; )          /* loop on lines */
         {
           for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
            {
	      *optr += *iptr;
              iptr++; optr++;
            }
           iptr += vis->oPointBetweenLine;        /*skip border points*/
           optr += vis->oPointBetweenLine;        /*skip border points*/
         }
        iptr += vis->oLineBetweenSlice; /*skip border lines*/
        iptr += vis->oLineBetweenSlice; /*skip border lines*/
      }
     iptr += vis->oSliceBetweenVolume; /*skip border slices*/
     optr = VipGetDataPtr_VFLOAT( volout ) + vos->oFirstPoint;
   }


   return(volout);
}

Volume *VipSumFrame_S16BIT(Volume *vol, int from , int to)
{
  VipOffsetStruct              *vos, *vis;
  int                          ix, iy, iz, it;
  Vip_S16BIT                   *iptr;
  Vip_FLOAT                    *optr;
  VIP_DEC_VOLUME(           volout);
  int bidon; /*compilation warning (mais ca doit etre un oubli, je prefere pas toucher*/
  bidon = from;
  bidon = to;
  bidon = 0;

 
  volout = VipCreate3DVolume (
			      mVipVolSizeX(vol),
			      mVipVolSizeY(vol),
			      mVipVolSizeZ(vol),
			      mVipVolVoxSizeX(vol),
			      mVipVolVoxSizeY(vol),
			      mVipVolVoxSizeZ(vol),
			      VFLOAT,
			      "framesummed",
			      0);
  if (volout == NULL)
    {
      VipPrintfError("volume(sumUtils.c).");
      return((Volume *) NULL);
    }
   VipSetShfjUnite(volout,mVipVolShfjUnite(vol));


   vis = VipGetOffsetStructure(vol);
   iptr = VipGetDataPtr_S16BIT( vol ) + vis->oFirstPoint;
   vos = VipGetOffsetStructure(volout);
   optr = VipGetDataPtr_VFLOAT( volout ) + vos->oFirstPoint;


   for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
   {
     for ( iz = mVipVolSizeZ(vol); iz-- ; )             /* loop on slices */
      {
        for ( iy = mVipVolSizeY(vol); iy-- ; )          /* loop on lines */
         {
           for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
            {
	      *optr += *iptr;
              iptr++; optr++;
            }
           iptr += vis->oPointBetweenLine;        /*skip border points*/
           optr += vis->oPointBetweenLine;        /*skip border points*/
         }
        iptr += vis->oLineBetweenSlice; /*skip border lines*/
        iptr += vis->oLineBetweenSlice; /*skip border lines*/
      }
     iptr += vis->oSliceBetweenVolume; /*skip border slices*/
     optr = VipGetDataPtr_VFLOAT( volout ) + vos->oFirstPoint;
   }


   return(volout);
}

Volume *VipSumFrame(Volume *vol, int from , int to)
{
  
 if (!VipVerifyAll(vol) || !VipTestType(vol,ANY_TYPE))
   {
     VipPrintfExit("(volume)VipGetSubVolume");
     return(PB);
   }
 switch( mVipVolType( vol ) )
   {
   case VFLOAT :
     return(VipSumFrame_VFLOAT(vol, from, to));
     break;
   case S16BIT :
     return(VipSumFrame_S16BIT(vol, from, to));
     break;
   default :
     return((Volume *) NULL);
   }
}
