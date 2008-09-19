/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_vfilter.c        * TYPE     : Source
 * AUTHOR      : POUPON F.            * CREATION : 30/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Application d'un V-Filtre sur un volume
 *               fpoupon@cea.fr
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
#include <math.h>

#include <vip/vfilter.h>

#include <vip/connex.h>
#include <vip/util.h>

Volume *VipVFilter(Volume *vol, int size)
{ 
  VIP_DEC_VOLUME(volout);
  VIP_DEC_VOLUME(table);
  int i, j, k, size2, tm1, x, y, z, xf, yf, zf, loc;
  int volslice, volline, tabslice;
  double somme, carre, somme1, carre1, vars, *pvar, moyenne;
  VipOffsetStruct *vos, *tos;
  Vip_S16BIT *imptr, *outptr, temp;
  Vip_DOUBLE *tableptr;

  if ((volout = VipDuplicateVolumeStructure(vol, "")) == NULL)
    { 
      VipPrintfError("volume structure duplication error");
      VipPrintfExit("(vfilter)VipVFilter");
      return((Volume *)NULL);
    }
  if ((VipAllocateVolumeData(volout)) == PB)
    {
      VipPrintfError("volume allocation error");
      VipPrintfExit("(vfilter)VipVFilter");
      return((Volume *)NULL);
    }

  vos = VipGetOffsetStructure(vol);
  imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  outptr = VipGetDataPtr_S16BIT(volout)+vos->oFirstPoint;

  tm1 = size-1;
  size2 = mVipSq3(size);
  xf = mVipVolSizeX(vol);
  yf = mVipVolSizeY(vol);
  zf = mVipVolSizeZ(vol);
  volslice = vos->oSlice;
  volline = vos->oLine;

  /*** volume de stockage des variances taille=dimx*dimy*size ***/
  table = VipCreate3DVolume(xf, yf, size, 1.0, 1.0, 1.0, VDOUBLE, "Table", 1);
  if (table == NULL)
  { 
    VipPrintfError("table allocation error");
    VipPrintfExit("(vfilter)VipVFilter");
    return((Volume *)NULL);
  }

  tos = VipGetOffsetStructure(table);
  tableptr = VipGetDataPtr_VDOUBLE(table)+tos->oFirstPoint;
  tabslice = tos->oSlice;

  /*** initialisation de volume de stockage ***/
  for (z=size; z--;)
    for (y=yf; y--;)
      for (x=xf; x--;)  *tableptr++ = MAXVAR;  

  tableptr = VipGetDataPtr_VDOUBLE(table)+tos->oFirstPoint;
  printf("Slice: %3d", 0);
  fflush(stdout);

  for (z=0; z+tm1<zf; z++)
  { 
    printf("\b\b\b%3d", z+1);
    fflush(stdout);

    i = (z+tm1)%size;

    /*** initialisation du plan courant ***/
    for (y=0; y<yf; y++)
      for (x=0; x<xf; x++)  *(tableptr+i*tabslice+y*xf+x) = MAXVAR;

    /*** calcul du cube initial de chaque coupe ***/
    somme = carre = (double)0L;
    for (k=z; k<z+size; k++)
      for (j=0; j<size; j++)
        for (i=0; i<size; i++)
        { 
	  temp = *(imptr+k*volslice+j*volline+i);
          somme += (double)temp;
          carre += mVipSq((double)temp);
        }
    vars = carre-mVipSq(somme)/(double)size2;
    moyenne = somme/(double)size2;
    somme1 = somme;
    carre1 = carre;

    pvar = tableptr+(z%size)*tabslice;
    if ((vars < *pvar) || (*pvar == MAXVAR))
    { 
      *(outptr+z*volslice) = moyenne;
      *pvar = vars;
    }
    pvar = tableptr+(z%size)*tabslice+tm1*xf;
    if ((vars < *pvar) || (*pvar == MAXVAR))
    { 
      *(outptr+z*volslice+tm1*volline) = moyenne;
      *pvar = vars;
    }
    pvar = tableptr+(z%size)*tabslice+tm1;
    if ((vars < *pvar) || (*pvar == MAXVAR))
    { 
      *(outptr+z*volslice+tm1) = moyenne;
      *pvar = vars;
    }
    pvar = tableptr+(z%size)*tabslice+tm1*xf+tm1;
    if ((vars < *pvar) || (*pvar == MAXVAR))
    { 
      *(outptr+z*volslice+tm1*volline+tm1) = moyenne;
      *pvar = vars;
    }
    pvar = tableptr+((z+tm1)%size)*tabslice;
    if ((vars < *pvar) || (*pvar == MAXVAR))
    { 
      *(outptr+(z+tm1)*volslice) = moyenne;
      *pvar = vars;
    }
    pvar = tableptr+((z+tm1)%size)*tabslice+tm1*xf;
    if ((vars < *pvar) || (*pvar == MAXVAR))
    { 
      *(outptr+(z+tm1)*volslice+tm1*volline) = moyenne;
      *pvar = vars;
    }
    pvar = tableptr+((z+tm1)%size)*tabslice+tm1;
    if ((vars < *pvar) || (*pvar == MAXVAR))
    { 
      *(outptr+(z+tm1)*volslice+tm1) = moyenne;
      *pvar = vars;
    }
    *(outptr+(z+tm1)*volslice+tm1*volline+tm1) = moyenne;
    *(tableptr+((z+tm1)%size)*tabslice+tm1*xf+tm1) = vars;

    /*** premier balayage en X ***/
    for (x=1; x+tm1<xf; x++)
    { 
      for (k=z; k<z+size; k++)
        for (j=0; j<size; j++)
        { 
	  loc = x-1; 
          temp = *(imptr+k*volslice+j*volline+loc);
          somme -= (double)temp;
          carre -= mVipSq((double)temp);
          loc = x+tm1;
          temp = *(imptr+k*volslice+j*volline+loc);
          somme += (double)temp;
          carre += mVipSq((double)temp);
        }
      vars = carre-mVipSq(somme)/(double)size2;
      moyenne = somme/(double)size2;

      pvar = tableptr+(z%size)*tabslice+x;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+x) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+(z%size)*tabslice+tm1*xf+x;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+tm1*volline+x) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+(z%size)*tabslice+tm1+x;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+tm1+x) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+(z%size)*tabslice+tm1*xf+tm1+x;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+tm1*volline+tm1+x) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+((z+tm1)%size)*tabslice+x;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+(z+tm1)*volslice+x) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+((z+tm1)%size)*tabslice+tm1*xf+x;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+(z+tm1)*volslice+tm1*volline+x) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+((z+tm1)%size)*tabslice+tm1+x;
      if ((vars < *pvar) || (*pvar == MAXVAR)) 
      { 
	*(outptr+(z+tm1)*volslice+tm1+x) = moyenne;
        *pvar = vars;
      }
      *(outptr+(z+tm1)*volslice+tm1*volline+tm1+x) = moyenne;
      *(tableptr+((z+tm1)%size)*tabslice+tm1*xf+tm1+x) = vars;
    }

    /*** boucle de calcul ***/
    for (y=1; y+tm1<yf; y++)
    {
      somme = somme1;
      carre = carre1;

      /*** calcul du nouveau cube pour chaque Y ***/
      for (k=z; k<z+size; k++)
        for (i=0; i<size; i++)
        { 
	  loc = y-1;
          temp = *(imptr+k*volslice+loc*volline+i);
          somme -= (double)temp;
          carre -= mVipSq((double)temp);
          loc = y+tm1;
          temp = *(imptr+k*volslice+loc*volline+i);
          somme += (double)temp;
          carre += mVipSq((double)temp);          
        }
      vars = carre-mVipSq(somme)/(double)size2;
      moyenne = somme/(double)size2;
      somme1 = somme;
      carre1 = carre;

      pvar = tableptr+(z%size)*tabslice+y*xf;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+y*volline) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+(z%size)*tabslice+(y+tm1)*xf;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+(y+tm1)*volline) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+(z%size)*tabslice+y*xf+tm1;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+y*volline+tm1) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+(z%size)*tabslice+(y+tm1)*xf+tm1;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+z*volslice+(y+tm1)*volline+tm1) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+((z+tm1)%size)*tabslice+y*xf;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+(z+tm1)*volslice+y*volline) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+((z+tm1)%size)*tabslice+(y+tm1)*xf;
      if ((vars < *pvar) || (*pvar == MAXVAR))
      { 
	*(outptr+(z+tm1)*volslice+(y+tm1)*volline) = moyenne;
        *pvar = vars;
      }
      pvar = tableptr+((z+tm1)%size)*tabslice+y*xf+tm1;
      if ((vars < *pvar) || (*pvar == MAXVAR)) 
      { 
	*(outptr+(z+tm1)*volslice+y*volline+tm1) = moyenne;
        *pvar = vars;
      }
      *(outptr+(z+tm1)*volslice+(y+tm1)*volline+tm1) = moyenne;
      *(tableptr+((z+tm1)%size)*tabslice+(y+tm1)*xf+tm1) = vars;      

      /*** balayage principal en X ***/
      for (x=1; x+tm1<xf; x++)
      { 
	for (k=z; k<z+size; k++)
          for (j=y; j<y+size; j++)
          { 
	    loc = x-1; 
            temp = *(imptr+k*volslice+j*volline+loc);
            somme -= (double)temp;
            carre -= mVipSq((double)temp);
            loc = x+tm1;
            temp = *(imptr+k*volslice+j*volline+loc);
            somme += (double)temp;
            carre += mVipSq((double)temp);
          }
        vars = carre-mVipSq(somme)/(double)size2;
        moyenne = somme/(double)size2;

        pvar = tableptr+(z%size)*tabslice+y*xf+x;
        if ((vars < *pvar) || (*pvar == MAXVAR))
        {
	  *(outptr+z*volslice+y*volline+x) = moyenne;
          *pvar = vars;
        }
        pvar = tableptr+(z%size)*tabslice+(y+tm1)*xf+x;
        if ((vars < *pvar) || (*pvar == MAXVAR))
        { 
	  *(outptr+z*volslice+(y+tm1)*volline+x) = moyenne;
          *pvar = vars;
        }
        pvar = tableptr+(z%size)*tabslice+y*xf+tm1+x;
        if ((vars < *pvar) || (*pvar == MAXVAR))
        { 
	  *(outptr+z*volslice+y*volline+tm1+x) = moyenne;
          *pvar = vars;
        }
        pvar = tableptr+(z%size)*tabslice+(y+tm1)*xf+tm1+x;
        if ((vars < *pvar) || (*pvar == MAXVAR))
        { 
	  *(outptr+z*volslice+(y+tm1)*volline+tm1+x) = moyenne;
          *pvar = vars;
        }
        pvar = tableptr+((z+tm1)%size)*tabslice+y*xf+x;
        if ((vars < *pvar) || (*pvar == MAXVAR))
        { 
	  *(outptr+(z+tm1)*volslice+y*volline+x) = moyenne;
          *pvar = vars;
        }
        pvar = tableptr+((z+tm1)%size)*tabslice+(y+tm1)*xf+x;
        if ((vars < *pvar) || (*pvar == MAXVAR))
        { 
	  *(outptr+(z+tm1)*volslice+(y+tm1)*volline+x) = moyenne;
          *pvar = vars;
        }
        pvar = tableptr+((z+tm1)%size)*tabslice+y*xf+tm1+x;
        if ((vars < *pvar) || (*pvar == MAXVAR)) 
        { 
	  *(outptr+(z+tm1)*volslice+y*volline+tm1+x) = moyenne;
          *pvar = vars;
        }
        *(outptr+(z+tm1)*volslice+(y+tm1)*volline+tm1+x) = moyenne;
        *(tableptr+((z+tm1)%size)*tabslice+(y+tm1)*xf+tm1+x) = vars;
      }
    }
  }

  printf("\n");

  VipFreeVolume(table);
  VipFree(table);

  return(volout);
}

