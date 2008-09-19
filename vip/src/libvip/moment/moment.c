/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_moment.c         * TYPE     : Function
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
 * USED MODULES : stdio.h - stdlib.h - string.h - Vip_util.h - Vip_alloc.h
 *                Vip_volume.h - Vip_eigen.h - Vip_complex.h - Vip_moment.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vip/util.h>
#include <vip/util/file.h>
#include <vip/alloc.h>
#include <vip/volume.h>
#include <vip/eigen.h>
#include <vip/vcomplex.h>
#include <vip/moment.h>

/***** Cree et initialise la structure de moments *****/
Moment *VipCreateMoment(Volume *vol)
{ Moment *mom;

  mom = (Moment *)VipCalloc(1, sizeof(Moment), NULL);
  if (!mom)  return((Moment *)NULL);

  mom->cx = (double)mVipVolVoxSizeX(vol);
  mom->cy = (double)mVipVolVoxSizeY(vol); 
  mom->cz = (double)mVipVolVoxSizeZ(vol); 
  mom->ct = mom->cx*mom->cy*mom->cz;

  mom->Lambda = VipAllocationVector_VDOUBLE(3, NULL);
  if (!mom->Lambda)
  { VipPrintfError("Lambda Moment structure field allocation failure.");
    VipPrintfExit("(Vip_moment.c)VipCreateMoment");
    VipFree(mom);
    return((Moment *)NULL);
  }

  mom->I2 = VipAllocationMatrix_VDOUBLE(3, 3, NULL);
  if (!mom->I2)
  { VipPrintfError("I2 Moment structure field allocation failure.");
    VipPrintfExit("(Vip_moment.c)VipCreateMoment");
    VipFreeVector_VDOUBLE(mom->Lambda);
    VipFree(mom);
    return((Moment *)NULL);
  }

  return(mom);
}

Moment2D *VipCreateMoment2D(Volume *vol)
{ Moment2D *mom;
 
  mom = (Moment2D *)VipCalloc(1, sizeof(Moment2D), NULL);
  if (!mom)  return((Moment2D *)NULL);

  mom->cx = (double)mVipVolVoxSizeX(vol);
  mom->cy = (double)mVipVolVoxSizeY(vol);
  mom->ct = mom->cx*mom->cy;

  mom->Lambda = VipAllocationVector_VDOUBLE(2, NULL);
  if (!mom->Lambda)
  { VipPrintfError("Lambda Moment2D structure field allocation failure.");
    VipPrintfExit("(Vip_moment.c)VipCreateMoment2D");
    VipFree(mom);
    return((Moment2D *)NULL);
  }

  mom->I2 = VipAllocationMatrix_VDOUBLE(2, 2, NULL);
  if (!mom->I2)
  { VipPrintfError("I2 Moment2D structure field allocation failure.");
    VipPrintfExit("(Vip_moment.c)VipCreateMoment2D");
    VipFreeVector_VDOUBLE(mom->Lambda);
    VipFree(mom);
    return((Moment2D *)NULL);
  }

  return(mom);
}

/***** Libere la structure de moments *****/
void VipFreeMoment(Moment *mom)
{ VipFreeVector_VDOUBLE(mom->Lambda);
  VipFreeMatrix_VDOUBLE(mom->I2);
  VipFree(mom);
}

void VipFreeMoment2D(Moment2D *mom)
{ VipFreeVector_VDOUBLE(mom->Lambda);
  VipFreeMatrix_VDOUBLE(mom->I2);
  VipFree(mom);
}

/***** Cree et initialise la structure des moments de Legendre *****/
Legendre *VipCreateLegendreMoment(Volume *vol)
{ Legendre *leg;

  leg = (Legendre *)VipCalloc(1, sizeof(Legendre), NULL);
  if (leg == NULL)  return((Legendre *)NULL);
  leg->moment = VipCreateMoment(vol);

  return(leg);
}

Legendre2D *VipCreateLegendreMoment2D(Volume *vol)
{ Legendre2D *leg;

  leg = (Legendre2D *)VipCalloc(1, sizeof(Legendre2D), NULL);
  if (leg == NULL)  return((Legendre2D *)NULL);
  leg->moment = VipCreateMoment2D(vol);

  return(leg);
}

/***** Libere la structure des moments de Legendre *****/
void VipFreeLegendreMoment(Legendre *leg)
{ VipFreeMoment(leg->moment);
  VipFree(leg);
}

void VipFreeLegendreMoment2D(Legendre2D *leg)
{ VipFreeMoment2D(leg->moment);
  VipFree(leg);
}

/***** Copie d'une structure de moment dans une autre *****/
void VipMomentCopy(Moment *mom1, Moment *mom2)
{ int i, j;

  mom1->sum = mom2->sum;
  mom1->m0 = mom2->m0;
  mom1->mx = mom2->mx;
  mom1->my = mom2->my;
  mom1->mz = mom2->mz;
  mom1->ct = mom2->ct;
  mom1->cx = mom2->cx;
  mom1->cy = mom2->cy;
  mom1->cz = mom2->cz;
  mom1->nu0.re = mom2->nu0.re;
  mom1->nu0.im = mom2->nu0.im;
  for (i=3;i--;)  
  { mom1->m1[i] = mom2->m1[i];
    mom1->nu1[i].re = mom2->nu1[i].re;
    mom1->nu1[i].im = mom2->nu1[i].im;
    mom1->Lambda->data[i] = mom2->Lambda->data[i];
  }
  for (i=6;i--;)  mom1->mu2[i] = mom2->mu2[i];
  for (i=10;i--;)  mom1->mu3[i] = mom2->mu3[i];
  for (i=12;i--;)  mom1->I[i] = mom2->I[i];
  for (i=5;i--;)  
  { mom1->nu2[i].re = mom2->nu2[i].re;
    mom1->nu2[i].im = mom2->nu2[i].im;
  }
  for (i=7;i--;)  
  { mom1->nu3[i].re = mom2->nu3[i].re;
    mom1->nu3[i].im = mom2->nu3[i].im;
  }
  for (j=3;j--;)
    for (i=3;i--;)  mom1->I2->data[j][i] = mom2->I2->data[j][i];
}

void VipMomentCopy2D(Moment2D *mom1, Moment2D *mom2)
{ int i, j;

  mom1->sum = mom2->sum;
  mom1->m0 = mom2->m0;
  mom1->mx = mom2->mx;
  mom1->my = mom2->my;
  mom1->ct = mom2->ct;
  mom1->cx = mom2->cx;
  mom1->cy = mom2->cy;
  for (i=2;i--;)  
  { mom1->m1[i] = mom2->m1[i];
    mom1->Lambda->data[i] = mom2->Lambda->data[i];
  }
  for (i=3;i--;)  mom1->mu2[i] = mom2->mu2[i];
  for (i=4;i--;)  mom1->mu3[i] = mom2->mu3[i];
  for (i=7;i--;)  mom1->I[i] = mom2->I[i];
  for (j=2;j--;)
    for (i=2;i--;)  mom1->I2->data[j][i] = mom2->I2->data[j][i];
}

void VipLegendreMomentCopy(Legendre *leg1, Legendre *leg2)
{ int i;

  VipMomentCopy(leg1->moment, leg2->moment);
  leg1->l0 = leg2->l0;
  leg1->lx = leg2->lx;
  leg1->ly = leg2->ly;
  leg1->lz = leg2->lz;
  for (i=3; i--;)  leg1->l1[i] = leg2->l1[i];
  for (i=6; i--;)  leg1->lu2[i] = leg2->lu2[i];
  for (i=10; i--;)  leg1->lu3[i] = leg2->lu3[i];
}

void VipLegendreMomentCopy2D(Legendre2D *leg1, Legendre2D *leg2)
{ int i;

  VipMomentCopy2D(leg1->moment, leg2->moment);
  leg1->l0 = leg2->l0;
  leg1->lx = leg2->lx;
  leg1->ly = leg2->ly;
  for (i=2; i--;)  leg1->l1[i] = leg2->l1[i];
  for (i=3; i--;)  leg1->lu2[i] = leg2->lu2[i];
  for (i=4; i--;)  leg1->lu3[i] = leg2->lu3[i];
}

