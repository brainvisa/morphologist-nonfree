/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_histo.c          * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/04/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : gestion des histogrammes (scalespace, Kmeans...)
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

#include <vip/volume.h>
#include <vip/histo.h> 
#include <vip/connex.h>  				 				
#include <vip/histo_static.h>				
#include <vip/math.h>


/*---------------------------------------------------------------*/
int VipCutRightFlatPartOfHisto( VipHisto *h, int cutvalue)
/*---------------------------------------------------------------*/  
{
  int xmax;
  int i;
  int compteur=20;
  int begin=60;

  if((!h))
    {
      VipPrintfError("VipGetLeftExtendedHisto arguments!");
      VipPrintfExit("VipCutRightFlatPartOfHisto");
      return(PB);
    }
  if((cutvalue<0) || (cutvalue>10000))
    {
      VipPrintfError("strange value as input");
      VipPrintfExit("VipCutRightFlatPartOfHisto");
      return(PB);
    }

  xmax = -10000;
  for(i=h->range_min;i<=h->range_max;i++)
    {
      if (h->val[i]>cutvalue) 
        {
          xmax = i;
          begin--;
        }
      else if (begin<0 && compteur-->0) {xmax=i;} /* to preserve at least 20 flat points, some people have to add them...*/
    }
  /*xmax = mVipMax(xmax, 400);*/
  printf("Cutting range [%d-%d] in histogram right part (almost zero)\n",xmax,h->range_max);
  h->range_max = xmax;
  h->val[h->range_max] = 0;
  if(h->range_min<0)
      {
	  printf("Cutting range [%d-0] in histogram left part\n",h->range_min);
      }
  h->range_min = 0;

  return(OK);
}


