/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : skeleton             * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 12/06/99
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : homotopic geodesic dilation
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

/*--------------------------------------------------------------------*/
/*Points become immortals according to immortal_elixir*/
/*The front is managed according to front_mode*/


#include <vip/math.h>
#include <vip/skeleton.h>
#include <vip/morpho.h>
#include <vip/distmap.h>
#include <vip/skeleton_static.h>

/*---------------------------------------------------------------------------*/
static float deltaGPotentialWtoG(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static float deltaFoetusGPotentialWtoG(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW);
/*---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
int VipFrontOrderFromDistanceToClosing(VipIntBucket *buck, Volume *vol,
				       int front_value, int object, int domain) ;
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipHomotopicInsideDilationSnake( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode,
			       float KISING, float mG, float sigmaG, float mW, float sigmaW)
/*-------------------------------------------------------------------------*/
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, totalcount;
  Vip_S16BIT *first, *ptr, *gptr, *voisin, *gfirst;
  int *buckptr;
  int i;
  float KW, KG;
  int nnG, nnW;
  float deltaU;
  int icon;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicInsideDilationSnake is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
      return(PB);
    }
  if (VipVerifyAll(graylevel)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
      return(PB);
    }
  if (VipTestType(graylevel,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicInsideDilationSnake is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicInsideDilationSnake is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
      return(PB);
    }
  if(VipTestEqualBorderWidth(vol,graylevel)==PB)
    {
      VipPrintfError("Sorry, border widths are different");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
      return(PB);
    }
  /*
    printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
  */
  printf("Homotopic snake dilation towards inside...\n");
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForDilation( vol, CONNECTIVITY_6, VIP_FRONT, object, inside, front_mode);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);
  gfirst = VipGetDataPtr_S16BIT(graylevel);

  /*main loop*/
  loop=0;
  count = 1;
  totalcount = 0;
  KW = 2*KISING - 0.00001;
  KG = 4*KISING + 0.00001;
  /*
    printf("gray matter (mean:%f, sigma:%f), white matter (mean:%f, sigma:%f)\n",
    mG,sigmaG,mW,sigmaW);
  */
  printf("loop: %3d, Added: %6d",loop,0);
  while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
    {
      if(loop==1) count=0;
      totalcount += count;
      count = 0;
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Added: %6d",loop,totalcount);
      fflush(stdout);
	  
      buckptr = buck->data;
      for(i=buck->n_points;i--;)
	{
	  ptr = first + *buckptr;
	  gptr = gfirst + *buckptr++;
	  nnG = 0;
	  nnW = 0;
	  deltaU = 0;
	  for ( icon=0;icon<vcs6->nb_neighbors;icon++)
	    {
	      voisin = ptr + vcs6->offset[icon];	      
	      if(*voisin==object || *voisin==outside) nnG++;
	      else nnW++;
	    }
	  deltaU = -KISING*(nnG-nnW);
	  deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
	  if(deltaU<=0)
	    {
	      if (VipIsASimplePointForLabel_S16BIT(topo26, ptr, object)==VTRUE)
		{
		  *ptr = object;
		  count++;
		}
	    }
	}
	      	      			
      VipFillNextFrontFromOldFrontForDilation(first,buck,nextbuck,vcs6,inside,VIP_FRONT,object);
		  
      /*bucket permutation*/
      VipPermuteTwoIntBucket(&buck, &nextbuck);
      nextbuck->n_points = 0;
	  
    }

  printf("\n");
  
  VipChangeIntLabel(vol,VIP_FRONT,inside);
  
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}      

/*---------------------------------------------------------------------------*/
int VipHomotopicInsideDilationSnakeRidge( Volume *ridge, Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode,
			       float KISING, float mG, float sigmaG, float mW, float sigmaW)