/***** Ajout (dir=1) ou suppression (dir=-1) d'un point *****/
void VipAddOrSubPointToMoment(Moment *mom, double x, double y, double z, int dir)
{ double mul, div, div2, div22, d1sd, sens=(double)dir, mx, my, mz;
  double x2, y2, z2, x3, y3, z3, xy, xz, yz, x2y, x2z, xy2, y2z, xz2, yz2, xyz;
  double xymz, xzmy, yzmx, mxsd, mysd, mzsd, mxmy, mxmz, mymz;  
  double mxmymzsd, mxmyz, mxymz, xmymz, mx2, my2, mz2;
  double m2x, m2y, m2z, m2xy, m2xz, m2yz;
  double sx, sy, sz, ct, ct2;

  sx = (double)mom->cx;
  sy = (double)mom->cy;
  sz = (double)mom->cz;
  ct = (double)mom->ct;
  ct2 = mVipSq(ct);
  mul = (dir == M_ADD) ? ct : IsAlone(mom->sum)*ct;
  div = (dir == M_ADD) ? IsNull(mom->sum)*ct : mom->sum*ct;
  div2 = (dir == M_ADD) ? (mom->sum+1.0)*ct : IsNull(mom->sum-1.0)*ct;
  div22 = mVipSq(div2);

  x *= sx, y *= sy, z *= sz, x2 = mVipSq(x), y2 = mVipSq(y), z2 = mVipSq(z);
  x3 = x*x2, y3 = y*y2, z3 = z*z2, xy = x*y, xz = x*z, yz = y*z;
  x2y = x2*y, x2z = x2*z, xy2 = x*y2, y2z = y2*z, xz2 = x*z2, yz2 = y*z2;
  xyz = xy*z;
  mx = mom->m1[0], my = mom->m1[1], mz = mom->m1[2];
  m2x = mom->mu2[0], m2y = mom->mu2[1], m2z = mom->mu2[2];
  m2xy = mom->mu2[3], m2xz = mom->mu2[4], m2yz = mom->mu2[5];
  xymz = xy*mz, xzmy = xz*my, yzmx = yz*mx;
  mxsd = mx/div, mysd = my/div, mzsd = mz/div;
  mxmy = mx*my, mxmz = mx*mz, mymz = my*mz;
  mxmymzsd = mxmy*mz/div;
  mxmyz = mxmy*z, mxymz = mxmz*y, xmymz = mymz*x;
  mx2 = mVipSq(mx); my2 = mVipSq(my); mz2 = mVipSq(mz);
  d1sd = sens-ct/div;

  mom->mu3[0] += sens*(x3-3.0*(m2x*(x-mxsd)+sens*x3*ct+x2*mx)/div2+(2.0*x3*ct2+
                 6.0*sens*x2*mx*ct+3.0*x*mx2-mx2/div*(3.0*sens*x*ct+mx-
                 sens*mxsd*ct))/div22)*mul;
  mom->mu3[1] += sens*(y3-3.0*(m2y*(y-mysd)+sens*y3*ct+y2*my)/div2+(2.0*y3*ct2+
                 6.0*sens*y2*my*ct+3.0*y*my2-my2/div*(3.0*sens*y*ct+my-
                 sens*mysd*ct))/div22)*mul;
  mom->mu3[2] += sens*(z3-3.0*(m2z*(z-mzsd)+sens*z3*ct+z2*mz)/div2+(2.0*z3*ct2+
                 6.0*sens*z2*mz*ct+3.0*z*mz2-mz2/div*(3.0*sens*z*ct+mz-
                 sens*mzsd*ct))/div22)*mul;
  mom->mu3[3] += sens*(x2y-(2.0*m2xy*(x-mxsd)+m2x*(y-mysd)+2.0*xy*mx+3.0*sens*
                 x2y*ct+x2*my)/div2+(2.0*x*mxmy+y*mx2+4.0*sens*xy*mx*ct+2.0*
                 sens*x2*my*ct+2.0*x2y*ct2-sens*mxsd*(mxmy*d1sd+2.0*x*my*ct+
                 y*mx*ct))/div22)*mul;
  mom->mu3[4] += sens*(x2z-(2.0*m2xz*(x-mxsd)+m2x*(z-mzsd)+2.0*xz*mx+3.0*sens*
                 x2z*ct+x2*mz)/div2+(2.0*x*mxmz+z*mx2+4.0*sens*xz*mx*ct+2.0*
                 sens*x2*mz*ct+2.0*x2z*ct2-sens*mxsd*(mxmz*d1sd+2.0*x*mz*ct+
                 z*mx*ct))/div22)*mul;
  mom->mu3[5] += sens*(xy2-(2.0*m2xy*(y-mysd)+m2y*(x-mxsd)+2.0*xy*my+3.0*sens*
                 xy2*ct+y2*mx)/div2+(2.0*y*mxmy+x*my2+4.0*sens*xy*my*ct+2.0*
                 sens*y2*mx*ct+2.0*xy2*ct2-sens*mysd*(mxmy*d1sd+2.0*y*mx*ct+
                 x*my*ct))/div22)*mul;
  mom->mu3[6] += sens*(y2z-(2.0*m2yz*(y-mysd)+m2y*(z-mzsd)+2.0*yz*my+3.0*sens*
                 y2z*ct+y2*mz)/div2+(2.0*y*mymz+z*my2+4.0*sens*yz*my*ct+2.0*
                 sens*y2*mz*ct+2.0*y2z*ct2-sens*mysd*(mymz*d1sd+2.0*y*mz*ct+
                 z*my*ct))/div22)*mul;
  mom->mu3[7] += sens*(xz2-(2.0*m2xz*(z-mzsd)+m2z*(x-mxsd)+2.0*xz*mz+3.0*sens*
                 xz2*ct+z2*mx)/div2+(2.0*z*mxmz+x*mz2+4.0*sens*xz*mz*ct+2.0*
                 sens*z2*mx*ct+2.0*xz2*ct2-sens*mzsd*(mxmz*d1sd+2.0*z*mx*ct+
                 x*mz*ct))/div22)*mul;
  mom->mu3[8] += sens*(yz2-(2.0*m2yz*(z-mzsd)+m2z*(y-mysd)+2.0*yz*mz+3.0*sens*
                 yz2*ct+z2*my)/div2+(2.0*z*mymz+y*mz2+4.0*sens*yz*mz*ct+2.0*
                 sens*z2*my*ct+2.0*yz2*ct2-sens*mzsd*(mymz*d1sd+2.0*z*my*ct+
                 y*mz*ct))/div22)*mul;
  mom->mu3[9] += sens*(xyz-(m2yz*(x-mxsd)+m2xz*(y-mysd)+m2xy*(z-mzsd)+3.0*sens*
                 xyz*ct+xymz+xzmy+yzmx)/div2+(2.0*xyz*ct2+2.0*sens*(yzmx+xymz+
                 xzmy)*ct+xmymz+mxymz+mxmyz-mxmymzsd-sens*ct*(xmymz+mxymz+mxmyz-
		 mxmymzsd)/div)/div22)*mul; /* sens*mxmymzsd */
  mom->mu2[0] += sens*(x2-(2*x*mx+sens*x2*ct-mx2/div)/div2)*mul;
  mom->mu2[1] += sens*(y2-(2*y*my+sens*y2*ct-my2/div)/div2)*mul;
  mom->mu2[2] += sens*(z2-(2*z*mz+sens*z2*ct-mz2/div)/div2)*mul;
  mom->mu2[3] += sens*(xy-(x*my+y*mx+sens*xy*ct-mxmy/div)/div2)*mul;
  mom->mu2[4] += sens*(xz-(x*mz+z*mx+sens*xz*ct-mxmz/div)/div2)*mul;
  mom->mu2[5] += sens*(yz-(y*mz+z*my+sens*yz*ct-mymz/div)/div2)*mul;
  mom->m1[0] += sens*x*ct;
  mom->m1[1] += sens*y*ct;
  mom->m1[2] += sens*z*ct;
  mom->m0 += sens*ct;
  mom->sum += sens;
  div = IsNull(mom->sum)*ct;
  mom->mx = mom->m1[0]/div;
  mom->my = mom->m1[1]/div;
  mom->mz = mom->m1[2]/div;
}

