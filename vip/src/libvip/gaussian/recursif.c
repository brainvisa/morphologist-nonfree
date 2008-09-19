/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : gaussian.c           * TYPE     : Source
 * AUTHOR      : COULON O/ MANGIN J.F.* CREATION : 07/12/98
 * VERSION     : 0.1                  * REVISION : 03/07/99
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
 *            10/04/00| V Frouin     | sx, sy, sz param
 *                    |              | for  Deriche3DSmoothing
 *****************************************************************************/
#include <vip/util.h>
#include <vip/alloc.h>
#include <vip/volume.h>
#include <vip/gaussian.h>
#include <vip/gaussian_static.h>


static int  VolumeSmoothingDericheAlongLines(Volume *vol ,Coefficient coef);
static int  VolumeSmoothingDericheAlongColumns(Volume *vol,Coefficient coef);
static int  VolumeSmoothingDericheAlongSlices(Volume *vol,Coefficient coef );
static void SmoothingDeriche(float *p1,float *p2,float *p3,int i,Coefficient coef );
static void calcul_coef(float sigma,Coefficient *coef, double a0,double a1,double b0,double b1,double c0,double c1,double w0,double w1);


/*---------------------------------------------------------------------------*/
void   calcul_coef(
float sigma,
Coefficient *coef,
double a0,
double a1,
double b0,
double b1,
double c0,
double c1,
double w0,
double w1
)
/*---------------------------------------------------------------------------*/   
{
 
  float b0s,b1s,w0s,w1s,eb0s,eb1s,e2b0s,e2b1s,cw1s,cw0s,sw0s,sw1s; 


  b0s=b0/sigma; 
  b1s=b1/sigma; 
  w0s=w0/sigma; 
  w1s=w1/sigma; 
  eb0s=exp((double)(-b0s));
  e2b0s=exp((double)(-2*b0s));
  eb1s=exp((double)(-b1s));
  e2b1s=exp((double)(-2*b1s));
  cw1s=(cos(w1s));
  cw0s=(cos(w0s));
  sw1s=(sin(w1s));
  sw0s=(sin(w0s));

  coef->n33= (float) (eb1s*e2b0s*(c1*sw1s  - cw1s*c0) + eb0s*e2b1s*(a1*sw0s - cw0s*a0));

  coef->n22=(float) (2*(eb1s*eb0s)*((a0+c0)*cw1s*cw0s - cw1s*a1*sw0s - cw0s*c1*sw1s)  + c0*e2b0s + a0*e2b1s) ;

  coef->n11= (float) (eb1s*(c1*sw1s - (c0+2*a0)*cw1s)  + eb0s*(a1*sw0s - (2*c0+a0)*cw0s));
  coef->n00 =( a0+c0);

  coef->d44= (float)(e2b0s*e2b1s);
  coef->d33= (float) ( -2*cw0s*eb0s*e2b1s -2*cw1s*eb1s*e2b0s);
  coef->d22 = (float)(4*cw1s*cw0s*eb0s*eb1s+e2b1s+e2b0s);
  coef->d11=(float)( -2*eb1s*cw1s-2*eb0s*cw0s);            

  coef->n11b= coef->n11 - coef->d11 * coef->n00;
  coef->n22b= coef->n22 - coef->d22 * coef->n00;
  coef->n33b=coef->n33 - coef->d33 * coef->n00;
  coef->n44b=(-1) *  coef->d44 * coef->n00;

}
/*---------------------------------------------------------------------------*/