/*---------------------------------------------------------------*/
VipHisto *VipGetHistoSurface( VipHisto *hin, Volume *vol)
/*---------------------------------------------------------------*/
{
  Volume *noisy;
  VipConnectivityStruct *vcs;
  VipHisto *hsurf, *historesamp;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr, *voisin;
  int VOLBORDER = -12345;
  int icon, i, j/*, k*/;
  int lemin, lemax, imax;
  float normalize;
  int ratio = 0,factor=0 ;
  RandomBox *rb;
  int total/*, pourcentage*/;
  /*int i50;*/
  int **tab2D;
  int deriv[40000];


  if((!hin) || !vol)
    {
  for(i=0;i<=hin->range_max;i++)
      VipPrintfError("VipGetHistoSurface arguments!");
      VipPrintfExit("VipGetHistoSurface");
      return(PB);
    }

  rb = VipCreateRandomBoxCircularList(11111,1);
  if(!rb)
    {
      VipPrintfError("VipRandomizeFrontOrder");
      return(PB);
    }
  
  noisy = VipCopyVolume(vol, "noisy");

  historesamp = VipGetPropUndersampledHisto(hin, 95, &ratio, &factor, 0, 50);
  if (factor>0)
    {
      printf("Undersampling histogram... (factor: %d, ratio: %d)\n",factor, ratio);
      hin = historesamp;
    }
  VipCutRightFlatPartOfHisto(hin,100);
  printf("Computing isosurfaces in the intensity range [%d,%d]\n",0,hin->range_max);

  hsurf = VipCreateHisto(0,hin->range_max);
  if(hsurf==PB) return(PB);

  for(i=0;i<=hin->range_max;i++) hsurf->val[i] = 0;

  vos = VipGetOffsetStructure(noisy);

  VipSetBorderLevel(noisy,VOLBORDER);

  vcs = VipGetConnectivityStruct( noisy, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  
  ptr = VipGetDataPtr_S16BIT( noisy ) + vos->oFirstPoint;
  for ( iz = mVipVolSizeZ(noisy); iz-- ; )              
    {
      for ( iy = mVipVolSizeY(noisy); iy-- ; )           
	{
	  for ( ix = mVipVolSizeX(noisy); ix-- ; )
	    {
              *ptr = *ptr >> factor;
              *ptr = mVipMax(0,*ptr+(rb->label - 0));
              rb = rb->next;
              ptr++;
	    }
	  ptr += vos->oPointBetweenLine;  
	}
      ptr += vos->oLineBetweenSlice; 
    }
  VipFreeRandomBox(rb);

  tab2D = (int **)calloc((hin->range_max+1),sizeof(int *));
  if (!tab2D) return(PB);
  for(i=0;i<=hin->range_max;i++)
    {
      tab2D[i] = (int *)calloc((hin->range_max+1),sizeof(int));
      if (!tab2D[i]) return(PB);
    }

  for(i=0;i<=hin->range_max;i++)
    for(j=i;j<=hin->range_max;j++)
      tab2D[i][j] = 0;

  ptr = VipGetDataPtr_S16BIT( noisy ) + vos->oFirstPoint;
  for ( iz = mVipVolSizeZ(noisy); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(noisy); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(noisy); ix-- ; )/* loop on points */
	    {
              for ( icon=0; icon<vcs->nb_neighbors;icon++)
                {
                  voisin =  ptr + vcs->offset[icon];
                  if (*voisin!=VOLBORDER)
                    {
                      lemin = mVipMin(*voisin,*ptr);
                      lemin = mVipMax(lemin,0);
                      lemin = mVipMin(lemin,hin->range_max);
                      lemax = mVipMax(*voisin,*ptr);
                      lemax = mVipMin(lemax,hin->range_max);
                      lemax = mVipMax(lemax,0);
                      tab2D[lemin][lemax]++;
                    }
                }
              ptr++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

 
  for(j=1;j<=hin->range_max;j++)
    {
      hsurf->val[0] += tab2D[0][j];
    }
  for(i=1;i<=hin->range_max;i++)
    {
      hsurf->val[i] = hsurf->val[i-1];
      for(j=0;j<=(i-1);j++) hsurf->val[i] -= tab2D[j][i];
      for(j=(i+1);j<=hin->range_max;j++) hsurf->val[i] += tab2D[i][j];
    }
 
  free(tab2D);

  normalize = mVipVolSizeX(noisy)*mVipVolSizeY(noisy)*mVipVolVoxSizeX(noisy)*mVipVolVoxSizeY(noisy)
    + mVipVolSizeX(noisy)*mVipVolSizeZ(noisy)*mVipVolVoxSizeX(noisy)*mVipVolVoxSizeZ(noisy) 
    + mVipVolSizeZ(noisy)*mVipVolSizeY(noisy)*mVipVolVoxSizeZ(noisy)*mVipVolVoxSizeY(noisy);
  normalize /= 100;

  for(i=0;i<=hin->range_max;i++) hsurf->val[i] = (int)(hsurf->val[i]/normalize);

  total = 0;
  for(i=0;i<=hin->range_max;i++) total+= hin->val[i];

  for ( i = 1; i <= hin->range_max-1 ; i++ )            
    {
      deriv[i] = hsurf->val[i+1] + hsurf->val[i-1] -2*hsurf->val[i];
    }
  deriv[0] = deriv[1]-1;
  deriv[hin->range_max] = deriv[hin->range_max-1]-1;

  if(VipWriteHisto(hsurf,"surf",WRITE_HISTO_ASCII)==PB) return(PB);          

  for ( i = 0; i <= hin->range_max ; i++ )            
    {
      hsurf->val[i] = deriv[i]+5000;
    }
  
  /*pourcentage = hin->val[0];*/
  imax = 1;
  lemax = -100000;
  for(i=1;i<=hin->range_max-1;i++) 
    {
      /*
      pourcentage += hin->val[i];
      if (((float)pourcentage/(float)total)>0.5)
        {
          i50 = i;
        }
      if (((float)pourcentage/(float)total)>0.8)
        {
          VipPrintfWarning("No isosurface minimum in first 4/5 of the histogram (small brain?)");
          i = i50;
          break;
        }
      
        if ((deriv[i]>=deriv[i-1]) && (deriv[i]>=deriv[i+1])) break;
      */
      if (deriv[i]>lemax)
        {
          imax=i;
          lemax = deriv[i];
        }
    }
  printf("Tissue/background threshold: %d\n", (imax+1) << factor);
  VipFreeVolume(noisy);
  
  return(hsurf);
}

/*---------------------------------------------------------------*/
VipHisto *VipGetLeftExtendedHisto( VipHisto *hin, int length )
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;

  if((!hin) || (length<0))
    {
      VipPrintfError("VipGetLeftExtendedHisto arguments!");
      VipPrintfExit("VipGetLeftExtendedHisto");
      return(PB);
    }

  hout = VipCreateHisto(hin->range_min-length,hin->range_max);
  if(hout==PB) return(PB);

  for(i=0;i<length;i++) hout->histo_ad[i] = 0/*hin->val[hin->range_min]*/;
  for(i=hin->range_min;i<=hin->range_max;i++) hout->val[i] = hin->val[i];

  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipGetRightExtendedHisto( VipHisto *hin, int length )
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;

  if((!hin) || (length<0))
    {
      VipPrintfError("VipGetRightExtendedHisto arguments!");
      VipPrintfExit("VipGetRightExtendedHisto");
      return(PB);
    }

  hout = VipCreateHisto(hin->range_min,hin->range_max+length);
  if(hout==PB) return(PB);

  for(i=hin->range_min;i<=hin->range_max;i++) hout->val[i] = hin->val[i];
  for(i=hin->range_max;i<hin->range_max+length;i++) hout->val[i] = 0;

  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipCreateThreeGaussianSumHisto( 
float m1,
float s1,
float h1,
float m2,
float s2,
float h2,
float m3,
float s3,
float h3
)
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;
  double temp;


  hout = VipCreateHisto(0,700);
  if(hout==PB) return(PB);

  for(i=0;i<700;i++) 
    {
      temp = h1 * exp( -(double)(i-m1)*(i-m1)/s1/s1/2.);
      temp += h2 * exp( -(double)(i-m2)*(i-m2)/s2/s2/2.);
      temp += h3 * exp( -(double)(i-m3)*(i-m3)/s3/s3/2.);
      mVipHistoVal(hout,i) = (int)temp;
    }

  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipCreateForGaussianSumHisto( 
float m1,
float s1,
float h1,
float m2,
float s2,
float h2,
float m3,
float s3,
float h3,
float m4,
float s4,
float h4
)
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;
  double temp;


  hout = VipCreateHisto(0,500);
  if(hout==PB) return(PB);

  for(i=0;i<500;i++) 
    {
      temp = h1 * exp( -(double)(i-m1)*(i-m1)/s1/s1/2.);
      temp += h2 * exp( -(double)(i-m2)*(i-m2)/s2/s2/2.);
      temp += h3 * exp( -(double)(i-m3)*(i-m3)/s3/s3/2.);
      temp += h4 * exp( -(double)(i-m4)*(i-m4)/s4/s4/2.);
      mVipHistoVal(hout,i) = (int)temp;
    }

  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipGetUndersampledHisto( VipHisto *hin, int *ratio )
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;
  int newlength;
  int factor;

  if((!hin) || (!ratio))
    {
      VipPrintfError("VipGetUndersampledHisto arguments!");
      return(PB);
    }

  newlength = hin->range_max - hin->range_min+1;
  factor = 1;
  while(newlength>200)
    {
      newlength = newlength / 2 + 1;
      factor *=2;
    }
  hout = VipCreateHisto(hin->range_min/factor,hin->range_max/factor+1);
  if(hout==PB)
      {
	  VipPrintfExit("VipGetUndersampledHisto");
	  return(PB);
      }

  *ratio =factor;
  for(i=hin->range_min;i<=hin->range_max;i++)
    {
      hout->val[i/factor] += hin->val[i]; 
    }
  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipGetEntropyForcedUndersampledHisto( VipHisto *hin, float entropy, int *ratio )
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;
  int factor;

  if((!hin) || (!ratio))
    {
      VipPrintfError("VipGetEntropyForcedUndersampledHisto arguments!");
      return(PB);
    }

  for(factor=1;factor<14;factor++)
    {
      hout = VipCreateHisto(hin->range_min/factor,hin->range_max/factor+1);
      if(hout==PB)
        {
	  VipPrintfExit("VipGetEntropyForcedUndersampledHisto");
	  return(PB);
        }
      for(i=hout->range_min;i<=hout->range_max;i++)
        hout->val[i]=0;
      for(i=hin->range_min;i<=hin->range_max;i++)
        {
          hout->val[i/factor] += hin->val[i]; 
        }
      VipGetHistoEntropy(hout, &entropy);
      printf("Factor %d, entropy:%f\n", factor,entropy);
      VipFreeHisto(hout);
    }

  return(hin);
}

/*---------------------------------------------------------------*/
VipHisto *VipGetForcedUndersampledHisto( VipHisto *hin, int *ratio )
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;
  int newlength;
  int factor;

  if((!hin) || (!ratio))
    {
      VipPrintfError("VipGetForcedUndersampledHisto arguments!");
      return(PB);
    }

  newlength = hin->range_max - hin->range_min+1;
  factor = 1;
  while((factor/(*ratio) >1.5) && (factor/(*ratio) <0.5))
    {
      newlength = newlength / 2 + 1;
      factor *=2;
    }
  hout = VipCreateHisto(hin->range_min/factor,hin->range_max/factor+1);
  if(hout==PB)
      {
	  VipPrintfExit("VipGetForcedUndersampledHisto");
	  return(PB);
      }

  *ratio =factor;
  for(i=hin->range_min;i<=hin->range_max;i++)
    {
      hout->val[i/factor] += hin->val[i]; 
    }
  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipGetPropUndersampledHisto( VipHisto *hin, int prop, int *ratio, int *power2, int lowthreshold, int final_length )
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;
  int newlength;
  int thefactor; 
  int size;
  int cumul;
  float threshold;

  if((!hin) || (!ratio) || (!power2))
    {
      VipPrintfError("VipGetPropUndersampledHisto arguments!");
      return(PB);
    }
  if(prop<1 || prop>100)
      {
	  VipPrintfError("prop should be in the range [1%-100%]");
	  VipPrintfExit("VipGetPropUndersampledHisto");
	  return(PB);
      }

  for(i=hin->range_min;i<lowthreshold;i++)
    {
      hin->val[i]=0;
    }

  if(*ratio==0)
    {
      size = 0;
      for(i=hin->range_min;i<=hin->range_max;i++)
	{
	  size += hin->val[i];
	}
      if(size==0)
	{
	  VipPrintfError("Empty histogram");
	  VipPrintfExit("VipGetPropUndersampledHisto");
	  return(PB);
	}
      cumul = 0;
      threshold = (float)prop/100.;
      for(i=hin->range_min;i<=hin->range_max;i++)
	{
	  cumul += hin->val[i];
	  if(((float)cumul/(float)size)>threshold)
	    {
	      printf("(%d percent threshold: %d) \n",prop,i);
	      break;
	    }
	}
      newlength = i+1;
      thefactor = 1;
      *power2 = 0;
      while(newlength>final_length)
	{
          /*printf("%d %d\n",newlength,*power2);*/
	  newlength = newlength / 2 + 1;
	  thefactor *=2;
	  *power2 +=1;
	}
      
    }
  else
    {
      thefactor = *ratio;
    }
  hout = VipCreateHisto(hin->range_min/thefactor,hin->range_max/thefactor+1);
  if(hout==PB)
      {
	  VipPrintfExit("VipGetUndersampledHisto");
	  return(PB);
      }

  *ratio =thefactor;
  for(i=hin->range_min;i<=hin->range_max;i++)
    {
      hout->val[i/thefactor] += hin->val[i]; 
    }
  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipGetCumulHisto( VipHisto *hin)
/*---------------------------------------------------------------*/

{
  VipHisto *hout;
  int i;

  if((!hin))
    {
      VipPrintfError("VipGetCumulHisto arguments!");
      return(PB);
    }

  hout = VipCreateHisto(hin->range_min,hin->range_max);
  if(hout==PB) return(PB);

  /* c'est quoi ca!!! hout->histo_ad[0] = hin->histo_ad[0];*/
  for(i=hin->range_min;i<hin->range_max;i++)
    hout->val[i+1] = hin->val[i+1] + hout->val[i];

  return(hout);
}

/*---------------------------------------------------------------*/
VipHisto *VipComputeVolumeHisto(Volume *vol)
/*---------------------------------------------------------------*/
{
  VipHisto *histo;
  int volmax, volmin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(histo)ComputeVolumeHisto");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK )
    {
      VipPrintfError("Sorry,  ComputeVolumeHisto is only implemented for S16BIT volume");
      VipPrintfExit("(histo)ComputeVolumeHisto");
      return(PB);
    }
  volmax = (int)VipGetVolumeMax(vol) + 1;
  volmin = (int)VipGetVolumeMin(vol);
  if(volmin>0) volmin = 0;
  histo = VipCreateHisto(volmin,volmax);
  if(histo==PB) return(PB);

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
		    histo->val[*ptr++]++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
  return(histo);
}

/*---------------------------------------------------------------*/
int VipPourcentageLowerThanThreshold(Volume *vol, int lowthreshold, int pourcentage)
/*---------------------------------------------------------------*/
{
  VipHisto *histo;
  int size;
  int cumul;
  int i;
  float threshold;

  if (pourcentage<=0 || pourcentage>100)
    {
      VipPrintfError("pourcentage should be in the range [1%-100%] VipPourcentageLowerThanThreshold");
      return(PB);
    }
  histo = VipComputeVolumeHisto(vol);
  for(i=histo->range_min;i<lowthreshold;i++)
    {
      histo->val[i]=0;
    }

  size = 0;
  for(i=histo->range_min;i<=histo->range_max;i++)
    {
      size += histo->val[i];
    }
  if(size==0)
    {
      VipPrintfWarning("Empty histogram");
    }
  cumul = 0;
  threshold = (float)pourcentage/100.;
  for(i=histo->range_min;i<=histo->range_max;i++)
    {
      cumul += histo->val[i];
      if(((float)cumul/(float)size)>threshold)
        {
          break;
        }
    }
  return(i);
	    
}
/*---------------------------------------------------------------*/
Volume *VipComputeMeanVolume(Volume *vol)
/*---------------------------------------------------------------*/
{
  Volume *mean;
  int loopsize, i;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr, *mptr;
  VipConnectivityStruct *vcs;
  int border = -1111;
  int connectivity = CONNECTIVITY_6;
  int n, temp;
  double m;

  printf("Compute spatial mean...\n");
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipComputeMeanVolume requires non zero border");
      VipPrintfExit("VipComputeMeanVolume");
      return(PB);
    }

  mean = VipDuplicateVolumeStructure( vol, "mean");
  if(mean==NULL) return(PB);
  if(VipAllocateVolumeData(mean)==PB) return(PB);
  VipSet3DImageLevel(mean,0,0); /*calloc...*/
  VipSetBorderLevel(vol,border); /*calloc...*/

  vcs = VipGetConnectivityStruct( vol, connectivity );
  if(vcs==NULL) return(PB);
  loopsize = vcs->nb_neighbors;

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  mptr = VipGetDataPtr_S16BIT( mean ) + vos->oFirstPoint;

  for ( iz = mVipVolSizeZ(vol); iz-- ; )
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )
	    {
              if(1/*O *ptr*/)
                {
                  m = *ptr;
                  n = 1;
              
                  for( i=0; i<loopsize; i++)
                    {
                      temp = *(ptr+vcs->offset[i]);
                      if (temp!=border)
                        {
                          n++;
                          m += temp;
                        }
                    }
                  if (n<=1)
                    {
                      VipPrintfError("Incoherence");
                      VipPrintfExit("VipComputeMeanVolume");
                      return(PB);
                    }
              
                  m /= n;
                  *mptr = (int)(m);
                }
              else *mptr=0;
	      ptr++;
              mptr++;
	    }
	  ptr += vos->oPointBetweenLine; 
	  mptr += vos->oPointBetweenLine; 
	}
      ptr += vos->oLineBetweenSlice; 
      mptr += vos->oLineBetweenSlice; 
    }
  return(mean);
}
/*---------------------------------------------------------------*/
Volume *VipComputeVarianceVolume(Volume *vol)
/*---------------------------------------------------------------*/
{
  Volume *var;
  int loopsize, i;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr, *varptr;
  VipConnectivityStruct *vcs;
  double sum;
  int border = -1111;
  int connectivity = CONNECTIVITY_26;
  int n, temp;
  double mean;

  printf("Compute spatial variance...\n");
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipComputeVarianceVolume requires non zero border");
      VipPrintfExit("VipComputeVarianceVolume");
      return(PB);
    }

  var = VipDuplicateVolumeStructure( vol, "variance");
  if(var==NULL) return(PB);
  if(VipAllocateVolumeData(var)==PB) return(PB);
  VipSet3DImageLevel(var,0,0); /*calloc...*/
  VipSetBorderLevel(vol,border); /*calloc...*/

  vcs = VipGetConnectivityStruct( vol, connectivity );
  if(vcs==NULL) return(PB);
  loopsize = vcs->nb_neighbors;

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  varptr = VipGetDataPtr_S16BIT( var ) + vos->oFirstPoint;

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )           
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )
	    {
              if(1/*O *ptr*/)
                {
                  mean = *ptr;
                  n = 1;
              
                  for( i=0; i<loopsize; i++)
                    {
                      temp = *(ptr+vcs->offset[i]);
                      if (temp!=border)
                        {
                          n++;
                          mean += temp;
                        }
                    }
                  if (n<=1)
                    {
                      VipPrintfError("Incoherence");
                      VipPrintfExit("VipComputeVarianceVolume");
                      return(PB);
                    }
              
                  mean /= n; /* essai de switch to standard variations to the center value
                                mean = *ptr;*/
                  sum = 0;
                  for( i=0; i<loopsize; i++)
                    {
                      temp = *(ptr+vcs->offset[i]);
                      if (temp!=border)
                        {
                          sum += (temp-mean)*(temp-mean);
                        }
                    }
                  sum /= (n-1);
                  *varptr = (int)(100*sqrt(sum))/mVipMax(1,mean);
                }
              else *varptr=0;
	      ptr++;
              varptr++;
	    }
	  ptr += vos->oPointBetweenLine; 
	  varptr += vos->oPointBetweenLine; 
	}
      ptr += vos->oLineBetweenSlice; 
      varptr += vos->oLineBetweenSlice; 
    }
  return(var);
}