void VipAddOrSubPointToMoment2D(Moment2D *mom, double x, double y, int dir)
{ double mul, div, div2, div22, d1sd, sens=(double)dir, mx, my;
  double x2, y2, x3, y3, xy, x2y, xy2, mxsd, mysd, mxmy;  
  double mx2, my2, m2x, m2y, m2xy, sx, sy, ct, ct2;

  sx = (double)mom->cx;
  sy = (double)mom->cy;
  ct = (double)mom->ct;
  ct2 = mVipSq(ct);
  mul = (dir == M_ADD) ? ct : IsAlone(mom->sum)*ct;
  div = (dir == M_ADD) ? IsNull(mom->sum)*ct : mom->sum*ct;
  div2 = (dir == M_ADD) ? (mom->sum+1.0)*ct : IsNull(mom->sum-1.0)*ct;
  div22 = mVipSq(div2);

  x *= sx, y *= sy, x2 = mVipSq(x), y2 = mVipSq(y);
  x3 = x*x2, y3 = y*y2, xy = x*y, x2y = x2*y, xy2 = x*y2;
  mx = mom->m1[0], my = mom->m1[1];
  m2x = mom->mu2[0], m2y = mom->mu2[1], m2xy = mom->mu2[2];
  mxsd = mx/div, mysd = my/div, mxmy = mx*my;
  mx2 = mVipSq(mx); my2 = mVipSq(my);
  d1sd = sens-ct/div;

  mom->mu3[0] += sens*(x3-3.0*(m2x*(x-mxsd)+sens*x3*ct+x2*mx)/div2+(2.0*x3*ct2+
                 6.0*sens*x2*mx*ct+3.0*x*mx2-mx2/div*(3.0*sens*x*ct+mx-
                 sens*mxsd*ct))/div22)*mul;
  mom->mu3[1] += sens*(y3-3.0*(m2y*(y-mysd)+sens*y3*ct+y2*my)/div2+(2.0*y3*ct2+
                 6.0*sens*y2*my*ct+3.0*y*my2-my2/div*(3.0*sens*y*ct+my-
                 sens*mysd*ct))/div22)*mul;
  mom->mu3[2] += sens*(x2y-(2.0*m2xy*(x-mxsd)+m2x*(y-mysd)+2.0*xy*mx+3.0*sens*
                 x2y*ct+x2*my)/div2+(2.0*x*mxmy+y*mx2+4.0*sens*xy*mx*ct+2.0*
                 sens*x2*my*ct+2.0*x2y*ct2-sens*mxsd*(mxmy*d1sd+2.0*x*my*ct+
                 y*mx*ct))/div22)*mul;
  mom->mu3[3] += sens*(xy2-(2.0*m2xy*(y-mysd)+m2y*(x-mxsd)+2.0*xy*my+3.0*sens*
                 xy2*ct+y2*mx)/div2+(2.0*y*mxmy+x*my2+4.0*sens*xy*my*ct+2.0*
                 sens*y2*mx*ct+2.0*xy2*ct2-sens*mysd*(mxmy*d1sd+2.0*y*mx*ct+
                 x*my*ct))/div22)*mul;
  mom->mu2[0] += sens*(x2-(2*x*mx+sens*x2*ct-mx2/div)/div2)*mul;
  mom->mu2[1] += sens*(y2-(2*y*my+sens*y2*ct-my2/div)/div2)*mul;
  mom->mu2[2] += sens*(xy-(x*my+y*mx+sens*xy*ct-mxmy/div)/div2)*mul;
  mom->m1[0] += sens*x*ct;
  mom->m1[1] += sens*y*ct;
  mom->m0 += sens*ct;
  mom->sum += sens;
  div = IsNull(mom->sum)*ct;
  mom->mx = mom->m1[0]/div;
  mom->my = mom->m1[1]/div;
}