/*---------------------------------------------------------------------------*/
int Deriche3DSmoothing(
Volume *vol,
float sx, float sy, float sz)
/*---------------------------------------------------------------------------*/
{
  Coefficient coefx, coefy, coefz;
  float sigmax, sigmay, sigmaz;
  double a0, a1, b0, b1, c0, c1, w0, w1;


  if(VipTestType(vol,VFLOAT)==PB)
    {
      VipPrintfError("volume type HAS to be float in VolumeDeriche3DSmoothing!");
      VipPrintfExit("Deriche3DSmoothing");
      return(PB);
    }
  if(mVipVolBorderWidth(vol)!=0)
    {
      VipPrintfError("volume borderwidth HAS to be 0 in VolumeDeriche3DSmoothing!");
      VipPrintfExit("Deriche3DSmoothing");
      return(PB);
    }

  sigmax = sx; /* / mVipVolVoxSizeX(vol); F.P. le 14/11/2000 */
  sigmay = sy; /* / mVipVolVoxSizeY(vol); */
  sigmaz = sz; /* / mVipVolVoxSizeZ(vol); */

  if((sigmax<0.6)||(sigmay<0.6)||(sigmaz<0.6))
      {
	  VipPrintfWarning("the small sigma relatively to voxel sizes could lead to a non conservation of levels...");
      }

  a0=0.657/sigmax;
  a1=1.979/sigmax; 
  b0=1.906;
  b1=1.881;
  c0=-0.258/sigmax;
  c1=-0.239/sigmax;
  w0=0.651;
  w1=2.053;
  calcul_coef(sigmax,&coefx, a0, a1, b0, b1, c0, c1, w0, w1);

  a0=0.657/sigmay;
  a1=1.979/sigmay; 
  c0=-0.258/sigmay;
  c1=-0.239/sigmay;
  calcul_coef(sigmay,&coefy, a0, a1, b0, b1, c0, c1, w0, w1);

  a0=0.657/sigmaz;
  a1=1.979/sigmaz; 
  c0=-0.258/sigmaz;
  c1=-0.239/sigmaz;
  calcul_coef(sigmaz,&coefz, a0, a1, b0, b1, c0, c1, w0, w1);
 
/*
  printf("Smoothing along slices...\n");
*/
  if (VolumeSmoothingDericheAlongSlices(vol,coefz)==PB) return(PB);
/*
  printf("Smoothing along lines...\n");
*/
  if (VolumeSmoothingDericheAlongLines(vol,coefx)==PB) return(PB);
/*
  printf("Smoothing along columns...\n");
*/
  if (VolumeSmoothingDericheAlongColumns(vol,coefy)==PB) return(PB);

  return (OK);

}
/*---------------------------------------------------------------------------*/








/*---------------------------------------------------------------------------*/
int  VolumeSmoothingDericheAlongLines(
Volume *vol,
Coefficient coef
)
/*---------------------------------------------------------------------------*/
{
  float *ptr;
  float *output,*work;
  int i,j;
  float *out;
  int xsize, ysize,zsize;

  if(VipTestType(vol,VFLOAT)==PB)
    {
      VipPrintfError("volume type HAS to be float in VolumeSmoothingDericheAlongLines!");
      VipPrintfExit("VolumeSmoothingDericheAlongLines");
      return(PB);
    }


  xsize = mVipVolSizeX(vol);
  ysize=mVipVolSizeY(vol);
  zsize=mVipVolSizeZ(vol);


  output = (float*) VipCalloc(xsize,sizeof(float),
                          "VolumeSmoothingDericheAlongLines");
  work = (float*) VipCalloc(xsize,sizeof(float),
                          "VolumeSmoothingDericheAlongLines");

 

  ptr = VipGetDataPtr_VFLOAT( vol );

  printf("     ");
  for (i=ysize*zsize;i--;)
    {  
/*
      printf("\b\b\b\b\b%5d",i);
      fflush(stdout);
*/
      SmoothingDeriche(ptr,output,work,xsize,coef);
      out = output;
      for (j=xsize;j--;)
	{ 
	  *ptr++ = *out++;
	}
      
    }
  /*printf("\n");*/

  VipFree(output);
  VipFree(work);
  return(OK);
}
/*--------------------------------------------------------------------------*/






/*------------------------------------------------------------------------------*/
int  VolumeSmoothingDericheAlongColumns(
Volume *vol,
Coefficient coef
)
/*------------------------------------------------------------------------------*/
{
  float *ptr, *p1, *in;
  float *input,*output,*work;

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

  printf("   ");
  for(z=0;z<mVipVolSizeZ(vol);z++)
    {
/*
      printf("\b\b\b%3d",mVipVolSizeZ(vol)-z);
      fflush(stdout);
*/
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
	  SmoothingDeriche(input,output,work,ysize,coef);
	  out = output;
	  ptr = p1++;
	  for (j=ysize;j--;) 
	    {
	      *ptr = *out++;
	      ptr += xsize;
	    }
	}
    }
/*printf("\n");*/

  VipFree(input);
  VipFree(output);
  VipFree(work);
  return(OK);
}
/*---------------------------------------------------------------------------*/







/*---------------------------------------------------------------------------*/
int  VolumeSmoothingDericheAlongSlices(
Volume *vol,
Coefficient coef
)
/*------------------------------------------------------------------------------*/
{
  float *ptr, *p1, *in;
  float *input,*output,*work;
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



  p1 = VipGetDataPtr_VFLOAT(vol);

  printf("     ");
  for(i=slicesize;i--;)
    {
/*
      printf("\b\b\b\b\b%5d",i);
      fflush(stdout);
*/
      ptr = p1;
      in = input;
      for(j=zsize;j--;)
	{
	  *in++ = *ptr;
	  ptr += slicesize;
	}

      SmoothingDeriche(input,output,work,zsize,coef);
      out = output;
      ptr = p1++;
     
      for (j=zsize;j--;) {
 	*ptr = *out++;
	ptr += slicesize;
      }
   
    }
/*printf("\n");*/

  VipFree(input);
  VipFree(output);
  VipFree(work);
  return(OK);
}
/*-------------------------------------------------------------------------*/