/*---------------------------------------------------------------*/
VipHisto *VipComputeCleanedUpVolumeHisto(Volume *vol, Volume *discard)
/*---------------------------------------------------------------*/
{
  VipHisto *histo;
  int volmax, volmin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr, *disptr;
  int compteur;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(histo)ComputeVolumeHisto");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK )
    {
      VipPrintfError("Sorry,  ComputeVolumeHisto is only implemented for S16BIT volume");
      VipPrintfExit("(histo)ComputeVolumeHisto");
      return(PB);
    }
  volmax = (int)VipGetVolumeMax(vol) + 1;
  volmin = (int)VipGetVolumeMin(vol);
  if(volmin>0) volmin = 0;
  histo = VipCreateHisto(volmin,volmax);
  if(histo==PB) return(PB);

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  disptr = VipGetDataPtr_S16BIT( discard ) + vos->oFirstPoint;

  compteur=0;
  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
              if(!*disptr)
                {
                  histo->val[*ptr]++;
                  compteur++;
                }
              ptr++;
              disptr++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	  disptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      disptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

  printf("Use %d/%d voxels to estimate histogram\n",compteur,mVipVolSizeZ(vol)*mVipVolSizeY(vol)*mVipVolSizeX(vol));
  return(histo);
}

/*---------------------------------------------------------------*/
VipHisto *VipCreateHisto(
int minvalue,
int maxvalue)
/*-------------------------------------------------------------*/
{
  VipHisto *histo;


  if(minvalue>maxvalue)
    {
      VipPrintfError("non coherent min and max value");
      VipPrintfExit("VipCreateHisto");
      return(NULL);
    }

  histo = (VipHisto *)VipCalloc(1,sizeof(VipHisto),"VipCreateHisto");
  if(histo==PB) return(PB);


  histo->buf_zero_position = -minvalue; /*warning, can be outside buffer*/
  histo->buf_size = maxvalue - minvalue + 1;
  histo->histo_ad = (int *)VipCalloc(histo->buf_size,sizeof(int),"VipCreateHisto");
  if(histo->histo_ad==PB) return(PB);

  histo->val = histo->histo_ad + histo->buf_zero_position;
  histo->min = minvalue;
  histo->max = minvalue;
  histo->range_min = minvalue;
  histo->range_max = maxvalue;
  
  return(histo);
}

