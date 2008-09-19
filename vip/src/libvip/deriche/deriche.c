/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_deriche.c        * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 04/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : PC Linux
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne les filtres recursifs de Deriche
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
#include <vip/alloc.h>
#include <vip/volume.h>
#include <vip/deriche.h>
#include <vip/deriche_static.h>


/*------------------------------------------------------------------------------*/
int  VolumeGradientDeriche3DX(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeGradientDeriche3DX!");
     VipPrintfExit("VolumeGradientDeriche3DX");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeGradientDeriche3DX!");
     VipPrintfExit("VolumeGradientDeriche3DX");
     return(PB);
  }

  if (VolumeGradientDericheAlongLines(vol,alpha)==PB) return(PB);

  if (VolumeSmoothingDericheAlongColumns(vol,alpha)==PB) return(PB);

  if (VolumeSmoothingDericheAlongSlices(vol,alpha)==PB) return(PB);


  return(OK);
}

/*------------------------------------------------------------------------------*/
int  VolumeGradientDeriche3DY(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeGradientDeriche3DY!");
     VipPrintfExit("VolumeGradientDeriche3DY");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeGradientDeriche3DY!");
     VipPrintfExit("VolumeGradientDeriche3DY");
     return(PB);
  }

  if (VolumeGradientDericheAlongColumns(vol,alpha)==PB) return(PB);
  if (VolumeSmoothingDericheAlongLines(vol,alpha)==PB) return(PB);
  if (VolumeSmoothingDericheAlongSlices(vol,alpha)==PB) return(PB);

  return(OK);
}



/*------------------------------------------------------------------------------*/
int  VolumeGradientDeriche3DZ(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeGradientDeriche3DZ!");
     VipPrintfExit("VolumeGradientDeriche3DZ");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeGradientDeriche3DZ!");
     VipPrintfExit("VolumeGradientDeriche3DZ");
     return(PB);
  }

  if (VolumeGradientDericheAlongSlices(vol,alpha)==PB) return(PB);
  if (VolumeSmoothingDericheAlongLines(vol,alpha)==PB) return(PB);
  if (VolumeSmoothingDericheAlongColumns(vol,alpha)==PB) return(PB);

  return(OK);
}

/*------------------------------------------------------------------------------*/
int  VolumeDeriche3DSmoothing(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeDeriche3DSmoothing!");
     VipPrintfExit("VolumeDeriche3DSmoothing");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeDeriche3DSmoothing!");
     VipPrintfExit("VolumeDeriche3DSmoothing");
     return(PB);
  }

  printf("Smoothing along slices...\n");
  if (VolumeSmoothingDericheAlongSlices(vol,alpha)==PB) return(PB);
  printf("Smoothing along lines...\n");
  if (VolumeSmoothingDericheAlongLines(vol,alpha)==PB) return(PB);
  printf("Smoothing along columns...\n");
  if (VolumeSmoothingDericheAlongColumns(vol,alpha)==PB) return(PB);

  return(OK);
}





/*------------------------------------------------------------------------------*/
int  VolumeGradientDericheAlongLines(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
float *ptr;
float *output,*work;
float afl1, bfl1, bfl2;
int i,j;
float *out;
int xsize;

  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeGradientDericheAlongLines!");
     VipPrintfExit("VolumeGradientDericheAlongLines");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeGradientDericheAlongLines!");
     VipPrintfExit("VolumeGradientDericheAlongLines");
     return(PB);
  }

  xsize = mVipVolSizeX(vol);

  output = (float*) VipCalloc(xsize,sizeof(float),
                          "VolumeGradientDericheAlongLines");
  work = (float*) VipCalloc(xsize,sizeof(float),
                          "VolumeGradientDericheAlongLines");

  CoefGradientDeriche(alpha,&afl1,&bfl1,&bfl2);

  ptr = VipGetDataPtr_VFLOAT( vol );
  for (i=mVipVolSizeY(vol)*mVipVolSizeZ(vol);i--;)
    { 
      GradientDeriche(ptr,output,work,afl1,bfl1,bfl2,xsize);
      out = output;
      for (j=xsize;j--;) *ptr++ = *out++;		
    }

  VipFree(output);
  VipFree(work);
  return(OK);
}