/*-------------------------------------------------------------------------*/
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, totalcount;
  Vip_S16BIT *first, *ptr, *gptr, *voisin, *gfirst, *rptr, *rfirst;
  int *buckptr;
  int i;
  float KW, KG;
  int nnG, nnW;
  float deltaU;
  int icon;


  if (VipVerifyAll(ridge)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnakeRidge");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicInsideDilationSnake is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnakeRidge");
      return(PB);
    }
  if (VipVerifyAll(graylevel)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnakeRidge");
      return(PB);
    }
  if (VipTestType(graylevel,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicInsideDilationSnake is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnakeRidge");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicInsideDilationSnake is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnakeRidge");
      return(PB);
    }
  if(VipTestEqualBorderWidth(vol,graylevel)==PB)
    {
      VipPrintfError("Sorry, border widths are different");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnakeRidge");
      return(PB);
    }
  /*
    printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
  */
  printf("Homotopic snake dilation towards inside...\n");
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForDilation( vol, CONNECTIVITY_6, VIP_FRONT, object, inside, front_mode);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);
  gfirst = VipGetDataPtr_S16BIT(graylevel);
  rfirst = VipGetDataPtr_S16BIT(ridge);

  /*main loop*/
  loop=0;
  count = 1;
  totalcount = 0;
  KW = 2*KISING - 0.00001;
  KG = 4*KISING + 0.00001;
  /*
    printf("gray matter (mean:%f, sigma:%f), white matter (mean:%f, sigma:%f)\n",
    mG,sigmaG,mW,sigmaW);
  */
  printf("loop: %3d, Added: %6d",loop,0);
  while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
    {
      if(loop==1) count=0;
      totalcount += count;
      count = 0;
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Added: %6d",loop,totalcount);
      fflush(stdout);
	  
      buckptr = buck->data;
      for(i=buck->n_points;i--;)
	{
	  ptr = first + *buckptr;
          rptr = rfirst + *buckptr;
	  gptr = gfirst + *buckptr++;
	  nnG = 0;
	  nnW = 0;
	  deltaU = 0;
          if(!*rptr)
            {
              for ( icon=0;icon<vcs6->nb_neighbors;icon++)
                {
                  voisin = ptr + vcs6->offset[icon];	      
                  if(*voisin==object || *voisin==outside) nnG++;
                  else nnW++;
                }
              deltaU = -KISING*(nnG-nnW);
              deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
              if(deltaU<=0)
                {
                  if (VipIsASimplePointForLabel_S16BIT(topo26, ptr, object)==VTRUE)
                    {
                      *ptr = object;
                      count++;
                    }
                }
            }
	}
	      	      			
      VipFillNextFrontFromOldFrontForDilation(first,buck,nextbuck,vcs6,inside,VIP_FRONT,object);
		  
      /*bucket permutation*/
      VipPermuteTwoIntBucket(&buck, &nextbuck);
      nextbuck->n_points = 0;
	  
    }

  printf("\n");
  
  VipChangeIntLabel(vol,VIP_FRONT,inside);
  
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}      

/*---------------------------------------------------------------------------*/
int VipHomotopicInsideDilation( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode )
/*-------------------------------------------------------------------------*/
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, totalcount;
  Vip_S16BIT *first, *ptr, *gptr, *gfirst;
  int *buckptr;
  int i, m;
  int max, min;
  float interval;


  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicInsideDilation");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicInsideDilation is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilation");
      return(PB);
    }
    if (VipVerifyAll(graylevel)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicInsideDilation");
      return(PB);
    }
    if (VipTestType(graylevel,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicInsideDilation is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilation");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicInsideDilation is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilation");
      return(PB);
    }
    if(VipTestEqualBorderWidth(vol,graylevel)==PB)
    {
      VipPrintfError("Sorry, border widths are different");
      VipPrintfExit("(skeleton)VipHomotopicInsideDilation");
      return(PB);
    }
  /*
    printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
  */
  printf("Homotopic dilation towards inside...\n");
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForDilation( vol, CONNECTIVITY_6, VIP_FRONT, object, inside, front_mode);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);
  gfirst = VipGetDataPtr_S16BIT(graylevel);

  loop=0;
  count = 1;
  totalcount = 0;
  printf("loop: %3d, Added: %6d",loop,0);
  
  while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
  {
      if(loop==1) count=0;
      totalcount += count;
      count = 0;
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Added: %6d",loop,totalcount);
      fflush(stdout);
      
      buckptr = buck->data;
      for(i=buck->n_points;i--;)
      {
          ptr = first + *buckptr;
          gptr = gfirst + *buckptr++;
          if(*gptr<200)
          {
              if (VipIsASimplePointForLabel_S16BIT(topo26, ptr, object)==VTRUE)
              {
                  *ptr = object;
                  count++;
              }
          }
      }
      
      VipFillNextFrontFromOldFrontForDilation(first,buck,nextbuck,vcs6,inside,VIP_FRONT,object);
      
      /*bucket permutation*/
      VipPermuteTwoIntBucket(&buck, &nextbuck);
      nextbuck->n_points = 0;
  }
  
  printf("\n");
  
  VipChangeIntLabel(vol,VIP_FRONT,inside);
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipHomotopicInsideVolumeDilation( Volume *vol1, Volume *vol2, int nb_iteration,
                                      int domain, int outside_domain, int front_mode )