int VipComputeInvariant(Moment *mom)
{ double c00, m200, m020, m002, m200_2, m020_2, m002_2, m0;
  double c22, m110_2, m101_2, m011_2, m200m020, m200m002, m020m002;
  double d1, d2, d3, d4, d5, d6, m110, m101, m011;
  double c222, m200_3, m020_3, m002_3, m200_2m020, m200_2m002, m020_2m200;
  double m020_2m002, m002_2m200, m002_2m020, m200m110_2, m200m101_2;
  double m020m110_2, m020m011_2, m002m101_2, m002m011_2, m200m020m002;
  double m200m011_2, m020m101_2, m002m110_2, m110m101m011;
  double c11, m300, m030, m003, m300_2, m030_2, m003_2, m210, m201, m120, m021;
  double m102, m012, m210_2, m201_2, m120_2, m021_2, m102_2, m012_2;
  double m300m120, m300m102, m030m210, m030m012, m003m201, m003m021;
  double m120m102, m210m012, m201m021, c33, m111, m111_2;
  VipVector_VDOUBLE *eigen_temp;
  Vip_DOUBLE **I2ptr;

  /***** pour le calcul des invariants par les moments complexes *****/
  Complex_VDOUBLE m3333, n3322, n3321, n3320, nm3321, nm3322;
  double  p1, p2, p3, p4, p5, p6, p7, p8, ce;
  Complex_VDOUBLE m3131, n3122, n3121, n3120, nm3121, nm3122;
  double  q1, q2, q3, q4, q5;
  Complex_VDOUBLE m3111, n1122, n1121, n1120, nm1121, nm1122;
  double r1, r2;
  Complex_VDOUBLE m3331, m112, m123, m233;
  /*******************************************************************/

  eigen_temp = VipAllocationVector_VDOUBLE(3, NULL);
  if (!eigen_temp)
  { VipPrintfError("Error while allocating temporary eigenvector vector");
    VipPrintfExit("(Vip_moment.c)VipComputeInvariant");
    return(PB);
  }

  m0 = mom->m0;
  d1 = pow(m0, 5.0/3.0);
  d2 = pow(m0, 10.0/3.0);
  d3 = pow(m0, 5.0);
  d4 = pow(m0, 4.0);
  d5 = pow(m0, 17.0/3.0);
  d6 = pow(m0, 8.0);

  c00 = 2.0*sqrt(M_PI)/3.0;
  m200 = mom->mu2[0], m020 = mom->mu2[1], m002 = mom->mu2[2];
  c22 = 16.0*M_PI/45.0/sqrt(5.0);
  m110 = mom->mu2[3], m101 = mom->mu2[4], m011 = mom->mu2[5];
  m200_2 = mVipSq(m200), m020_2 = mVipSq(m020), m002_2 = mVipSq(m002);
  m110_2 = mVipSq(m110), m101_2 = mVipSq(m101), m011_2 = mVipSq(m011);
  m200m020 = m200*m020, m200m002 = m200*m002, m020m002 = m020*m002;
  c222 = 32.0*M_PI*sqrt(2.0*M_PI)/675.0/sqrt(7.0);
  m200_3 = m200_2*m200, m020_3 = m020_2*m020, m002_3 = m002_2*m002;
  m200_2m020 = m200_2*m020, m200_2m002 = m200_2*m002, m020_2m200 = m020_2*m200;
  m020_2m002 = m020_2*m002, m002_2m200 = m002_2*m200, m002_2m020 = m002_2*m020;
  m200m110_2 = m200*m110_2, m200m101_2 = m200*m101_2, m200m011_2 = m200*m011_2;
  m020m110_2 = m020*m110_2, m020m101_2 = m020*m101_2, m020m011_2 = m020*m011_2;
  m002m110_2 = m002*m110_2, m002m101_2 = m002*m101_2, m002m011_2 = m002*m011_2;
  m200m020m002 = m200m020*m002, m110m101m011 = m110*m101*m011;
  c11 = -4.0*M_PI*sqrt(3.0)/25.0;
  m300 = mom->mu3[0], m030 = mom->mu3[1], m003 = mom->mu3[2];
  m300_2 = mVipSq(m300), m030_2 = mVipSq(m030), m003_2 = mVipSq(m003);
  m210 = mom->mu3[3], m201 = mom->mu3[4], m120 = mom->mu3[5];
  m021 = mom->mu3[6], m102 = mom->mu3[7], m012 = mom->mu3[8];
  m210_2 = mVipSq(m210), m201_2 = mVipSq(m201), m120_2 = mVipSq(m120);
  m021_2 = mVipSq(m021), m102_2 = mVipSq(m102), m012_2 = mVipSq(m012);
  m300m120 = m300*m120, m300m102 = m300*m102, m030m210 = m030*m210;
  m030m012 = m030*m012, m003m201 = m003*m201, m003m021 = m003*m021;
  m120m102 = m120*m102, m210m012 = m210*m012, m201m021 = m201*m021;
  c33 = -16.0*M_PI/175.0/sqrt(7.0);
  m111 = mom->mu3[9], m111_2 = mVipSq(m111);

  mom->I[0] = sqrt(c00*(m200+m020+m002)/d1);
  mom->I[1] = mVipRootN(c22*(m200_2+m020_2+m002_2+3.0*(m110_2+m101_2+m011_2)-
                    m200m020-m200m002-m020m002)/d2, 4.0);
  mom->I[2] =mVipRootN(c222*(-2.0*(m200_3+m020_3+m002_3)+3*(m200_2m020+m200_2m002+
                    m020_2m200+m020_2m002+m002_2m200+m002_2m020)-9.0*(
                    m200m110_2+m200m101_2+m020m110_2+m020m011_2+m002m101_2+
                    m002m011_2)+18.0*(m200m011_2+m020m101_2+m002m110_2)-
                    12.0*m200m020m002-54.0*m110m101m011)/d3, 6.0);
  mom->I[3] = mVipRootN(c11*(m300_2+m030_2+m003_2+m210_2+m201_2+m120_2+m021_2+
                    m102_2+m012_2+2.0*(m300m120+m300m102+m030m210+m030m012+
                    m003m201+m003m021+m120m102+m210m012+m201m021))/d4, 6.0);
  mom->I[4] = mVipRootN(c33*(m300_2+m030_2+m003_2+6.0*(m210_2+m201_2+m120_2+m021_2+
                    m102_2+m012_2)+15.0*m111_2-3.0*(m300m120+m300m102+m030m210+
                    m030m012+m003m201+m003m021+m201m021+m120m102+m210m012))/d4,
                    6.0);

  /***** Calcul des invariants par les moments complexes *****/
  ce = 1.0/sqrt(5.0);
  p1 = 2.0*sqrt(5.0/42.0);
  p2 = -2.0*sqrt(5.0/21.0);
  p3 = sqrt(2.0/7.0);
  p4 = 5.0/sqrt(21.0);
  p5 = -sqrt(5.0/7.0);
  p6 = 2.0/sqrt(42.0);
  p7 = -sqrt(3.0/7.0);
  p8 = 2.0/sqrt(21.0);
  n3322 = VipComplexProduct_VDOUBLE(mom->nu3[6], mom->nu3[2], p1);
  n3322 = VipComplexSum_VDOUBLE(n3322, VipComplexProduct_VDOUBLE(mom->nu3[5], mom->nu3[3], p2));
  n3322 = VipComplexSum_VDOUBLE(n3322, VipComplexSq_VDOUBLE(mom->nu3[4], p3));
  n3321 = VipComplexProduct_VDOUBLE(mom->nu3[6], mom->nu3[1], p4);
  n3321 = VipComplexSum_VDOUBLE(n3321, VipComplexProduct_VDOUBLE(mom->nu3[5], mom->nu3[2], p5));
  n3321 = VipComplexSum_VDOUBLE(n3321, VipComplexProduct_VDOUBLE(mom->nu3[4], mom->nu3[3], p6));
  n3320 = VipComplexProduct_VDOUBLE(mom->nu3[6], mom->nu3[0], p4);
  n3320 = VipComplexSum_VDOUBLE(n3320, VipComplexProduct_VDOUBLE(mom->nu3[4], mom->nu3[2], p7));
  n3320 = VipComplexSum_VDOUBLE(n3320, VipComplexSq_VDOUBLE(mom->nu3[3], p8));
  nm3321 = VipComplexProduct_VDOUBLE(mom->nu3[0], mom->nu3[5], p4);
  nm3321 = VipComplexSum_VDOUBLE(nm3321, VipComplexProduct_VDOUBLE(mom->nu3[1], mom->nu3[4], p5));
  nm3321 = VipComplexSum_VDOUBLE(nm3321, VipComplexProduct_VDOUBLE(mom->nu3[2], mom->nu3[3], p6));
  nm3322 = VipComplexProduct_VDOUBLE(mom->nu3[0], mom->nu3[4], p1);
  nm3322 = VipComplexSum_VDOUBLE(nm3322, VipComplexProduct_VDOUBLE(mom->nu3[1], mom->nu3[3], p2));
  nm3322 = VipComplexSum_VDOUBLE(nm3322, VipComplexSq_VDOUBLE(mom->nu3[2], p3));
  m3333 = VipComplexProduct_VDOUBLE(n3322, nm3322, 2.0);
  m3333 = VipComplexSum_VDOUBLE(m3333, VipComplexProduct_VDOUBLE(n3321, nm3321, -2.0));
  m3333 = VipComplexSum_VDOUBLE(m3333, VipComplexSq_VDOUBLE(n3320, 1.0));
  q1 = p2/2.0;
  q2 = p8/2.0;
  q3 = sqrt(10.0/21.0);
  q4 = 1.0/sqrt(7.0);
  q5 = -p8*sqrt(2.0);
  n3122 = VipComplexProduct_VDOUBLE(mom->nu3[5], mom->nu1[1], q1); 
  n3122 = VipComplexSum_VDOUBLE(n3122, VipComplexProduct_VDOUBLE(mom->nu3[6], mom->nu1[0], -p5));
  n3122 = VipComplexSum_VDOUBLE(n3122, VipComplexProduct_VDOUBLE(mom->nu3[4], mom->nu1[2], q2));
  n3121 = VipComplexProduct_VDOUBLE(mom->nu3[5], mom->nu1[0], q3);
  n3121 = VipComplexSum_VDOUBLE(n3121, VipComplexProduct_VDOUBLE(mom->nu3[3], mom->nu1[2], q4));
  n3121 = VipComplexSum_VDOUBLE(n3121, VipComplexProduct_VDOUBLE(mom->nu3[4], mom->nu1[1], q5));
  n3120 = VipComplexProduct_VDOUBLE(mom->nu3[3], mom->nu1[1], p7);
  n3120 = VipComplexSum_VDOUBLE(n3120, VipComplexProduct_VDOUBLE(mom->nu3[4], mom->nu1[0], p3));
  n3120 = VipComplexSum_VDOUBLE(n3120, VipComplexProduct_VDOUBLE(mom->nu3[2], mom->nu1[2], p3));
  nm3121 = VipComplexProduct_VDOUBLE(mom->nu3[1], mom->nu1[2], q3);
  nm3121 = VipComplexSum_VDOUBLE(nm3121, VipComplexProduct_VDOUBLE(mom->nu3[3], mom->nu1[0], q4));
  nm3121 = VipComplexSum_VDOUBLE(nm3121, VipComplexProduct_VDOUBLE(mom->nu3[2], mom->nu1[1], q5));
  nm3122 = VipComplexProduct_VDOUBLE(mom->nu3[1], mom->nu1[1], q1);
  nm3122 = VipComplexSum_VDOUBLE(nm3122, VipComplexProduct_VDOUBLE(mom->nu3[0], mom->nu1[2], -p5));
  nm3122 = VipComplexSum_VDOUBLE(nm3122, VipComplexProduct_VDOUBLE(mom->nu3[2], mom->nu1[0], q2));
  m3131 = VipComplexProduct_VDOUBLE(n3122, nm3122, 2.0);
  m3131 = VipComplexSum_VDOUBLE(m3131, VipComplexProduct_VDOUBLE(n3121, nm3121, -2.0));
  m3131 = VipComplexSum_VDOUBLE(m3131, VipComplexSq_VDOUBLE(n3120, 1.0));
  r1 = sqrt(2.0);
  r2 = sqrt(2.0/3.0);
  n1122 = VipComplexSq_VDOUBLE(mom->nu1[2], 1.0);
  n1121 = VipComplexProduct_VDOUBLE(mom->nu1[1], mom->nu1[2], r1);
  n1120 = VipComplexProduct_VDOUBLE(mom->nu1[0], mom->nu1[2], r2);
  n1120 = VipComplexSum_VDOUBLE(n1120, VipComplexSq_VDOUBLE(mom->nu1[1], r2));
  nm1121 = VipComplexProduct_VDOUBLE(mom->nu1[1], mom->nu1[0], r1);
  nm1122 = VipComplexSq_VDOUBLE(mom->nu1[0], 1.0);
  m3111 = VipComplexProduct_VDOUBLE(n3122, nm1122, 1.0);
  m3111 = VipComplexSum_VDOUBLE(m3111, VipComplexProduct_VDOUBLE(n3121, nm1121, -1.0));
  m3111 = VipComplexSum_VDOUBLE(m3111, VipComplexProduct_VDOUBLE(n3120, n1120, 1.0));
  m3111 = VipComplexSum_VDOUBLE(m3111, VipComplexProduct_VDOUBLE(nm3121, n1121, -1.0));
  m3111 = VipComplexSum_VDOUBLE(m3111, VipComplexProduct_VDOUBLE(nm3122, n1122, 1.0));
  m3331 = VipComplexProduct_VDOUBLE(n3322, nm3122, 1.0);
  m3331 = VipComplexSum_VDOUBLE(m3331, VipComplexProduct_VDOUBLE(n3321, nm3121, -1.0));
  m3331 = VipComplexSum_VDOUBLE(m3331, VipComplexProduct_VDOUBLE(n3320, n3120, 1.0));
  m3331 = VipComplexSum_VDOUBLE(m3331, VipComplexProduct_VDOUBLE(nm3321, n3121, -1.0));
  m3331 = VipComplexSum_VDOUBLE(m3331, VipComplexProduct_VDOUBLE(nm3322, n3122, 1.0));
  m112 = VipComplexProduct_VDOUBLE(n1122, mom->nu2[0], 1.0);
  m112 = VipComplexSum_VDOUBLE(m112, VipComplexProduct_VDOUBLE(n1121, mom->nu2[1], -1.0));
  m112 = VipComplexSum_VDOUBLE(m112, VipComplexProduct_VDOUBLE(n1120, mom->nu2[2], 1.0));
  m112 = VipComplexSum_VDOUBLE(m112, VipComplexProduct_VDOUBLE(nm1121, mom->nu2[3], -1.0));
  m112 = VipComplexSum_VDOUBLE(m112, VipComplexProduct_VDOUBLE(nm1122, mom->nu2[4], 1.0));
  m123 = VipComplexProduct_VDOUBLE(n3122, mom->nu2[0], 1.0);
  m123 = VipComplexSum_VDOUBLE(m123, VipComplexProduct_VDOUBLE(n3121, mom->nu2[1], -1.0));
  m123 = VipComplexSum_VDOUBLE(m123, VipComplexProduct_VDOUBLE(n3120, mom->nu2[2], 1.0));
  m123 = VipComplexSum_VDOUBLE(m123, VipComplexProduct_VDOUBLE(nm3121, mom->nu2[3], -1.0));
  m123 = VipComplexSum_VDOUBLE(m123, VipComplexProduct_VDOUBLE(nm3122, mom->nu2[4], 1.0));
  m233 = VipComplexProduct_VDOUBLE(n3322, mom->nu2[0], 1.0);
  m233 = VipComplexSum_VDOUBLE(m233, VipComplexProduct_VDOUBLE(n3321, mom->nu2[1], -1.0));
  m233 = VipComplexSum_VDOUBLE(m233, VipComplexProduct_VDOUBLE(n3320, mom->nu2[2], 1.0));
  m233 = VipComplexSum_VDOUBLE(m233, VipComplexProduct_VDOUBLE(nm3321, mom->nu2[3], -1.0));
  m233 = VipComplexSum_VDOUBLE(m233, VipComplexProduct_VDOUBLE(nm3322, mom->nu2[4], 1.0));

  mom->I[5] = mVipRootN(ce*m3111.re/d6, 12.0);
  mom->I[6] = mVipRootN(ce*m3131.re/d6, 12.0);
  mom->I[7] = mVipRootN(ce*m3331.re/d6, 12.0);
  mom->I[8] = mVipRootN(ce*m3333.re/d6, 12.0);
  mom->I[9] = mVipRootN(ce*m112.re/d5, 8.0);
  mom->I[10] = mVipRootN(ce*m123.re/d5, 8.0);
  mom->I[11] = mVipRootN(ce*m233.re/d5, 8.0);

  /***** rempli la matrice d'inertie *****/
  I2ptr = VipGetMatrixPtr_VDOUBLE(mom->I2);
  I2ptr[0][0] = mom->mu2[0];
  I2ptr[1][1] = mom->mu2[1];
  I2ptr[2][2] = mom->mu2[2];
  I2ptr[0][1] = I2ptr[1][0] = mom->mu2[3];
  I2ptr[0][2] = I2ptr[2][0] = mom->mu2[4];
  I2ptr[1][2] = I2ptr[2][1] = mom->mu2[5];

  /***** calcule ses valeurs et vecteurs propres *****/
  VipTRED2_VDOUBLE(mom->I2, mom->Lambda, eigen_temp);
  VipTQLI_VDOUBLE(mom->Lambda, eigen_temp, mom->I2);

  VipFreeVector_VDOUBLE(eigen_temp);

  return(OK);
}