/*------------------------------------------------------------------------------*/
int  VolumeSmoothingDericheAlongLines(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
float *ptr;
float *output,*work;
float afl1;
float afl2;
float aflr1;
float aflr2;
float bfl1;
float bfl2;
int i,j;
float *out;
int xsize;

  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeSmoothingDericheAlongLines!");
     VipPrintfExit("VolumeSmoothingDericheAlongLines");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeGradientDericheAlongLines!");
     VipPrintfExit("VolumeSmoothingDericheAlongLines");
     return(PB);
  }

  xsize = mVipVolSizeX(vol);

  output = (float*) VipCalloc(xsize,sizeof(float),
                          "VolumeSmoothingDericheAlongLines");
  work = (float*) VipCalloc(xsize,sizeof(float),
                          "VolumeSmoothingDericheAlongLines");

  CoefSmoothingDeriche(alpha,&afl1,&afl2,&aflr1,&aflr2,&bfl1,&bfl2);

  ptr = VipGetDataPtr_VFLOAT( vol );
  for (i=mVipVolSizeY(vol)*mVipVolSizeZ(vol);i--;)
    { 
      SmoothingDeriche(ptr,output,work,afl1,afl2,aflr1,aflr2,bfl1,bfl2,xsize);
      out = output;
      for (j=xsize;j--;) *ptr++ = *out++;		
    }

  VipFree(output);
  VipFree(work);
  return(OK);
}




/*------------------------------------------------------------------------------*/
void  GradientDericheAlongLines(
float *p1,
float *p2,
float *output,
float *work,
float afl1_g,
float bfl1_g,
float bfl2_g,
int xsize,
int ysize)
/*------------------------------------------------------------------------------*/
{
  int i,j;
  float *out;

  for (j=ysize;j--;)
    { 
      GradientDeriche(p1,output,work,afl1_g,bfl1_g,bfl2_g,xsize);
      p1 += xsize;
      out = output;
      for (i=xsize;i--;) *p2++ = *out++;		
    }
}


/*------------------------------------------------------------------------------*/
int  VolumeGradientDericheAlongColumns(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
float *ptr, *p1, *in;
float *input,*output,*work;
float afl1, bfl1, bfl2;
int i,j, z;
float *out;
int xsize, ysize, slicesize;

  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeGradientDericheAlongColumns!");
     VipPrintfExit("VolumeGradientDericheAlongColumns");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeGradientDericheAlongColumns!");
     VipPrintfExit("VolumeGradientDericheAlongColumns");
     return(PB);
  }

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  slicesize = xsize*ysize;

  input = (float*) VipCalloc(ysize,sizeof(float),
                          "VolumeGradientDericheAlongColumns");
  output = (float*) VipCalloc(ysize,sizeof(float),
                          "VolumeGradientDericheAlongColumns");
  work = (float*) VipCalloc(ysize,sizeof(float),
                          "VolumeGradientDericheAlongColumns");

  CoefGradientDeriche(alpha,&afl1,&bfl1,&bfl2);

  for(z=0;z<mVipVolSizeZ(vol);z++)
  {
     p1 = VipGetDataPtr_VFLOAT( vol ) + z * slicesize;
     for (i=xsize;i--;)
     {
        ptr = p1;
        in = input;
        for(j=ysize;j--;)
	{
	  *in++ = *ptr;
	  ptr += xsize;
	}
        GradientDeriche(input,output,work,afl1,bfl1,bfl2,ysize);
        out = output;
        ptr = p1++;
        for (j=ysize;j--;) 
	{
 	   *ptr = *out++;
	    ptr += xsize;
	}
     }
  }
  VipFree(input);
  VipFree(output);
  VipFree(work);
  return(OK);
}



