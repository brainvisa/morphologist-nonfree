#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/util.h>
#include <vip/bucket.h>
#include <vip/histo.h>	
#include <vip/math.h>	


#define ICM 11
#define ANNEALING 12
/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/
double  DataPotentiel(float K, int label, int data);
double ComputeDeltaEntropy(float K, VipHisto *h, int old, int new, int volsize);
/*---------------------------------------------------------------------------*/
Volume *MinimizeEntropy(Volume *vol, int nb_iterations, float KPOTTS, int connectivity,
			float Tinit, float geom,  int maxincrement, int algo);
/*---------------------------------------------------------------------------*/


int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(classif); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "mininfo";
  int niterations = 10;

  /* temporary stuff */
  int readlib, writelib;
 
  float KPOTTS =0.001;
  int i;
  int borderwidth = 2;

  readlib = ANY_FORMAT;
  writelib = TIVOLI;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else if(argv[i][0]=='a') readlib = ANY_FORMAT;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipMinimizeInformation");
	      return(PB);
	    }
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-niter", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  niterations = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipClosing");
	      return(PB);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipMinimizeInformation");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(PB);
   }

  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,borderwidth);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,borderwidth);
  else if (readlib == VIDA)
    vol = VipReadVidaVolumeWithBorder(input,borderwidth);
  else
    vol = VipReadVolumeWithBorder(input,borderwidth);

  if(vol==NULL) return(PB);
   
  classif = MinimizeEntropy(vol, niterations, KPOTTS, CONNECTIVITY_8, 0.10, 0.98, 5, ANNEALING);
      
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(classif,output)==PB) return(PB);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(classif,output)==PB) return(PB);
    }
  else
    if(VipWriteVolume(classif,output)==PB) return(PB);

  return(OK);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipMinimizeInformation\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-a[lgorithm]] { i[cm] or a[nnealing] (default:i)\n");
  (void)fprintf(stderr,"        [-T[init]] <float> \n");
  (void)fprintf(stderr,"        [-g[eom]] <float> \n");
  (void)fprintf(stderr,"        [-n[iter]] <int> \n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"mininfo\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(PB);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("to be writen");
  return(PB);

}

/******************************************************/
/*---------------------------------------------------------------------------*/
Volume *MinimizeEntropy(Volume *vol, int nb_iterations, float KPOTTS, int connectivity,
float Tinit, float geom,  int maxincrement, int algo)
/*---------------------------------------------------------------------------*/
{
  int BORDER_LABEL = -100;
  Volume *classif;
  Vip_S16BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  float T;
  int ix, iy, iz, n;
  int nmodif;
  int currentlabel, newlabel[10000], new;
  double deltaU[10000];
  int i;
  int besti;
  double minU;
  RandomBox *rbox, *rboxptr;
  double proba_tab[10000],locZ,probasum;
  float Kdata = 0.001;
  float Kentropy = 5000.;
  VipHisto *histo, *hcurrent;
  float entropy;
  int volsize;


  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("MinimizeEntropy");
      return(PB);
    }
  if(Tinit<0 || Tinit>100000)
    {
      VipPrintfError("Unconsistent Initial temperature");
      VipPrintfExit("MinimizeEntropy");
      return(PB);
    }
  if(geom<0.01 || geom>=1)
    {
      VipPrintfError("Unconsistent geometric reason for annealing");
      VipPrintfExit("MinimizeEntropy");
      return(PB);
    }

  rbox = VipCreateRandomBoxCircularList( 10101, 2);
  if(rbox==PB)	
    {
      VipPrintfExit("MinimizeEntropy");
      return(PB);
    }
  rboxptr = rbox;

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  if(mVipVolBorderWidth(classif)<=0) 
    {
      VipPrintfError("Null border!");
      return(PB);
    }

  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_S16BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
	       *classif_ptr = *data_ptr;
	       data_ptr++;
	       classif_ptr++;
	    }
	  classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	  data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	}
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
    }

  VipSetBorderLevel(classif,BORDER_LABEL);
  vcs = VipGetConnectivityStruct( classif, connectivity );
  KPOTTS /= vcs->nb_neighbors; /* Convention to simplify user interface feeling */

  histo = VipComputeVolumeHisto(vol); 
  if(histo==PB) 
    {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }
  hcurrent = VipCreateHisto(0,  mVipHistoRangeMax(histo)*10);
  if(hcurrent==PB) 
    {
      VipPrintfExit("(VipComputeBiasField");
      return(PB);
    }

  for(i=0;i<=mVipHistoRangeMax(hcurrent);i++)
    {
      VipSetHistoVal(hcurrent,i,0);
    }
  for(i=0;i<=mVipHistoRangeMax(histo);i++)
    {
      VipSetHistoVal(hcurrent,i,mVipHistoVal(histo,i));
    }

  volsize = mVipVolSizeX(vol)*mVipVolSizeY(vol)*mVipVolSizeZ(vol);
  printf("Number of points : %d\n",volsize);  
  VipGetHistoEntropy(histo,&entropy);
  printf("Initial entropy:  %f\n",entropy);


  /*Minimisation*/

  nmodif = -1;
  T = Tinit;
  for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
    {
      T *= geom;
      nmodif = 0;
      data_ptr = data_first;
      classif_ptr = classif_first;
      for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	{
	  for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	    {
	      for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		{
		  if(rboxptr->label==1)
		    {
		      currentlabel = *classif_ptr;
		      for (i=0;i<=(2*maxincrement+1);i++)
			{
			  newlabel[i] = currentlabel + i - maxincrement;
			  new = newlabel[i];
			  if(new<0) deltaU[i] = 100000000.;
			  else if(new==currentlabel) deltaU[i]=0.;
			  else
			    {
			      deltaU[i] = 0.;
			      /*
			      for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
				{
				  neighbor_label = *(classif_ptr + vcs->offset[icon]);
				  if(currentlabel==neighbor_label)
				    {
				      deltaU[i] += KPOTTS;
				    }
				  else if (new==neighbor_label) 
				    {
				      deltaU[i] -= KPOTTS;
				    }
				}
			      */
			      deltaU[i] +=DataPotentiel(Kdata,newlabel[i],*data_ptr)
				- DataPotentiel(Kdata,currentlabel,*data_ptr);
			      deltaU[i] += ComputeDeltaEntropy(Kentropy, hcurrent, currentlabel, newlabel[i], volsize);	           
			    }
			}
		      if(algo==ICM)
			{
			  besti = maxincrement;
			  minU = deltaU[besti];
			  for (i=0;i<=(2*maxincrement+1);i++)
			    {
			      if(deltaU[i]<minU)
				{
				  besti = i;
				  minU = deltaU[besti];
				}
			    }
			}
		      else
			{
			  locZ=0.;
			  for (i = 0; i<= (2*maxincrement+1); i++)
			    {
			      if((-deltaU[i]/T)<-70) proba_tab[i] = exp(-70.);
			      else if ((-deltaU[i]/T)>70) proba_tab[i] = exp(70.);
			      else proba_tab[i]=  exp(-deltaU[i]/T);
			      locZ += proba_tab[i];
			    }
			  for (i = 0; i<= (2*maxincrement+1); i++)
			    proba_tab[i] /= locZ;

			  probasum = proba_tab[0];
			  besti = 0;
			  while(rboxptr->proba>=probasum && (besti<=(2*maxincrement+1)))
			    {
			      besti++;
			      probasum += proba_tab[besti];
			    }
			  rboxptr = rboxptr->next;
			}
		      if (newlabel[besti] != currentlabel && newlabel[besti]>=0)
			{
			  nmodif++;
			  *classif_ptr = newlabel[besti];
			  /*
			  entropy += ComputeDeltaEntropy(Kentropy, hcurrent, 
			  currentlabel, newlabel[besti], volsize) / Kentropy/ log(2.); */
			  /*printf("New entropy:  %f\n",entropy);*/
			  VipSetHistoVal(hcurrent,currentlabel,mVipHistoVal(hcurrent,currentlabel)-1);
			  VipSetHistoVal(hcurrent,newlabel[besti],mVipHistoVal(hcurrent,newlabel[besti])+1);
			}
		      /*printf("\n");*/
		    }
		  rboxptr = rboxptr->next;
		  data_ptr++;
		  classif_ptr++;
		}
	      classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	      data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	    }
	  classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	  data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	}
      printf("iteration %d/%d, modif: %d, T: %f\n",n,nb_iterations,nmodif,T);
      /*printf("New cumul  :  %f\n",entropy);*/
      VipGetHistoEntropy(hcurrent,&entropy);
      printf("New entropy:  %f\n",entropy);
    }

  VipFreeConnectivityStruct(vcs);

  return(classif);
}


double  DataPotentiel(float K, int label, int data)
{
  double P;
  
  P = label - data;
  P *= P;
  P *=K;
  return(P);
}

double ComputeDeltaEntropy(float K, VipHisto *h, int old, int new, int volsize)
{
  double delta;
  int oldni,newni;
  double oldpi, newpi;

  if (new==old)
    delta = 0.;
  else
    {
      oldni = mVipHistoVal(h,old);
      newni = oldni -1;
      oldpi = (double)(oldni)/(double)volsize;
      if(oldni==1) newpi=0.00000000001;
      else newpi = (double)(newni)/(double)volsize;
      delta = ( oldpi * log(oldpi) - newpi * log(newpi));
      /*      printf("%f %f ",oldpi, newpi);*/

      oldni = mVipHistoVal(h,new);
      newni = oldni +1;
      if(oldni==0) oldpi=0.00000000001;
      else oldpi = (double)(oldni)/(double)volsize;
      newpi = (double)(newni)/(double)volsize;
      delta += ( oldpi * log(oldpi) - newpi * log(newpi));
      /*      printf("%f %f ",oldpi, newpi);
	      printf("%f\n",delta);*/
      delta *= K;
    }

  return(delta);
}