/*---------------------------------------------------------------*/
VipHisto *VipReadHisto(char *name)
/*---------------------------------------------------------------*/
{
  FILE *f;
  char filename[512], error[512];
  int i;
  int mode;
  int count;
  int buf[100000];
  VipHisto *h;
  int rangemin;
  int rangemax;
  int test1, test2;
  char line[256];
  int oldformat=VFALSE;
  
  if(name==NULL)
    {
      VipPrintfError("NULL name in VipReadHisto!");
      VipPrintfExit("VipReadHisto");
      return(PB);
    }
  if(strlen(name)>256)
    {
      VipPrintfError("Strange histo name in VipReadHisto (length>256)!");
      VipPrintfExit("VipReadHisto");
      return(PB);
    }

  strcpy(filename,name);
  strcat(filename,".his");
  f = fopen(filename,"r");
  if(f==NULL)
    {
      strcpy(filename,name);
      strcat(filename,".his_bin");
      f = fopen(filename,"r");
      if(f==NULL)
	{
	  sprintf(error,"Can not open file %s.his nor file %s.his_bin",name,name);
	  VipPrintfError(error);
	  VipPrintfExit("VipReadHisto");
	  return(PB);
	}
      else mode = WRITE_HISTO_BINARY;
    }
  else mode = WRITE_HISTO_ASCII;
      
  h = NULL;
  if(mode == WRITE_HISTO_ASCII)
    {
	/*test old format versus new format*/
	if ( !fgets(line, 256, f) )
  {
    VipPrintfExit("VipReadHisto : Corrupted file");
	  return(PB);
  }
	i = sscanf(line,"%d%d",&test1,&test2);
	if(i!=2) oldformat=VTRUE;
	rewind(f);

	if(oldformat==VTRUE)
	    {
		count = 0;
		while((count<100000)&& (i = fscanf(f,"%d",&(buf[count++]))==1)) ;
		if(count==100000)
		    {
			VipPrintfWarning("Sorry, only 100000 values read in VipReadHisto...\n");
		    }
		count--;
		if(buf[count-2]==-1)
		    {
			rangemin = buf[count-1];
			rangemax = count+rangemin-3;
		    }
		else
		    {
			rangemin = 0;
			rangemax = count -1;
		    }
		h = VipCreateHisto(rangemin,rangemax);
		if(h==PB) return(PB);
		for(i=rangemin;i<rangemax;i++) mVipHistoVal(h,i) = buf[i-rangemin];
	    }
	else
	    {
		rangemin = 10000000;
		rangemax = -10000000;
    if ( !fgets(line, 256, f) )
    {
      VipPrintfExit("VipReadHisto : Corrupted file");
	    return(PB);
    }
		while(!feof(f))
		    {
			i = sscanf(line,"%d%d",&test1,&test2);
			if(i!=2) 
			    {
				VipPrintfWarning("Stange line in histogram file");
			    }
			else
			    {
				if(test1>rangemax) rangemax=test1;
				if(test1<rangemin) rangemin=test1;
			    }
      if ( !fgets(line, 256, f) && !feof(f) )
      {
        VipPrintfExit("VipReadHisto : Corrupted file");
	      return(PB);
      }
		    }
		rewind(f);
		h = VipCreateHisto(rangemin,rangemax);
    if ( !fgets(line, 256, f) )
    {
      VipPrintfExit("VipReadHisto : Corrupted file");
	    return(PB);
    }
		while(!feof(f))
		    {
			i = sscanf(line,"%d%d",&test1,&test2);
			if(i==2)
			    {
				mVipHistoVal(h,test1)=test2;
			    }
      if ( !fgets(line, 256, f) && !feof(f) )
      {
        VipPrintfExit("VipReadHisto : Corrupted file");
	      return(PB);
      }
		    }
	    }
    }
  else
    {
      VipPrintfError("Sorry, binary mode not implemented in VipReadHisto");
      return(PB);
    }
  return(h);
	  
}