/*------------------------------------------------------------------------------*/
int  VolumeSmoothingDericheAlongColumns(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
float *ptr, *p1, *in;
float *input,*output,*work;
float afl1;
float afl2;
float aflr1;
float aflr2;
float bfl1;
float bfl2;
int i,j, z;
float *out;
int xsize, ysize, slicesize;

  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeSmoothingDericheAlongColumns!");
     VipPrintfExit("VolumeSmoothingDericheAlongColumns");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeSmoothingDericheAlongColumns!");
     VipPrintfExit("VolumeSmoothingDericheAlongColumns");
     return(PB);
  }

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  slicesize = xsize*ysize;

  input = (float*) VipCalloc(ysize,sizeof(float),
                          "VolumeSmoothingDericheAlongColumns");
  output = (float*) VipCalloc(ysize,sizeof(float),
                          "VolumeSmoothingDericheAlongColumns");
  work = (float*) VipCalloc(ysize,sizeof(float),
                          "VolumeSmoothingDericheAlongColumns");

  CoefSmoothingDeriche(alpha,&afl1,&afl2,&aflr1,&aflr2,&bfl1,&bfl2);

  for(z=0;z<mVipVolSizeZ(vol);z++)
  {
     p1 = VipGetDataPtr_VFLOAT( vol ) + z * slicesize;
     for (i=xsize;i--;)
     {
        ptr = p1;
        in = input;
        for(j=ysize;j--;)
	{
	  *in++ = *ptr;
	  ptr += xsize;
	}
        SmoothingDeriche(input,output,work,afl1,afl2,aflr1,aflr2,bfl1,bfl2,ysize);
        out = output;
        ptr = p1++;
        for (j=ysize;j--;) 
	{
 	   *ptr = *out++;
	    ptr += xsize;
	}
     }
  }
  VipFree(input);
  VipFree(output);
  VipFree(work);
  return(OK);
}


/*------------------------------------------------------------------------------*/
int  VolumeGradientDericheAlongSlices(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
float *ptr, *p1, *in;
float *input,*output,*work;
float afl1, bfl1, bfl2;
int i,j;
float *out;
int xsize, ysize, zsize, slicesize;

  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeGradientDericheAlongSlices!");
     VipPrintfExit("VolumeGradientDericheAlongSlices");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeGradientDericheAlongSlices!");
     VipPrintfExit("VolumeGradientDericheAlongSlices");
     return(PB);
  }

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  slicesize = xsize*ysize;

  input = (float*) VipCalloc(zsize,sizeof(float),
                          "VolumeGradientDericheAlongSlices");
  output = (float*) VipCalloc(zsize,sizeof(float),
                          "VolumeGradientDericheAlongSlices");
  work = (float*) VipCalloc(zsize,sizeof(float),
                          "VolumeGradientDericheAlongSlices");

  CoefGradientDeriche(alpha,&afl1,&bfl1,&bfl2);

  p1 = VipGetDataPtr_VFLOAT( vol );

  for(i=slicesize;i--;)
  {
     ptr = p1;
     in = input;
     for(j=zsize;j--;)
     {
        *in++ = *ptr;
	ptr += slicesize;
     }
     GradientDeriche(input,output,work,afl1,bfl1,bfl2,zsize);
     out = output;
     ptr = p1++;
     for (j=zsize;j--;) 
     {
 	*ptr = *out++;
	 ptr += slicesize;
     }
  }
  VipFree(input);
  VipFree(output);
  VipFree(work);
  return(OK);
}


/*------------------------------------------------------------------------------*/
int  VolumeSmoothingDericheAlongSlices(
Volume *vol,
float alpha)
/*------------------------------------------------------------------------------*/
{
float *ptr, *p1, *in;
float *input,*output,*work;
float afl1;
float afl2;
float aflr1;
float aflr2;
float bfl1;
float bfl2;
int i,j;
float *out;
int xsize, ysize, zsize, slicesize;

  if(VipTestType(vol,VFLOAT)==PB)
  {
     VipPrintfError("volume type HAS to be float in VolumeSmoothingDericheAlongSlices!");
     VipPrintfExit("VolumeSmoothingDericheAlongSlices");
     return(PB);
  }
  if(mVipVolBorderWidth(vol)!=0)
  {
     VipPrintfError("volume borderwidth HAS to be 0 in VolumeSmoothingDericheAlongSlices!");
     VipPrintfExit("VolumeSmoothingDericheAlongSlices");
     return(PB);
  }

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  slicesize = xsize*ysize;

  input = (float*) VipCalloc(zsize,sizeof(float),
                          "VolumeSmoothingDericheAlongSlices");
  output = (float*) VipCalloc(zsize,sizeof(float),
                          "VolumeSmoothingDericheAlongSlices");
  work = (float*) VipCalloc(zsize,sizeof(float),
                          "VolumeSmoothingDericheAlongSlices");

  CoefSmoothingDeriche(alpha,&afl1,&afl2,&aflr1,&aflr2,&bfl1,&bfl2);

  p1 = VipGetDataPtr_VFLOAT( vol );

  for(i=slicesize;i--;)
  {
     ptr = p1;
     in = input;
     for(j=zsize;j--;)
     {
        *in++ = *ptr;
	ptr += slicesize;
     }
     SmoothingDeriche(input,output,work,afl1,afl2,aflr1,aflr2,bfl1,bfl2,zsize);
     out = output;
     ptr = p1++;
     for (j=zsize;j--;) 
     {
 	*ptr = *out++;
	 ptr += slicesize;
     }
  }
  VipFree(input);
  VipFree(output);
  VipFree(work);
  return(OK);
}