/*-------------------------------------------------------------------------*/
{
    VipIntBucket *buck, *nextbuck;
    Topology26Neighborhood *topo26;
    VipConnectivityStruct *vcs6;
    int loop, count, totalcount;
    Vip_S16BIT *first1, *ptr1, *ptr2, *first2;
    int *buckptr;
    int i;


    if (VipVerifyAll(vol1)==PB && VipVerifyAll(vol2)==PB)
    {
        VipPrintfExit("(skeleton)VipHomotopicInsideVolumeDilation");
        return(PB);
    }
    if (VipTestType(vol1,S16BIT)!=OK && VipTestType(vol2,S16BIT)!=OK)
    {
        VipPrintfError("Sorry,  VipHomotopicInsideVolumeDilation is only implemented for S16BIT volume");
        VipPrintfExit("(skeleton)VipHomotopicInsideVolumeDilation");
        return(PB);
    }
    if (mVipVolBorderWidth(vol1) < 1)
    {
        VipPrintfError("Sorry, VipHomotopicInsideVolumeDilation is only implemented with border");
        VipPrintfExit("(skeleton)VipHomotopicInsideVolumeDilation");
        return(PB);
    }
    if(VipTestEqualBorderWidth(vol1,vol2)==PB)
    {
        VipPrintfError("Sorry, border widths are different");
        VipPrintfExit("(skeleton)VipHomotopicInsideVolumeDilation");
        return(PB);
    }
    
    
    printf("Homotopic dilation...\n");
    VipSetBorderLevel( vol1, outside_domain );
    
    buck = VipCreateFrontIntBucketForDilation( vol1, CONNECTIVITY_6, VIP_FRONT, domain, outside_domain, front_mode);
    if(buck==PB) return(PB);
    nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(nextbuck==PB) return(PB);

    topo26 = VipCreateTopology26Neighborhood( vol1 );
    if(topo26==PB) return(PB);

    vcs6 = VipGetConnectivityStruct( vol1, CONNECTIVITY_6 );

    nextbuck->n_points = 0;

    first1 = VipGetDataPtr_S16BIT(vol1);
    first2 = VipGetDataPtr_S16BIT(vol2);
     
    /*main loop*/
    loop=0;
    count = 1;
    totalcount = 0;
    
    printf("loop:   , Added:      ");
    while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
    {
        totalcount += count;
        count = 0;
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Added: %5d",loop,totalcount);
        fflush(stdout);
        
        buckptr = buck->data;
        for(i=buck->n_points;i--;)
        {
            ptr1 = first1 + *buckptr;
            ptr2 = first2 + *buckptr;
            if(*ptr2==domain)
            {
                if (VipComputeTopologicalClassificationForLabel_S16BIT(topo26,ptr1,outside_domain)!=TOPO_VOLUME_POINT)
                {
                    *ptr1 = domain;
                    count++;
                }
            }
            buckptr++;
        }
        
        VipFillNextFrontFromOldFrontForDilation(first1,buck,nextbuck,vcs6,outside_domain,VIP_FRONT,domain);
        
        /*bucket permutation*/
        VipPermuteTwoIntBucket(&buck, &nextbuck);
        nextbuck->n_points = 0;
    }
    
    printf("\n");
    
    VipChangeIntLabel(vol1,VIP_FRONT,outside_domain);
    
    VipFreeIntBucket(buck);
    VipFreeIntBucket(nextbuck);
    
    return(OK);
}

/*---------------------------------------------------------------------------*/
int VipFoetusHomotopicInsideDilationSnake( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode,
			       float KISING, float mG, float sigmaG, float mW, float sigmaW)