/*---------------------------------------------------------------*/
int VipTestHistoExists(char *name)
/*---------------------------------------------------------------*/
{
  FILE *f;
  char filename[512];
  int mode;

  if(name==NULL)
    {
      VipPrintfError("NULL name in VipTestHistoExists!");
      VipPrintfExit("VipTestHistoExists");
      return(PB);
    }
  if(strlen(name)>256)
    {
      VipPrintfError("Strange histo name in VipTestHistoExists (length>256)!");
      VipPrintfExit("VipTestHistoExists");
      return(PB);
    }

  strcpy(filename,name);
  strcat(filename,".his");
  f = fopen(filename,"r");
  if(f==NULL)
    {
      strcpy(filename,name);
      strcat(filename,".his_bin");
      f = fopen(filename,"r");
      if(f==NULL)
	{
	  return(PB);
	}
      else mode = WRITE_HISTO_BINARY;
    }
  else mode = WRITE_HISTO_ASCII;
      
  fclose(f);
  return(mode);
	  
}
/*---------------------------------------------------------------*/
int VipWriteHisto(VipHisto *histo, char *name, int mode)
/*---------------------------------------------------------------*/
{
  FILE *f;
  char filename[512], error[512];
  int i,j;

  if(histo==NULL || name==NULL)
    {
      VipPrintfError("NULL pointer in VipWriteHisto!");
      VipPrintfExit("VipWriteHisto");
      return(PB);
    }
  if((mode!=WRITE_HISTO_ASCII)&&(mode!=WRITE_HISTO_BINARY))
    {
      VipPrintfError("Unkown mode in VipWriteHisto!");
      VipPrintfExit("VipWriteHisto");
      return(PB);
    }

  if(strlen(name)>256)
    {
      VipPrintfError("Strange histo name in VipWriteHisto (length>256)!");
      VipPrintfExit("VipWriteHisto");
      return(PB);
    }
  strcpy(filename,name);
  if( strlen( filename ) >= 4
      && ( strcmp( filename + strlen( filename ) - 4, ".his" ) == 0
      || strcmp( filename + strlen( filename ) - 4, ".han" ) == 0 ) )
    filename[ strlen( filename ) - 4 ] = '\0';
  if(mode==WRITE_HISTO_ASCII) strcat(filename,".his");
  else strcat(filename,".his_bin");
  f = fopen(filename,"w");
  if(f==NULL)
    {
      sprintf(error,"Can not open file %s",filename);
      VipPrintfError(error);
      VipPrintfExit("VipWriteHisto");
      return(PB);
    }
  if(mode==WRITE_HISTO_BINARY)
    {
      i = fwrite(histo->val+histo->range_min,sizeof(int),histo->range_max-histo->range_min+1,f);
      if(i!=histo->range_max-histo->range_min+1)
	{
	  fclose(f);
	  sprintf(error,"I/O problem for file %s",filename);
	  VipPrintfError(error);
	  VipPrintfExit("VipWriteHisto");
	  return(PB);
	}
      i = fwrite(histo,sizeof(VipHisto),1,f);
      if(i!=1)
	{
	  fclose(f);
	  sprintf(error,"I/O problem for file %s",filename);
	  VipPrintfError(error);
	  VipPrintfExit("VipWriteHisto");
	  return(PB);
	}
    }
  else
    {
      for(i=histo->range_min;i<=histo->range_max;i++)
	{
	    /*old
	  j = fprintf(f,"%d\n",histo->val[i]);
	    */
	  j = fprintf(f,"%d %d\n",i,histo->val[i]);
	  if(j<=0)
	    {
	      fclose(f);
	      sprintf(error,"I/O problem for file %s",filename);
	      VipPrintfError(error);
	      VipPrintfExit("VipWriteHisto");
	      return(PB);
	    }
	}
      /*
      j = fprintf(f,"-1\n");
      j = fprintf(f,"%d\n",histo->range_min);
      if(j==0)
	  {
	      fclose(f);
	      sprintf(error,"I/O problem for file %s",filename);
	      VipPrintfError(error);
	      VipPrintfExit("VipWriteHisto");
	      return(PB);
	  }
      */
    }
  fclose(f);
  return(OK);
}

