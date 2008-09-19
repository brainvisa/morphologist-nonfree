/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/moment.h         * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 26/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Calcul les moments d'une forme
 *
 ****************************************************************************
 *
 * USED MODULES : math.h - values.h - vip/alloc.h - vip/complex.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_MOMENT_H
#define VIP_MOMENT_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <math.h>
#include <float.h>
#include <vip/alloc.h>
#include <vip/vcomplex.h>

#define M_SUB -1
#define M_ADD  1

#define M_2D 2
#define M_3D 3

#define M_NORMAL   100
#define M_LEGENDRE 101

#define IsNull(a)  ((a)+(double)((a) == 0.0))
#define IsAlone(a) (double)((a-1) != 0.0)

#define Module(a,b,c)         (sqrt(Sq(a)+Sq(b)+Sq(c)))
#define Scalaire(a,b,c,d,e,f) ((a)*(d)+(b)*(e)+(c)*(f))

typedef struct 
{ double sum;               /* nombre de voxels de l'objet 3D */
  double m0;                /* aussi egal a mu0 */
  double m1[3];             /* 0:m100; 1:m010; 2:m001 */
  double mu2[6];            /* 0:mu200; 1:mu020; 2:mu002; 3:mu110; 4:mu101; 5:mu011 */
  double mu3[10];           /* 0:mu300; 1:mu030; 2:mu003; 3:mu210; 4:mu201; 
                               5:mu120; 6:mu021; 7:mu102; 8:mu012; 9:mu111 */
  double mx, my, mz;        /* centre de gravite */
  Complex_VDOUBLE nu0;       /* moment complex nu0 */
  Complex_VDOUBLE nu1[3];    /* 0:nu1,-1; 1:nu1,0;  2:nu1,1 */
  Complex_VDOUBLE nu2[5];    /* 0:nu2,-2; 1:nu2,-1; 2:nu2,0;  3:nu2,1; 4:nu2,2 */
  Complex_VDOUBLE nu3[7];    /* 0:nu3,-3; 1:nu3,-2; 2:nu3,-1; 3:nu3,0; 
                               4:nu3,1;  5:nu3,2;  6:nu3,3 */
  double I[12];             /* 0:I^2_00; 1:I^2_22; 2:I^2_222; 3:I^3_11; 4:I^3_33;
                               5:I^3_1113; 6:I^3_1133; 7:I^3_1333; 8:I^3_3333;
                               9:I^2,3_112; 10:I^2,3_123; 11:I^2,3_233 */
  double ct, cx, cy, cz;    /* corrections d'anisotropie de voxel */
  VipVector_VDOUBLE *Lambda; /* valeurs propres pour l'orientation */
  VipMatrix_VDOUBLE *I2;     /* matrice d'inertie, et vecteurs propres (orientation) */
} Moment;

typedef struct
{ double sum;               /* nombre de pixels de l'objet 2D */
  double m0;                /* aussi egal a mu0 */
  double m1[2];             /* 0:m10; 1:m01 */
  double mu2[3];            /* 0:mu20; 1:mu02: 2:mu11 */
  double mu3[4];            /* 0:mu30; 1:mu03; 2:mu21; 3:mu12 */
  double mx, my;            /* centre de gravite */
  double I[7];              /* invariants : Phi0 a Phi6 (Cf HU-62) */
  double ct, cx, cy;        /* corrections d'anisotropie de pixel */
  VipVector_VDOUBLE *Lambda; /* valeurs propres pour l'orientation */
  VipMatrix_VDOUBLE *I2;     /* matrice d'inertie, et vecteurs propres (orientation) */
} Moment2D;

typedef struct
{ Moment *moment;
  double l0;
  double l1[3];
  double lu2[6];
  double lu3[10];
  double lx, ly, lz;
} Legendre;

typedef struct
{ Moment2D *moment;
  double l0;
  double l1[2];
  double lu2[3];
  double lu3[4];
  double lx, ly;
} Legendre2D;