int VipComputeInvariant2D(Moment2D *mom)
{ double m20, m02, m11, m30, m03, m21, m12, m20mm02, m0;
  double m30mm12, m21mm03, m30pm12, m21pm03, m30pm122, m21pm032;
  double d1, d2, d3, d4, d5;
  VipVector_VDOUBLE *eigen_temp;
  Vip_DOUBLE **I2ptr;

  eigen_temp = VipAllocationVector_VDOUBLE(2, NULL);
  if (!eigen_temp)
  { VipPrintfError("Error while allocating temporary eigenvector vector");
    VipPrintfExit("(Vip_moment.c)VipComputeInvariant2D");
    return(PB);
  }

  m0 = mom->m0;
  d1 = mVipSq(m0);
  d2 = mVipSq(d1);
  d3 = m0*d2;
  d4 = d3*d1;
  d5 = mVipSq(d3);

  m20 = mom->mu2[0], m02 = mom->mu2[1], m11 = mom->mu2[2];
  m30 = mom->mu3[0], m03 = mom->mu3[1], m21 = mom->mu3[2], m12 = mom->mu3[3];
  m20mm02 = m20-m02, m30mm12 = m30-3.0*m12, m21mm03 = 3.0*m21-m03;
  m30pm12 = m30+m12, m21pm03 = m21+m03;
  m30pm122 = mVipSq(m30pm12), m21pm032 = mVipSq(m21pm03);

  mom->I[0] = sqrt((m20+m02)/d1);
  mom->I[1] = mVipRootN((mVipSq(m20mm02)+4.0*mVipSq(m11))/d2, 4.0);
  mom->I[2] = mVipRootN((mVipSq(m30mm12)+mVipSq(m21mm03))/d3, 6.0);
  mom->I[3] = mVipRootN((m30pm122+m21pm032)/d3, 6.0);
  mom->I[4] = mVipRootN((m30mm12*m30pm12*(m30pm122-3.0*m21pm032)+
                         m21mm03*m21pm03*(3.0*m30pm122-m21pm032))/d5, 12.0);
  mom->I[5] = mVipRootN((m20mm02*(m30pm122-m21pm032)+4.0*m11*m30pm12*m21pm03)/d4, 8.0);
  mom->I[6] = mVipRootN((m21mm03*m30pm12*(m30pm122-3.0*m21pm032)-
                         m30mm12*m21pm03*(3.0*m30pm122-m21pm032))/d5, 12.0);

  /***** rempli la matrice d'inertie *****/
  I2ptr = VipGetMatrixPtr_VDOUBLE(mom->I2);
  I2ptr[0][0] = mom->mu2[0];
  I2ptr[1][1] = mom->mu2[1];
  I2ptr[0][1] = I2ptr[1][0] = mom->mu2[2];

  /***** calcule ses valeurs et vecteurs propres *****/
  VipTRED2_VDOUBLE(mom->I2, mom->Lambda, eigen_temp);
  VipTQLI_VDOUBLE(mom->Lambda, eigen_temp, mom->I2);

  VipFreeVector_VDOUBLE(eigen_temp);

  return(OK);
}

void VipGetComplexMoment(Moment *mom)
{ double c00, c11, c10, c22, c20, c33, c32, c31, c30;

  c00 = 2.0*sqrt(M_PI)/3.0;
  c11 = sqrt(6.0*M_PI)/5.0;
  c10 = 2.0*sqrt(3.0*M_PI)/5.0;
  c22 = sqrt(2.0*M_PI/15.0);
  c20 = 2.0*sqrt(M_PI/5.0)/3.0;
  c33 = sqrt(M_PI/35.0);
  c32 = sqrt(6.0*M_PI/35.0);
  c31 = sqrt(3.0*M_PI/7.0)/5.0;
  c30 = 2.0*sqrt(M_PI/7.0)/5.0;

  mom->nu0.re = c00*(mom->mu2[0]+mom->mu2[1]+mom->mu2[2]);
  mom->nu1[0].re = c11*(mom->mu3[0]+mom->mu3[5]+mom->mu3[7]);
  mom->nu1[0].im = -c11*(mom->mu3[1]+mom->mu3[3]+mom->mu3[8]);
  mom->nu1[1].re = c10*(mom->mu3[2]+mom->mu3[4]+mom->mu3[6]);
  mom->nu1[2].re = -mom->nu1[0].re;
  mom->nu1[2].im = mom->nu1[0].im;  
  mom->nu2[0].re = c22*(mom->mu2[0]-mom->mu2[1]);
  mom->nu2[0].im = -2.0*c22*mom->mu2[3];
  mom->nu2[1].re = 2.0*c22*mom->mu2[4];
  mom->nu2[1].im = -2.0*c22*mom->mu2[5];
  mom->nu2[2].re = c20*(2.0*mom->mu2[2]-mom->mu2[0]-mom->mu2[1]);
  mom->nu2[3].re = -mom->nu2[1].re;
  mom->nu2[3].im = mom->nu2[1].im;
  mom->nu2[4].re = mom->nu2[0].re;
  mom->nu2[4].im = -mom->nu2[0].im; 
  mom->nu3[0].re = c33*(mom->mu3[0]-3.0*mom->mu3[5]);
  mom->nu3[0].im = c33*(mom->mu3[1]-3.0*mom->mu3[3]);
  mom->nu3[1].re = c32*(mom->mu3[4]-mom->mu3[6]);
  mom->nu3[1].im = -2.0*c32*mom->mu3[9];
  mom->nu3[2].re = -c31*(mom->mu3[0]+mom->mu3[5]-4.0*mom->mu3[7]);
  mom->nu3[2].im = c31*(mom->mu3[1]+mom->mu3[3]-4.0*mom->mu3[8]);
  mom->nu3[3].re = c30*(2.0*mom->mu3[2]-3.0*(mom->mu3[4]+mom->mu3[6]));
  mom->nu3[4].re = -mom->nu3[2].re;
  mom->nu3[4].im = mom->nu3[2].im;
  mom->nu3[5].re = mom->nu3[1].re;
  mom->nu3[5].im = -mom->nu3[1].im;
  mom->nu3[6].re = -mom->nu3[0].re;
  mom->nu3[6].im = mom->nu3[0].im; 
}

int VipComputeMomentInvariants(Moment *mom)
{ VipGetComplexMoment(mom);
  if (VipComputeInvariant(mom) == PB)  return(PB);
  
  return(OK);
}

void VipComputeMoment(Volume *vol, Vip_U16BIT label, Moment *mom, int dir)
{ double  x, y, z;
  int xsize, ysize, zsize;
  Vip_S16BIT *imptr;
  VipOffsetStruct *vos;

  vos = VipGetOffsetStructure(vol);
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  for (z=0; z<zsize; z++, imptr+=vos->oLineBetweenSlice)
    for (y=0; y<ysize; y++, imptr+=vos->oPointBetweenLine)
      for (x=0; x<xsize; x++)
        if (*imptr++ == label)  VipAddOrSubPointToMoment(mom, x, y, z, dir);
}

void VipComputeMomentM(Volume *vol, Vip_U16BIT label, Moment *mom, int dir,
		       int mask)
{ double  x, y, z;
  int xsize, ysize, zsize;
  Vip_S16BIT *imptr;
  VipOffsetStruct *vos;

  vos = VipGetOffsetStructure(vol);
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  for (z=0; z<zsize; z++, imptr+=vos->oLineBetweenSlice)
    for (y=0; y<ysize; y++, imptr+=vos->oPointBetweenLine)
      for (x=0; x<xsize; x++, imptr++)
        if (((*imptr) & (Vip_S16BIT)mask) == label)  VipAddOrSubPointToMoment(mom, x, y, z, dir);
}