/*---------------------------------------------------------------*/
int VipFreeHisto(VipHisto *histo)
/*---------------------------------------------------------------*/
{
  if(histo==NULL)
    {
      VipPrintfError("NULL pointer in VipFreeHisto!");
      VipPrintfExit("VipFreeHisto");
      return(PB);
    }
  if(histo->histo_ad==NULL)
    {
      VipPrintfError("strange histo in VipFreeHisto!");
      VipPrintfExit("VipFreeHisto");
      return(PB);
    }
  VipFree(histo->histo_ad);
  VipFree(histo);
  return(OK);
}

/*---------------------------------------------------------------*/
int VipSetHistoVal(
VipHisto *histo,
int greylevel,
int value)
/*---------------------------------------------------------------*/
{
 if(histo==NULL)
    {
      VipPrintfError("NULL pointer in VipSetHistoVal!");
      VipPrintfExit("VipSetHistoVal");
      return(PB);
    }

 if((greylevel<histo->range_min)||(greylevel>histo->range_max))
   {
     VipPrintfError("Out of histogram range");
     VipPrintfExit("VipSetHistoVal");
     return(PB);
   }
 histo->val[greylevel] = value;
 return(OK);
}

/*---------------------------------------------------------------*/
int VipGetHistoVal(
VipHisto *histo,
int greylevel,
int *value)
/*---------------------------------------------------------------*/
{
 if(histo==NULL || value==NULL)
    {
      VipPrintfError("NULL pointer in VipGetHistoVal!");
      VipPrintfExit("VipGetHistoVal");
      return(PB);
    }

 if((greylevel<histo->range_min)||(greylevel>histo->range_max))
   {
     VipPrintfError("Out of histogram range");
     VipPrintfExit("VipGetHistoVal");
     return(PB);
   }
 *value = histo->val[greylevel];
 return(OK);
}

