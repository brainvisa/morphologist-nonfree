/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_deriche3D.c      * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 10/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
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
#include <vip/alloc.h>
#include <vip/volume.h>
#include <vip/bucket.h>
#include <vip/deriche.h>
#include <vip/deriche_static.h>


/*---------------------------------------------------------------------------*/
Volume  *VipDeriche3DGradientX(
  Volume *vol,
  float alpha,
  int how)
/*---------------------------------------------------------------------------*/
{
  char name[VIP_NAME_MAXLEN];
  Volume *newv, *volnew;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(deriche3D.c)VipDeriche3DGradientX");
      return(PB);
    }
  if((alpha<0.3)||(alpha>3))
    {
      VipPrintfWarning("Alpha Deriche parameter should be in the range [0.3,3]");
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3DgradientX");
  newv = VipDuplicateVolumeStructure (vol,name);
  VipSetType(newv,VFLOAT);
  VipSetBorderWidth(newv,0);
  if(VipAllocateVolumeData(newv)==PB)
    {
      VipPrintfExit("VipDeriche3DGradientX");
      return(PB);
    }
  VipTransferDataToFloatVolume(vol,newv);
  VolumeGradientDeriche3DX(newv,alpha);

  if(how==SAME_VOLUME)
    {
      VipTransferDataFromFloatVolume(vol,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(vol);
    }
  else
    {
      (void)strcpy(name,mVipVolName(vol));
      (void)strcat(name,"_deriche_3DgradientX");
      volnew = VipDuplicateVolumeStructure (vol,name);
      if(VipAllocateVolumeData(volnew)==PB)
	{
	  VipPrintfExit("VipDeriche3DGradientX");
	  return(PB);
	}
      VipTransferDataFromFloatVolume(volnew,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(volnew);
    }

}

/*---------------------------------------------------------------------------*/
Volume  *VipDeriche3DGradientY(
  Volume *vol,
  float alpha,
  int how)
/*---------------------------------------------------------------------------*/
{
  char name[VIP_NAME_MAXLEN];
  Volume *newv, *volnew;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(deriche3D.c)VipDeriche3DGradientY");
      return(PB);
    }
  if((alpha<0.3)||(alpha>3.))
    {
      VipPrintfWarning("Alpha Deriche parameter should be in the range [0.3,3]");
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3DgradientY");
  newv = VipDuplicateVolumeStructure (vol,name);
  VipSetType(newv,VFLOAT);
  VipSetBorderWidth(newv,0);
  if(VipAllocateVolumeData(newv)==PB)
    {
      VipPrintfExit("VipDeriche3DGradientY");
      return(PB);
    }
  VipTransferDataToFloatVolume(vol,newv);
  VolumeGradientDeriche3DY(newv,alpha);

  if(how==SAME_VOLUME)
    {
      VipTransferDataFromFloatVolume(vol,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(vol);
    }
  else
    {
      (void)strcpy(name,mVipVolName(vol));
      (void)strcat(name,"_deriche_3DgradientY");
      volnew = VipDuplicateVolumeStructure (vol,name);
      if(VipAllocateVolumeData(volnew)==PB)
	{
	  VipPrintfExit("VipDeriche3DGradientY");
	  return(PB);
	}
      VipTransferDataFromFloatVolume(volnew,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(volnew);
    }

}

/*---------------------------------------------------------------------------*/
Volume  *VipDeriche3DGradientZ(
  Volume *vol,
  float alpha,
  int how)
/*---------------------------------------------------------------------------*/
{
  char name[VIP_NAME_MAXLEN];
  Volume *newv, *volnew;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(deriche3D.c)VipDeriche3DGradientZ");
      return(PB);
    }
  if((alpha<0.3)||(alpha>3))
    {
      VipPrintfWarning("Alpha Deriche parameter should be in the range [0.3,3]");
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3DgradientZ");
  newv = VipDuplicateVolumeStructure (vol,name);
  VipSetType(newv,VFLOAT);
  VipSetBorderWidth(newv,0);
  if(VipAllocateVolumeData(newv)==PB)
    {
      VipPrintfExit("VipDeriche3DGradientZ");
      return(PB);
    }
  VipTransferDataToFloatVolume(vol,newv);
  VolumeGradientDeriche3DZ(newv,alpha);

  if(how==SAME_VOLUME)
    {
      VipTransferDataFromFloatVolume(vol,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(vol);
    }
  else
    {
      (void)strcpy(name,mVipVolName(vol));
      (void)strcat(name,"_deriche_3DgradientZ");
      volnew = VipDuplicateVolumeStructure (vol,name);
      if(VipAllocateVolumeData(volnew)==PB)
	{
	  VipPrintfExit("VipDeriche3DGradientZ");
	  return(PB);
	}
      VipTransferDataFromFloatVolume(volnew,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(volnew);
    }

}

/*---------------------------------------------------------------------------*/
Volume  *VipDeriche3DSmoothing(
  Volume *vol,
  float alpha,
  int how)
/*---------------------------------------------------------------------------*/
{
  char name[VIP_NAME_MAXLEN];
  Volume *newv, *volnew;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(deriche3D.c)VipDeriche3DSmoothing");
      return(PB);
    }
  if((alpha<0.3)||(alpha>3))
    {
      VipPrintfWarning("Alpha Deriche parameter should be in the range [0.3,3]");
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3Dsmoothing");
  newv = VipDuplicateVolumeStructure (vol,name);
  VipSetType(newv,VFLOAT);
  VipSetBorderWidth(newv,0);
  if(VipAllocateVolumeData(newv)==PB)
    {
      VipPrintfExit("VipDeriche3DGradientSmoothing");
      return(PB);
    }
  VipTransferDataToFloatVolume(vol,newv);
  VolumeDeriche3DSmoothing(newv,alpha);

  if(how==SAME_VOLUME)
    {
      VipTransferDataFromFloatVolume(vol,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(vol);
    }
  else
    {
      (void)strcpy(name,mVipVolName(vol));
      (void)strcat(name,"_deriche_3Dsmoothing");
      volnew = VipDuplicateVolumeStructure (vol,name);
      if(VipAllocateVolumeData(volnew)==PB)
	{
	  VipPrintfExit("VipDeriche3DSmoothing");
	  return(PB);
	}
      VipTransferDataFromFloatVolume(volnew,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      return(volnew);
    }

}
/*---------------------------------------------------------------------------*/
int  Compute3DNorm(
Volume *fvol,
Volume *gx,
Volume *gy,
Volume *gz,
float gxabsmax,
float gyabsmax,
float gzabsmax,
float threshold)
/*---------------------------------------------------------------------------*/
{
  int xsize, ysize, zsize;
  int i;
  float *fptr;
  Vip_S16BIT *ptrx, *ptry, *ptrz;
  float ratio;
  float temp, temp2;

  if(mVipVolVoxSizeZ(fvol)<0.000001)
    {
      VipPrintfError("NULL Zvoxsize!");
      VipPrintfExit("Compute3DNorm");
      return(PB);
    }

//   if(mVipVolVoxSizeX(fvol)!=mVipVolVoxSizeY(fvol))
//     {
//       VipPrintfWarning("Different Xvoxsize ans Yvoxsize in Compute3DNorm!");
//     }

  if(mVipVolBorderWidth(gx)!=0 ||mVipVolBorderWidth(gy)!=0 ||
     mVipVolBorderWidth(gz)!=0 ||mVipVolBorderWidth(fvol)!=0) 
    {
      VipPrintfError("Compute3DNorm requires 0 borderwidth volumes");
      VipPrintfExit("Compute3DNorm");
      return(PB);
    }

  gxabsmax /= D_S16BITMAX;
  gyabsmax /= D_S16BITMAX;
  gzabsmax /= D_S16BITMAX;

  ratio = mVipVolVoxSizeX(fvol) / mVipVolVoxSizeZ(fvol);
  VipGet3DSize(fvol,&xsize,&ysize,&zsize);

  ptrx = VipGetDataPtr_S16BIT( gx );
  ptry = VipGetDataPtr_S16BIT( gy );
  ptrz = VipGetDataPtr_S16BIT( gz );
  fptr = VipGetDataPtr_VFLOAT( fvol );

  gzabsmax *= ratio;
  for(i=xsize*ysize*zsize;i--;)
    {
      temp = (float)*ptrx++ * gxabsmax;
      temp2 = temp*temp;
      temp = (float)*ptry++ * gyabsmax;
      temp2 += temp*temp;
      temp = (float)*ptrz++ * gzabsmax;
      temp2 += temp*temp;
      temp2 = (float)sqrt((double)temp2);
      if(temp2<=threshold) *fptr++ = 0.;
      else *fptr++ = temp2;	    
    }

  return(OK);
}

/*---------------------------------------------------------------------------*/
int  Extract3DMaxima(
Volume *fvol,
Volume *gx,
Volume *gy,
Volume *gz,
float gxabsmax,
float gyabsmax,
float gzabsmax,
float threshold)
/*---------------------------------------------------------------------------*/
{
  int xsize, ysize, zsize;
  float temp, *fptr;
  Vip_S16BIT *ptrx, *ptry, *ptrz;
  float ratio;
  Volume *ext;
  Vip_U8BIT *extptr;
  int offset10, offset11, offset01, offset1m1;
  float tanpisur8, tan3pisur8, ysurx, absysurx;
  float rapztox;
  int ix, iy, iz;
  int size_zoffsettab, settozero;
  int offset, xplanz, yplanz;
  char name[512];
  int slicesize;
  VipOffsetStruct *vos;

  if(mVipVolVoxSizeZ(fvol)<0.000001)
    {
      VipPrintfError("NULL Zvoxsize!");
      VipPrintfExit("Compute3DNorm");
      return(PB);
    }

//   if(mVipVolVoxSizeX(fvol)!=mVipVolVoxSizeY(fvol))
//     {
//       VipPrintfWarning("Different Xvoxsize ans Yvoxsize in Extract3DExtrema!");
//     }

  if(mVipVolBorderWidth(gx)!=0 ||mVipVolBorderWidth(gy)!=0 ||
     mVipVolBorderWidth(fvol)!=0) 
    {
      VipPrintfError("Extract3DExtrema requires 0 borderwidth volumes for gx, gy, fvol");
      VipPrintfExit("Extract3DExtrema");
      return(PB);
    }

  vos = VipGetOffsetStructure(gz);

  if ((mVipVolSizeX(fvol)<5)||(mVipVolSizeY(fvol)<5)||(mVipVolSizeZ(fvol)<5))
    {
      VipPrintfError("Extract3DExtrema requires larger sizes...");
      VipPrintfExit("Extract3DExtrema");
      return(PB);
    }

  (void)strcpy(name,mVipVolName(gx));
  (void)strcat(name,"_ext");
  ext = VipDuplicateVolumeStructure (gx,name);
  VipSetType(ext,U8BIT);
  VipSetBorderWidth(ext,0);
  if(VipAllocateVolumeData(ext)==PB)
    {
      VipPrintfExit("Extract3DExtrema");
      return(PB);
    }

  VipGet3DSize(fvol,&xsize,&ysize,&zsize);
  slicesize = xsize * ysize;
  
  /* offset pour atteindre les 8 voisins dans le plan */
  offset10 = 1;
  offset11 = 1+xsize;
  offset01 = xsize;
  offset1m1 = -xsize+1;
 

  ptrx = VipGetDataPtr_S16BIT( gx );
  ptry = VipGetDataPtr_S16BIT( gy );
  ptrz = VipGetDataPtr_S16BIT( gz ) + vos->oFirstPoint;
  extptr = VipGetDataPtr_U8BIT( ext );
  fptr = VipGetDataPtr_VFLOAT( fvol );

  tanpisur8 = (float)tan(M_PI*(double)(1./8.));
  tan3pisur8 = (float)tan(M_PI*(double)(3./8.));

  gxabsmax /= D_S16BITMAX;
  gyabsmax /= D_S16BITMAX;
  gzabsmax /= D_S16BITMAX;

  ratio = mVipVolVoxSizeX(fvol) / mVipVolVoxSizeZ(fvol);
  rapztox = 1.0f / ratio;
  gzabsmax *= ratio;
  if (threshold<0.00001f) threshold = 0.1f;

  size_zoffsettab = (int)(rapztox + 0.5);
  
  /*NB: par soucis de simplicite, les images sont supposees carrees*/
  if(size_zoffsettab == 0) size_zoffsettab = 1;
  if (size_zoffsettab < 2) settozero = 2;
  else settozero = size_zoffsettab;

  for(ix=2*slicesize;ix--;) *extptr++ = VFALSE;
  ptrx += 2*slicesize;
  ptry += 2*slicesize;
  ptrz += 2*vos->oSlice;
  fptr += 2*slicesize;
  for(iz=zsize-4;iz--;)
  {
    for(ix=settozero*xsize;ix--;) *extptr++ = VFALSE;
    ptrx += settozero*xsize;
    ptry += settozero*xsize;
    ptrz += settozero*vos->oLine;
    fptr += settozero*xsize;

    for(iy=ysize-2*settozero;iy--;)
    {
      for(ix=settozero;ix--;) *extptr++ = VFALSE;
      ptrx += settozero;
      ptry += settozero;
      ptrz += settozero;
      fptr += settozero;
      for(ix=xsize-2*settozero;ix--;)
      {
        if(*fptr<threshold)
        {
          fptr++;
          *extptr++ = VFALSE;
          ptrx++;
          ptry++;
          ptrz++;
        }
        else
        {
          temp = (float)*ptrz * gzabsmax;
          if(fabs((double)temp)>0.0001)
          {
            /* intersection avec les plans contigus */
            temp = rapztox/temp;
            xplanz = (int)(temp * *ptrx * gxabsmax + 0.5);
            yplanz = (int)(temp * *ptry * gyabsmax + 0.5);
            if((abs(xplanz)<=size_zoffsettab)
              && (abs(yplanz)<=size_zoffsettab) )
            /* le point reste ds le cube de cote zvoxsize */
            {
              temp = *fptr;
              offset = xplanz + yplanz*xsize + slicesize;
              if ((fptr[offset]>temp) || (fptr[-offset]>temp) )
              {
                *extptr++ = VFALSE;
              }
              else *extptr++ = VTRUE;
              fptr++;
              ptrx++;
              ptry++;
              ptrz++;
              continue;
            }
          }
          
          temp = *ptrx * gxabsmax;
          if(fabs((double)temp)<0.00001)
          {
            offset = offset01;
          }
          else
          {
            ysurx = *ptry * gyabsmax / temp;
            absysurx = (float)fabs((double)ysurx);
            if (absysurx<tanpisur8)
            {
              offset = offset10;
            }
            else if(absysurx>tan3pisur8)
            {
              offset = offset01;
            }
            else if(ysurx>0)
            {
              offset = offset11;
            }
            else
            {
              offset = offset1m1;
            }
          }
          temp = *fptr;
          if ((fptr[offset]>temp) || (fptr[-offset]>temp) )
          {
            *extptr++ = VFALSE;
          }
          else *extptr++ = VTRUE;
          fptr++;
          ptrx++;
          ptry++;
          ptrz++;
        }
      }
      for(ix=settozero;ix--;) *extptr++ = VFALSE;
      ptrx += settozero;
      ptry += settozero;
      ptrz += settozero + vos->oPointBetweenLine;
      fptr += settozero;
    }
    for(ix=settozero*xsize;ix--;) *extptr++ = VFALSE;
    ptrx += settozero*xsize;
    ptry += settozero*xsize;
    ptrz += settozero*xsize + vos->oLineBetweenSlice;
    fptr += settozero*xsize;
  }
  for(ix=2*slicesize;ix--;) *extptr++ = VFALSE;

  VipMaskVolume_U8BIT(fvol,ext);  
  VipFreeVolume(ext);
  VipFree(ext);
  VipFree(vos);

  return(OK);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipDeriche3DGradientNorm(
  Volume *vol,
  float alpha,
  int mode,
  float threshold)
/*---------------------------------------------------------------------------*/
{
  
 if(mode==DERICHE_EXTREMA_DIRECTION)
    {
      VipPrintfError("Use VipDeriche3DGradientNormAndReturnG3DBucket for DIRECTION mode");
      VipPrintfExit("VipDeriche3DGradientNorm");
      return(PB);
    }
   return( VipDeriche3DGradientNormAndReturnG3DBucket(vol,
                                        alpha,mode,threshold,NULL));
}


/*---------------------------------------------------------------------------*/
int VipDeriche3DGradientNormAndReturnG3DBucket(
  Volume *vol,
  float alpha,
  int mode,
  float threshold,
  VipG3DBucket **gbuck)
/*---------------------------------------------------------------------------*/
{
  char name[VIP_NAME_MAXLEN]; 
  VIP_DEC_VOLUME(newv);
  VIP_DEC_VOLUME(gx); 
  VIP_DEC_VOLUME(gy); 
  VIP_DEC_VOLUME(gz);
  float gxabsmax, gyabsmax, gzabsmax;
  VipG3DBucket *thebuck;
  VipG3DPoint *gptr;
  int x,y,z;
  int xsize,ysize,zsize;
  int nextrema;
  float *fptr;
  Vip_S16BIT *gxptr, *gyptr, *gzptr;
  VipOffsetStruct *vos=NULL;

  if (VipTestExist(vol)==PB || VipTestType(vol,ANY_TYPE)==PB)
    {
      VipPrintfExit("(deriche3D.c)VipDeriche3DGradientNorm");
      return(PB);
    }
  if((alpha<0.3)||(alpha>3))
    {
      VipPrintfWarning("Alpha Deriche parameter should be in the range [0.3,3]");
    }
  if (mode!=DERICHE_NORM && mode!=DERICHE_EXTREMA && 
      mode!= DERICHE_EXTREMA_DIRECTION)
    {
      VipPrintfError("Unknown mode in VipDeriche3DGradientNorm");
      VipPrintfExit("VipDeriche3DGradientNorm");
      return(PB);
    }

  if ((mode==DERICHE_EXTREMA_DIRECTION)&&(gbuck==NULL))
    {
      VipPrintfError("NULL gbuck in DIRECTION mode!");
      VipPrintfExit("VipDeriche3DGradientNoemAndReturnG3DBucket");
      return(PB);
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3DgradientX");
  gx = VipDuplicateVolumeStructure (vol,name);
  VipSetType(gx,S16BIT);
  VipSetBorderWidth(gx,0);
  if(VipAllocateVolumeData(gx)==PB)
    {
      VipPrintfExit("VipDeriche3DGradientNorm");
      return(PB);
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3DgradientY");
  gy = VipDuplicateVolumeStructure (vol,name);
  VipSetType(gy,S16BIT);
  VipSetBorderWidth(gy,0);
  if(VipAllocateVolumeData(gy)==PB)
    {
      VipPrintfExit("VipDeriche3DGradientNorm");
      return(PB);
    }

  if ((mVipVolType(vol) == S16BIT) && (mVipVolBorderWidth(vol) == 0))
    gz = vol; /*WARNING tricky*/
  else
    {
      (void)strcpy(name,mVipVolName(vol));
      (void)strcat(name,"_deriche_3DgradientZ");
      gz = VipDuplicateVolumeStructure (vol,name);
      VipSetType(gz,S16BIT);
      VipSetBorderWidth(gz,0);
      if(VipAllocateVolumeData(gz)==PB)
        {
          VipPrintfExit("VipDeriche3DGradientNorm");
          return(PB);
        }
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_deriche_3DgradientNorm");
  newv = VipDuplicateVolumeStructure (vol,name);
  VipSetType(newv,VFLOAT);
  VipSetBorderWidth(newv,0);
  if(VipAllocateVolumeData(newv)==PB)
    {
      VipPrintfExit("VipDeriche3DGradientNorm");
      return(PB);
    }


  /*gx*/
  printf("Computing 3D gradient x coord...\n");

  VipTransferDataToFloatVolume(vol,newv);

  VolumeGradientDeriche3DX(newv,alpha);

  PreserveFloatVolume_S16BIT(newv,gx,&gxabsmax);

  /*gy*/
  printf("Computing 3D gradient y coord...\n");
  VipTransferDataToFloatVolume(vol,newv);
  VolumeGradientDeriche3DY(newv,alpha);
  PreserveFloatVolume_S16BIT(newv,gy,&gyabsmax);

  /*gz*/
  printf("Computing 3D gradient z coord...\n");
  VipTransferDataToFloatVolume(vol,newv);
  VolumeGradientDeriche3DZ(newv,alpha);
  PreserveFloatVolume_S16BIT(newv,gz,&gzabsmax);

  Compute3DNorm(newv,gx,gy,gz,gxabsmax,gyabsmax,gzabsmax,threshold);

  switch(mode)
    {
    case DERICHE_NORM:
      VipFreeVolume(gx);
      VipFree(gx);
      VipFreeVolume(gy);
      VipFree(gy);
      if(gz!=vol)
      {
        VipFreeVolume(gz);
        VipFree(gz);
      }
      VipTransferDataFromFloatVolume(vol,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      break;
    case DERICHE_EXTREMA:
      printf("Entering Extract3DMaxima\n");
      Extract3DMaxima(newv,gx,gy,gz,gxabsmax,gyabsmax,gzabsmax,threshold);
      VipFreeVolume(gx);
      VipFree(gx);
      VipFreeVolume(gy);
      VipFree(gy);
      if(gz!=vol)
      {
        VipFreeVolume(gz);
        VipFree(gz);
      }
      VipTransferDataFromFloatVolume(vol,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      break;
    case DERICHE_EXTREMA_DIRECTION:
      Extract3DMaxima(newv,gx,gy,gz,gxabsmax,gyabsmax,gzabsmax,threshold);
      nextrema = VipGetNumberNonZeroPoints(newv);
      thebuck = VipAllocG3DBucket(nextrema);
      if(!thebuck)
      {
        VipFreeVolume(gx);
        VipFree(gx);
        VipFreeVolume(gy); 
        VipFree(gy);
        if(gz!=vol) 
          {
            VipFreeVolume(gz);
            VipFree(gz);
          }
        VipFreeVolume(newv);
        VipFree(newv);
        return(PB);
      }
      *gbuck = thebuck;
      vos = VipGetOffsetStructure(gz);
      /*in case gz is the initial volume and has non zero borderwidth*/

      gxabsmax /= D_S16BITMAX;
      gyabsmax /= D_S16BITMAX;
      gzabsmax /= D_S16BITMAX;
      gzabsmax *= mVipVolVoxSizeX(newv) / mVipVolVoxSizeZ(newv);


      fptr = VipGetDataPtr_VFLOAT(newv);
      gxptr = VipGetDataPtr_S16BIT(gx);
      gyptr = VipGetDataPtr_S16BIT(gy);
      gzptr = VipGetDataPtr_S16BIT(gz) + vos->oFirstPoint;

      gptr = thebuck->data;
      xsize = mVipVolSizeX(newv);
      ysize = mVipVolSizeY(newv);
      zsize = mVipVolSizeZ(newv);

      for(z=0;z<zsize;z++)
      {
        for(y=0;y<ysize;y++)
          {
            for(x=0;x<xsize;x++)
            {
              if(*fptr)
                {
                  gptr->p.x = x;
                  gptr->p.y = y;
                  gptr->p.z = z;
                  gptr->g3D.x = *gxptr * gxabsmax / *fptr;
                  gptr->g3D.y = *gyptr * gyabsmax / *fptr;
                  gptr->g3D.z = *gzptr * gzabsmax / *fptr;
                  gptr++;
                  thebuck->n_points++;
                }
              fptr++;
              gxptr++;
              gyptr++;
              gzptr++;
            }
            gzptr += vos->oPointBetweenLine;
          }
        gzptr += vos->oLineBetweenSlice;
      }
      VipTransferDataFromFloatVolume(vol,newv);
      VipFreeVolume(newv);
      VipFree(newv);
      VipFreeVolume(gx);
      VipFree(gx);
      VipFreeVolume(gy);
      VipFree(gy);
      if (gz!=vol)
      {
        VipFreeVolume(gz);
        VipFree(gz);
      }
      VipFree(vos);
      break;
    default:
      VipPrintfError("Unknown mode in VipDeriche3DGradientNorm\n");
      return(PB);

    }

  return(OK);
}