Moment *VipCreateAndComputeMoment(Volume *vol, Vip_U16BIT label, int dir)
{ Moment *moment;

  moment = VipCreateMoment(vol);
  if (!moment)
  { VipPrintfError("Error while allocating Moment structure");
    VipPrintfExit("(Vip_moment.c)VipCreateAndComputeMoment");
    return((Moment *)NULL);
  }

  VipComputeMoment(vol, label, moment, dir);

  return(moment);
}

void VipComputeMoment2D(Volume *vol, Vip_U16BIT label, Moment2D *mom, int dir)
{ double x, y;
  int xsize, ysize;
  Vip_S16BIT *imptr;
  VipOffsetStruct *vos;

  vos = VipGetOffsetStructure(vol);
  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  for (y=0; y<ysize; y++, imptr+=vos->oPointBetweenLine)
    for (x=0; x<xsize; x++)
      if (*imptr++ == label)  VipAddOrSubPointToMoment2D(mom, x, y, dir);
}

Moment2D *VipCreateAndComputeMoment2D(Volume *vol, Vip_U16BIT label, int dir)
{ Moment2D *moment;

  moment = VipCreateMoment2D(vol);
  if (!moment)
  { VipPrintfError("Error while allocating Moment structure");
    VipPrintfExit("(Vip_moment.c)VipCreateAndComputeMoment2D");
    return((Moment2D *)NULL);
  }

  VipComputeMoment2D(vol, label, moment, dir);

  return(moment);
}

int VipComputeMomentAndInvariant(Volume *vol, Vip_U16BIT label, Moment *mom, int dir)
{ VipComputeMoment(vol, label, mom, dir);
  if (VipComputeMomentInvariants(mom) == PB)  return(PB);

  return(OK);
}

int VipComputeMomentAndInvariantM(Volume *vol, Vip_U16BIT label, Moment *mom, int dir, int mask)
{ VipComputeMomentM(vol, label, mom, dir, mask);
  if (VipComputeMomentInvariants(mom) == PB)  return(PB);

  return(OK);
}

Moment *VipCreateAndComputeMomentAndInvariant(Volume *vol, Vip_U16BIT label, int dir)
{ Moment *moment;

  moment = VipCreateMoment(vol);
  if (!moment)
  { VipPrintfError("Error while allocating Moment structure");
    VipPrintfExit("(Vip_moment.c)VipCreateAndComputeMomentAndInvariant");
    return((Moment *)NULL);
  }

  if (VipComputeMomentAndInvariant(vol, label, moment, dir) == PB)  
    return((Moment *)NULL);

  return(moment);
}

Moment *VipCreateAndComputeMomentAndInvariantM(Volume *vol, Vip_U16BIT label, int dir, int mask)
{ Moment *moment;

  moment = VipCreateMoment(vol);
  if (!moment)
  { VipPrintfError("Error while allocating Moment structure");
    VipPrintfExit("(Vip_moment.c)VipCreateAndComputeMomentAndInvariant");
    return((Moment *)NULL);
  }

  if (VipComputeMomentAndInvariantM(vol, label, moment, dir, mask) == PB)  
    return((Moment *)NULL);

  return(moment);
}

int VipComputeMomentAndInvariant2D(Volume *vol, Vip_U16BIT label, Moment2D *mom, int dir)
{ VipComputeMoment2D(vol, label, mom, dir);
  if (VipComputeInvariant2D(mom) == PB)  return(PB);

  return(OK);
}

Moment2D *VipCreateAndComputeMomentAndInvariant2D(Volume *vol, Vip_U16BIT label, int dir)
{ Moment2D *moment;

  moment = VipCreateMoment2D(vol);
  if (!moment)
  { VipPrintfError("Error while allocating Moment2D structure");
    VipPrintfExit("(Vip_moment.c)VipCreateAndComputeMomentInvariant2D");
    return((Moment2D *)NULL);
  }

  if (VipComputeMomentAndInvariant2D(vol, label, moment, dir) == PB)
    return((Moment2D *)NULL);

  return(moment);
}

int VipUpdateMomentAndInvariant(Moment *mom, int x, int y, int z, int dir)
{ VipAddOrSubPointToMoment(mom, (double)x, (double)y, (double)z, dir);
  if (VipComputeMomentInvariants(mom) == PB)  return(PB);

  return(OK);
}

int VipUpdateMomentAndInvariant2D(Moment2D *mom, int x, int y, int dir)
{ VipAddOrSubPointToMoment2D(mom, (double)x, (double)y, dir);
  if (VipComputeInvariant2D(mom) == PB)  return(PB);

  return(OK);
}

/***** Fonction de calcul des moments de Legendre *****/
int VipComputeLegendreMoment(Legendre *leg)
{ double c0, c1, c2, c11, c3, c21, c111;
  int i;

  c0 = 1.0/8.0; 
  c1 = 3.0*c0;
  c2 = 5.0*c0;
  c11 = 9.0*c0;
  c3 = 35.0/16.0;
  c21 = 45.0/16.0;
  c111 = 27.0*c0;

  leg->l0 = c0*leg->moment->m0;
  for (i=0; i<3; i++)  
  { leg->l1[i] = c1*leg->moment->m1[i];
    leg->lu2[i] = c2*(1.5*leg->moment->mu2[i]-0.5*leg->moment->m0);
    leg->lu2[i+3] = c11*leg->moment->mu2[i+3];
    leg->lu3[i] = c3*leg->moment->mu3[i];
    leg->lu3[i+3] = c21*leg->moment->mu3[i+3];
    leg->lu3[i+6] = c21*leg->moment->mu3[i+6];
  }
  leg->lu3[9] = c111*leg->moment->mu3[9];
  leg->lx = leg->l1[0]/leg->l0;
  leg->ly = leg->l1[1]/leg->l0;
  leg->lz = leg->l1[2]/leg->l0;

  return(OK);
}

int VipComputeLegendreMoment2D(Legendre2D *leg)
{ double c0, c1, c2, c11, c3, c21;
  int i;

  c0 = 1.0/4.0;
  c1 = 3.0*c0;
  c2 = 5.0*c0;
  c11 = 9.0*c0;
  c3 = 35.0/8.0;
  c21 = 30.0/8.0;

  leg->l0 = c0*leg->moment->m0;
  leg->lu2[2] = c11*leg->moment->mu2[2];
  for (i=0; i<2; i++)
  { leg->l1[i] = c1*leg->moment->m1[i];
    leg->lu2[i] = c2*(1.5*leg->moment->mu2[i]-0.5*leg->moment->m0);
    leg->lu3[i] = c3*leg->moment->mu3[i];
    leg->lu3[i+2] = c21*leg->moment->mu3[i+2];
  }
  leg->lx = leg->l1[0]/leg->l0;
  leg->ly = leg->l1[1]/leg->l0;

  return(OK);
}

int VipComputeMomentAndLegendreMoment(Volume *vol, Vip_U16BIT label, Legendre *leg, int dir)
{ int i;

  VipComputeMoment(vol, label, leg->moment, dir);
  if (VipComputeLegendreMoment(leg) == PB)  return(PB);

  leg->moment->m0 = leg->l0;
  for (i=0; i<3; i++)
  { leg->moment->m1[i] = leg->l1[i];
    leg->moment->mu2[i] = leg->lu2[i];
    leg->moment->mu2[i+3] = leg->lu2[i+3];
    leg->moment->mu3[i] = leg->lu3[i]; 
    leg->moment->mu3[i+3] = leg->lu3[i+3]; 
    leg->moment->mu3[i+6] = leg->lu3[i+6]; 
  }
  leg->moment->mu3[9] = leg->lu3[9];
  leg->moment->mx = leg->lx;
  leg->moment->my = leg->ly;
  leg->moment->mz = leg->lz;

  if (VipComputeMomentInvariants(leg->moment) == PB)  return(PB);

  return(OK);
}

Legendre *VipCreateAndComputeMomentAndLegendreMoment(Volume *vol, Vip_U16BIT label, int dir)
{ Legendre *legendre;

  legendre = VipCreateLegendreMoment(vol);
  if (!legendre)
  { VipPrintfError("Error while allocating Legendre structure");
    VipPrintfExit("(Vip_moment.c)VipCreateAndComputeMomentAndLegendreMoment");
    return((Legendre *)NULL);
  }

  if (VipComputeMomentAndLegendreMoment(vol, label, legendre, dir) == PB)
    return((Legendre *)NULL);

  return(legendre);
}

int VipComputeMomentAndLegendreMoment2D(Volume *vol, Vip_U16BIT label, Legendre2D *leg, int dir)
{ int i;

  VipComputeMoment2D(vol, label, leg->moment, dir);
  if (VipComputeLegendreMoment2D(leg) == PB)  return(PB);

  leg->moment->m0 = leg->l0;
  leg->moment->mu2[2] = leg->lu2[2];
  for (i=0; i<2; i++)
  { leg->moment->m1[i] = leg->l1[i];
    leg->moment->mu2[i] = leg->lu2[i];
    leg->moment->mu3[i] = leg->lu3[i];
    leg->moment->mu3[i+2] = leg->lu3[i+2];
  }
  leg->moment->mx = leg->lx;
  leg->moment->my = leg->ly;

  if (VipComputeInvariant2D(leg->moment) == PB)  return(PB);

  return(OK);
}