/*-------------------------------------------------------------------------*/
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, totalcount;
  Vip_S16BIT *first, *ptr, *gptr, *voisin, *gfirst;
  int *buckptr;
  int i;
  float KW, KG;
  int nnG, nnW;
  float deltaU;
  int icon;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipFoetusHomotopicInsideDilationSnake");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipFoetusHomotopicInsideDilationSnake is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipFoetusHomotopicInsideDilationSnake");
      return(PB);
    }
  if (VipVerifyAll(graylevel)==PB)
    {
      VipPrintfExit("(skeleton)VipFoetusHomotopicInsideDilationSnake");
      return(PB);
    }
  if (VipTestType(graylevel,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipFoetusHomotopicInsideDilationSnake is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipFoetusHomotopicInsideDilationSnake");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipFoetusHomotopicInsideDilationSnake is only implemented with border");
      VipPrintfExit("(skeleton)VipFoetusHomotopicInsideDilationSnake");
      return(PB);
    }
  if(VipTestEqualBorderWidth(vol,graylevel)==PB)
    {
      VipPrintfError("Sorry, border widths are different");
      VipPrintfExit("(skeleton)VipFoetusHomotopicInsideDilationSnake");
      return(PB);
    }
  /*
    printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
  */
  printf("Homotopic snake dilation towards inside...\n");
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForDilation( vol, CONNECTIVITY_6, VIP_FRONT, object, inside, front_mode);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
  /*vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );*/

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);
  gfirst = VipGetDataPtr_S16BIT(graylevel);

  /*main loop*/
  loop=0;
  count = 1;
  totalcount = 0;
  /*
  KW = 9*KISING - 0.001;
  KG = 18*KISING + 0.001;
  */
   KW = 2*KISING - 0.00001;
     KG = 4*KISING + 0.00001;
  
  /*
    printf("gray matter (mean:%f, sigma:%f), white matter (mean:%f, sigma:%f)\n",
    mG,sigmaG,mW,sigmaW);
  */
  printf("loop: %3d, Added: %6d",loop,0);
  while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
    {
      if(loop==1) count=0;
      totalcount += count;
      count = 0;
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Added: %6d",loop,totalcount);
      fflush(stdout);
	  
      buckptr = buck->data;
      for(i=buck->n_points;i--;)
	{
	  ptr = first + *buckptr;
	  gptr = gfirst + *buckptr++;
	  nnG = 0;
	  nnW = 0;
	  deltaU = 0;
	  for ( icon=0;icon<vcs6->nb_neighbors;icon++)
	    {
	      voisin = ptr + vcs6->offset[icon];	      
	      if(*voisin==object || *voisin==outside) nnG++;
	      else nnW++;
	    }
	  deltaU = -KISING*(nnG-nnW);
	  deltaU += deltaFoetusGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
	  if(deltaU<=0)
	    {
	      if (VipIsASimplePointForLabel_S16BIT(topo26, ptr, object)==VTRUE)
		{
		  *ptr = object;
		  count++;
		}
	    }
	}
	      	      			
      VipFillNextFrontFromOldFrontForDilation(first,buck,nextbuck,vcs6,inside,VIP_FRONT,object);
		  
      /*bucket permutation*/
      VipPermuteTwoIntBucket(&buck, &nextbuck);
      nextbuck->n_points = 0;
	  
    }

  printf("\n");
  
  VipChangeIntLabel(vol,VIP_FRONT,inside);
  
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}    


/*---------------------------------------------------------------------------*/
static float deltaGPotentialWtoG(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW)
/*---------------------------------------------------------------------------*/
{
    float potG, potW;

    if(glevel<=(mG+sigmaG)) potG = -1.;
    else if(glevel>=(mG+2*sigmaG)) potG = 0.;
    else potG = -1. + (glevel-mG-sigmaG)/sigmaG;

    if(glevel<(mW-2*sigmaW)) potW = 0.;
    else if(glevel>(mW-sigmaW)) potW = -1.;
    else potW = -1. - (glevel-mW+sigmaW)/sigmaW;

    return(KG*potG-KW*potW);
}

/*---------------------------------------------------------------------------*/
static float deltaFoetusGPotentialWtoG( int glevel,float KG,float mG,
                                        float sigmaG,
                                        float KW,float mW,float sigmaW)
