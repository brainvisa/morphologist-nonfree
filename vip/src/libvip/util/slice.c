/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_slice.c          * TYPE     : Function
 * AUTHOR      : POUPON F.            * CREATION : 12/02/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Extraction d'une coupe dans un volume
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_slice.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/util.h>

Volume *VipGetAxialSlice(Volume *vol, int slice)
{ 
  VIP_DEC_VOLUME(volout);
  float xvoxsize, yvoxsize;
  int type, dx, dy, i, j;
  Vip_S16BIT *outptr, *inptr;

  dx = mVipVolSizeX(vol);
  dy = mVipVolSizeY(vol);
  xvoxsize = mVipVolVoxSizeX(vol);
  yvoxsize = mVipVolVoxSizeY(vol);
  type = mVipVolType(vol);
  
  volout = VipCreate3DVolume(dx, dy, 1, xvoxsize, yvoxsize, 1.0, type, "", 0);
  outptr = VipGetDataPtr_S16BIT(volout);
  inptr = VipGetDataPtr_S16BIT(vol)+slice*dx*dy;

  for (j=dy; j--;)
    for (i=dx; i--;)
      *outptr++ = *inptr++;

  return(volout);
}

Volume *VipGetCoronalSlice(Volume *vol, int slice)
{ VIP_DEC_VOLUME(volout);
  float xvoxsize, yvoxsize;
  int type, dx, dy, i, j, zdec;
  Vip_S16BIT *inptr, *outptr;

  dx = mVipVolSizeX(vol);
  dy = mVipVolSizeZ(vol);
  xvoxsize = mVipVolVoxSizeX(vol);
  yvoxsize = mVipVolVoxSizeZ(vol);
  type = mVipVolType(vol);
  zdec = dx*(mVipVolSizeY(vol)-1);

  volout = VipCreate3DVolume(dx, dy, 1, xvoxsize, yvoxsize, 1.0, type, "", 0);
  outptr = VipGetDataPtr_S16BIT(volout);
  inptr = VipGetDataPtr_S16BIT(vol)+slice*dx;

  for (j=dy; j--; inptr+=zdec)
    for (i=dx; i--; inptr++)
      *outptr++ = *inptr;

  return(volout);
}

Volume *VipGetSagittalSlice(Volume *vol, int slice)
{ VIP_DEC_VOLUME(volout);
  float xvoxsize, yvoxsize;
  int type, dx, dy, i, j;
  Vip_S16BIT  *inptr, *outptr;

  dx = mVipVolSizeY(vol);
  dy = mVipVolSizeZ(vol);
  xvoxsize = mVipVolVoxSizeY(vol);
  yvoxsize = mVipVolVoxSizeZ(vol);
  type = mVipVolType(vol);

  volout = VipCreate3DVolume(dx, dy, 1, xvoxsize, yvoxsize, 1.0, type, "", 0);
  outptr = VipGetDataPtr_S16BIT(volout);
  inptr = VipGetDataPtr_S16BIT(vol)+slice;

  for (j=dy; j--;)
    for (i=dx; i--; inptr+=dx)
      *outptr++ = *inptr;

  return(volout);
}

Volume *VipGetSlice(Volume *vol, int slice, int slice_type)
{ VIP_DEC_VOLUME(volout);
  
  if (mVipVolBorderWidth(vol) != 0)
  { VipPrintfError("Volume border width must be null");
    VipPrintfExit("(Vip_slice.c)VipGetSlice");
    return((Volume *)NULL);
  }

  switch(slice_type)
  { case VIP_AXIAL_SLICE : if ((slice < 0) || (slice >= mVipVolSizeZ(vol)))
                           { VipPrintfError("Wrong axial slice value");
                             VipPrintfExit("(Vip_slice.c)VipGetSlice");
                             return((Volume *)NULL);
			   }
                           volout = VipGetAxialSlice(vol, slice);
                           if (!volout)
			   { VipPrintfError("Unable to get axial slice");
                             VipPrintfExit("(Vip_slice.c)VipGetSlice");
                             return((Volume *)NULL);
			   }
                           break;
    case VIP_CORONAL_SLICE : if ((slice < 0) || (slice >= mVipVolSizeY(vol)))
                             { VipPrintfError("Wrong coronal slice value");
                               VipPrintfExit("(Vip_slice.c)VipGetSlice");
                               return((Volume *)NULL);
			     }
                             volout = VipGetCoronalSlice(vol, slice);
                             if (!volout)
			     { VipPrintfError("Unable to get coronal slice");
                               VipPrintfExit("(Vip_slice.c)VipGetSlice");
                               return((Volume *)NULL);
			     }
                             break;
    case VIP_SAGITTAL_SLICE : if ((slice < 0) || (slice >= mVipVolSizeX(vol)))
                              { VipPrintfError("Wrong sagittal slice value");
                                VipPrintfExit("(Vip_slice.c)VipGetSlice");
                                return((Volume *)NULL);
			      }
                              volout = VipGetSagittalSlice(vol, slice);
                              if (!volout)
			      { VipPrintfError("Unable to get sagittal slice");
                                VipPrintfExit("(Vip_slice.c)VipGetSlice");
                                return((Volume *)NULL);
			      }
                              break;
  }

  return(volout);
}