/*---------------------------------------------------------------*/
int VipGetHistoMin(
VipHisto *histo,
int *min)
/*---------------------------------------------------------------*/
{
  int i;
  int temp;
  int empty = VTRUE;
  
  if(histo==NULL || min==NULL)
    {
      VipPrintfError("NULL pointer in VipGetHistoMin!");
      VipPrintfExit("VipGetHistoMin");
      return(PB);
    }
  for(i=mVipHistoRangeMin(histo);i<=mVipHistoRangeMax(histo);i++)
    {
      temp = mVipHistoVal(histo,i);
      if(temp<0)
	{
	  VipPrintfError("Negative value in histogram!");
	  VipPrintfExit("VipGetHistoMax");
	}
      else if (temp>0)
	{
	  if(histo->min>temp) histo->min = temp;
	  empty = VFALSE;
	}
    }
  if(empty==VTRUE)
    {
      VipPrintfWarning("Empty histogram!");
    }
  *min = histo->min;
  return(OK);
}

/*---------------------------------------------------------------*/
int VipGetHistoMax(
VipHisto *histo,
int *max)
/*---------------------------------------------------------------*/
{
  int i;
  int temp;
  int empty = VTRUE;
  
  if(histo==NULL || max==NULL)
    {
      VipPrintfError("NULL pointer in VipGetHistoMax!");
      VipPrintfExit("VipGetHistoMax");
      return(PB);
    }
  for(i=mVipHistoRangeMax(histo);i>=mVipHistoRangeMin(histo);i--)
    {
      temp = mVipHistoVal(histo,i);
      if(temp<0)
	{
	  VipPrintfError("Negative value in histogram!");
	  VipPrintfExit("VipGetHistoMin");
	}
      else if (temp>0)
	{
	  if(histo->max<temp) histo->max = temp;
	  empty = VFALSE;
	}
    }
  if(empty==VTRUE)
    {
      VipPrintfWarning("Empty histogram!");
    }
  *max = histo->max;
  return(OK);
}

