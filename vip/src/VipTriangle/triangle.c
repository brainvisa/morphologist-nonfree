#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/util.h>
#include <vip/math.h>


#define ICM 11
#define ANNEALING 12
/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*---------------------------------------------------------------------------*/
static int MinimizeTriangle(Volume *vol, int nb_iterations, float Tinit, float geom,  int algo);
/*---------------------------------------------------------------------------*/


int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char output[VIP_NAME_MAXLEN] = "triangle";
  int niterations = 300000;
  int i;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-niter", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  niterations = atoi(argv[i]);
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  vol = VipCreate3DVolume (100,100,5,1.,1.,1.,S16BIT,"triangle",1);
  if(!vol) return(PB);
  
  MinimizeTriangle(vol, niterations, 5., 0.99999, ANNEALING);
      
  VipWriteTivoliVolume(vol,output);

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
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"triangle\")}]\n");
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
static int MinimizeTriangle(Volume *vol, int nb_iterations, 
float Tinit, float geom,  int algo)
/*---------------------------------------------------------------------------*/
{
  int BORDER_LABEL = 0;
  Vip_S16BIT *data_ptr, *data_first, *ptr;
  VipOffsetStruct *data_vos;
  float T;
  int ix, iy, iz, n;
  int nmodif;
  int currentlabel, newlabel;
  double deltaU, proba;
  RandomBox *rbox, *rboxptr;
  int volsize;
  float pot[2][2][2][2];
  int i,j,k,l;
  int a,b,c,d,e,f,g,h;
  float Kreplica = 0;
  int zreplica;
  int labelreplica;
  int n1, n0;
  double Ufinal[10000];
  int tunnel[401][401], itunnel[401];
  int ntunnel = 0;
  RandomBox *rboxtunnel, *rboxptrtunnel;
  int decal, imin, imax;;


  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("MinimizeTriangle");
      return(PB);
    }
  if(Tinit<0 || Tinit>100000)
    {
      VipPrintfError("Unconsistent Initial temperature");
      VipPrintfExit("MinimizeTriangle");
      return(PB);
    }
  if(geom<0.00001 || geom>10)
    {
      VipPrintfError("Unconsistent geometric reason for annealing");
      VipPrintfExit("MinimizeTriangle");
      return(PB);
    }
  for(i=0;i<=1;i++)   for(j=0;j<=1;j++)   for(k=0;k<=1;k++)   for(l=0;l<=1;l++) 
    pot[i][j][k][l] = 1.;
  pot[0][0][0][0] = 0.;
  pot[1][1][1][1] = 0.;
  pot[1][1][0][0] = 0.;
  pot[0][1][0][1] = 0.;
  pot[1][0][1][1] = 0.;
  pot[0][0][1][0] = 0.;

  rbox = VipCreateRandomBoxCircularList( 1111111, 2);
  if(rbox==PB)	
    {
      VipPrintfExit("MinimizeTriangle");
      return(PB);
    }
  rboxptr = rbox;

  if(mVipVolBorderWidth(vol)<=0) 
    {
      VipPrintfError("Null border!");
      return(PB);
    }

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  data_ptr = data_first;

  /*initialisation*/

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
	       *data_ptr = rboxptr->label;
	       data_ptr++;
	       rboxptr = rboxptr->next;
	    }
	  data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	}
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
    }

  VipSetBorderLevel(vol,BORDER_LABEL);

  /*interractions entre duplica*/

  rboxtunnel = VipCreateRandomBoxCircularList( 11111, mVipVolSizeZ(vol)-1);
  if(rboxtunnel==PB)	
    {
      VipPrintfExit("MinimizeTriangle");
      return(PB);
    }
  rboxptrtunnel = rboxtunnel;

  for(i=0;i<mVipVolSizeZ(vol);i++)
    {
      itunnel[i]=0;
      for(j=0;j<mVipVolSizeZ(vol);j++)
	tunnel[i][j] = 0;
    }

  /* NB, on peut avoir plusieurs fois la meme interaction*/
  if(ntunnel >0)
    {
      for(i=1;i<mVipVolSizeZ(vol);i++)
	for(j=0;j<ntunnel;j++)
	  {
	    rboxptrtunnel = rboxptrtunnel->next;
	    newlabel = rboxptrtunnel->label + 1;
	    while (i==newlabel)
	      {
		rboxptrtunnel = rboxptrtunnel->next;	
		newlabel = rboxptrtunnel->label + 1;
	      }
	    imin = mVipMin(i,newlabel);
	    imax = mVipMax(i,newlabel);
	    decal = imax - imin;
	    tunnel[imin][itunnel[imin]++] = decal * data_vos->oSlice;
	    tunnel[imax][itunnel[imax]++] = -decal * data_vos->oSlice;
	  }
    }


  volsize = mVipVolSizeX(vol)*mVipVolSizeY(vol)*mVipVolSizeZ(vol);
  printf("Number of points : %d\n",volsize);  

  /*Minimisation*/

  nmodif = 1;
  T = Tinit;
  for(n=1;(n<=nb_iterations)&&(nmodif>0);n++)
    {
      if(nmodif<(mVipVolSizeZ(vol)) && nmodif>1) algo=ICM;
      Kreplica += geom;
      T *= geom;
      nmodif = 0;
      data_ptr = data_first;
      for ( iz = 0; iz< mVipVolSizeZ(vol); iz++ )               /* loop on slices */
	{
	  for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	    {
	      for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		{
		  if(iz == 0)
		    {
		      n1 = 0;
		      n0 = 0;
		      for(zreplica = 1;zreplica<mVipVolSizeZ(vol);zreplica++)
			{
			  if(*(data_ptr + zreplica*data_vos->oSlice)==0) n0++;
			  else n1++;
			}
		      if (n1>n0) *data_ptr = 1;
		      else *data_ptr = 0;
		    }
		  else if(*data_ptr!=rboxptr->label)
		    {
		      currentlabel = *data_ptr;
		      newlabel = rboxptr->label;
		      rboxptr = rboxptr->next;

		      deltaU = 0.;
		      ptr = data_ptr-1-data_vos->oLine;
		      a = *ptr++;
		      b = *ptr++;
		      c = *ptr;
		      d = *(data_ptr-1);
		      e = *(data_ptr+1);
		      ptr = data_ptr-1+data_vos->oLine;
		      f = *ptr++;
		      g = *ptr++;
		      h = *ptr;
		      deltaU += pot[a][b][d][newlabel] - pot[a][b][d][currentlabel];
		      deltaU += pot[b][c][newlabel][e] - pot[b][c][currentlabel][e];
		      deltaU += pot[d][newlabel][f][g] - pot[d][currentlabel][f][g];
		      deltaU += pot[newlabel][e][g][h] - pot[currentlabel][e][g][h];
		      
		      for (i=0;i<2*ntunnel;i++)
			{
			  labelreplica = *(data_ptr + tunnel[iz][i]);
			  if (labelreplica==newlabel) deltaU -= Kreplica;
			  else deltaU += Kreplica;
			     
			}
		      
		      if(algo==ICM)
			{
			  if(deltaU<0)
			    {
			      *data_ptr = newlabel;
			      nmodif++;
			    } 
			}
		      else
			{
			  if((-deltaU/T)<-70) proba = exp(-70.);
			  else if ((-deltaU/T)>70) proba = exp(70.);
			  else proba=  exp(-deltaU/T);
			  proba = proba / (1+proba);
			  rboxptr = rboxptr->next;
			  if (proba > rboxptr->proba)
			    {
			      *data_ptr = newlabel;
			      nmodif++;
			    }
			}
		    }
		    
		  rboxptr = rboxptr->next;
		  data_ptr++;
		}
	      data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	    }
	  data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	}
      printf("iteration %d/%d, modif: %d, Kreplica: %f, T:%f\n",n,nb_iterations,nmodif,Kreplica,T);
      VipWriteTivoliVolume(vol,"temp");
    }
   
  data_ptr = data_first;
  for ( iz = 0; iz< mVipVolSizeZ(vol); iz++ )               /* loop on slices */
    {
      Ufinal[iz]=0.;
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
	      if(ix!=0 && iy!=0)
		{
		  currentlabel = *data_ptr;
		  ptr = data_ptr-1-data_vos->oLine;
		  a = *ptr++;
		  b = *ptr++;
		  d = *(data_ptr-1);
		  Ufinal[iz] += pot[a][b][d][currentlabel];
		}
	      data_ptr++;
	    }
	  data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	}
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/

      printf("%f\n", Ufinal[iz]);
    }
  
  return(OK);
}