/*------------------------------------------------------------------------------*/
void  GradientDericheAlongColumns(
float *p1,
float *p2,
float *input,
float *output,
float *work,
float afl1_g,
float bfl1_g,
float bfl2_g,
int xsize,
int ysize)
/*------------------------------------------------------------------------------*/
{
  int i,j;
  float *out;
  float *ptr, *in;

  for (i=xsize;i--;)
    {
      ptr = p1++;
      in = input;
      for(j=ysize;j--;)
	{
	  *in++ = *ptr;
	  ptr += xsize;
	}
      GradientDeriche(input,output,work,afl1_g,bfl1_g,bfl2_g,ysize);
      out = output;
      ptr = p2++;
      for (j=ysize;j--;) 
	{
	  *ptr = *out++;
	  ptr += xsize;
	}
    }
}

/*------------------------------------------------------------------------------*/
void  SmoothingDericheAlongLines(
float *p1,
float *p2,
float *output,
float *work,
float afl1,
float afl2,
float aflr1,
float aflr2,
float bfl1,
float bfl2,
int xsize,
int ysize)
/*------------------------------------------------------------------------------*/
{
  int i,j;
  float *out;

  for (j=ysize;j--;)
    { 
      SmoothingDeriche(p1,output,work,afl1,afl2,aflr1,aflr2,bfl1,bfl2,xsize);
      p1 += xsize;
      out = output;
      for (i=xsize;i--;) *p2++ = *out++;		
    }
}

/*------------------------------------------------------------------------------*/
void  SmoothingDericheAlongColumns(
float *p1,
float *p2,
float *input,
float *output,
float *work,
float afl1,
float afl2,
float aflr1,
float aflr2,
float bfl1,
float bfl2,
int xsize,
int ysize)
/*------------------------------------------------------------------------------*/
{
  int i,j;
  float *out;
  float *ptr, *in;

  for (i=xsize;i--;)
    {
      ptr = p1++;
      in = input;
      for(j=ysize;j--;)
	{
	  *in++ = *ptr;
	  ptr += xsize;
	}
      SmoothingDeriche(input,output,work,afl1,afl2,aflr1,aflr2,bfl1,bfl2,ysize);
      out = output;
      ptr = p2++;
      for (j=ysize;j--;) 
	{
	  *ptr = *out++;
	  ptr += xsize;
	}
    }

}

/*------------------------------------------------------------------------------*/
void CoefSmoothingDeriche(
float alpha,
float *afl1,
float *afl2,
float *aflr1,
float *aflr2,
float *bfl1,
float *bfl2)
/*------------------------------------------------------------------------------*/
{
	*afl1= (float)(((1.0-exp((double)(-alpha)))*(1.0-exp((double)(-alpha))))/
		       (1.0+((2.0*(double)alpha)*(exp((double)(-alpha))))-(exp((double)(-2.0*(double)alpha)))));
	*afl2= (float)(exp((double)(-alpha)))*((double)alpha-1.0)*(*afl1);

	*bfl1= (float)(2.0*exp((double)(-alpha)));
	*bfl2= (float)(-exp((double)(-2.0*(double)alpha)));

	*aflr1= (float)(exp((double)(-alpha))*((double)alpha+1.0))*(*afl1);
	*aflr2= (float)(-exp((double)(-2.0*(double)alpha)))*(*afl1);

}