/*---------------------------------------------------------------*/
int VipGetHistoEntropy(
VipHisto *histo,
float *entropo)
/*---------------------------------------------------------------*/
{
  int i;
  int temp;
  int empty = VTRUE;
  double entropy, p;
  int total;
  
  if(histo==NULL || entropo==NULL)
    {
      VipPrintfError("NULL pointer in VipGetHistoEntropy!");
      VipPrintfExit("VipGetHistoEntropy");
      return(PB);
    }

  total = 0;
  for(i=mVipHistoRangeMax(histo);i>=mVipHistoRangeMin(histo);i--)
    {
      temp = mVipHistoVal(histo,i);
      if(temp<0)
	{
	  VipPrintfError("Negative value in histogram!");
	  VipPrintfExit("VipGetHistoEntropy");
	}
      else if (temp>0)
	{
	  total += temp;  
	  empty = VFALSE;
	}
    }

  if(empty==VTRUE)
    {
      VipPrintfWarning("Empty histogram!");
      VipPrintfExit("VipGetHistoEntropy");
      return(PB);
    }
  entropy = 0.;
  for(i=mVipHistoRangeMax(histo);i>=mVipHistoRangeMin(histo);i--)
      {
	  temp = mVipHistoVal(histo,i);
	  if(temp>0)
	      {
		  p = (double)(temp)/(double)(total);
		  entropy -= p * log(p);
		  /*printf("val:%d, n:%d, +:%f\n",i,temp,-p*log(p));*/
	      }
      }

  *entropo = (float)entropy / log(2.);
  return(OK);
}