/*---------------------------------------------------------------------------*/
{
    float potG, potW;

    /*
    if(glevel<=(mG+sigmaG)) potG = -1.;
    else if(glevel>=(mG+2*sigmaG)) potG = 0.;
    else potG = -1. + (glevel-mG-sigmaG)/sigmaG;
    
    if(glevel<(mW-2*sigmaW)) potW = 0.;
    else if(glevel>(mW-sigmaW)) potW = -1.;
    else potW = -1. - (glevel-mW+sigmaW)/sigmaW;
    */
    /*   
    if(glevel<(mW-2*sigmaW)) potG = -1.;
    else if (glevel<(mW-1*sigmaW)) potG = -1. + (glevel-mW +2*sigmaW)/sigmaW;
    else if (glevel<mW+sigmaW) potG = 0.;
    else if (glevel<(mW+2*sigmaW)) potG = -1. - (glevel-mW -2*sigmaW)/sigmaW;
    else potG = -1.;

    if(glevel<(mW-2*sigmaW)) potW = 0.;
    else if (glevel<(mW-1*sigmaW)) potW = -1. - (glevel-mW +sigmaW)/sigmaW;
    else if (glevel<mW+sigmaW) potW = -1;
    else if (glevel<(mW+2*sigmaW)) potW = -1. + (glevel-mW -sigmaW)/sigmaW;
    else potW = 0.;
    */
    
    if (glevel<mW+sigmaW) potG = 0.;
    else if (glevel<(mW+2*sigmaW)) potG = -1. - (glevel-mW -2*sigmaW)/sigmaW;
    else potG = -1.;
    
    if (glevel<mW+sigmaW) potW = -1;
    else if (glevel<(mW+2*sigmaW)) potW = -1. + (glevel-mW -sigmaW)/sigmaW;
    else potW = 0.;

    return(KG*potG-KW*potW);
}


/*---------------------------------------------------------------------------*/
int VipHomotopicGeodesicDilation( Volume *vol, int nb_iteration,
				  int object, int domain, int forbiden, int outside, int front_mode )
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, totalcount;
  Vip_S16BIT *first, *ptr;
  int *buckptr;
  int i;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicGeodesicDilation");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicGeodesicDilation is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicGeodesicDilation");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicGeodesicDilation is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicGeodesicDilation");
      return(PB);
    }
  /*
  printf("Initialization (object:%d, domain:%d, forbiden:%d, outside:%d)...\n",object,domain,forbiden,outside);
  */
  printf("Homotopic dilation...\n");
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForDilation( vol, CONNECTIVITY_6, VIP_FRONT, object, domain, front_mode);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);
  nextbuck->n_points = 0;

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
  if(vcs6==PB) return(PB);


  first = VipGetDataPtr_S16BIT(vol);

  /*main loop*/
  loop=0;
  count = 1;
  totalcount = 0;
  printf("loop: %3d, Added %6d",loop,0);	      	  

  while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
      {
	if(loop==1) count=0;
	totalcount += count;
	count = 0;
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Added %6d",loop,totalcount);
	  fflush(stdout);
	  count = 0;
	  /* printf("Front size: %d\n",buck->n_points);*/
	  
	  buckptr = buck->data;
	  for(i=buck->n_points;i--;)
	    {
	      ptr = first + *buckptr++;		 

	      if (VipIsASimplePointForLabel_S16BIT(topo26, ptr, object)==VTRUE)
		  {
		      *ptr = object;
		      count++;
		  }
	    }
	      	      			
	  VipFillNextFrontFromOldFrontForDilation(first,buck,nextbuck,vcs6,domain,VIP_FRONT,object);
		  
	  /*bucket permutation*/
	  VipPermuteTwoIntBucket(&buck, &nextbuck);
	  nextbuck->n_points = 0;
	  
      }
  
  printf("\n");
  VipChangeIntLabel(vol,domain,forbiden);
  VipChangeIntLabel(vol,VIP_FRONT,forbiden);
  
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}      

/*---------------------------------------------------------------------------*/
VipIntBucket *VipCreateFrontIntBucketForDilation( Volume *vol, int connectivity, int front_value,
				       int object, int domain, int front_mode)