/*------------------------------------------------------------------------------*/
/*   computation of the smoothing for a line "input" of length "dim"   */
/*   the result is stored in "output", "work is used for computation   */
/*   the coefficients are already computed                             */
void SmoothingDeriche(
float *input,
float *output,
float *work,
float a1,
float a2,
float a11,
float a22,
float b1,
float b2,
int dim)
/*------------------------------------------------------------------------------*/
{
    int test;
    int n;
    float *output_ptr0, *output_ptr1, *output_ptr2;
    float *input_ptr0, *input_ptr1;

    /*With images full of zero, we have pb with sparc stations (very slow computations),
      this few lines should partly correct the pb*/

    test = VTRUE;
    input_ptr0 = input;
    
    for (n=dim;n--;)
	if(*input_ptr0++!=0)
	    {
		test=VFALSE;
		break;
	    }

    if(test==VFALSE)
	{
	    output_ptr0 = output;
	    output_ptr1 = output+1;
	    output_ptr2 = output+2;
	    input_ptr0 = input;
	    input_ptr1 = input + 1;

	    *output_ptr0 = a1 * *input_ptr0;
	    *output_ptr1 = a1 * *input_ptr1++ + a2 * *input_ptr0++ + b1 * *output_ptr0;
	    for(n=(dim-2);n--;)
		*output_ptr2++ = a1 * *input_ptr1++ + a2 * *input_ptr0++
		    + b1 * *output_ptr1++ + b2 * *output_ptr0++; 
  

	    output_ptr0 = work + dim-1;
	    output_ptr1 = output_ptr0 - 1;
	    output_ptr2 = output_ptr1 - 1;
	    input_ptr0 = input + dim-1;
	    input_ptr1 = input_ptr0 - 1;

	    *output_ptr0 = 0.0;
	    *output_ptr1 = a11 * *input_ptr0;
	    for(n=(dim-2);n--;)
		*output_ptr2-- = a11 * *input_ptr1-- +
		    a22 * *input_ptr0-- + b1 * *output_ptr1-- + b2 * *output_ptr0--; 
	    
	}

    if(test==VFALSE) 
	{
	    output_ptr0 = output;
	    input_ptr0 = work;

	    for (n=dim;n--;)
		{
		    *output_ptr0 += *input_ptr0++;
		    output_ptr0++;
		}
	}
    else
	{
	    output_ptr0 = output;
	    for (n=dim;n--;)
		{
		    *output_ptr0++ = 0.;
		}
	}
	
}

/*------------------------------------------------------------------------------*/
void CoefGradientDeriche(
 float alpha,
 float *afd1,
 float *bfd1,
 float *bfd2)
/*------------------------------------------------------------------------------*/
{
	*afd1 =  (float)(1.0-exp((double)(-alpha)));
	*afd1=(*afd1)*(*afd1);

	*bfd1= (float)(2.0*exp((double)(-alpha)));
	*bfd2= (float)(-exp((double)(-2.0*(double)alpha)));
}

/*------------------------------------------------------------------------------*/
/*   computation of the first derivative for a line "input" of length "dim"   */
/*   the result is stored in "output", "work is used for computation          */
/*   the coefficients are already computed                                    */
void GradientDeriche(
float *input,
float *output,
float *work,
float a1,
float b1,
float b2,
int dim)
/*------------------------------------------------------------------------------*/
{
    int test;
    int n;
    float *output_ptr0, *output_ptr1, *output_ptr2;
    float *input_ptr;
    float temp;

    test = VTRUE;
    input_ptr = input;
    
    for (n=dim;n--;)
	if(*input_ptr++!=0)
	    {
		test=VFALSE;
		break;
	    }

    if(test==VFALSE)
	{
	    output_ptr0 = output;
	    output_ptr1 = output+1;
	    output_ptr2 = output+2;
	    input_ptr = input;
  
	    *output_ptr0 = 0.0;
	    *output_ptr1 = a1* *input_ptr++ ;
	    for(n=(dim-2);n--;)
		*output_ptr2++ = a1 * *input_ptr++ + b1 * *output_ptr1++
		    + b2 * *output_ptr0++; 
  

	    output_ptr0 = work + dim-1;
	    output_ptr1 = output_ptr0 - 1;
	    output_ptr2 = output_ptr1 - 1;
	    input_ptr = input + dim-1;
  
	    *output_ptr0 = 0.0;
	    *output_ptr1 = -a1 * *input_ptr--;
	    for(n=(dim-2);n--;)
		*output_ptr2-- = -a1 * *input_ptr-- + b1 * *output_ptr1-- 
		    + b2 * *output_ptr0--; 

	}

    if(test==VFALSE)
	{
	    output_ptr0 = output;
	    input_ptr = work;
	    for (n=dim;n--;)
		{
		    temp = *output_ptr0 + *input_ptr++;
		    *output_ptr0++ = -temp;
		}
	}
   else
	{
	    output_ptr0 = output;
	    for (n=dim;n--;)
		{
		    *output_ptr0++ = 0.;
		}
	}
}