Volume *VipNonOptimizedVFilter(Volume *vol, int size)
{
  VIP_DEC_VOLUME(volout);
  Vip_S16BIT *imptr, *outptr, temp;
  int x, y, z, i, j, k, xf, yf, zf, index, tm1;
  double t3, sum, sum2, varmin, var[8], moy[8];
  long offsl, offli, offpbl, offlbs, offfp;
  
  t3 = (double)mVipSq3(size);
  tm1 = size-1;

  if ((volout = VipDuplicateVolumeStructure(vol, "")) == NULL)
    { 
      VipPrintfError("volume structure duplication error");
      VipPrintfExit("(vfilter)VipVFilter");
      return((Volume *)NULL);
    }

  if ((VipAllocateVolumeData(volout)) == PB)
    {
      VipPrintfError("volume allocation error");
      VipPrintfExit("(vfilter)VipVFilter");
      return((Volume *)NULL);
    }

  VipGet3DSize(vol, &xf, &yf, &zf);
  offli = VipOffsetLine(vol);
  offsl = VipOffsetSlice(vol);
  offpbl = VipOffsetPointBetweenLine(vol);
  offlbs = VipOffsetLineBetweenSlice(vol);
  offfp = VipOffsetFirstPoint(vol);

  imptr = VipGetDataPtr_S16BIT(vol)+offfp;
  outptr = VipGetDataPtr_S16BIT(volout)+offfp;

  printf("Slice: %3d", 0);
  fflush(stdout);

  for (z=0; z<zf; z++, outptr+=offlbs)
    {
      printf("\b\b\b%3d", z);
      fflush(stdout);

      for (y=0; y<yf; y++, outptr+=offpbl)
	for(x=0; x<xf; x++)
	  {
	    /* Initialisation */
	    for (i=0; i<8; i++)
	      {
		var[i] = 1.0e20;
		moy[i] = 0.0;
	      }

	    /* Premier cube (-1,-1,-1) */
	    if ((x >= tm1) && (y >= tm1) && (z >= tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z-tm1; k<z+1; k++)
		  for (j=y-tm1; j<y+1; j++)
		    for (i=x-tm1; i<x+1; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[0] = sum2-mVipSq(sum)/t3;
		moy[0] = sum/t3;
	      }

	    /* Deuxieme cube (-1,1,-1) */
	    if ((x >= tm1) && (y < yf-tm1) && (z >= tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z-tm1; k<z+1; k++)
		  for (j=y; j<y+size; j++)
		    for (i=x-tm1; i<x+1; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[1] = sum2-mVipSq(sum)/t3;
		moy[1] = sum/t3;
	      }

	    /* Troisieme cube (1,1,-1) */
	    if ((x < xf-tm1) && (y < yf-tm1) && (z >= tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z-tm1; k<z+1; k++)
		  for (j=y; j<y+size; j++)
		    for (i=x; i<x+size; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[2] = sum2-mVipSq(sum)/t3;
		moy[2] = sum/t3;
	      }

	    /* Quatrieme cube (1,-1,-1) */
	    if ((x < xf-tm1) && (y >= tm1) && (z >= tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z-tm1; k<z+1; k++)
		  for (j=y-tm1; j<y+1; j++)
		    for (i=x; i<x+size; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[3] = sum2-mVipSq(sum)/t3;
		moy[3] = sum/t3;
	      }

	    /* Cinquieme cube (-1,-1,1) */
	    if ((x >= tm1) && (y >= tm1) && (z < zf-tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z; k<z+size; k++)
		  for (j=y-tm1; j<y+1; j++)
		    for (i=x-tm1; i<x+1; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[4] = sum2-mVipSq(sum)/t3;
		moy[4] = sum/t3;
	      }

	    /* Sixieme cube (-1,1,1) */
	    if ((x >= tm1) && (y < yf-tm1) && (z < zf-tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z; k<z+size; k++)
		  for (j=y; j<y+size; j++)
		    for (i=x-tm1; i<x+1; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[5] = sum2-mVipSq(sum)/t3;
		moy[5] = sum/t3;
	      }

	    /* Septieme cube (1,1,1) */
	    if ((x < xf-tm1) && (y < yf-tm1) && (z < zf-tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z; k<z+size; k++)
		  for (j=y; j<y+size; j++)
		    for (i=x; i<x+size; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[6] = sum2-mVipSq(sum)/t3;
		moy[6] = sum/t3;
	      }

	    /* Huitieme cube (1,-1,1) */
	    if ((x < xf-tm1) && (y >= tm1) && (z < zf-tm1))
	      {
		sum = sum2 = 0.0;
		for (k=z; k<z+size; k++)
		  for (j=y-tm1; j<y+1; j++)
		    for (i=x; i<x+size; i++)
		      {
			temp = *(imptr+i+j*offli+k*offsl);
			sum += (double)temp;
			sum2 += mVipSq((double)temp);
		      }
		var[7] = sum2-mVipSq(sum)/t3;
		moy[7] = sum/t3;
	      }

	    /* Recherche du meilleur cube */
	    index = 0;
	    varmin = var[0];
	    for (i=1; i<8; i++)
	      if (var[i] < varmin)
		{
		  varmin = var[i];
		  index = i;
		}

	    /* Affectation du point */
	    *outptr++ = (Vip_S16BIT)moy[index];
	  }
    }

  printf("\n");

  return(volout);
}