/*----------------------------------------------------------------------------*/
extern Moment *VipCreateMoment(
  Volume *vol
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipFreeMoment(
  Moment *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Legendre *VipCreateLegendreMoment(
  Volume *vol
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipFreeLegendreMoment(
  Legendre *leg
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipMomentCopy(
  Moment *mom1,
  Moment *mom2
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipMomentCopy2D(
  Moment2D *mom1,
  Moment2D *mom2
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipLegendreMomentCopy(
  Legendre *leg1,
  Legendre *leg2
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipLegendreMomentCopy2D(
  Legendre2D *leg1,
  Legendre2D *leg2
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipAddOrSubPointToMoment(
  Moment *mom,
  double  x,
  double  y,
  double  z,
  int     dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeInvariant(
  Moment *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipGetComplexMoment(
    Moment *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeMomentInvariants(
  Moment *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipComputeMoment(
  Volume     *vol,
  Vip_U16BIT  label,
  Moment     *mom,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipComputeMomentM(
  Volume     *vol,
  Vip_U16BIT  label,
  Moment     *mom,
  int         dir,
  int         mask
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Moment *VipCreateAndComputeMoment(
  Volume     *vol,
  Vip_U16BIT  label,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeMomentAndInvariant(
  Volume     *vol,
  Vip_U16BIT  label,
  Moment     *mom,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeMomentAndInvariantM(
  Volume     *vol,
  Vip_U16BIT  label,
  Moment     *mom,
  int         dir,
  int         mask
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Moment *VipCreateAndComputeMomentAndInvariant(
  Volume     *vol,
  Vip_U16BIT  label,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Moment *VipCreateAndComputeMomentAndInvariantM(
  Volume     *vol,
  Vip_U16BIT  label,
  int         dir,
  int         mask
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipUpdateMomentAndInvariant(
  Moment *mom,
  int     x, 
  int     y, 
  int     z, 
  int     dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeLegendreMoment(
  Legendre   *leg
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeMomentAndLegendreMoment(
  Volume     *vol,
  Vip_U16BIT  label,
  Legendre   *leg,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Legendre *VipCreateAndComputeMomentAndLegendreMoment(
  Volume     *vol,
  Vip_U16BIT  label,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamMoment(
  FILE   *stream,
  Moment *mom,
  char   *filename
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamLegendreMoment(
  FILE     *stream,
  Legendre *leg,
  char     *filename
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamInvariantForPCA(
  FILE *stream, 
  Moment *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamInvariantForCovarianceMatrix(
  FILE *stream, 
  Moment *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipPrintMoment(
  Moment *mom,
  char   *filename, 
  char   *resultfile
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipPrintLegendreMoment(
  Legendre *leg, 
  char     *filename, 
  char     *resultfile
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Moment2D *VipCreateMoment2D(
  Volume *vol
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipFreeMoment2D(
  Moment2D *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Legendre2D *VipCreateLegendreMoment2D(
  Volume *vol
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipFreeLegendreMoment2D(
  Legendre2D *leg
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipAddOrSubPointToMoment2D(
  Moment2D *mom,
  double    x,
  double    y,
  int       dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeInvariant2D(
  Moment2D *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipComputeMoment2D(
  Volume     *vol,
  Vip_U16BIT  label,
  Moment2D   *mom,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Moment2D *VipCreateAndComputeMoment2D(
  Volume     *vol,
  Vip_U16BIT  label,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeMomentAndInvariant2D(
  Volume     *vol,
  Vip_U16BIT  label,
  Moment2D   *mom,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Moment2D *VipCreateAndComputeMomentAndInvariant2D(
  Volume     *vol,
  Vip_U16BIT  label,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipUpdateMomentAndInvariant2D(
  Moment2D *mom,
  int       x, 
  int       y, 
  int       dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeLegendreMoment2D(
  Legendre2D *leg
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeMomentAndLegendreMoment2D(
  Volume     *vol,
  Vip_U16BIT  label,
  Legendre2D *leg,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Legendre2D *VipCreateAndComputeMomentAndLegendreMoment2D(
  Volume     *vol,
  Vip_U16BIT  label,
  int         dir
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamMoment2D(
  FILE     *stream,
  Moment2D *mom,
  char     *filename
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamLegendreMoment2D(
  FILE       *stream,
  Legendre2D *leg,
  char       *filename
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamInvariantForPCA2D(
  FILE     *stream, 
  Moment2D *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipPrintStreamInvariantForCovarianceMatrix2D(
  FILE     *stream, 
  Moment2D *mom
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipPrintMoment2D(
  Moment2D *mom,
  char     *filename, 
  char     *resultfile
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipPrintLegendreMoment2D(
  Legendre2D *leg, 
  char       *filename, 
  char       *resultfile
);
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
