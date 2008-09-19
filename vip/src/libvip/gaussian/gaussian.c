/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : gaussian.c           * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 07/12/98
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : jfmangin@cea.fr
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            10/04/00| V Frouin     | Distinguish 2 funcs to 3DGaussian and
 *                    |              |                        2DAxiGaussian 
 *****************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/alloc.h>
#include <vip/volume.h>
#include <vip/gaussian.h>
#include <vip/gaussian_static.h>

/*----------------------------------------------------------------------------*/
static int  VipPutZeroToEpsilon(Volume *vol, float epsilon);
/*----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
Volume  *VipDeriche3DGaussian(
  Volume *vol,
  float sigma,
  int how)
/*---------------------------------------------------------------------------*/
{
  return (VipDericheCustomGaussian( vol, sigma, sigma, sigma, how ) );
}

/*---------------------------------------------------------------------------*/
Volume  *VipDeriche2DAxiGaussian(
  Volume *vol,
  float sigmaTrans, float sigmaAxial,
  int how)
/*---------------------------------------------------------------------------*/
{
  return (
	  VipDericheCustomGaussian(vol,sigmaTrans,sigmaTrans,sigmaAxial,how)
	  );
}

/*----------------------------------------------------------------------------*/
Volume  *VipDericheCustomGaussian(
  Volume *vol,
  float sigmax, float sigmay, float sigmaz,
  int how)
/*----------------------------------------------------------------------------*/
{
  char name[VIP_NAME_MAXLEN];
  Volume *new, *volnew;
  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(gaussian.c)VipDeriche3DGaussian");
      return(PB);
    }
  if(  (sigmax<0.1)||(sigmax>100) 
     ||(sigmay<0.1)||(sigmay>100)
     ||(sigmaz<0.1)||(sigmaz>100))
    {
      VipPrintfWarning("Gaussian sigma should be in the range [0.1,100]");
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3Dgaussian");
  new = VipDuplicateVolumeStructure (vol,name);
  VipSetType(new,VFLOAT);
  VipSetBorderWidth(new,0);
  if(VipAllocateVolumeData(new)==PB)
    {
      VipPrintfExit("VipDeriche3DGaussian");
      return(PB);
    }
  VipTransferDataToFloatVolume(vol,new);
  /*I try to solve the problem with lines of zero: jeff*/
  /*
  VipPrintfWarning("Putting zero to epsilon to solve the hard pb with sparc stations");
  */
  VipPutZeroToEpsilon(new,0.00000000001);

  Deriche3DSmoothing(new, sigmax, sigmay, sigmaz);

  if(how==SAME_VOLUME)
    {
      VipTransferDataFromFloatVolume(vol,new);
      VipFreeVolume(new);
      return(vol);
    }
  else if(how==NEW_FLOAT_VOLUME || mVipVolType(vol)==VFLOAT)
      {
	  return(new);
      }
  else
    {
      (void)strcpy(name,mVipVolName(vol));
      (void)strcat(name,"_deriche_3Dgaussian");
      volnew = VipDuplicateVolumeStructure (vol,name);
      if(VipAllocateVolumeData(volnew)==PB)
	{
	  VipPrintfExit("VipDeriche3DGaussian");
	  return(PB);
	}
      VipTransferDataFromFloatVolume(volnew,new);
      VipFreeVolume(new);
      return(new);
    }

}

/*----------------------------------------------------------------------------*/
static int  VipPutZeroToEpsilon(Volume *vol, float epsilon)
/*----------------------------------------------------------------------------*/
{
    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_FLOAT *ptr;


    if(VipTestType(vol,VFLOAT)==PB)
	{
	    VipPrintfError("volume type HAS to be float in VipPutZeroToEpsilon!");
	    VipPrintfExit("VipPutZeroToEpsilon");
	    return(PB);
	}

    /*printf("epsilon = %f\n", epsilon);*/

    vos = VipGetOffsetStructure(vol);
    ptr = VipGetDataPtr_VFLOAT( vol ) + vos->oFirstPoint;

    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	{
	    for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
		{
		    for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
			{
			    if(*ptr==0.) *ptr=epsilon;
			    ptr++;
			}
		    ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	    ptr += vos->oLineBetweenSlice; /*skip border lines*/
	}
    return(OK);
}