/*------------------------------------------------------------------------------*/
void CoefLaplacianDeriche(
float alpha,
float *a1,
float *a2,
float *b1,
float *b2)
/*------------------------------------------------------------------------------*/
{
	float k;

	k=(float)(1.0-(exp((double)(-2.0*(double)alpha))))/(2.0*(double)alpha*(exp((double)(-alpha))));
	*a1= -(float)(1.0+((double)k*(double)alpha))*(exp((double)(-alpha)));
	*a2= (float)(1.0-((double)k*(double)alpha))*(exp((double)(-alpha)));
	*b1= (float)(2.0*exp((double)(-alpha)));
	*b2= -(float)(exp((double)(-2.0*(double)alpha)));
}

/*------------------------------------------------------------------------------*/
void LaplacianDeriche(
float a1,
float a2,
float b1,
float b2,
float *input,
float *output,
float *work,
int dim)
/*------------------------------------------------------------------------------*/

{
    /*WARNING: a optimiser comme les autres...*/
    int n;
    int test;
    float *input_ptr;

    test = VTRUE;
    input_ptr = input;
    
    for (n=dim;n--;)
	if(*input_ptr++!=0)
	    {
		test=VFALSE;
		break;
	    }

    if(test==VFALSE)
	{

	    *(output)=(*(input));
	    *(output+1)=(*(input+1))+(a1*(*(input)))+(b1*(*(output)));
	    for (n=2;n<dim;n++)
		*(output+n)=(*(input+n))+
		    (a1*(*(input+n-1)))+(b1*(*(output+n-1)))+(b2*(*(output+n-2)));

	    *(work+dim-1)=0.0;
	    *(work+dim-2)=(a2*(*(input+dim-1)));
	    for (n=dim-3;n>=0;n--)
		*(work+n)=
		    (a2*(*(input+n+1)))+(b2*(*(input+n+2)))
		    +(b1*(*(work+n+1)))+(b2*(*(work+n+2)));	

	    for (n=0;n<dim;n++)
		*(output+n)= -((*(output+n))+(*(work+n)));
	}
    else
	for (n=0;n<dim;n++)
	    *(output+n)= 0.;
}



/*------------------------------------------------------------------------------*/
void Compute2dGradientNorme( 
  float *gx,
  float *gy,
  float *gn,
  int size)
/*------------------------------------------------------------------------------*/
	/* Calcul la norme du gradient pour une liste de taille size */
{
  int i;
  float temp1, temp2;
  float *ptr;

  if (!gx || !gy || !gn)
    {
      VipPrintfError("No buffer in Compute2dGradientNorme");
      return;
    }

  ptr = gn;
  for(i=size;i--;)
    {
      temp1 = *gx++;
      temp2 = *gy++;
      temp1 = (float)sqrt((double)(temp1*temp1+temp2*temp2));
      *ptr++ = temp1;
    }
}