/*---------------------------------------------------------------------------*/
/*   computation of the smoothing for a line "input" of length "dim"   */
/*   the result is stored in "output", "work is used for computation   */
/*   the coefficients are already computed                             */
/*---------------------------------------------------------------------------*/
void SmoothingDeriche(
float *input,
float *output,
float *work,
int dim,
Coefficient coef
)
/*------------------------------------------------------------------------------*/
{
    int test;
    float value;
    int n;
    float *output_ptr0, *output_ptr1, *output_ptr2,*output_ptr3,*output_ptr4;
    float *input_ptr0, *input_ptr1,*input_ptr2,*input_ptr3;
    float n00,n11,n22,n33,n11b,n22b,n33b,n44b,d44,d33,d22,d11;

    
    /*With images full of zero, we have pb with sparc stations (very slow computations),
      this few lines should partly correct the pb*/
    input_ptr0 = input;
    value = *input_ptr0;
    test = VTRUE;
    for (n=dim;n--;)
	if(*input_ptr0++!=value)
	    {
		test=VFALSE;
		break;
	    }

    if(test==VFALSE)
	{

	    n00=coef.n00;
	    n11=coef.n11;
	    n22=coef.n22;
	    n33=coef.n33;
	    n11b=coef.n11b;
	    n22b=coef.n22b;
	    n33b=coef.n33b;
	    n44b=coef.n44b;
	    d44=coef.d44;
	    d33=coef.d33;
	    d22=coef.d22;
	    d11=coef.d11;





	    /*calcul de yk+ */

	    output_ptr0 = output;
	    output_ptr1 = output+1;
	    output_ptr2 = output+2;
	    output_ptr3 = output+3;
	    output_ptr4 = output+4;


	    input_ptr0 = input;
	    input_ptr1 = input + 1;
	    input_ptr2 = input + 2;
	    input_ptr3 = input + 3;
  
  

	    *output_ptr0 = n00 * *input_ptr0; 
	    *output_ptr1 = n00 * *input_ptr1 + n11 * *input_ptr0  - d11 * *output_ptr0; 
	    *output_ptr2 =  n00 * *input_ptr2 + n11 * *input_ptr1 + n22 * *input_ptr0  - d11 * *output_ptr1 - d22 *  *output_ptr0;
	    *output_ptr3 = n00* *input_ptr3++ +n11* *input_ptr2++ + n22* *input_ptr1++ + n33 * *input_ptr0++ - d11 * *output_ptr2 - d22 * *output_ptr1 - d33 * *output_ptr0;





	    for(n=(dim-4);n--;)
		{

		    *output_ptr4++ = n00 * *input_ptr3++ + n11 * *input_ptr2++ + n22 * *input_ptr1++ + n33 * *input_ptr0++ -  d11 * *output_ptr3++ -  d22 * *output_ptr2++ - d33 * *output_ptr1++  - d44 * *output_ptr0++;


		}
  


	    /* calcul des yk- */


	    output_ptr0 = work + dim-1;
	    output_ptr1 = output_ptr0 - 1;
	    output_ptr2 = output_ptr0 - 2;
	    output_ptr3 = output_ptr0- 3;
	    output_ptr4 =output_ptr0 - 4;

	    input_ptr0 = input + dim-1;
	    input_ptr1 = input_ptr0 - 1;
	    input_ptr2 = input_ptr0 - 2;
	    input_ptr3 = input_ptr0 - 3;



	    *output_ptr0 =0.0;
	    *output_ptr1 = n11b * *input_ptr0 ;
	    *output_ptr2= n11b *  *input_ptr1 + n22b *  *input_ptr0 - d11*  *output_ptr1- d22 *        *output_ptr0;


	    *output_ptr3= n11b *  *input_ptr2 + n22b *  *input_ptr1 + n33b *  *input_ptr0 - d11 *       *output_ptr2 - d22 *  *output_ptr1 - d33 *  *output_ptr0 ;


	    for(n=(dim-4);n--;)
		{ *output_ptr4-- = n11b * *input_ptr3-- + n22b * *input_ptr2-- +  n33b * *input_ptr1-- + n44b * *input_ptr0-- - d11 * *output_ptr3-- - d22 * *output_ptr2-- - d33 * *output_ptr1--  - d44 * *output_ptr0-- ; 
 
		}

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
		    *output_ptr0++ = value; 
		}
	}
	
}

  





	