Legendre2D *VipCreateAndComputeMomentAndLegendreMoment2D(Volume *vol, Vip_U16BIT label, int dir)
{ Legendre2D *legendre;

  legendre = VipCreateLegendreMoment2D(vol);
  if (!legendre)
  { VipPrintfError("Error while allocating Legendre structure");
    VipPrintfExit("(Vip_moment.c)VipCreateAndComputeMomentAndLegendreMoment2D");
    return((Legendre2D *)NULL);
  }

  if (VipComputeMomentAndLegendreMoment2D(vol, label, legendre, dir) == PB)
    return((Legendre2D *)NULL);

  return(legendre);
}

void VipPrintStreamMoment(FILE *stream, Moment *mom, char *filename)
{ fprintf(stream, "\nMoments of : %s.vimg\n\n", filename);
  fprintf(stream, "Correction : %f\n", mom->ct);
  fprintf(stream, "correction X : %f\n", mom->cx);
  fprintf(stream, "correction Y : %f\n", mom->cy);
  fprintf(stream, "correction Z : %f\n\n", mom->cz);
  fprintf(stream, "m0\t%f\n", mom->m0);
  fprintf(stream, "m1x\t%f\n", mom->m1[0]);
  fprintf(stream, "m1y\t%f\n", mom->m1[1]);
  fprintf(stream, "m1z\t%f\n", mom->m1[2]);
  fprintf(stream, "mx\t%f\n", mom->mx);
  fprintf(stream, "my\t%f\n", mom->my);
  fprintf(stream, "mz\t%f\n", mom->mz);
  fprintf(stream, "nu2x\t%f\n", mom->mu2[0]);
  fprintf(stream, "nu2y\t%f\n", mom->mu2[1]);
  fprintf(stream, "nu2z\t%f\n", mom->mu2[2]);
  fprintf(stream, "nu2xy\t%f\n", mom->mu2[3]);
  fprintf(stream, "nu2xz\t%f\n", mom->mu2[4]);
  fprintf(stream, "nu2yz\t%f\n", mom->mu2[5]);
  fprintf(stream, "nu3x\t%f\n", mom->mu3[0]);
  fprintf(stream, "nu3y\t%f\n", mom->mu3[1]);
  fprintf(stream, "nu3z\t%f\n", mom->mu3[2]);
  fprintf(stream, "nu3x2y\t%f\n", mom->mu3[3]);
  fprintf(stream, "nu3x2z\t%f\n", mom->mu3[4]);
  fprintf(stream, "nu3xy2\t%f\n", mom->mu3[5]);
  fprintf(stream, "nu3y2z\t%f\n", mom->mu3[6]);
  fprintf(stream, "nu3xz2\t%f\n", mom->mu3[7]);
  fprintf(stream, "nu3yz2\t%f\n", mom->mu3[8]);
  fprintf(stream, "nu3xyz\t%f\n", mom->mu3[9]);
  fprintf(stream, "I^2_00\t\t%f\n", mom->I[0]);
  fprintf(stream, "I^2_22\t\t%f\n", mom->I[1]);
  fprintf(stream, "I^2_222\t\t%f\n", mom->I[2]);
  fprintf(stream, "I^3_11\t\t%f\n", mom->I[3]);
  fprintf(stream, "I^3_33\t\t%f\n", mom->I[4]);
  fprintf(stream, "I^3_1113\t%f\n", mom->I[5]);
  fprintf(stream, "I^3_1133\t%f\n", mom->I[6]);
  fprintf(stream, "I^3_1333\t%f\n", mom->I[7]);
  fprintf(stream, "I^3_3333\t%f\n", mom->I[8]);
  fprintf(stream, "I^2,3_112\t%f\n", mom->I[9]);
  fprintf(stream, "I^2,3_123\t%f\n", mom->I[10]);
  fprintf(stream, "I^2,3_233\t%f\n", mom->I[11]);
  fprintf(stream, "Lambda1\t%f\n", mom->Lambda->data[0]);
  fprintf(stream, "Lambda2\t%f\n", mom->Lambda->data[1]);
  fprintf(stream, "Lambda3\t%f\n", mom->Lambda->data[2]);
  fprintf(stream, "Ux\t%f\n", mom->I2->data[0][0]);
  fprintf(stream, "Uy\t%f\n", mom->I2->data[1][0]);
  fprintf(stream, "Uz\t%f\n", mom->I2->data[2][0]);
  fprintf(stream, "Vx\t%f\n", mom->I2->data[0][1]);
  fprintf(stream, "Vy\t%f\n", mom->I2->data[1][1]);
  fprintf(stream, "Vz\t%f\n", mom->I2->data[2][1]);
  fprintf(stream, "Wx\t%f\n", mom->I2->data[0][2]);
  fprintf(stream, "Wy\t%f\n", mom->I2->data[1][2]);
  fprintf(stream, "Wz\t%f\n", mom->I2->data[2][2]);
}

void VipPrintStreamLegendreMoment(FILE *stream, Legendre *leg, char *filename)
{ fprintf(stream, "\nLegendre moments of : %s.vimg\n\n", filename);
  fprintf(stream, "l0\t%f\n", leg->l0);
  fprintf(stream, "l1x\t%f\n", leg->l1[0]);
  fprintf(stream, "l1y\t%f\n", leg->l1[1]);
  fprintf(stream, "l1z\t%f\n", leg->l1[2]);
  fprintf(stream, "lx\t%f\n", leg->lx);
  fprintf(stream, "ly\t%f\n", leg->ly);
  fprintf(stream, "lz\t%f\n", leg->lz);
  fprintf(stream, "lu2x2\t%f\n", leg->lu2[0]);
  fprintf(stream, "lu2y2\t%f\n", leg->lu2[1]);
  fprintf(stream, "lu2z2\t%f\n", leg->lu2[2]);
  fprintf(stream, "lu2xy\t%f\n", leg->lu2[3]);
  fprintf(stream, "lu2xz\t%f\n", leg->lu2[4]);
  fprintf(stream, "lu2yz\t%f\n", leg->lu2[5]);
  fprintf(stream, "lu3x3\t%f\n", leg->lu3[0]);
  fprintf(stream, "lu3y3\t%f\n", leg->lu3[1]);
  fprintf(stream, "lu3z3\t%f\n", leg->lu3[2]);
  fprintf(stream, "lu3x2y\t%f\n", leg->lu3[3]);
  fprintf(stream, "lu3x2z\t%f\n", leg->lu3[4]);
  fprintf(stream, "lu3xy2\t%f\n", leg->lu3[5]);
  fprintf(stream, "lu3y2z\t%f\n", leg->lu3[6]);
  fprintf(stream, "lu3xz2\t%f\n", leg->lu3[7]);
  fprintf(stream, "lu3yz2\t%f\n", leg->lu3[8]);
  fprintf(stream, "lu3xyz\t%f\n", leg->lu3[9]);
}

void VipPrintStreamInvariantForCovarianceMatrix(FILE *stream, Moment *mom)
{ fprintf(stream, "%f ", mom->I[0]);
  fprintf(stream, "%f ", mom->I[1]);
  fprintf(stream, "%f ", mom->I[2]);
  fprintf(stream, "%f ", mom->I[3]);
  fprintf(stream, "%f ", mom->I[4]);
  fprintf(stream, "%f ", mom->I[5]);
  fprintf(stream, "%f ", mom->I[6]);
  fprintf(stream, "%f ", mom->I[7]);
  fprintf(stream, "%f ", mom->I[8]);
  fprintf(stream, "%f ", mom->I[9]);
  fprintf(stream, "%f ", mom->I[10]);
  fprintf(stream, "%f\n", mom->I[11]);
}

void VipPrintStreamInvariantForPCA(FILE *stream, Moment *mom)
{ fprintf(stream, "%f ", mom->m0);
  fprintf(stream, "%f ", mom->mx);
  fprintf(stream, "%f ", mom->my);
  fprintf(stream, "%f ", mom->mz);
  fprintf(stream, "%f ", mom->I2->data[0][0]);
  fprintf(stream, "%f ", mom->I2->data[1][0]);
  fprintf(stream, "%f ", mom->I2->data[2][0]);
  fprintf(stream, "%f ", mom->I2->data[0][1]);
  fprintf(stream, "%f ", mom->I2->data[1][1]);
  fprintf(stream, "%f ", mom->I2->data[2][1]);
  fprintf(stream, "%f ", mom->I2->data[0][2]);
  fprintf(stream, "%f ", mom->I2->data[1][2]);
  fprintf(stream, "%f ", mom->I2->data[2][2]);
  VipPrintStreamInvariantForCovarianceMatrix(stream, mom);
}

