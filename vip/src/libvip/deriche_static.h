/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/deriche_static.h     * TYPE     : Header
 * AUTHOR      : MANGIN J.F.              * CREATION : 04/01/1997
 * VERSION     : 0.1                      * REVISION :
 * LANGUAGE    : C                        * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Informations privees du package deriche
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


#ifndef VIP_DERICHE_STATIC_H
#define VIP_DERICHE_STATIC_H

#ifndef M_PI /* defini dans <math.h> seulement sur Sun, Borland, etc. */
#define M_PI 3.14159265358979323846
#endif

#define EPSILON 0.01

#include <vip/deriche/deriche_gen_static.h>
#include <vip/deriche/extedge_gen_static.h>

#define D_S16BITMAX 32500
#define D_S8BITMAX 125

/*----------------------------------------------------------------------------*/
int  Compute3DNorm(
Volume *newv,
Volume *gx,
Volume *gy,
Volume *gz,
float gxabsmax,
float gyabsmax,
float gzabsmax,
float threshold);
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void CoefSmoothingDeriche(
float alpha,
float *afl1,
float *afl2,
float *bfl1,
float *bfl2,
float *aflr1,
float *aflr2);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
/*   computation of the smoothing for a line "input" of length "dim"   */
/*   the result is stored in "output", "work is used for computation   */
/*   the coefficients are already computed                             */
extern void SmoothingDeriche(
float *input,
float *output,
float *work,
float a1,
float a2,
float a11,
float a22,
float b1,
float b2,
int dim);
/*------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int  Extract3DMaxima(
Volume *fvol,
Volume *gx,
Volume *gy,
Volume *gz,
float gxabsmax,
float gyabsmax,
float gzabsmax,
float threshold);
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void CoefGradientDeriche(
 float alpha,
 float *afd1,
 float *bfd1,
 float *bfd2);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
/*   computation of the first derivative for a line "input" of length "dim"   */
/*   the result is stored in "output", "work is used for computation          */
/*   the coefficients are already computed                                    */
extern void GradientDeriche(
float *input,
float *output,
float *work,
float a1,
float b1,
float b2,
int dim);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void CoefLaplacianDeriche(
float alpha,
float *a1,
float *a2,
float *b1,
float *b2);

/*------------------------------------------------------------------------------*/
extern void LaplacianDeriche(
float a1,
float a2,
float b1,
float b2,
float *input,
float *output,
float *work,
int dim);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void Compute2dGradientNorme( 
  float *gx,
  float *gy,
  float *gn,
  int size);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void Extract2dMaximaFloat( 
   float *gx,
   float *gy,
   float *gn,
   float *gext,   
   int xsize,
   int ysize,
   float threshold
);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern int  VolumeGradientDericheAlongLines(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern int  VolumeGradientDericheAlongColumns(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------*/
extern int  VolumeSmoothingDericheAlongSlices(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern int  VolumeSmoothingDericheAlongLines(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern int  VolumeSmoothingDericheAlongColumns(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------*/
extern int  VolumeGradientDericheAlongSlices(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void  GradientDericheAlongLines(
float *p1,
float *p2,
float *output,
float *work,
float afl1_g,
float bfl1_g,
float bfl2_g,
int xsize,
int ysize);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void  GradientDericheAlongColumns(
float *p1,
float *p2,
float *input,
float *output,
float *work,
float afl1_g,
float bfl1_g,
float bfl2_g,
int xsize,
int ysize);
/*------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/
extern void  SmoothingDericheAlongLines(
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
int ysize);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern void  SmoothingDericheAlongColumns(
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
int ysize);
/*------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/
extern int  VolumeGradientDeriche3DX(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/
extern int  VolumeGradientDeriche3DY(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern int  VolumeGradientDeriche3DZ(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
extern int  VolumeDeriche3DSmoothing(
Volume *vol,
float alpha);
/*------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------*/
#endif /* VIP_DERICHE_STATIC_H */