/*------------------------------------------------------------------------------*/
void Extract2dMaximaFloat( 
   float *gx,
   float *gy,
   float *gn,
   float *gext,   
   int xsize,
   int ysize,
   float threshold
)
/*------------------------------------------------------------------------------*/

	/* extraction des maxima locaux dans la direction du gradient,
		les normes doivent etre prealablement calculees dans gn, 
		le resultat est dans gext */
{
  float *gxptr, *gyptr, *gnptr, *gextptr;
  float tanpisur8, tan3pisur8, ysurx, absysurx;
  int offset10, offset11, offset01, offsetm11;
  int offsetm10, offsetm1m1, offset0m1, offset1m1;
  int offset1, offset2;
  int x, y;
  
  if(!gn || !gx || !gy || !gext)
    {
      VipPrintfError("Not all buffers in Extract2dMaximaFloat");
      return;	
    }

  /* offset pour atteindre les 8 voisins */

  offset10 = 1;
  offset11 = 1+xsize;
  offset01 = xsize;
  offsetm11 = xsize-1;
  offsetm10 = -1;
  offsetm1m1 = -xsize-1;
  offset0m1 = -xsize;
  offset1m1 = -xsize+1;
  
  tanpisur8 = (float)tan(M_PI*(double)(1./8.));
  tan3pisur8 = (float)tan(M_PI*(double)(3./8.));

  if(threshold<=0.) threshold = 1.;

  /*   the borders of the slice are filled with 0 (borderwidth2)*/

  gxptr = gx+2*xsize;
  gyptr = gy+2*xsize;
  gextptr = gext;
  gnptr = gn+2*xsize;  
  for(x=2*xsize;x--;) *gextptr++ = 0.0;

  for(y=ysize-4;y--;)
    {
      *gextptr++ = 0.0;
      *gextptr++ = 0.0;
      gnptr+=2;
      gxptr+=2;
      gyptr+=2;
      for(x=xsize-4;x--;)
	{
	  if(*gnptr<threshold)
	    {
	      *gextptr++ = 0.0;
	      gnptr++;
	      gxptr++;
	      gyptr++;
	    }
	  else
	    {
	      if(fabs((double)*gxptr)<EPSILON)
		{
		  offset1 = offset01;
		  offset2 = offset0m1;
		  gxptr++;
		  gyptr++;
		}
	      else
		{
		  ysurx = *gyptr++ / *gxptr++;
		  absysurx = (float)fabs((double)ysurx);
		  if (absysurx<tanpisur8)
		    {
		      offset2 = offsetm10;
		      offset1 = offset10;
		    }
		  else if(absysurx>tan3pisur8)
		    {
		      offset1 = offset01;
		      offset2 = offset0m1;
		    }
		  else if(ysurx>0)
		    {
		      offset1 = offset11;
		      offset2 = offsetm1m1;
		    }
		  else
		    {
		      offset1 = offset1m1;
		      offset2 = offsetm11;
		    }
		}
	      if ((*(gnptr+offset1)>*gnptr) || (*(gnptr+offset2)>*gnptr) )
		{
		  *gextptr++ = 0.0;
		  gnptr++;
		}
	      else *gextptr++ = *gnptr++;
	    }			
	}
      *gextptr++ = 0.0;
      *gextptr++ = 0.0;
      gnptr+=2;
      gxptr+=2;
      gyptr+=2;
    }
  for(x=2*xsize;x--;) *gextptr++ = 0.0;
}
/************************************************************/

/*----------------------------------------------------------------------------*/
int VipConvertStringToExtedgeMode(
char *thestring
)
/*----------------------------------------------------------------------------*/
{
  int extedge;

  if(!thestring)
    {
      VipPrintfError("NULL string in VipConvertStringToExtedgeMode");
      VipPrintfExit("VipConvertStringToExtedgeMode");
      return(PB);
    }
  
  if(!strcmp(thestring,"3Da"))
    extedge = EXTEDGE3D_ALL;
  else if(!strcmp(thestring,"3D-ztop"))
    extedge = EXTEDGE3D_ALL_EXCEPT_Z_TOP;
  else if(!strcmp(thestring,"3D-zbot"))
    extedge = EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM;
  else if(!strcmp(thestring,"2Da"))
    extedge = EXTEDGE2D_ALL;
  else if(!strcmp(thestring,"2D-ytop"))
    extedge = EXTEDGE2D_ALL_EXCEPT_Y_TOP;
  else if(!strcmp(thestring,"2D-ybot"))
    extedge = EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM;

  else
    {
      VipPrintfWarning("Unknown mode string in VipConvertStringToExtedgeMode");
      return(PB);
    }

  return(extedge);
 
}

/*----------------------------------------------------------------------------*/
Volume  *VipDeriche2DGradientNorm(
  Volume *vol,
  float ALPHA,
  int how,
  int mode,
  float threshold)
/*----------------------------------------------------------------------------*/
{
  if(mode==DERICHE_EXTREMA_DIRECTION)
    {
      VipPrintfError("Use VipDeriche2DGradientNormAndReturnG2DBucket for DIRECTION mode");
      VipPrintfExit("VipDeriche2DGradientNorm");
      return((Volume *)NULL);
    }
   return( VipDeriche2DGradientNormAndReturnG2DBucket(vol,
                                        ALPHA,how,mode,threshold,NULL));
}