/*---------------------------------------------------------------------------*/
{ 
 
    
  Vip_S16BIT *ptr, *voisin;
  int i, NbTotalPts;
  VipIntBucket *buck;
  VipConnectivityStruct *vcs;
  int icon;
  

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketForDilation");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucketForDilation is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketForDilation");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1) /*note that the border is supposed to have been put to forbiden*/
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucketForDilation is only implemented with border");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketForDilation");
      return(PB);
    }
  if(front_mode!=FRONT_RANDOM_ORDER && front_mode!=FRONT_RANDOM_AND_DEPTH)
      {
	  VipPrintfError("Unknown front mode");
	  VipPrintfExit("VipCreateFrontIntBucketForDilation");
	  return(PB);
      }

  vcs = VipGetConnectivityStruct( vol, connectivity );

  ptr = VipGetDataPtr_S16BIT(vol);

  NbTotalPts = VipOffsetVolume(vol);
 
  buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);
  
  for ( i=0; i<NbTotalPts; i++ )
    {
      if (*ptr == domain)
	{
	  for ( icon=0; icon<vcs->nb_neighbors;icon++)
	    {
	      voisin = ptr + vcs->offset[icon];
	      if(*voisin==object)
		{
		  if(buck->n_points==buck->size)
		    {
		      if(VipIncreaseIntBucket(buck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
		    }
		  buck->data[buck->n_points++] = i;
		  *ptr = front_value;
		  break;
		}
	    }
	}
      ptr++;
    }
  
  if(VipRandomizeFrontOrder(buck,10)==PB) return(PB); 

  if(front_mode==FRONT_RANDOM_AND_DEPTH)
      if(VipFrontOrderFromDistanceToClosing(buck,vol,front_value,object,domain)==PB) return(PB); 

  VipFreeConnectivityStruct(vcs);

  return(buck);
}

/*-------------------------------------------------------------------------*/
int VipFillNextFrontFromOldFrontForDilation(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int object)
/*-------------------------------------------------------------------------*/
{
  int *buckptr, *dirptr;
  Vip_S16BIT *ptr, *ptr_neighbor;
  int i, dir2;

  if(first_vol_point==NULL)
    {
      VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontForDilation");
      VipPrintfExit("VipFillNextFrontFromOldFrontForDilation");
      return(PB);
    }
 if((buck==NULL) || (nextbuck==NULL))
    {
      VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontForDilation");
      VipPrintfExit("VipFillNextFrontFromOldFrontForDilation");
      return(PB);
    }
  if(vcs==NULL)
    {
      VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontForDilation");
      VipPrintfExit("VipFillNextFrontFromOldFrontForDilation");
      return(PB);
    }

 buckptr = buck->data;
 for(i=buck->n_points;i--;)
   {
     ptr = first_vol_point + *buckptr;
     if(*ptr==front_value) /*the point was not simple and has not be added to the object
			    or is not become immortal*/
       {
	 if(nextbuck->n_points==nextbuck->size)
	   {
	     if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
	   }
	 nextbuck->data[nextbuck->n_points++]=*buckptr;
       }
     else if (*ptr==object ) /*the point has been added to the object*/
       {
	 dirptr = vcs->offset;
	 for(dir2=vcs->nb_neighbors;dir2--;)
	   {
	     ptr_neighbor = ptr + *dirptr;
	     if(*ptr_neighbor==next_value) /*NOT ALREADY IN FRONT*/
	       {		     
		 *ptr_neighbor = front_value;
		 if(nextbuck->n_points==nextbuck->size)
		   {
		     if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
		   }
		 nextbuck->data[nextbuck->n_points++]=*buckptr+*dirptr;
	       }
	     dirptr++;
	   }
       }
     buckptr++;
   }

 return(OK);
}

/*-------------------------------------------------------------------------*/
int VipFrontOrderFromDistanceToClosing(VipIntBucket *buck, Volume *vol,
				       int front_value, int object, int domain) 
/*-------------------------------------------------------------------------*/
{
    Volume *dmap;
    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_S16BIT *ptr, *first;
    int *dist;
    int i, d;
    int touched;
    int *newbuck;

    dmap = VipCopyVolume(vol, "dmap");
    if(dmap==PB) return(PB);

    vos = VipGetOffsetStructure(dmap);
    first = VipGetDataPtr_S16BIT( dmap );
    ptr = first + vos->oFirstPoint;

    for ( iz = mVipVolSizeZ(dmap); iz-- ; )               /* loop on slices */
	{
	    for ( iy = mVipVolSizeY(dmap); iy-- ; )            /* loop on lines */
		{
		    for ( ix = mVipVolSizeX(dmap); ix-- ; )/* loop on points */
			{
			   if(*ptr==front_value || *ptr==object || *ptr==domain) *ptr=0;
			   else *ptr = 255;
			   ptr++;
			}
		    ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	    ptr += vos->oLineBetweenSlice; /*skip border lines*/
	}

    /* opening of the complement = closing */

    printf("closing...\n");
    if(VipOpening(dmap,CHAMFER_BALL_3D,10)==PB) return(PB);
    printf("computing 26-connectivity distance to closing outside\n");
    if(VipComputeFrontPropagationConnectivityDistanceMap(dmap,0,-1,
							 VIP_NO_LIMIT_IN_PROPAGATION,
							 0,CONNECTIVITY_26)==PB) return(PB);
    /*
    VipWriteVolume(dmap,"dmap");
    */

    dist = (int *)VipCalloc(buck->n_points,sizeof(int),"VipFrontOrderFromDistanceToClosing");
    if(dist==PB) return(PB);
    newbuck = (int *)VipCalloc(buck->n_points,sizeof(int),"VipFrontOrderFromDistanceToClosing");
    if(newbuck==PB) return(PB);

    for(i=0;i<buck->n_points;i++)
	{
	    dist[i]=*(first+buck->data[i]);
	}

    printf("Sorting front by distances...\n");
    touched=0;
    d = 1;                          
    while(touched<buck->n_points && d<1000)
	{
	    for(i=0;i<buck->n_points;i++)
		{
		    if(dist[i]==d) newbuck[touched++]=buck->data[i];			
		}
	    d++;
	}
    if(touched!=buck->n_points)
	VipPrintfWarning("Some front points have nor been sorted by depth");

    for(i=0;i<touched;i++)
	{
	    buck->data[i] = newbuck[i];
	}

    free(dist);
    free(newbuck);
    VipFreeVolume(dmap);

    return(OK);
}

/*---------------------------------------------------------------------------*/
// int VipHomotopicInsideDilationSnake( Volume *vol, Volume *graylevel, int nb_iteration,
// 				  int object, int inside, int outside, int front_mode,
// 			       float KISING, float mG, float sigmaG, float mW, float sigmaW)
/*-------------------------------------------------------------------------*/
// {
//   VipIntBucket *buck, *nextbuck;
//   Topology26Neighborhood *topo26;
//   VipConnectivityStruct *vcs6;
//   int loop, count, totalcount;
//   Vip_S16BIT *first, *ptr, *gptr, *voisin, *gfirst;
//   int *buckptr;
//   int i;
//   float KW, KG;
//   int nnG, nnW;
//   float deltaU;
//   int icon;
// 
//   if (VipVerifyAll(vol)==PB)
//     {
//       VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
//       return(PB);
//     }
//   if (VipTestType(vol,S16BIT)!=OK)
//     {
//       VipPrintfError("Sorry,  VipHomotopicInsideDilationSnake is only implemented for S16BIT volume");
//       VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
//       return(PB);
//     }
//   if (VipVerifyAll(graylevel)==PB)
//     {
//       VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
//       return(PB);
//     }
//   if (VipTestType(graylevel,S16BIT)!=OK)
//     {
//       VipPrintfError("Sorry,  VipHomotopicInsideDilationSnake is only implemented for S16BIT volume");
//       VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
//       return(PB);
//     }
//   if (mVipVolBorderWidth(vol) < 1)
//     {
//       VipPrintfError("Sorry, VipHomotopicInsideDilationSnake is only implemented with border");
//       VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
//       return(PB);
//     }
//   if(VipTestEqualBorderWidth(vol,graylevel)==PB)
//     {
//       VipPrintfError("Sorry, border widths are different");
//       VipPrintfExit("(skeleton)VipHomotopicInsideDilationSnake");
//       return(PB);
//     }
//   /*
//     printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
//   */
//   printf("Homotopic snake dilation towards inside...\n");
//   VipSetBorderLevel( vol, outside ); 
// 
//   buck = VipCreateFrontIntBucketForDilation( vol, CONNECTIVITY_6, VIP_FRONT, object, inside, front_mode);
//   if(buck==PB) return(PB);
//   nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
//   if(nextbuck==PB) return(PB);
// 
//   topo26 = VipCreateTopology26Neighborhood( vol);
//   if(topo26==PB) return(PB);
// 
//   vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
//   vcs26 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
//   
//   nextbuck->n_points = 0;
// 
//   first = VipGetDataPtr_S16BIT(vol);
//   gfirst = VipGetDataPtr_S16BIT(graylevel);
//   cfirst = VipGetDataPtr_S16BIT(classif);
//   
//   /*main loop*/
//   loop=0;
//   count = 1;
//   totalcount = 0;
//   KW = 2*KISING - 0.00001;
//   KG = 4*KISING + 0.00001;
//   /*
//     printf("gray matter (mean:%f, sigma:%f), white matter (mean:%f, sigma:%f)\n",
//     mG,sigmaG,mW,sigmaW);
//   */
//   printf("loop: %3d, Added: %6d",loop,0);
//   while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
//     {
//       if(loop==1) count=0;
//       totalcount += count;
//       count = 0;
//       printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Added: %6d",loop,totalcount);
//       fflush(stdout);
// 	  
//       buckptr = buck->data;
//       for(i=buck->n_points;i--;)
// 	{
// 	  ptr = first + *buckptr;
// 	  gptr = gfirst + *buckptr++;
// 	  nnG = 0;
// 	  nnW = 0;
// 	  mG = 0.;
//           mW = 0.;
//           sG = 0.;
//           sW = 0.;
// 	  nbG = 0;
// 	  nbW = 0;
// 	  deltaU = 0;
// 	  for ( icon=0;icon<vcs6->nb_neighbors;icon++)
// 	    {
// 	      voisin = ptr + vcs6->offset[icon];	      
// 	      if(*voisin==object || *voisin==outside) nnG++;
// 	      else nnW++;
// 	    }
// 	  
// 	  for (icon=0; icon<vcs26->nb_neighbors; icon++)
// 	  {
// 	      voisin = ptr + vcs26->offset[icon];
// 	      gvoisin = gptr + vcs26->offset[icon];
// 	      cvoisin = cptr + vcs26->offset[icon];
// 	      if(*voisin==inside)
// //		              if(*cvoisin>199)
//                               {
//                                   mW += (float)(*gvoisin);
//                                   nbW++;
//                               }
// //                              else if(*cvoisin<200 && *cvoisin>11)
//                               else if(*cvoisin>11)
//                               {
//                                   mG += (float)(*gvoisin);
//                                   nbG++;
//                               }
//                           }
//                       mW /= (float)nbW;
//                       mG /= (float)nbG;
//                       for (icon=0; icon<vcs26->nb_neighbors; icon++)
// 	        	  {
//                               voisin = ptr + vcs26->offset[icon];
//                               gvoisin = gptr + vcs26->offset[icon];
//                               cvoisin = cptr + vcs26->offset[icon]; 	      
// 		              if(*voisin==inside)
// //	        	      if(*cvoisin>199)
//                               {
//                                   sW += ((float)(*gvoisin) - mW)*((float)(*gvoisin) - mW);
//                               }
// //                              else if(*cvoisin<200 && *cvoisin>11)
//                               else if(*cvoisin>11)
//                               {
//                                   sG += ((float)(*gvoisin) - mG)*((float)(*gvoisin) - mG);
//                               }
//                           }
//                       sW = sqrt(sW/nbW);
//                       sG = sqrt(sG/nbG);
//                       deltaU = -KISING*(nnW-nnG);
// 	              deltaU += deltaGPotentialGtoW(*gptr,KG,mG,sG,KW,mW,sW);
// //	              deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
// //	              if((deltaU<=0 && *cptr>199) || nbG==0)
//       	              if(deltaU<=0 && *cptr>199)
//     		      {
// 		          *ptr = inside;
// 		          count++;
// 		      }
// 	  deltaU = -KISING*(nnG-nnW);
// 	  deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
// 	  if(deltaU<=0)
// 	    {
// 	      if (VipIsASimplePointForLabel_S16BIT(topo26, ptr, object)==VTRUE)
// 		{
// 		  *ptr = object;
// 		  count++;
// 		}
// 	    }
// 	}
// 	      	      			
//       VipFillNextFrontFromOldFrontForDilation(first,buck,nextbuck,vcs6,inside,VIP_FRONT,object);
// 		  
//       /*bucket permutation*/
//       VipPermuteTwoIntBucket(&buck, &nextbuck);
//       nextbuck->n_points = 0;
// 	  
//     }
// 
//   printf("\n");
//   
//   VipChangeIntLabel(vol,VIP_FRONT,inside);
//   
//   
//   VipFreeIntBucket(buck);
//   VipFreeIntBucket(nextbuck);
//   
//   return(OK);
// }