int VipPrintMoment(Moment *mom, char *filename, char *resultfile)
{ FILE *file;

  if (resultfile == NULL)  VipPrintStreamMoment(stdout, mom, filename);
  else 
  { file = fopen(resultfile, VIP_WRITE_TEXT);
    if (file == NULL)
    { VipPrintfError("Error, can't open moment result file.");
      VipPrintfExit("(Vip_moment.c)VipPrintMoment");
      return(PB);
    }
    if (strstr(resultfile, ".pca") != NULL)  VipPrintStreamInvariantForPCA(file, mom);
    else if (strstr(resultfile, ".inv") != NULL)  
      VipPrintStreamInvariantForCovarianceMatrix(file, mom);
    else VipPrintStreamMoment(file, mom, filename);
    fclose(file);
  }

  return(OK);
}

int VipPrintLegendreMoment(Legendre *leg, char *filename, char *resultfile)
{ FILE *file;
  char temp[256];

  if (resultfile == NULL)  VipPrintStreamLegendreMoment(stdout, leg, filename);
  else if (strstr(resultfile, ".inv") != NULL)
  { file = fopen(resultfile, VIP_WRITE_TEXT);
    if (!file)
    { VipPrintfError("Error, can't open Invariant moment result file.");
      VipPrintfExit("(Vip_moment.c)VipPrintLegendreMoment");
      return(PB);
    }
    VipPrintStreamInvariantForCovarianceMatrix(file, leg->moment);
    fclose(file);
  }
  else 
  { strcpy(temp, resultfile);
    file = fopen("legendre.file", VIP_WRITE_TEXT);
    if (file == NULL)
    { VipPrintfError("Error, can't open Legendre moment result file.");
      VipPrintfExit("(Vip_moment.c)VipPrintLegendreMoment");
      return(PB);
    }
    if (!strstr(resultfile, ".pca"))
    { VipPrintStreamLegendreMoment(file, leg, filename);
      fclose(file);
      VipFileCat( "legendre.file", resultfile );
      VipUnlink("legendre.file");
    }
    else
    { VipPrintStreamInvariantForPCA(file, leg->moment);
      fclose(file);
      if ((file = fopen(resultfile, VIP_READ_TEXT)) != NULL)
      {
        fclose(file);
        VipUnlink( resultfile );
      }
      VipRename( "legendre.file", resultfile );
    }
  }

  return(OK);
}

void VipPrintStreamMoment2D(FILE *stream, Moment2D *mom, char *filename)
{ fprintf(stream, "\n2D Moments of : %s.vimg\n\n", filename);
  fprintf(stream, "Correction : %f\n", mom->ct);
  fprintf(stream, "correction X : %f\n", mom->cx);
  fprintf(stream, "correction Y : %f\n", mom->cy);
  fprintf(stream, "m0\t%f\n", mom->m0);
  fprintf(stream, "m1x\t%f\n", mom->m1[0]);
  fprintf(stream, "m1y\t%f\n", mom->m1[1]);
  fprintf(stream, "mx\t%f\n", mom->mx);
  fprintf(stream, "my\t%f\n", mom->my);
  fprintf(stream, "nu2x\t%f\n", mom->mu2[0]);
  fprintf(stream, "nu2y\t%f\n", mom->mu2[1]);
  fprintf(stream, "nu2xy\t%f\n", mom->mu2[2]);
  fprintf(stream, "nu3x\t%f\n", mom->mu3[0]);
  fprintf(stream, "nu3y\t%f\n", mom->mu3[1]);
  fprintf(stream, "nu3x2y\t%f\n", mom->mu3[2]);
  fprintf(stream, "nu3xy2\t%f\n", mom->mu3[3]);
  fprintf(stream, "I0\t\t%f\n", mom->I[0]);
  fprintf(stream, "I1\t\t%f\n", mom->I[1]);
  fprintf(stream, "I2\t\t%f\n", mom->I[2]);
  fprintf(stream, "I3\t\t%f\n", mom->I[3]);
  fprintf(stream, "I4\t\t%f\n", mom->I[4]);
  fprintf(stream, "I5\t%f\n", mom->I[5]);
  fprintf(stream, "I6\t%f\n", mom->I[6]);
  fprintf(stream, "Lambda1\t%f\n", mom->Lambda->data[0]);
  fprintf(stream, "Lambda2\t%f\n", mom->Lambda->data[1]);
  fprintf(stream, "Ux\t%f\n", mom->I2->data[0][0]);
  fprintf(stream, "Uy\t%f\n", mom->I2->data[1][0]);
  fprintf(stream, "Vx\t%f\n", mom->I2->data[0][1]);
  fprintf(stream, "Vy\t%f\n", mom->I2->data[1][1]);
}

void VipPrintStreamLegendreMoment2D(FILE *stream, Legendre2D *leg, char *filename)
{ fprintf(stream, "\n2D Legendre moments of : %s.vimg\n\n", filename);
  fprintf(stream, "l0\t%f\n", leg->l0);
  fprintf(stream, "l1x\t%f\n", leg->l1[0]);
  fprintf(stream, "l1y\t%f\n", leg->l1[1]);
  fprintf(stream, "lx\t%f\n", leg->lx);
  fprintf(stream, "ly\t%f\n", leg->ly);
  fprintf(stream, "lu2x\t%f\n", leg->lu2[0]);
  fprintf(stream, "lu2y\t%f\n", leg->lu2[1]);
  fprintf(stream, "lu2xy\t%f\n", leg->lu2[2]);
  fprintf(stream, "lu3x\t%f\n", leg->lu3[0]);
  fprintf(stream, "lu3y\t%f\n", leg->lu3[1]);
  fprintf(stream, "lu3x2y\t%f\n", leg->lu3[2]);
  fprintf(stream, "lu3xy2\t%f\n", leg->lu3[3]);
}

void VipPrintStreamInvariantForCovarianceMatrix2D(FILE *stream, Moment2D *mom)
{ fprintf(stream, "%f ", mom->I[0]);
  fprintf(stream, "%f ", mom->I[1]);
  fprintf(stream, "%f ", mom->I[2]);
  fprintf(stream, "%f ", mom->I[3]);
  fprintf(stream, "%f ", mom->I[4]);
  fprintf(stream, "%f ", mom->I[5]);
  fprintf(stream, "%f\n", mom->I[6]);
}

void VipPrintStreamInvariantForPCA2D(FILE *stream, Moment2D *mom)
{ fprintf(stream, "%f ", mom->m0);
  fprintf(stream, "%f ", mom->mx);
  fprintf(stream, "%f ", mom->my);
  fprintf(stream, "%f ", mom->I2->data[0][0]);
  fprintf(stream, "%f ", mom->I2->data[1][0]);
  fprintf(stream, "%f ", mom->I2->data[0][1]);
  fprintf(stream, "%f ", mom->I2->data[1][1]);
  VipPrintStreamInvariantForCovarianceMatrix2D(stream, mom);
}

int VipPrintMoment2D(Moment2D *mom, char *filename, char *resultfile)
{ FILE *file;

  if (resultfile == NULL)  VipPrintStreamMoment2D(stdout, mom, filename);
  else 
  { file = fopen(resultfile, VIP_WRITE_TEXT);
    if (file == NULL)
    { VipPrintfError("Error, can't open moment result file.");
      VipPrintfExit("(Vip_moment.c)VipPrintMoment2D");
      return(PB);
    }
    if (strstr(resultfile, ".pca") != NULL)  VipPrintStreamInvariantForPCA2D(file, mom);
    else if (strstr(resultfile, ".inv") != NULL)  
      VipPrintStreamInvariantForCovarianceMatrix2D(file, mom);
    else VipPrintStreamMoment2D(file, mom, filename);
    fclose(file);
  }

  return(OK);
}

int VipPrintLegendreMoment2D(Legendre2D *leg, char *filename, char *resultfile)
{ FILE *file;
  char temp[256];

  if (resultfile == NULL)  VipPrintStreamLegendreMoment2D(stdout, leg, filename);
  else if (strstr(resultfile, ".inv") != NULL)
  { file = fopen(resultfile, VIP_WRITE_TEXT);
    if (!file)
    { VipPrintfError("Error, can't open Invariant moment result file.");
      VipPrintfExit("(Vip_moment.c)VipPrintLegendreMoment2D");
      return(PB);
    }
    VipPrintStreamInvariantForCovarianceMatrix2D(file, leg->moment);
    fclose(file);
  }
  else 
  { strcpy(temp, resultfile);
    file = fopen("legendre2D.file", VIP_WRITE_TEXT);
    if (file == NULL)
    { VipPrintfError("Error, can't open Legendre moment result file.");
      VipPrintfExit("(Vip_moment.c)VipPrintLegendreMoment2D");
      return(PB);
    }
    if (!strstr(resultfile, ".pca"))
    { VipPrintStreamLegendreMoment2D(file, leg, filename);
      fclose(file);
      VipFileCat( "legendre2D.file", resultfile );
      VipUnlink("legendre2D.file");
    }
    else
    { VipPrintStreamInvariantForPCA2D(file, leg->moment);
      fclose(file);
      if ((file = fopen(resultfile, VIP_READ_TEXT)) != NULL)
      { fclose(file);
        VipUnlink( resultfile );
      }
      VipRename( "legendre2D.file", resultfile );
    }
  }

  return(OK);
}
