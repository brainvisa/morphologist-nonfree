/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_histo_extrema.c  * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 27/04/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : Build primal sketch from histogram various scale spacea
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


#include <vip/histo.h>	
#include <vip/histo_static.h>	
#include <vip/util/file.h>
#include <vip/util/shelltools.h>

/*---------------------------------------------------------------------------*/
static int DetectLengthExtrema(
int *input,
Vip_S8BIT *extrema,
int dim,
int *numberplus,
int *numbermoins
);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipComputeFixedRangeCascadeVolume( SSCascade *c,  VipHisto *hcumul, int radius );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static int CheckExtremaConsistencePruneAndCountS8BITInputInt(
Vip_S8BIT *extrema,
int size,
int *numberplus,
int *numbermoins,
int *val);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static double *GetValBuf(SSSingularity *s, Singularity *sval, int nval );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int NatureSign( int n);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int FittingNatures(int n1, int n2);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static char *StringNature( int n);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int CheckExtremaConsistenceAndPruneS8BIT(
Vip_S8BIT *extrema,
int size,
int *first_extrema,
int *last_extrema,
Vip_DOUBLE *val);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static double *GetSpeedBuf(SSSingularity *s, Singularity *sval, int nval );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int CheckExtremaConsistencePruneAndCountS8BIT(
Vip_S8BIT *extrema,
int size,
int *numberplus,
int *numbermoins,
Vip_DOUBLE *val);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static int AddSingularityInHighestCascade( SSSingularity *s,   SSCascade *c);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipComputeCascadeVolume( SSCascade *c,  VipHisto *hcumul )
/*---------------------------------------------------------------------------*/
{
  /*SSSingularity *closest;
    int dist;*/
  int locinf, locsup;

  /*juin 2004 I try to change my strategy, the D1 can not be controlled at low scales;
    I add a security
    july, I go back to  stragegy, because small cascade emerging from big modes
can screw up the analysis
I think this stuff could be improve using hisgher scale than 0 to estimate location,
but this is problematic for trajectories dying very low. Hence it is a bit painful to develop now...*/

  if(c==NULL || hcumul==NULL)
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipCreateHighestCascade");
      return(PB);
    }
  /* The "bidouille" with range_min take into account offset on histogram left...*/
  /*je change encore, montreal hivers 2004  
  if(c->D2ms!=NULL && c->D1ms!=NULL && c->D1Ms!=NULL)
    {
      if(c->D1ms->loc[0]<c->D1Ms->loc[0])
	{
	  if(abs(c->D1ms->loc[0]-c->D2ms->loc[0]) > abs(c->D1Ms->loc[0]-c->D2ms->loc[0]))
	    closest = c->D1Ms;
	  else closest = c->D1ms;
          dist = mVipMax(5,abs(closest->loc[0]-c->D2ms->loc[0]));
          dist = mVipMin(10,abs(closest->loc[0]-c->D2ms->loc[0]));
          if ((closest->loc[0]<c->D2ms->loc[0])<0) dist = -dist;
          loc = mVipMin(c->D2ms->loc[0]+ dist,hcumul->range_min);
          loc = mVipMax(c->D2ms->loc[0]+ dist,hcumul->range_min);
	  c->volume = 2*abs(mVipHistoVal(hcumul,c->D2ms->loc[0]) - mVipHistoVal(hcumul,loc));
	}
      else
	{
	  c->volume = abs(mVipHistoVal(hcumul,c->D1ms->loc[0]) - mVipHistoVal(hcumul,c->D1Ms->loc[0]));
	}
    }
  else if((c->D2ms!=NULL))
    {
      loc = mVipMin(c->D2ms->loc[0]-2,hcumul->range_max);
      loc = mVipMax(c->D2ms->loc[0]-2,hcumul->range_min);
      c->volume = abs(mVipHistoVal(hcumul,c->D2ms->loc[0]) - mVipHistoVal(hcumul,loc));
      loc = mVipMin(c->D2ms->loc[0]+2,hcumul->range_max);
      loc = mVipMax(c->D2ms->loc[0]+2,hcumul->range_min);
      c->volume += abs(mVipHistoVal(hcumul,c->D2ms->loc[0]) - mVipHistoVal(hcumul,loc));     
    }
  */
  
  if((c->D2ms!=NULL))
    {
      locinf = mVipMax(c->D2ms->loc[0]-5,hcumul->range_min);
      locsup = mVipMin(c->D2ms->loc[0]+5,hcumul->range_max);
      c->volume = mVipHistoVal(hcumul,locsup) - mVipHistoVal(hcumul,locinf);
    }
  
  else c->volume = 0;
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipComputeFixedRangeCascadeVolume( SSCascade *c,  VipHisto *hcumul, int radius )
/*---------------------------------------------------------------------------*/
{
  /*  SSSingularity *closest;
      int dist;*/
  int locinf, locsup;

  if(c==NULL || hcumul==NULL)
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipCreateHighestCascade");
      return(PB);
    }
  

  if((c->D2ms!=NULL))
    {
      locinf = mVipMax(c->D2ms->loc[0]-radius,hcumul->range_min);
      locsup = mVipMin(c->D2ms->loc[0]+radius,hcumul->range_max-1);
      c->volume = abs(mVipHistoVal(hcumul,locsup) - mVipHistoVal(hcumul,locinf)); 
    }
  
  else c->volume = 0;
  return(OK);
}

/*---------------------------------------------------------------------------*/
SSCascade *VipCreateHighestCascade(SSSingularity *slist, int nbiter, int ninfinite )
/*---------------------------------------------------------------------------*/
{
  SSCascade *c;
  int i;

  if(slist==NULL)
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipCreateHighestCascade");
      return(PB);
    }
  if(nbiter<0 || (ninfinite!=4 && ninfinite!=5 && ninfinite!=3))
    {
      VipPrintfError("Strange Argument");
      VipPrintfExit("VipCreateHighestCascade");
      return(PB);
    }

  c = (SSCascade *)VipCalloc(1,sizeof(SSCascade),"VipDetectCascade");
  if(c==PB) return(PB);
  c->complet = VFALSE;

  while((slist!=NULL)&&(slist->scale_event!=nbiter)) slist = slist->right;
  if(slist==NULL)
    {
      VipPrintfError("Inconsistence");
      VipPrintfExit("VipCreateHighestCascade");
      return(PB);
    }
  if(ninfinite==5)
    {
      if(slist->nature!=D2M)
	{
	  VipPrintfError("Nature inconsistence");
	  VipPrintfExit("VipCreateHighestCascade");
	  return(PB);
	}
      else
	{
	  slist = slist->right;
	  while((slist!=NULL)&&(slist->scale_event!=nbiter)) slist = slist->right;
	  if(slist==NULL)
	    {
	      VipPrintfError("Nothing found Inconsistence");
	      VipPrintfExit("VipCreateHighestCascade");
	      return(PB);
	    }
	}
      ninfinite--;
    }
  AddSingularityInHighestCascade(slist,c);
  for(i=0;i<(ninfinite-1);i++)
    {
        slist = slist->right;
	while((slist!=NULL)&&(slist->scale_event!=nbiter)) slist = slist->right;
	if(AddSingularityInHighestCascade(slist,c)==PB) return(PB);
    }
 
  if((c->D2ms==NULL)||(c->D1ms==NULL)||(c->D1Ms==NULL))
    {
      VipPrintfError("Uncomplete infinite family:");
      if(c->D2ms==NULL)
	  {
	      fprintf(stderr,"No minimum of second derivative trajectory\n");
	  }
      else if(c->D1ms==NULL)
	  {
	      fprintf(stderr,"No minimum of first derivative trajectory\n");
	  }
      else if(c->D1Ms==NULL)
	  {
	      fprintf(stderr,"No maximum of first derivative trajectory\n");
	  }
      VipPrintfExit("VipCreateHighestCascade");
      return(PB);
    }
  c->scale_event = nbiter;
  if(c->D2Ms!=NULL)
    {
      c->D2Ms->mate = c->D2ms->mate;
      c->D2ms->mate = c->D2Ms->mate;
    }
  c->D1Ms->mate = c->D1ms->mate;
  c->D1ms->mate = c->D1Ms->mate;
  if( c->D2Ms!=NULL) c->complet = VTRUE;
  c->volume = 1000000000.; /*the biggest one*/

  return(c);
}

/*---------------------------------------------------------------------------*/
static int AddSingularityInHighestCascade( SSSingularity *s,   SSCascade *c)
/*---------------------------------------------------------------------------*/
{
  if((s==NULL)||(c==NULL))
    {
      VipPrintfError("NULL Argument");
      VipPrintfExit("AddSingularityInHighesCascade");
      return(PB);
    }

  if(s->nature==D1M)
    {
      if(c->D1Ms!=NULL)
	{
	  VipPrintfError("Inconsistence");
	  VipPrintfExit("AddSingularityInHighesCascade");
	  return(PB);
	}
      c->D1Ms = s;
      c->D1_scale_event = s->scale_event;
      s->cascade = c;
    }
  else if(s->nature==D1m)
    {
      if(c->D1ms!=NULL)
	{
	  VipPrintfError("Inconsistence");
	  VipPrintfExit("AddSingularityInHighesCascade");
	  return(PB);
	}
      c->D1ms = s;
      c->D1_scale_event = s->scale_event;
      s->cascade = c;
    }
  else if(s->nature==D2M)
    {
      if(c->D2Ms!=NULL)
	{
	  VipPrintfError("Inconsistence");
	  VipPrintfExit("AddSingularityInHighesCascade");
	  return(PB);
	}
      c->D2Ms = s;
      c->D2_scale_event = s->scale_event;
      s->cascade = c;
    }
  else if(s->nature==D2m)
    {
      if(c->D2ms!=NULL)
	{
	  VipPrintfError("Inconsistence");
	  VipPrintfExit("AddSingularityInHighesCascade");
	  return(PB);
	}
      c->D2ms = s;
      c->D2_scale_event = s->scale_event;
      s->cascade = c;
    }

  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipMarkSSExtremum( SSSingularity *s, Vip1DScaleSpaceStruct *ssstruct)
/*---------------------------------------------------------------------------*/
{
  Vip_S8BIT *ptr=NULL;
  int xsize;
  int i;
  int mark;
  int test = VFALSE;

  if((s==NULL)||(ssstruct==NULL))
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipMarkSSExtremum");
      return(PB);
    }
  if((ssstruct->d1_e==NULL)||(ssstruct->d2_e==NULL))
    {
      VipPrintfError("Null extremum volume");
      VipPrintfExit("VipMarkSSExtremum");
      return(PB);
    }

  xsize = VipOffsetLine(ssstruct->d1_e);
  /*NB: ysize = scalemax+1*/
  if((s->nature==D2M)||(s->nature==D2m))
    ptr = VipGetDataPtr_S8BIT(ssstruct->d2_e);
  else   if((s->nature==D1M)||(s->nature==D1m))
    ptr = VipGetDataPtr_S8BIT(ssstruct->d1_e);

  mark = 3*NatureSign(s->nature);

  for(i=s->scale_event;i>=0;i--)
    *(ptr+i*xsize+s->loc[i]) = mark;

  /*
  for(i=0;i<s->n_scale_extrema;i++)
    *(ptr+s->scale_extrema[i]*xsize+s->loc[s->scale_extrema[i]]) = mark;
    */
  mark = 4*NatureSign(s->nature);

  for(i=0;i<s->n_scale_extrema;i++)
    {
      if(s->loc[s->scale_extrema[i]]==s->analysed_loc && s->analysed_loc!=0)
        {
          *(ptr+s->scale_extrema[i]*xsize+s->loc[s->scale_extrema[i]]) = mark;
          test = VTRUE;
          break;
        }
    }
  if (test==VFALSE && s->nature==D2m)
    *(ptr + s->loc[0]) = mark;


  return(OK);
  

}

/*---------------------------------------------------------------------------*/
int VipMarkNBiggestCascades ( SSCascade *clist, int n,  Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{
 if((clist==NULL)||(volstruct==NULL))
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipMarkNBiggestCascades");
      return(PB);
    }
 if((n<0)||(n>10000))
    {
      VipPrintfError("Strange value");
      VipPrintfExit("VipMarkNBiggestCascades");
      return(PB);
    }

 while((n>0)&&(clist!=NULL))
   {
     if( VipMarkSSCascadeExtrema(clist, volstruct)==PB) return(PB);
     n--;
     clist = clist->next;
   }
 if((clist==NULL)&&(n>0))
   {
     VipPrintfWarning("Less singularity cascades than assumed");
   }
 return(OK);
}

/*---------------------------------------------------------------------------*/
int VipMarkNHighestCascades ( SSCascade *clist, int n,  Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{
 if((clist==NULL)||(volstruct==NULL))
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipMarkNHighestCascades");
      return(PB);
    }
 if((n<0)||(n>10000))
    {
      VipPrintfError("Strange value");
      VipPrintfExit("VipMarkNHighestCascades");
      return(PB);
    }

 while((n>0)&&(clist!=NULL))
   {
     if( VipMarkSSCascadeExtrema(clist, volstruct)==PB) return(PB);
     n--;
     clist = clist->next;
   }
 if((clist==NULL)&&(n>0))
   {
     VipPrintfWarning("Less singularity cascades than assumed");
   }
 return(OK);
}

/*---------------------------------------------------------------------------*/
int VipMarkAnalysedObjects( VipT1HistoAnalysis *ana, Vip1DScaleSpaceStruct *volstruct )
/*---------------------------------------------------------------------------*/
{
  if((ana==NULL)||(volstruct==NULL))
    {
      VipPrintfError("Null Arg");
      VipPrintfExit("VipMarkAnalysedObjects");
      return(PB);
    }

  if(ana->background!=NULL) 
	VipMarkSSObjectExtrema(ana->background, volstruct);
  if(ana->brain!=NULL)
	VipMarkSSObjectExtrema(ana->brain, volstruct);
  if(ana->white!=NULL)
	VipMarkSSObjectExtrema(ana->white, volstruct);
  if(ana->gray!=NULL) 
	VipMarkSSObjectExtrema(ana->gray, volstruct);
  if(ana->csf!=NULL)
	VipMarkSSObjectExtrema(ana->csf, volstruct);
  if(ana->skull!=NULL)
	VipMarkSSObjectExtrema(ana->skull, volstruct);
  if(ana->mode1!=NULL)
	VipMarkSSObjectExtrema(ana->mode1, volstruct);
  if(ana->mode2!=NULL)
	VipMarkSSObjectExtrema(ana->mode2, volstruct);
  if(ana->mode3!=NULL)
	VipMarkSSObjectExtrema(ana->mode3, volstruct);
  if(ana->mode4!=NULL)
	VipMarkSSObjectExtrema(ana->mode4, volstruct);
  return(OK);

}

/*---------------------------------------------------------------------------*/
int VipMarkAnalysedObjectsHisto( VipT1HistoAnalysis *ana, Vip1DScaleSpaceStruct *volstruct )
/*---------------------------------------------------------------------------*/
{
  int i;

  if((ana==NULL)||(volstruct==NULL))
    {
      VipPrintfError("Null Arg");
      VipPrintfExit("VipMarkAnalysedObjects");
      return(PB);
    }

  for (i=0;i<6;i++)
    {
      if (ana->candidate[i]!=NULL && ana->candidate[i]!=ana->white
          && ana->candidate[i]!=ana->gray 
          && ana->candidate[i]!=ana->csf)
        {
          ana->candidate[i]->D2ms->nature = D1M;
          VipMarkSSExtremum(ana->candidate[i]->D2ms, volstruct);
        }
    }

  if(ana->white!=NULL)
    if(ana->white->D2ms!=NULL) VipMarkSSExtremum(ana->white->D2ms, volstruct);
  if(ana->gray!=NULL) 
    if(ana->gray->D2ms!=NULL) VipMarkSSExtremum(ana->gray->D2ms, volstruct);
  if(ana->csf!=NULL)
    if(ana->csf->D2ms!=NULL) VipMarkSSExtremum(ana->csf->D2ms, volstruct);
  return(OK);

}

/*---------------------------------------------------------------------------*/
int VipMarkSSCascadeExtrema( SSCascade *c, Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{

  if((c==NULL)||(volstruct==NULL))
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipMarkSSCascadeExtrema");
      return(PB);
    }

  if(c->D1Ms!=NULL) VipMarkSSExtremum(c->D1Ms, volstruct);
  if(c->D1ms!=NULL) VipMarkSSExtremum(c->D1ms, volstruct);
  if(c->D2Ms!=NULL) VipMarkSSExtremum(c->D2Ms, volstruct);
  if(c->D2ms!=NULL) VipMarkSSExtremum(c->D2ms, volstruct);

  return(OK);
}



/*---------------------------------------------------------------------------*/
int VipMarkSSObjectExtrema( SSObject *o, Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{

  if((o==NULL)||(volstruct==NULL))
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipMarkSSObjectExtrema");
      return(PB);
    }

  if(o->D2Mls!=NULL) VipMarkSSExtremum(o->D2Mls, volstruct);
  if(o->D1Ms!=NULL) VipMarkSSExtremum(o->D1Ms, volstruct);
  if(o->D2ms!=NULL) VipMarkSSExtremum(o->D2ms, volstruct);
  if(o->D1ms!=NULL) VipMarkSSExtremum(o->D1ms, volstruct);
  if(o->D2Mrs!=NULL) VipMarkSSExtremum(o->D2Mrs, volstruct);

  return(OK);
}
 
/*---------------------------------------------------------------------------*/
SSCascade *VipComputeOrderedCascadeList( SSSingularity *list,int itermax, VipHisto *hcumul )
/*---------------------------------------------------------------------------*/
{
  SSSingularity *s;
  SSCascade *clist, *c;

  if(list==NULL || hcumul==NULL)
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipComputeOrderedCascadeList");
      return(PB);
    }
  if(itermax<0 || itermax>1000000)
    {
      VipPrintfError("Strange Arguments");
      VipPrintfExit("VipComputeOrderedCascadeList");
      return(PB);
    }

  clist = NULL;
  s = list;
  while(s!=NULL)
    {
      if((s->nature==D1m || s->nature==D1M)&&(s->mate!=NULL)&&(s->cascade==NULL))
	{
	  c = VipDetectCascadeFromD1( list, s);
	  if(c==PB) return(PB);
	  VipComputeCascadeVolume( c,  hcumul );
	  if(VipInsertSSCascadeInList( c, &clist)==PB) return(PB);
	}
      s = s->right;
    }
  s = list;
  while(s!=NULL)
    {
      if((s->nature==D2m || s->nature==D2M)&&(s->mate!=NULL)&&(s->cascade==NULL))
	{
	  c = VipDetectCascadeFromD2( list, s);
	  if(c==PB) return(PB);
	  VipComputeCascadeVolume( c,  hcumul );
	  if(VipInsertSSCascadeInList( c, &clist)==PB) return(PB);
	}
      s = s->right;
    }
  return(clist);
}

/*---------------------------------------------------------------------------*/
SSCascade *VipComputeScaleOrderedCascadeList( SSSingularity *list,int itermax, VipHisto *hcumul )
/*---------------------------------------------------------------------------*/
{
  SSSingularity *s;
  SSCascade *clist, *c;

  if(list==NULL || hcumul==NULL)
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipComputeOrderedCascadeList");
      return(PB);
    }
  if(itermax<0 || itermax>1000000)
    {
      VipPrintfError("Strange Arguments");
      VipPrintfExit("VipComputeOrderedCascadeList");
      return(PB);
    }

  clist = NULL;
  s = list;
  while(s!=NULL)
    {
      if((s->nature==D1m || s->nature==D1M)&&(s->mate!=NULL)&&(s->cascade==NULL))
	{
	  c = VipDetectCascadeFromD1( list, s);
	  if(c==PB) return(PB);
	  VipComputeFixedRangeCascadeVolume( c,  hcumul, 2 );
	  if(VipScaleInsertSSCascadeInList( c, &clist)==PB) return(PB);
	}
      s = s->right;
    }
  s = list;
  while(s!=NULL)
    {
      if((s->nature==D2m || s->nature==D2M)&&(s->mate!=NULL)&&(s->cascade==NULL))
	{
	  c = VipDetectCascadeFromD2( list, s);
	  if(c==PB) return(PB);
	  VipComputeFixedRangeCascadeVolume( c,  hcumul, 2 );
	  if(VipScaleInsertSSCascadeInList( c, &clist)==PB) return(PB);
	}
      s = s->right;
    }
  return(clist);
}

/*---------------------------------------------------------------------------*/
SSCascade *VipDetectCascadeFromD1( SSSingularity *list,SSSingularity *s)
/*---------------------------------------------------------------------------*/
{
  SSCascade *c;
  SSSingularity *w;
  int left, right;
  int temp;
  int wloc;

  if(s==NULL || list==NULL)
    {
      VipPrintfError("Null Argument");
      VipPrintfExit("VipDetectCascadeFromD1");
      return(PB);
    }

  if(s->cascade!=NULL)
    { 
      VipPrintfError("cascade already detected");
      VipPrintfExit("VipDetectCascadeFromD1");
      return(PB);
    }


  c = (SSCascade *)VipCalloc(1,sizeof(SSCascade),"VipDetectCascadeFromD1");
  if(c==PB) return(PB);
  c->complet = VFALSE;

  if(s->mate!=NULL)
    {
      if(s->nature==D1M)
	{
	  c->D1Ms = s;
	  c->D1ms = s->mate;
	  c->D1_scale_event = s->scale_event;
	}
      else if(s->nature==D1m)
	{
	  c->D1Ms = s->mate;
	  c->D1ms = s;
	  c->D1_scale_event = s->scale_event;
	}
      s->cascade = c;
      s->mate->cascade = c;

      left = c->D1Ms->loc[c->D1Ms->scale_event];
      right = c->D1ms->loc[c->D1ms->scale_event];
      if(left>right)
	{
	  temp = left;
	  left = right;
	  right = temp;
	}
      left -=1;
      right +=1;
      w = list;
      while(w!=NULL)
	{
	  if((w->cascade==NULL)&&((w->nature==D2M)||(w->nature==D2m))&&(w->scale_event>=c->D1_scale_event))
	    {
	      wloc = w->loc[c->D1_scale_event];
	      if((wloc>=left)&&(wloc<=right))
		{
		  if(w->nature==D2M)
		    {
		      c->D2Ms = w;
		      c->D2_scale_event = w->scale_event;
		      w->cascade = c;
		      if(w->mate!=NULL)
			{
			  c->D2ms = w->mate;
			  w->mate->cascade = c;
			  c->complet = VTRUE;
			}
		    }
		  else
		    {
		      c->D2ms = w;
		      c->D2_scale_event = w->scale_event;
		      w->cascade = c;
		      if(w->mate!=NULL)
			{
			  c->D2Ms = w->mate;
			  w->mate->cascade = c;
			  c->complet = VTRUE;
			}
		    }
		  break;
		}
	    }
	  w = w->right;
	}
    }
  else
    {
      VipPrintfError("not implemented for non mated singularity");
      VipPrintfExit("VipDetectCascadeFromD1");
      return(PB);
    }

  c->scale_event = c->D2_scale_event;

  return(c);  
}

/*---------------------------------------------------------------------------*/
SSCascade *VipDetectCascadeFromD2( SSSingularity *list,SSSingularity *s)
/*---------------------------------------------------------------------------*/
{
    SSCascade *c;

    if(s==NULL || list==NULL)
	{
	    VipPrintfError("Null Argument");
	    VipPrintfExit("VipDetectCascadeFromD2");
	    return(PB);
	}

    if(s->cascade!=NULL)
	{ 
	    VipPrintfError("cascade already detected");
	    VipPrintfExit("VipDetectCascadeFromD2");
	    return(PB);
	}


    c = (SSCascade *)VipCalloc(1,sizeof(SSCascade),"VipDetectCascadeFromD2");
    if(c==PB) return(PB);
    c->complet = VFALSE;

    if(s->mate!=NULL)
	{
	    if(s->nature==D2M)
		{
		    c->D2Ms = s;
		    c->D2ms = s->mate;
		    c->D2_scale_event = s->scale_event;
		}
	    else if(s->nature==D2m)
		{
		    c->D2Ms = s->mate;
		    c->D2ms = s;
		    c->D2_scale_event = s->scale_event;
		}
	    s->cascade = c;
	    s->mate->cascade = c;
	}
    else
	{
	    VipPrintfError("not implemented for non mated singularity");
	    VipPrintfExit("VipDetectCascadeFromD2");
	    return(PB);
	}

    c->scale_event = c->D2_scale_event;

    return(c);  
}
/*---------------------------------------------------------------------------*/
int VipCountSingularitiesStillAlive( SSSingularity *slist, int *n, int scalemax )
/*---------------------------------------------------------------------------*/
{
    char *nat;

  if(slist==NULL)
    {
      VipPrintfError("NULL argument");
      VipPrintfExit("VipCountSingularitiesStillAlive");
      return(PB);
    }
  *n = 0;
  while(slist!=NULL)
    {
      if((slist->mate==NULL)&&(slist->scale_event==scalemax))
	{
	  (*n)++;
	  nat = StringNature(slist->nature);
	  /*
	  printf("%s singularity moving from %d to %d\n",nat,slist->loc[0],slist->loc[slist->scale_event]);
	  */
	}
      slist = slist->right;
    }
  return(OK);
			      
}

/*---------------------------------------------------------------------------*/
static char *StringNature( int n)
/*---------------------------------------------------------------------------*/
{
  char *s;
  
  s = VipMalloc(6,"StringNature");
  if(s==PB) return(PB);
  switch(n)
    {
    case D0M: strcpy(s,"D0Max");break;
    case D0m: strcpy(s,"D0min");break;
    case D1M: strcpy(s,"D1Max");break;
    case D1m: strcpy(s,"D1min");break;
    case D2M: strcpy(s,"D2Max");break;
    case D2m: strcpy(s,"D2min");break;
    case D3M: strcpy(s,"D3Max");break;
    case D3m: strcpy(s,"D3min");break;
    case D4M: strcpy(s,"D4Max");break;
    case D4m: strcpy(s,"D4min");break;
    default:
      VipPrintfError("Unknown nature");
      VipPrintfExit("StringNature");
      return(PB);
    }
  return(s);
}

/*---------------------------------------------------------------------------*/
static int NatureSign( int n)
/*---------------------------------------------------------------------------*/
{
  int s;
   
  switch(n)
    {
    case D0M: s = MAX1D ;break;
    case D0m: s = MIN1D ;break;
    case D1M: s = MAX1D;break;
    case D1m: s = MIN1D ;break;
    case D2M: s = MAX1D;break;
    case D2m: s = MIN1D ;break;
    default:
      VipPrintfError("Unknown nature");
      VipPrintfExit("NatureSign");
      return(PB);
    }
  return(s);
}

/*---------------------------------------------------------------------------*/
static int FittingNatures(int n1, int n2)
/*---------------------------------------------------------------------------*/
{
  if( ((n1==D1M)&&(n2==D1m))||
      ((n1==D1m)&&(n2==D1M))||
      ((n1==D2M)&&(n2==D2m))||
      ((n1==D2m)&&(n2==D2M)) ) return(OK);
  else return(PB);
}

/*---------------------------------------------------------------------------*/
int VipFindSSSingularityMates( SSSingularity *slist )
/*---------------------------------------------------------------------------*/
{
  SSSingularity *l, *r, *s;
  int dl, dr;
  int lim = 5;
  char message[512];

  if(slist==NULL)
    {
      VipPrintfError("NULL argument");
      VipPrintfExit("VipFindSSSingularityMates");
      return(PB);
    }

  s = slist;
  while(s!=NULL)
    {
      if(s->mate==NULL)
	{
	  l = s->left;
	  r = s->right;
	  while((l!=NULL)&&((l->scale_event!=s->scale_event)
			    ||(FittingNatures(l->nature,s->nature)!=OK))) l = l->left;
	  while((r!=NULL)&&((r->scale_event!=s->scale_event)
		||(FittingNatures(r->nature,s->nature)!=OK))) r = r->right;
	  if((l==NULL)&&(r!=NULL))
	    {
	      dr=abs((int)(r->loc[r->scale_event]-s->loc[s->scale_event]));
	      if((r->mate==NULL) && (dr<lim))
		{
		  r->mate = s;
		  s->mate = r;
		}
	    }
	    
	  else if((l!=NULL)&&(r==NULL))
	    {
	      dl=abs((int)(l->loc[l->scale_event]-s->loc[s->scale_event]));
	      if((l->mate==NULL) && (dl<lim))
		{
		  l->mate = s;
		  s->mate = l;
		}
	    }
	  else if((l!=NULL)&&(r!=NULL))
	    {
	      dr=abs((int)(r->loc[r->scale_event]-s->loc[s->scale_event]));
	      dl=abs((int)(l->loc[l->scale_event]-s->loc[s->scale_event]));
	      if(dr<dl)
		{
		  if(r->mate!=NULL)
		    {
		      if(dl<lim)
			{
			  sprintf(message,"Suspicious singularity mating at scale %d between %d and %d localization",
				  s->scale_event,s->loc[s->scale_event],l->loc[l->scale_event]);
			  VipPrintfWarning(message);
			  s->mate = l;
			  l->mate = s;
			}
		    }
		  else if (dr<lim)
		    {
		      r->mate = s;
		      s->mate = r;
		    }
		}
	      else
		if(l->mate!=NULL)
		  {
		    if(dr<lim)
		      {
			sprintf(message,"Suspicious singularity mating at scale %d between %d and %d localization",
				s->scale_event,s->loc[s->scale_event],r->loc[r->scale_event]);
			VipPrintfWarning(message);
			s->mate = r;
			r->mate = s;
		      }
		  }
		else if (dl<lim)
		  {
		    l->mate = s;
		    s->mate = l;
		  }
	    }
	}
      s = s->right;
    }
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipFree1DScaleSpaceStruct( Vip1DScaleSpaceStruct *ssstruct)
/*---------------------------------------------------------------------------*/
{
    
  if(ssstruct==NULL)
    {
      VipPrintfError("Inconsistent arguments");
      VipPrintfExit("VipFree1DScaleSpaceStruct");
      return(PB);
    }

  if(ssstruct->ss_e!=NULL) VipFreeVolume(ssstruct->ss_e);
  if(ssstruct->d1_e!=NULL) VipFreeVolume(ssstruct->d1_e);
  if(ssstruct->d2_e!=NULL) VipFreeVolume(ssstruct->d2_e);
  if(ssstruct->d3_e!=NULL) VipFreeVolume(ssstruct->d3_e);
  if(ssstruct->d4_e!=NULL) VipFreeVolume(ssstruct->d4_e);
    
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipFreeCascadeList( SSCascade *c)
/*---------------------------------------------------------------------------*/
{
    SSCascade *c2;
    
   if(c==NULL)
    {
      VipPrintfError("Inconsistent arguments");
      VipPrintfExit("VipFreeCascadeList");
      return(PB);
    }

   while(c!=NULL)
	{
	    c2=c->next;
	    free(c);
	    c = c2;
	}
    
    return(OK);
}
/*---------------------------------------------------------------------------*/
int VipFreeSSSingularityList( SSSingularity *ss)
/*---------------------------------------------------------------------------*/
{
    SSSingularity *ss2;

    
    if(ss==NULL)
    {
      VipPrintfError("Inconsistent arguments");
      VipPrintfExit("VipFreeSSSingularityList");
      return(PB);
    }

    while(ss!=NULL)
	{
	    ss2=ss->right;
	    if(ss->loc!=NULL) free(ss->loc);	 
	    if(ss->n_scale_extrema!=0 && ss->scale_extrema!=NULL)
		{
		    free(ss->scale_extrema);	
		}
	    free(ss);
	    ss = ss2;
	}
    printf("\n");
    
    return(OK);
}
/*---------------------------------------------------------------------------*/
SSSingularity *VipComputeSSSingularityList(
Vip1DScaleSpaceStruct *ssstruct,
int scalemin,
int trackD0,
int trackD1,
int trackD2,
int trackD3,
int trackD4
)
/*---------------------------------------------------------------------------*/
  /*NB in fact borderwidth should be zero for a sure tracking*/
{
  Vip_S8BIT *ptr0=NULL, *ptr1=NULL, *ptr2=NULL, *ptr3=NULL, *ptr4=NULL;
  SSSingularity *slist, *s;
  int error;
  int xsize;
  int loc, scale;

  if(ssstruct==NULL)
    {
      VipPrintfError("Inconsistent arguments");
      VipPrintfExit("VipComputeSSSingularityList");
      return(PB);
    }
  if( scalemin<0 || scalemin>ssstruct->itermax)
    {
      VipPrintfError("Inconsistent min scale for tracking");
      fprintf(stderr,"min tracking scale: %d\n",scalemin);
      VipPrintfExit("VipComputeSSSingularityList");
      return(PB);
    }
  if(ssstruct->d1_e==NULL)
    {
      VipPrintfError("Null extremum volume");
      VipPrintfExit("VipComputeSSSingularityList");
      return(PB);
    }

  xsize = VipOffsetLine(ssstruct->d1_e);
  /*NB: ysize = itermax+1*/
  if(trackD0==VTRUE && ssstruct->ss_e!=NULL)
      ptr0 = VipGetDataPtr_S8BIT(ssstruct->ss_e)+xsize*(ssstruct->itermax);
  if(trackD1==VTRUE && ssstruct->d1_e!=NULL)
      ptr1 = VipGetDataPtr_S8BIT(ssstruct->d1_e)+xsize*(ssstruct->itermax);
  if(trackD2==VTRUE && ssstruct->d2_e!=NULL)
      ptr2 = VipGetDataPtr_S8BIT(ssstruct->d2_e)+xsize*(ssstruct->itermax);
  if(trackD3==VTRUE && ssstruct->d3_e!=NULL)
      ptr3 = VipGetDataPtr_S8BIT(ssstruct->d3_e)+xsize*(ssstruct->itermax);
  if(trackD4==VTRUE && ssstruct->d4_e!=NULL)
      ptr4 = VipGetDataPtr_S8BIT(ssstruct->d4_e)+xsize*(ssstruct->itermax);


  slist = NULL;
  for(scale=ssstruct->itermax;scale>=scalemin;scale--)
    {
      for(loc=0;loc<mVipVolSizeX(ssstruct->d1_e);loc++)
	{
	    if(trackD0==VTRUE)
		{
		    if(*ptr0!=0) 
			{
			    if(*ptr0==MIN1D)
				{
				    error = VipTrackSSSingularity(ptr0,loc,scale,D0m,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if(*ptr0==MAX1D)
				{
				    error = VipTrackSSSingularity(ptr0,loc,scale,D0M,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if((*ptr0!=MAX1DTRACKED)&&(*ptr0!=MIN1DTRACKED))
				{
				    VipPrintfError("Strange value in D0 extremum volume");
				    VipPrintfExit("VipComputeSSSingularityList");
				    return(PB);
				}
			}
		}
	    if(trackD1==VTRUE)
		{
		    if(*ptr1!=0) 
			{
			    if(*ptr1==MIN1D)
				{
				    error = VipTrackSSSingularity(ptr1,loc,scale,D1m,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if(*ptr1==MAX1D)
				{
				    error = VipTrackSSSingularity(ptr1,loc,scale,D1M,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if((*ptr1!=MAX1DTRACKED)&&(*ptr1!=MIN1DTRACKED))
				{
				    VipPrintfError("Strange value in D1 extremum volume");
				    VipPrintfExit("VipComputeSSSingularityList");
				    return(PB);
				}
			}
		}
	    if(trackD2==VTRUE)
		{
		    if(*ptr2!=0) 
			{
			    if(*ptr2==MIN1D)
				{
				    error = VipTrackSSSingularity(ptr2,loc,scale,D2m,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if(*ptr2==MAX1D)
				{
				    error = VipTrackSSSingularity(ptr2,loc,scale,D2M,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if((*ptr2!=MAX1DTRACKED)&&(*ptr2!=MIN1DTRACKED))
				{
				    VipPrintfError("Strange value in D2 extremum volume");
				    VipPrintfExit("VipComputeSSSingularityList");
				    return(PB);
				}
			}
		}
	    if(trackD3==VTRUE)
		{
		    /*
		    if(*ptr3!=0 && (loc<mVipVolSizeX(ssstruct->d1_e)-5) && loc>5) 
		    */
		    if(*ptr3!=0) 
			{
			    if(*ptr3==MIN1D)
				{
				    error = VipTrackSSSingularity(ptr3,loc,scale,D3m,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if(*ptr3==MAX1D)
				{
				    error = VipTrackSSSingularity(ptr3,loc,scale,D3M,xsize,&s);
				    if(error==PB) return(PB);
				    else if(error!=LOST) VipInsertSSSingularity(s,&slist);
				}
			    else if((*ptr3!=MAX1DTRACKED)&&(*ptr3!=MIN1DTRACKED))
				{
				    VipPrintfError("Strange value in D3 extremum volume");
				    VipPrintfExit("VipComputeSSSingularityList");
				    return(PB);
				}
			}
		}
	    if(trackD4==VTRUE)
	      {
		  /*
		  if(*ptr4!=0 && (loc<mVipVolSizeX(ssstruct->d1_e)-5) && loc>5) 
		  */
		  if(*ptr4!=0) 
		      {
			  if(*ptr4==MIN1D)
			      {
				  error = VipTrackSSSingularity(ptr4,loc,scale,D4m,xsize,&s);
				  if(error==PB) return(PB);
				  else if(error!=LOST) VipInsertSSSingularity(s,&slist);
			      }
			  else if(*ptr4==MAX1D)
			      {
				  error = VipTrackSSSingularity(ptr4,loc,scale,D4M,xsize,&s);
				  if(error==PB) return(PB);
				  else if(error!=LOST) VipInsertSSSingularity(s,&slist);
			      }
			  else if((*ptr4!=MAX1DTRACKED)&&(*ptr4!=MIN1DTRACKED))
			      {
				  VipPrintfError("Strange value in D4 extremum volume");
				  VipPrintfExit("VipComputeSSSingularityList");
				  return(PB);
			      }
		      }
	    }
	  if(trackD0==VTRUE) ptr0++;
	  if(trackD1==VTRUE) ptr1++;
	  if(trackD2==VTRUE) ptr2++;
	  if(trackD3==VTRUE) ptr3++;
	  if(trackD4==VTRUE) ptr4++;

	}
      if(trackD0 ==VTRUE) ptr0 += VipOffsetPointBetweenLine(ssstruct->ss_e)-2*xsize;
      if(trackD1 ==VTRUE) ptr1 += VipOffsetPointBetweenLine(ssstruct->d1_e)-2*xsize;
      if(trackD2 ==VTRUE) ptr2 += VipOffsetPointBetweenLine(ssstruct->d2_e)-2*xsize;
      if(trackD3 ==VTRUE) ptr3 += VipOffsetPointBetweenLine(ssstruct->d3_e)-2*xsize;
      if(trackD4 ==VTRUE) ptr4 += VipOffsetPointBetweenLine(ssstruct->d4_e)-2*xsize;
    }


  return(slist);
}




/*---------------------------------------------------------------------------*/
int VipTrackSSSingularity(
Vip_S8BIT *event,
int loc,
int scale_event,
int nature,
int xsize,
SSSingularity **sarg) 
/*---------------------------------------------------------------------------*/
{
  Vip_S8BIT *ptr;
  SSSingularity *s;
  int flag;
  int i;
  int jump;
  int lim = 5;
  /* Here we suppose that localizations are in the range [0,xsize[ */

  if((event==NULL)||(sarg==NULL)||(scale_event<0))
    {
      VipPrintfError("Inconsistent arguments");
      VipPrintfExit("VipTrackSSSingularity");
      return(PB);
    }
  if((xsize>100000)||(xsize<=0))
    {
      VipPrintfError("Strange histogram length");
      VipPrintfExit("VipTrackSSSingularity");
      return(PB);
    }
  if((loc>=xsize)||(loc<0))
    {
      VipPrintfError("Strange localization");
      VipPrintfExit("VipTrackSSSingularity");
      return(PB);
    }


  s = VipAllocSSSingularity( scale_event, nature);
  if(s==PB) return(PB);

  flag = *event;
  s->loc[scale_event] = loc;
  ptr = event;
  *event *= 2; /*for tracked*/
  for(i=scale_event-1;i>=0;i--)
    {
      jump = 0;
      ptr-=xsize;
      while(jump<lim) 
	{
	  if (((s->loc[i+1]+jump)<xsize)&&(*(ptr+jump)==flag))
	    {
	      s->loc[i] = s->loc[i+1]+jump;
	      ptr+=jump;
	      *ptr *= 2;
	      break;
	    }
	  else if (((s->loc[i+1]-jump)>=0)&&(*(ptr-jump)==flag))
	    {
	      s->loc[i] = s->loc[i+1]-jump;
	      ptr-=jump;
	      *ptr *= 2;
	      break;
	    }
	  jump++;
	}
      if(jump==lim)
	{
	    if(loc<=1) /*border pb which can also be solved by a left offset...*/
		{
		    s->loc[i] = s->loc[i+1];
		    *ptr *=2; /*jump=0*/
		}
	    else
		{
		    VipPrintfWarning("Strange: gap of more than 5 points along this trajectory (use smaller scale step)");
		    fprintf(stderr,"Singularity steming from iteration %d and localization %d\n",scale_event,(int)s->loc[scale_event]);
		    fprintf(stderr,"lost at iteration %d\n",i);
		    return(LOST);
		}
	}
    }
  *sarg = s;
  return(OK);

}

/*---------------------------------------------------------------------------*/
SSSingularity * VipAllocSSSingularity( int scale_event, int nature )
/*---------------------------------------------------------------------------*/
{
  SSSingularity *s;

  if((scale_event<0)||(scale_event>100000))
    {
      VipPrintfError("Inconsistent event scale");
      VipPrintfExit("VipAllocSSSingularity");
      return(PB);
    }
  if((nature!=D0M)&&(nature!=D0m)&&(nature!=D1M)&&(nature!=D1m)&&(nature!=D2M)&&(nature!=D2m)
     &&(nature!=D3M)&&(nature!=D3m)&&(nature!=D4M)&&(nature!=D4m))
    {
      VipPrintfError("Inconsistent event nature");
      VipPrintfExit("VipAllocSSSingularity");
      return(PB);
    }
  s = (SSSingularity *)VipCalloc(1,sizeof(SSSingularity),"VipAllocSSSingularity");
  if(s ==PB) return(PB);


  s->scale_event = scale_event;
  s->nature = nature;
  s->loc = (short *)VipCalloc(scale_event+100,sizeof(short),"VipAllocSSSingularity");
  s->analysed_loc = 0;
  if(s->loc==PB) return(PB);
  s->right = NULL;
  s->left = NULL;
  s->mate = NULL;
  s->cascade = NULL;
  s->n_scale_extrema = 0;
  s->scale_extrema = NULL;

  return(s);
  
}

/*---------------------------------------------------------------------------*/
int VipComputeSSSingularitySimpleSpeedMinima( SSSingularity *s )
/*---------------------------------------------------------------------------*/
{
  int segmentlength[50000];
  int segmentloc[50000];
  int segmentscale[50000];
  Vip_S8BIT lengthextrema[50000];
  int nsegments;
  int i;
  int nmin, nmax;

  if(s==NULL)
    {
      VipPrintfError("NULL pointer in arguments");
      VipPrintfExit("VipComputeSSSingularitySimpleSpeedMinima");
      return(PB);
    }

  if(s->n_scale_extrema!=0) return(OK);
  nsegments= 0;
  segmentloc[0] = s->loc[s->scale_event];
  segmentlength[0]=0;
  for(i=s->scale_event;i>=0;i--)
    {
      if(s->loc[i]==segmentloc[nsegments])
	{
	  segmentlength[nsegments]++;
	  segmentscale[nsegments] = i;
	}
      else
	{
	  if(nsegments>0)
	    {
	      if(s->loc[i]==segmentloc[nsegments-1]) /*speed changes sign*/
		{
		  segmentlength[nsegments] = 10000;
		  nsegments++;
		  segmentlength[nsegments] = 9999;
		  segmentloc[nsegments]=segmentloc[nsegments-1];
		  segmentscale[nsegments]=segmentscale[nsegments-1];
		}
	    }
	  nsegments++;
	  segmentloc[nsegments] = s->loc[i];
	  segmentlength[nsegments]=1;
	  segmentscale[nsegments] = i;
	}
    }
  nsegments++;
  segmentlength[nsegments] = 0;
  segmentscale[nsegments] = -1;
  segmentloc[nsegments]=segmentloc[nsegments-1];
  nsegments++;
  
  DetectLengthExtrema(segmentlength,lengthextrema,nsegments,&nmax,&nmin);

  s->scale_extrema = VipCalloc(nmax+1,sizeof(int),"VipComputeSSSingularitySimpleSpeedMinima");
  if(s->scale_extrema==PB) return(PB);

  s->n_scale_extrema = 0;
  for(i=0;i<nsegments;i++)
    {
      if (lengthextrema[i]==MAX1D)
	{
	  s->scale_extrema[s->n_scale_extrema++] = segmentscale[i];
	}
    }

  if(s->n_scale_extrema==0)
    {
      /*
	VipPrintfWarning("Scale space singularity without speed minimum( assuming scale 10 is correct\n");*/
      s->n_scale_extrema = 1;
      s->scale_extrema = VipCalloc(1,sizeof(int),"VipComputeSSSingularitySimpleSpeedMinima");
      if(s->scale_extrema==PB) return(PB);
      s->scale_extrema[0] = 10;
    }
  return(OK);
}

/*---------------------------------------------------------------------------*/
static int DetectLengthExtrema(
int *input,
Vip_S8BIT *extrema,
int dim,
int *numberplus,
int *numbermoins
)
/*---------------------------------------------------------------------------*/
{
  int temp, left, right;
  int i;
  int *ssptr, *ssptrp1, *ssptrm1;
  Vip_S8BIT *extremaptr;
 
  *numberplus = 0;
  *numbermoins = 0;

  ssptr = input+1;
  /*skip first point*/
  ssptrm1 = ssptr-1;
  ssptrp1 = ssptr+1;
  extremaptr = extrema;
  *extremaptr++=0;

  for ( i = 2; i<dim; i++)/* loop on points */
    {
      temp = *ssptr++;
      left = *ssptrm1++;
      right = *ssptrp1++;
      /* un palier genere un min et un max a chaque extremite, ou deux min et deux max */
      /* dans l'absolu ca n'est pas un probleme, si on trouve deux min ou deux max consecutifs,
	 on les remplace par leur moyenne*/	   
      if(((temp>left)&&(temp>=right))||((temp>=left)&&(temp>right))) *extremaptr++ = MAX1D;
      else if(((temp<left)&&(temp<=right))||((temp<=left)&&(temp<right))) *extremaptr++ = MIN1D;
      else *extremaptr++ = 0;
    }
  *extremaptr++=0;
  if(CheckExtremaConsistencePruneAndCountS8BITInputInt(extrema,dim,numberplus,numbermoins,input)==PB)
    {
      VipPrintfExit(" DetectLengthExtrema");
      return(PB);
    }
  return(OK);
}

/*---------------------------------------------------------------------------*/
static double *GetValBuf(SSSingularity *s, Singularity *sval, int nval )
/*---------------------------------------------------------------------------*/
{
  int loc;
  int iter;
  double *valbuf;

  valbuf = (double *)VipMalloc((s->scale_event+1)*sizeof(double),"GetValBuf");
  if(valbuf==PB) return(PB);

  for(iter=0;iter<=s->scale_event;iter++)
    {
      loc = s->loc[iter];
      while(nval--)
	{
	  if((sval->iter==iter)&&(sval->loc==loc))
	    {
	      valbuf[iter]=sval->val;
	      sval++;
	      break;
	    }
	  sval++;
	}
      if(nval==-1 && iter!=s->scale_event)
	{
	  VipPrintfError("Unconsistent matching");
	  VipPrintfExit("GetValBuf");
	  return(PB);
	}
    }
  return(valbuf);
      
}

/*---------------------------------------------------------------------------*/
static double *GetSpeedBuf(SSSingularity *s, Singularity *sval, int nval )
/*---------------------------------------------------------------------------*/
{
  int loc;
  int iter;
  double *valbuf;

  valbuf = (double *)VipMalloc((s->scale_event+1)*sizeof(double),"GetSpeedBuf");
  if(valbuf==PB) return(PB);

  for(iter=0;iter<=s->scale_event;iter++)
    {
      loc = s->loc[iter];
      while(nval--)
	{
	  if((sval->iter==iter)&&(sval->loc==loc))
	    {
	      valbuf[iter]=sval->speed;
	      sval++;
	      break;
	    }
	  sval++;
	}
      if(nval==-1 && iter!=s->scale_event)
	{
	  VipPrintfError("Unconsistent matching");
	  VipPrintfExit("GetSpeedBuf");
	  return(PB);
	}
    }
  return(valbuf);
      
}
/*---------------------------------------------------------------------------*/
int VipComputeSSSingularityGammaExtrema( SSSingularity *s, float gamma, Singularity *sval, int nval, float dscale )
/*---------------------------------------------------------------------------*/
{
  double *gammabuf, *valbuf; 
  int i;

  if(s==NULL || !sval)
    {
      VipPrintfError("NULL pointer in arguments");
      VipPrintfExit("VipComputeSSSingularityGammaExtrema");
      return(PB);
    }
  if(gamma<0 || gamma>100)
   {
      VipPrintfError("Strange in arguments");
      VipPrintfExit("VipComputeSSSingularityGammaExtrema");
      return(PB);
   }

  if(s->n_scale_extrema!=0) return(OK);

  valbuf = GetValBuf( s, sval, nval);
  if(valbuf==PB) return(PB);

  gammabuf = (double *)VipMalloc((s->scale_event+1)*sizeof(double),"VipComputeSSSingularityGammaExtrema");
  if(gammabuf==PB) return(PB);
  for(i=0;i<=s->scale_event;i++)
    {
      gammabuf[i] = (double)pow((double)(i*dscale),(double)gamma) * valbuf[i] * valbuf[i];
    }
  for(i=1;i<s->scale_event;i++)
    {
      if((gammabuf[i]>gammabuf[i-1])&&(gammabuf[i]>gammabuf[i+1])) s->n_scale_extrema++;
    }
  if(gammabuf[0]>gammabuf[1]) s->n_scale_extrema++;
  if(s->n_scale_extrema==0) 
    {
      printf("No extremum across scale\n");
      fflush(stdout);
    }

  s->scale_extrema = (int *)VipCalloc(s->n_scale_extrema,sizeof(int),"VipComputeSSSingularitySpeedMinima");
  if(s->scale_extrema==PB) return(PB);

  s->n_scale_extrema = 0;
  for(i=1;i<s->scale_event;i++)
    {
      if(((gammabuf[i])>(gammabuf[i-1]))&&((gammabuf[i])>(gammabuf[i+1]))) s->scale_extrema[s->n_scale_extrema++]=i;
    }
  if(gammabuf[0]>gammabuf[1]) s->scale_extrema[s->n_scale_extrema++]=0;

  VipFree(gammabuf);
  VipFree(valbuf);
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipComputeSSSingularitySpeedMinima( SSSingularity *s, Singularity *sval, int nval, float dscale )
/*---------------------------------------------------------------------------*/
{
  double *speedbuf; 
  int i;
  float bidon = dscale; /*compilation warning*/
  bidon = 0.;

  if(s==NULL || !sval)
    {
      VipPrintfError("NULL pointer in arguments");
      VipPrintfExit("VipComputeSSSingularitySpeedMinima");
      return(PB);
    }
 

  if(s->n_scale_extrema!=0) return(OK);

  speedbuf = GetSpeedBuf( s, sval, nval);
  if(speedbuf==PB) return(PB);



  for(i=1;i<s->scale_event;i++)
    {
      if((s->loc[i]==s->loc[i-1])&&(s->loc[i]==s->loc[i+1])&&(fabs(speedbuf[i])<fabs(speedbuf[i-1]))&&(fabs(speedbuf[i])<fabs(speedbuf[i+1]))) s->n_scale_extrema++;
    }
  if(fabs(speedbuf[0])<fabs(speedbuf[1])) s->n_scale_extrema++;
  if(s->n_scale_extrema==0) 
    {
      printf("No extremum across scale\n");
      fflush(stdout);
    }

  s->scale_extrema = (int *)VipCalloc(s->n_scale_extrema,sizeof(int),"VipComputeSSSingularitySpeedMinima");
  if(s->scale_extrema==PB) return(PB);

  s->n_scale_extrema = 0;
  for(i=1;i<s->scale_event;i++)
    {
      if((s->loc[i]==s->loc[i-1])&&(s->loc[i]==s->loc[i+1])&&(fabs(speedbuf[i])<fabs(speedbuf[i-1]))&&(fabs(speedbuf[i])<fabs(speedbuf[i+1])))
	{
	  s->scale_extrema[s->n_scale_extrema++]=i;
	  /*
	  printf("scale %d, speed minimum:%f %f %f\n",(int)(i*dscale),speedbuf[i-1],speedbuf[i],speedbuf[i+1]);
	  */
	}
    }
  if(fabs(speedbuf[0])<fabs(speedbuf[1]))
    {
      s->scale_extrema[s->n_scale_extrema++]=0;
      /*
      printf("scale %d, speed minimum:%f %f %f\n",(int)(i*dscale),speedbuf[i-1],speedbuf[i],speedbuf[i+1]);
      */
    }
  /*
  printf("\n");
  */
  VipFree(speedbuf);
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipComputeSSSingularityGaussianLikeSpeed( SSSingularity *s, Singularity *sval, int nval, float dscale, float sigmamin,  int nature)
/*---------------------------------------------------------------------------*/
{
  double *speedbuf; 
  int i;
  double sigmamin2;
  double EPSILON = 0.0000001;
  double speed_factor;
  float bidon = dscale; /*compilation warning*/
  bidon = 0.;

  if(s==NULL || !sval)
    {
      VipPrintfError("NULL pointer in arguments");
      VipPrintfExit("VipComputeSSSingularityGaussianLikeSpeed");
      return(PB);
    }
  if(sigmamin<EPSILON)
    {
      VipPrintfError("strange values");
      VipPrintfExit("VipComputeSSSingularityGaussianLikeSpeed");
      return(PB);
    }
  if(nature==D1M || nature==D1m)
    speed_factor = 0.5+0.1;
  else if (nature==D2M) speed_factor = sqrt(3.)/2.+0.1;
  else if (nature==D2m) speed_factor = 0.1;
  else
    {
      VipPrintfError("Unknown nature");
      VipPrintfExit("VipComputeSSSingularityGaussianLikeSpeed");
      return(PB);
    }
  sigmamin2 = sigmamin*sigmamin;

  if(s->n_scale_extrema!=0) return(OK);

  speedbuf = GetSpeedBuf( s, sval, nval);
  if(speedbuf==PB) return(PB);

  for(i=0;i<=s->scale_event;i++)
    { 
	  if(fabs(speedbuf[i])<(speed_factor/sqrt(sigmamin2+i))) s->n_scale_extrema++;
    }
  if(s->n_scale_extrema==0) 
    {
      printf("No extremum across scale\n");
      fflush(stdout);
    }

  s->scale_extrema = (int *)VipCalloc(s->n_scale_extrema,sizeof(int),"VipComputeSSSingularitySpeedMinima");
  if(s->scale_extrema==PB) return(PB);

  s->n_scale_extrema = 0;
  for(i=1;i<s->scale_event;i++)
    {
      if(fabs(speedbuf[i])<(speed_factor/sqrt(sigmamin2+i)))
	{
	  s->scale_extrema[s->n_scale_extrema++]=i;
	}
    }
  VipFree(speedbuf);
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipInsertSSSingularity(
SSSingularity *inserted,
SSSingularity **list)
/*---------------------------------------------------------------------------*/
{
  SSSingularity *walker, *followwalker=NULL;
  int lemin;

  if((inserted==NULL)||(list==NULL))
    {
      VipPrintfError("NULL pointer in arguments");
      VipPrintfExit("VipInsertSSSingularityOnRight");
      return(PB);
    }

  inserted->right = NULL;
  inserted->left = NULL;
  if(*list==NULL) *list = inserted;
  else
    {
      walker = *list;
      while(walker!=NULL)
	{
	  lemin = mVipMin(walker->scale_event,inserted->scale_event);
	  if(((walker->loc[0]<inserted->loc[0]))
	    ||((walker->loc[0]==inserted->loc[0])&&(walker->loc[lemin]<inserted->loc[lemin])))
	    {
	      followwalker = walker;

	      walker = walker->right;
	    }
	  else break;
	}
      if(walker==NULL)
	{
	  followwalker->right = inserted;
	  inserted->left = followwalker;
	}
      else if(walker==*list)
	{
	  inserted->right = *list;
	  (*list)->left = inserted;
	  *list = inserted;
	}
      else
	{
	  followwalker->right = inserted;
	  inserted->left = followwalker;
	  inserted->right = walker;
	  walker->left = inserted;
	}
    }

  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipInsertSSCascadeInList(
SSCascade *inserted,
SSCascade **list)
/*---------------------------------------------------------------------------*/
{
  SSCascade *walker, *followwalker=NULL;

  if((inserted==NULL)||(list==NULL))
    {
      VipPrintfError("NULL pointer in arguments");
      VipPrintfExit("VipInsertSSCascadeInList");
      return(PB);
    }

  if(*list==NULL) *list = inserted;
  else
    {
      walker = *list;
      while((walker!=NULL)&&(walker->volume>inserted->volume))
	{
	  followwalker = walker;
	  walker = walker->next;
	}
      if(walker==NULL)
	{
	  followwalker->next = inserted;
	}
      else if(walker==*list)
	{
	  inserted->next = *list;
	  *list = inserted;
	}
      else
	{
	  followwalker->next = inserted;
	  inserted->next = walker;
	}
    }
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipScaleInsertSSCascadeInList(
SSCascade *inserted,
SSCascade **list)
/*---------------------------------------------------------------------------*/
{
  SSCascade *walker, *followwalker=NULL;

  if((inserted==NULL)||(list==NULL))
    {
      VipPrintfError("NULL pointer in arguments");
      VipPrintfExit("VipInsertSSCascadeInList");
      return(PB);
    }

  if(*list==NULL) *list = inserted;
  else
    {
      walker = *list;
      while((walker!=NULL)&&(walker->scale_event>inserted->scale_event))
	{
	  followwalker = walker;
	  walker = walker->next;
	}
      if(walker==NULL)
	{
	  followwalker->next = inserted;
	}
      else if(walker==*list)
	{
	  inserted->next = *list;
	  *list = inserted;
	}
      else
	{
	  followwalker->next = inserted;
	  inserted->next = walker;
	}
    }
  return(OK);
}




/*---------------------------------------------------------------------------*/
Volume *VipDetectExtremaFrom1DSS( Volume *ss)
/*---------------------------------------------------------------------------*/
{
  Volume *extremavol;
  int scalemin;
  int scalemax;
  Vip_DOUBLE *ssptr, *lineptr;
  Vip_S8BIT *extrema, *extremaptr;
  VipOffsetStruct *vos;
  int i,scale;
  int xsize;
  int nblobs;
  double temp,left,right;
  int first_extremum;
  int last_extremum;

  if(!ss)
    {
      VipPrintfError("Arg in VipDetectExtremaFrom1DSS\n");
      VipPrintfExit("VipDetectExtremaFrom1DSS");
      return(PB);
    }
  if (VipVerifyAll(ss)==PB)
    {
      VipPrintfExit("(histo)VipDetectExtremaFrom1DSS");
      return(PB);
    }
  if (VipTestType(ss,VDOUBLE)!=OK)
    {
      VipPrintfError("Sorry,  VipDetectExtremaFrom1DSS is only implemented for VDOUBLE volume");
      VipPrintfExit("(histo)VipDetectExtremaFrom1DSS");
      return(PB);
    }

  extremavol = VipDuplicateVolumeStructure(ss,"extrema");
  if(extremavol==PB) return(PB);
  VipSetType(extremavol,S8BIT);
  if(VipAllocateVolumeData(extremavol)==PB) return(PB);

  vos = VipGetOffsetStructure(ss);

  ssptr = VipGetDataPtr_VDOUBLE(ss) + vos->oFirstPoint;
  extrema = VipGetDataPtr_S8BIT(extremavol) + vos->oFirstPoint;

  scalemin = 0;
  scalemax = mVipVolSizeY(ss)-1;
  xsize = mVipVolSizeX(ss)-2;

  for ( scale = scalemin; scale<=scalemax; scale++ )            /* loop on lines */
    {
      lineptr = ssptr;
      extremaptr = extrema;
      for(i=mVipVolSizeX(ss);i--;) *extremaptr++=0;
      nblobs = 0;
      ssptr++; /*skip first point*/
      for ( i = 1; i<=xsize; i++)/* loop on points */
	{
	  temp = *ssptr;
	  left = *(ssptr-1);
	  right = *(ssptr+1);
	  /* un palier genere un min et un max a chaque extremite, ou deux min et deux max */
	  /* dans l'absolu ca n'est pas un probleme, si on trouve deux min ou deux max consecutifs,
	     on les remplace par leur moyenne*/
	   
	  if(((temp>left)&&(temp>=right))||((temp>=left)&&(temp>right))) extrema[i] = MAX1D;
	  else if(((temp<left)&&(temp<=right))||((temp<=left)&&(temp<right))) extrema[i] = MIN1D;
	 
          ssptr++;
	}
      ssptr++; /*skip last point*/
      ssptr += vos->oPointBetweenLine;  /*skip border points*/
      if(CheckExtremaConsistenceAndPruneS8BIT(extrema,mVipVolSizeX(ss),&first_extremum,&last_extremum,lineptr)==PB)
	{
	  VipPrintfExit(" VipDetectExtremaFrom1DSS");
	  return(PB);
	}
      extrema += vos->oLine;
    }
  return(extremavol);

}



/*---------------------------------------------------------------------------*/
static int CheckExtremaConsistenceAndPruneS8BIT(
Vip_S8BIT *extrema,
int size,
int *first_extrema,
int *last_extrema,
Vip_DOUBLE *val)
/*---------------------------------------------------------------------------*/
{
  int i, last, antelast;
  int lasti, antelasti=0;
  int VIERGE = -12;

  last = VIERGE;
  antelast = VIERGE;
  i=0;
  lasti = 0;
  while((last==VIERGE)&&(i<size))
    {
      if(extrema[i])
	{
	  last = extrema[i];
	  lasti = i;
	  *first_extrema = i;
	}
      i++;
    }
  for(;i<size;i++)
    {
      if(extrema[i])
	{
	  if (last==extrema[i])  /*les plateaux extrema sont reduits a un seul extremum*/
	    {
	      /*VipPrintfWarning("Two consecutive equivalent extrema (no generic histogram Olivier?...)");*/
	      extrema[lasti] = 0;
	      extrema[i] = 0;
	      extrema[(i+lasti)/2] = last;
	      lasti = (i+lasti)/2;
	    }
	  else if (val[lasti]==val[i]) /*palier sans extremum*/
	    {
	      extrema[lasti] = 0;
	      extrema[i]=0;
	      if(antelast!=VIERGE)
		{
		  last = antelast;
		  lasti = antelasti;
		}
	      else
		{
		  last=VIERGE;
		  while((last==VIERGE)&&(i<size))
		    {
		      if(extrema[i])
			{
			  last = extrema[i];
			  lasti = i;
			  *first_extrema = i;
			}
		      i++;
		    }
		}
	    }
	  else
	    {
	      antelast = last;
	      antelasti = lasti;
	      last = extrema[i];
	      lasti = i;
	    }
	}
    }
  *last_extrema = lasti;
  return(OK);
}



/*---------------------------------------------------------------------------*/
int VipCreateGnuplotFileFromExtrema(
  Vip1DScaleSpaceStruct *volstruct,
  char *name,
  int tracked,
  int psfile,
  int title,
  int writeD0,
  int writeD1,
  int writeD2,
  int writeD3,
  int writeD4
)
{
  return VipCreatePlotFileFromExtrema( 0, volstruct, name, tracked, psfile,
                                       title, writeD0, writeD1, writeD2,
                                       writeD3, writeD4 );
}

/*---------------------------------------------------------------------------*/
int VipCreatePlotFileFromExtrema(
  int plottype,
  Vip1DScaleSpaceStruct *volstruct,
  char *name,
  int tracked,
  int psfile,
  int title,
  int writeD0,
  int writeD1,
  int writeD2,
  int writeD3,
  int writeD4
)
/*---------------------------------------------------------------------------*/

{
  float ratio;
  FILE *f[10], *fcommand, *fcommandps, *minimafile;
  char filename[10][512];
  char command[512];
  char histoname[512];
  char minimafilename[512];
  int some_minima=VFALSE;
  int some_D1M=VFALSE;
  int some_D1m=VFALSE;

  int lemin1d=0, lemax1d=0;
  int i;
  Vip_S8BIT *extrema;
  int x, y;
  VipOffsetStruct *vos;
  char fcommandname[512], fcommandpsname[512];
  int xmax, ymax, hmin, hmax;
  float t10, t20, t50,  t100, t200, t500,  t1000, t2000;
  char gpdir[1024];

  if(name==NULL || volstruct==NULL)
    {
      VipPrintfError("NULL arg");
      VipPrintfExit("VipCreatePlotFileFromExtrema");
      return(PB);
    }

  if(tracked==SS_TRACKED_EXTREMUM)
    {
      lemin1d = MIN1DTRACKED;
      lemax1d = MAX1DTRACKED;
    }
  else if(tracked==SS_SIMPLE_EXTREMUM)
    {
      lemin1d = MIN1D;
      lemax1d = MAX1D;
    }
  else if(tracked==SS_CASCADE_EXTREMUM)
    {
      lemin1d = MIN1DCASCADE;
      lemax1d = MAX1DCASCADE;
    }


  sprintf( gpdir, "%s%cgpdir_%s", VipTmpDirectory(), VipFileSeparator(), name );
  VipMkdir( gpdir );

  xmax = 0;
  ymax = 0;
  for(i=mVipHistoRangeMin(volstruct->histo);i<=mVipHistoRangeMax(volstruct->histo);i++)   
    {
      if( mVipHistoVal(volstruct->histo,i) > -1) xmax = i;
      if(i>(20))
	if( mVipHistoVal(volstruct->histo,i) > ymax) ymax = mVipHistoVal(volstruct->histo,i);
    }
  xmax = (int)(1.*xmax) + volstruct->offset;
  ymax = (int)(1.1*ymax);
  /*
  printf("computed xmax: %d, ymax: %d\n", xmax, ymax);
  */
  hmax = (int)(log10((double)volstruct->itermax*0.01+1)*100);
  hmin = -(int)(0.5*hmax);

  ratio = -hmin / (float)ymax;

  sprintf(fcommandname,"%s%c", VipTmpDirectory(), VipFileSeparator() );
  strcat(fcommandname,name);
  if( plottype == 0 )
    strcat(fcommandname,".gp");
  else
    strcat( fcommandname, ".py" );
  fcommand = fopen(fcommandname,"w");
  if(!fcommand)
    {
      fprintf(stderr,"can not open %s for writing\n", fcommandname);
      VipPrintfExit("VipCreatePlotFileFromExtrema");
      return(PB);
    }

  if(psfile==VTRUE)
    {
      sprintf(fcommandpsname,"%s%c", VipTmpDirectory(), VipFileSeparator() );
      strcat(fcommandpsname,name);
      if( plottype == 0 )
      {
        strcat(fcommandpsname,".gpps");

        fcommandps= fopen(fcommandpsname,"w");
        if(!fcommandps)
        {
          fprintf(stderr,"can not open %s for writing\n", fcommandpsname);
          VipPrintfExit("VipCreatePlotFileFromExtrema");
          return(PB);
        }

        fprintf(fcommandps,"set terminal post default color\n");
        fprintf(fcommandps,"set output \"%s.ps\"\n",name);
        fclose(fcommandps);
      }
      else
      {
        /* matplotlib ps output */
        strcat(fcommandpsname,"_ps.py");

        fcommandps= fopen(fcommandpsname,"w");
        if(!fcommandps)
        {
          fprintf(stderr,"can not open %s for writing\n", fcommandpsname);
          VipPrintfExit("VipCreatePlotFileFromExtrema");
          return(PB);
        }
        fprintf( fcommandps, "#!/usr/bin/env python\n" );
        fprintf( fcommandps, "import matplotlib, os\n" );
        fprintf( fcommandps, "matplotlib.use( 'ps' )\n" );
        fprintf( fcommandps, "import pylab\n" );
        fprintf( fcommandps, "def show():\n" );
        fprintf( fcommandps, "  pylab.savefig( os.path.join( '%s', '%s.ps' ) )\n", VipTmpDirectory(), name );
        fprintf( fcommandps, "pylab.show = show\n" );
        fprintf( fcommandps, "execfile( '%s' )\n", fcommandname );
        fclose( fcommandps );
      }
    }
 
 
  t10 = (float)log10((double)10/volstruct->dscale*0.01+1)*100;
  t20 = (float)log10((double)20/volstruct->dscale*0.01+1)*100;
  t50 = (float)log10((double)50/volstruct->dscale*0.01+1)*100;
  t100 = (float)log10((double)100/volstruct->dscale*0.01+1)*100;
  t200 = (float)log10((double)200/volstruct->dscale*0.01+1)*100;
  t500 = (float)log10((double)500/volstruct->dscale*0.01+1)*100;
  t1000 = (float)log10((double)1000/volstruct->dscale*0.01+1)*100;
  t2000 = (float)log10((double)2000/volstruct->dscale*0.01+1)*100;

  switch( plottype )
  {
  case 0:
    /* gnuplot*/
    fprintf(fcommand,"#gnuplot command file to look at %s histogram and primal sketch analysis result\n",name);
    fprintf(fcommand,"cd \"%s%cgpdir_%s\"\n", VipTmpDirectory(),
            VipFileSeparator(), name );
    fprintf(fcommand,"set xtics (\"0\" 0, \"\" 10, \"\" 20, \"\" 30, \"\"40, \"50\" 50,\"\" 60,\"\" 70,\"\" 80,\"\" 90,\"100\" 100,\"\" 110,\"\" 120,\"\" 130,\"\" 140,\"150\" 150,\"\" 160,\"\" 170,\"\" 180,\"\" 190,\"200\" 200,\"\" 210,\"\" 220,\"\" 230,\"\" 240,\"250\" 250,\"\" 260,\"\" 270,\"\" 280,\"\" 290,\"300\" 300,\"\" 310,\"\" 320,\"\" 330,\"\" 340,\"350\" 350)\n");
    fprintf(fcommand,"set ytics (\"10\" %f,\"20\" %f, \"50\" %f, \"100\" %f, \"200\" %f,\"500\" %f, \"1000\" %f, \"2000\" %f)\n",t10,t20,t50,t100,t200,t500,t1000,t2000);
    fprintf(fcommand,"p(x)=%d+%f*x\n",hmin,ratio);
    if(title==VTRUE)
      fprintf(fcommand,"plot [%d:%d] [%d:%d] \"%s_offset.his\" thru p(x) w l lw 5",0,xmax,hmin,hmax,name);
    else
      fprintf(fcommand,"plot [%d:%d] [%d:%d] \"%s_offset.his\" thru p(x) notitle w l lw 5",0,xmax,hmin,hmax,name);
    break;
  case 1:
    /* matplotlib */
    fprintf( fcommand, "#!/usr/bin/env python\n" );
    fprintf( fcommand, "#matplotlib command file to look at %s histogram and "
      "primal sketch analysis result\n\n", name );
    fprintf( fcommand, "import pylab, numpy, os\n\n" );
    /* fprintf( fcommand, "pylab.yscale( 'log' )\n" ); */
    fprintf( fcommand, "name = '%s'\n", name );
    fprintf( fcommand, "tmpdir = os.path.join( '%s', 'gpdir_' + name )\n",
             VipTmpDirectory() );
    fprintf( fcommand, "his_offset_name = os.path.join( tmpdir, "
      "name + '_offset.his' )\n" );
    fprintf( fcommand, "tab = open( his_offset_name ).readlines()\n" );
    fprintf( fcommand, "his_offset = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
    if( title == VTRUE )
    {
      fprintf( fcommand, "pylab.title( name )\n" );
      fprintf( fcommand, "kw = { 'label': name + '_offset' }\n" );
    }
    else
      fprintf( fcommand, "kw = {}\n" );
    fprintf( fcommand, "pylab.plot( his_offset[0], %d + %f * his_offset[1], lw=5, **kw )\n", hmin, ratio );
    fprintf( fcommand, "pylab.yticks( [ %f, %f, %f, %f, %f, %f, %f, %f ], [ 10, 20, 50, 100, 200, 500, 1000, 2000 ] )\n",
             t10, t20, t50, t100, t200, t500, t1000, t2000 );
    break;
  default:
    break;
  }

 
  sprintf(histoname,"%s%cgpdir_%s%c", VipTmpDirectory(), VipFileSeparator(),name, 
          VipFileSeparator() );
  strcat(histoname,name);
  strcat(histoname,"_offset");
  if(VipWriteHisto(volstruct->histo,histoname,WRITE_HISTO_ASCII)==PB) return(PB);
  
 
  sprintf(minimafilename,"%s%cgpdir_%s%c", VipTmpDirectory(), VipFileSeparator(), name,
          VipFileSeparator() );
  strcat(minimafilename,name);
  strcat(minimafilename,".smin");
  minimafile = fopen(minimafilename,"w");
  if(!minimafile)
    {
      fprintf(stderr,"can not open %s for writing\n", minimafilename);
      VipPrintfExit("VipCreateGnuplotFileFromExtrema");
      return(PB);
    }



  for(i=0;i<10;i++)
    {
      sprintf(filename[i],"%s%Cgpdir_%s%c", VipTmpDirectory(), 
              VipFileSeparator(), name, VipFileSeparator() );
      strcat(filename[i],name);
    }
  if(writeD0==VTRUE)
      {
	  strcat(filename[D0m],".D0min");
	  strcat(filename[D0M],".D0Max");
      }
  else
      {
	  strcpy(filename[D0m],"NO");
	  strcpy(filename[D0M],"NO");
      }
  if(writeD1==VTRUE)
      {
	  strcat(filename[D1m],".D1min");
	  strcat(filename[D1M],".D1Max");
      }
  else
      {
	  strcpy(filename[D1m],"NO");
	  strcpy(filename[D1M],"NO");
      }
  if(writeD2==VTRUE)
      {
	  strcat(filename[D2m],".D2min");
	  strcat(filename[D2M],".D2Max");
      }
  else
      {
	  strcpy(filename[D2m],"NO");
	  strcpy(filename[D2M],"NO");
      }
  if(writeD3==VTRUE)
      {
	  strcat(filename[D3m],".D3min");
	  strcat(filename[D3M],".D3Max");
      }
  else
      {
	  strcpy(filename[D3m],"NO");
	  strcpy(filename[D3M],"NO");
      }
  if(writeD4==VTRUE)
      {
	  strcat(filename[D4m],".D4min");
	  strcat(filename[D4M],".D4Max");
      }
  else
      {
	  strcpy(filename[D4m],"NO");
	  strcpy(filename[D4M],"NO");
      }

  for(i=0;i<10;i++)
    {
	if(!strcmp(filename[i],"NO"))
	   {
	       f[i] = NULL;
	       continue;
	   }
	   f[i] = fopen(filename[i],"w");
	   if(f[i]==NULL)
	   {
	       fprintf(stderr,"can not open %s for writing\n", filename[i]);
	       VipPrintfExit("VipCreateGnuplotFileFromExtrema");
	       return(PB);
	   }
    }

  vos = VipGetOffsetStructure(volstruct->d1_e);  

  if(writeD0==VTRUE && volstruct->ss_e!=NULL)
      {
	  extrema = VipGetDataPtr_S8BIT(volstruct->ss_e) + vos->oFirstPoint;
	  for ( y=0;y <=volstruct->itermax; y++ )            
	      {
		  for ( x=0 ; x<mVipVolSizeX(volstruct->ss_e); x++  )
		      {
			  if(*extrema==lemin1d) fprintf(f[D0m],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			  else if(*extrema==lemax1d) fprintf(f[D0M],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			  else if((*extrema==MAX1DSPEEDMINIMUM) || (*extrema==MIN1DSPEEDMINIMUM))
                            {
                              fprintf(minimafile,"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
                              some_minima=VTRUE;
                            }
			  extrema++;
		      }
		  extrema += vos->oPointBetweenLine;  
	      }
      }

  if(writeD1==VTRUE && volstruct->d1_e!=NULL)
      {
	  extrema = VipGetDataPtr_S8BIT(volstruct->d1_e) + vos->oFirstPoint;
	  for ( y=0;y <mVipVolSizeY(volstruct->d1_e); y++ )            
	      {
		  for ( x=0 ; x<mVipVolSizeX(volstruct->d1_e); x++  )
		      {
			  if(*extrema==lemin1d)
                            {
                              fprintf(f[D1m],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
                              some_D1m=VTRUE;
                            }
			  else if(*extrema==lemax1d)
                            {
                              fprintf(f[D1M],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
                              some_D1M=VTRUE;
                            }
			  else if((*extrema==MAX1DSPEEDMINIMUM) || (*extrema==MIN1DSPEEDMINIMUM))
                            {
                              fprintf(minimafile,"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
                              some_minima=VTRUE;
                            }
			  extrema++;
		      }
		  extrema += vos->oPointBetweenLine;  
	      }
      }
 
  if(writeD2==VTRUE && volstruct->d2_e!=NULL)
      {
	  extrema = VipGetDataPtr_S8BIT(volstruct->d2_e) + vos->oFirstPoint;
	  for ( y=0;y <mVipVolSizeY(volstruct->d2_e); y++ )            
	      {
		  for ( x=0 ; x<mVipVolSizeX(volstruct->d2_e); x++  )
		      {
			  
			  if(*extrema==lemin1d) fprintf(f[D2m],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			  else if(*extrema==lemax1d) fprintf(f[D2M],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			  else if((*extrema==MAX1DSPEEDMINIMUM) || (*extrema==MIN1DSPEEDMINIMUM))
                            {
                              fprintf(minimafile,"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
                              some_minima=VTRUE;
                            }
			  extrema++;
		      }
		  extrema += vos->oPointBetweenLine;  
	      }
      }

 
   if(writeD3==VTRUE && volstruct->d3_e!=NULL)
       {
	   extrema = VipGetDataPtr_S8BIT(volstruct->d3_e) + vos->oFirstPoint;
	   for ( y=0;y <mVipVolSizeY(volstruct->d3_e); y++ )            /* loop on lines */
	       {
		   for ( x=0 ; x<mVipVolSizeX(volstruct->d3_e); x++  )/* loop on points */
		       {
			   if(*extrema==lemin1d) fprintf(f[D3m],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			   else if(*extrema==lemax1d) fprintf(f[D3M],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			   else if((*extrema==MAX1DSPEEDMINIMUM) || (*extrema==MIN1DSPEEDMINIMUM))
                             {
                               fprintf(minimafile,"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
                               some_minima=VTRUE;
                             }
			   extrema++;
		       }
		   extrema += vos->oPointBetweenLine;  /*skip border points*/
	       }
       }
  if(writeD4==VTRUE && volstruct->d4_e!=NULL)
       {
	   extrema = VipGetDataPtr_S8BIT(volstruct->d4_e) + vos->oFirstPoint;
	   for ( y=0;y <mVipVolSizeY(volstruct->d4_e); y++ )            /* loop on lines */
	       {
		   for ( x=0 ; x<mVipVolSizeX(volstruct->d4_e); x++  )/* loop on points */
		       {
			   if(*extrema==lemin1d) fprintf(f[D4m],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			   else if(*extrema==lemax1d) fprintf(f[D4M],"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
			   else if((*extrema==MAX1DSPEEDMINIMUM) || (*extrema==MIN1DSPEEDMINIMUM)) 
                             {
                               fprintf(minimafile,"%d %f\n",x,(float)log10((double)y*0.01+1)*100);
                               some_minima=VTRUE;
                             }
			   extrema++;
		       }
		   extrema += vos->oPointBetweenLine;  /*skip border points*/
	       }
       }
  
  for(i=0;i<10;i++)
      {
	  if (f[i]!=NULL) fclose(f[i]);
      }
  fclose(minimafile);

  switch( plottype )
  {
  case 0:
    /* gnuplot */
    if(writeD0==VTRUE)
    {
      if(title==VTRUE)
      {
        fprintf(fcommand,", \"%s.D0Max\" w p pt 6",name);
        fprintf(fcommand,", \"%s.D0min\" w p pt 6",name);
      }
      else
      {
        fprintf(fcommand,", \"%s.D0Max\" notitle w p pt 6",name);
        fprintf(fcommand,", \"%s.D0min\" notitle w p pt 6",name);
      }
    }
    if(writeD1==VTRUE)
    {
      if(title==VTRUE)
      {
        if (some_D1M==VTRUE)  fprintf(fcommand,", \"%s.D1Max\" w p pt 6",name);
        if (some_D1m==VTRUE)  fprintf(fcommand,", \"%s.D1min\" w p pt 6",name);
      }
      else
      {
        if (some_D1M==VTRUE) fprintf(fcommand,", \"%s.D1Max\" notitle w p pt 6",name);
        if (some_D1m==VTRUE) fprintf(fcommand,", \"%s.D1min\" notitle w p pt 6",name);
      }
    }
    if(writeD2==VTRUE)
    {
      if(title==VTRUE)
      {
        fprintf(fcommand,", \"%s.D2Max\" w p pt 6",name);
        fprintf(fcommand,", \"%s.D2min\" w p pt 6",name);
      }
      else
      {
        fprintf(fcommand,", \"%s.D2Max\" notitle w p pt 6",name);
        fprintf(fcommand,", \"%s.D2min\" notitle w p pt 6",name);
      }
    }
    if(writeD3==VTRUE)
    {
      if(title==VTRUE)
      {
        fprintf(fcommand,", \"%s.D3Max\" w p pt 6",name);
        fprintf(fcommand,", \"%s.D3min\" w p pt 6",name);
      }
      else
      {
        fprintf(fcommand,", \"%s.D3Max\" notitle w p pt 6",name);
        fprintf(fcommand,", \"%s.D3min\" notitle w p pt 6",name);
      }
    }
    if(writeD4==VTRUE)
    {
      if(title==VTRUE)
      {
        fprintf(fcommand,", \"%s.D4min\" w p pt 6",name);
        fprintf(fcommand,", \"%s.D4Max\" w p pt 6",name);
      }
      else
      {
        fprintf(fcommand,", \"%s.D4min\" notitle w p pt 6",name);
        fprintf(fcommand,", \"%s.D4Max\" notitle w p pt 6",name);
      }
    }

    if (some_minima==VTRUE)
    {
      if(title==VTRUE)
      {
        fprintf(fcommand,", \"%s.smin\" w p pt 1 ps 5\n",name);
      }
      else
      {
        fprintf(fcommand,", \"%s.smin\" notitle w p pt 1 ps 5",name);
        /*
          fprintf(fcommand,", \"%s.smin\" notitle w p pt 1 ps 4",name);
          fprintf(fcommand,", \"%s.smin\" notitle w p pt 1 ps 3",name);
          fprintf(fcommand,", \"%s.smin\" notitle w p pt 1 ps 2",name);
          fprintf(fcommand,", \"%s.smin\" notitle w p pt 1 ps 1\n",name);*/
      }
    }
    else
      fprintf(fcommand,"\n");

    fprintf(fcommand,"cd \"..\"\n");
    fprintf(fcommand,"pause mouse any\n");
    break;

  case 1: /* matplotlib */
    fprintf( fcommand, "kw = {}\n" );
    if(writeD0==VTRUE)
    {
      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D0Max' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D0M'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );

      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D0min' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D0m'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );
    }

    if(writeD1==VTRUE)
    {
      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D1Max' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D1M'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );

      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D1min' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D1m'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );
    }

    if(writeD2==VTRUE)
    {
      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D2Max' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D2M'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );

      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D2min' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D2m'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );
    }

    if(writeD3==VTRUE)
    {
      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D3Max' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D3M'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );

      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D3min' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D3m'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );
    }

    if(writeD4==VTRUE)
    {
      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D4Max' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D4M'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );

      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.D4min' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.D4m'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], '.', **kw )\n" );
    }

    if (some_minima==VTRUE)
    {
      fprintf( fcommand, "tab = open( os.path.join( tmpdir, name + '.smin' ) ).readlines()\n" );
      fprintf( fcommand, "arr = numpy.array( [[float(x) for x in l.split()] for l in tab ] ).transpose()\n" );
      if( title == VTRUE )
        fprintf( fcommand, "kw[ 'label' ] = name + '.smin'\n" );
      fprintf( fcommand, "pylab.plot( arr[0], arr[1], 'o', **kw )\n" );
    }

    fprintf( fcommand, "pylab.xlim( xmin=%d, xmax=%d )\n", 0, xmax );
    fprintf( fcommand, "pylab.ylim( ymin=%d, ymax=%d )\n", hmin, hmax );
    if( title == VTRUE )
      fprintf( fcommand, "pylab.legend()\n" );
    fprintf( fcommand, "pylab.show()\n" );
  }
  fclose(fcommand);

  if(psfile==VTRUE)
  {
    switch( plottype )
    {
    case 0: /* gnuplot */
      sprintf(command,"gnuplot %s %s", fcommandpsname, fcommandname);
      system(command);
      break;
    case 1: /* matplotlib */
      sprintf(command,"python %s", fcommandpsname );
      printf( "%s\n", command );
      system(command);
    }
    VipRm( fcommandpsname, 0 );
  }


  return(OK);
}



/*---------------------------------------------------------------------------*/
int VipDetectSSExtrema(
double *input,
Vip_S8BIT *extrema,
int dim,
int *numberplus,
int *numbermoins
)
/*---------------------------------------------------------------------------*/
{
  double temp, left, right;
  int i;
  double *ssptr, *ssptrp1, *ssptrm1;
  Vip_S8BIT *extremaptr;
 
  *numberplus = 0;
  *numbermoins = 0;

  ssptr = input+1;
  /*skip first point*/
  ssptrm1 = ssptr-1;
  ssptrp1 = ssptr+1;
  extremaptr = extrema;
  *extremaptr++=0;
  for ( i = 2; i<dim; i++)/* loop on points */
    {
      temp = *ssptr++;
      left = *ssptrm1++;
      right = *ssptrp1++;
      /* un palier genere un min et un max a chaque extremite, ou deux min et deux max */
      /* dans l'absolu ca n'est pas un probleme, si on trouve deux min ou deux max consecutifs,
	 on les remplace par leur moyenne*/	   
      if(((temp>left)&&(temp>=right))||((temp>=left)&&(temp>right))) *extremaptr++ = MAX1D;
      else if(((temp<left)&&(temp<=right))||((temp<=left)&&(temp<right))) *extremaptr++ = MIN1D;
      else *extremaptr++ = 0;
    }
  *extremaptr++=0;
  if(CheckExtremaConsistencePruneAndCountS8BIT(extrema,dim,numberplus,numbermoins,input)==PB)
    {
      VipPrintfExit(" VipDetectExtremaFrom1DSS");
      return(PB);
    }
  return(OK);
}

/*---------------------------------------------------------------------------*/
static int CheckExtremaConsistencePruneAndCountS8BIT(
Vip_S8BIT *extrema,
int size,
int *numberplus,
int *numbermoins,
Vip_DOUBLE *val)
/*---------------------------------------------------------------------------*/
{
  int i, last, antelast;
  int lasti=0, antelasti=0;
  int VIERGE = -12;

  last = VIERGE;
  antelast = VIERGE;
  i=0;
  lasti = 0;
  while((last==VIERGE)&&(i<size))
    {
      if(extrema[i])
	{
	  last = extrema[i];
	  lasti = i;
	  if(extrema[i]==MAX1D) *numberplus +=1;
	  else *numbermoins +=1;
	}
      i++;
    }
  for(;i<size;i++)
    {
      if(extrema[i])
	{
	  if (last==extrema[i])  /*les plateaux extrema sont reduits a un seul extremum*/
	    {
	      /*VipPrintfWarning("Two consecutive equivalent extrema (no generic histogram Olivier?...)");*/
	      extrema[lasti] = 0;
	      extrema[i] = 0;
	      extrema[(i+lasti)/2] = last;
	      lasti = (i+lasti)/2;
	    }
	  else if (val[lasti]==val[i]) /*palier sans extremum*/
	    {
	      if(extrema[lasti]==MAX1D) *numberplus -=1;
	      else *numbermoins -=1;
	      extrema[lasti] = 0;
	      extrema[i]=0;
	      if(antelast!=VIERGE)
		{
		  last = antelast;
		  lasti = antelasti;
		}
	      else
		{
		  last=VIERGE;
		  while((last==VIERGE)&&(i<size))
		    {
		      if(extrema[i])
			{
			  last = extrema[i];
			  lasti = i;
                          if(extrema[i]==MAX1D) *numberplus +=1;
                          else *numbermoins +=1;
			}
		      i++;
		    }
		  i--;
		}
	    }
	  else
	    {
	      if(extrema[i]==MAX1D) *numberplus +=1;
	      else *numbermoins +=1;
	      antelast = last;
	      antelasti = lasti;
	      last = extrema[i];
	      lasti = i;
	    }
	}
    }
  return(OK);
}

/*---------------------------------------------------------------------------*/
static int CheckExtremaConsistencePruneAndCountS8BITInputInt(
Vip_S8BIT *extrema,
int size,
int *numberplus,
int *numbermoins,
int *val)
/*---------------------------------------------------------------------------*/
{
  int i, last, antelast;
  int lasti, antelasti=0;
  int VIERGE = -12;

  last = VIERGE;
  antelast = VIERGE;
  i=0;
  lasti = 0;
  while((last==VIERGE)&&(i<size))
    {
      if(extrema[i])
	{
	  last = extrema[i];
	  lasti = i;
	  if(extrema[i]==MAX1D) *numberplus +=1;
	  else *numbermoins +=1;
	}
      i++;
    }
  for(;i<size;i++)
    {
      if(extrema[i])
	{
	  if (last==extrema[i])  /*les plateaux extrema sont reduits a un seul extremum*/
	    {
	      /*VipPrintfWarning("Two consecutive equivalent extrema (no generic histogram Olivier?...)");*/
	      extrema[lasti] = 0;
	      extrema[i] = 0;
	      extrema[(i+lasti)/2] = last;
	      lasti = (i+lasti)/2;
	    }
	  else if (val[lasti]==val[i]) /*palier sans extremum*/
	    {
	      if(extrema[lasti]==MAX1D) *numberplus -=1;
	      else *numbermoins -=1;
	      extrema[lasti] = 0;
	      extrema[i]=0;
	      if(antelast!=VIERGE)
		{
		  last = antelast;
		  lasti = antelasti;
		}
	      else
		{
		  last=VIERGE;
		  while((last==VIERGE)&&(i<size))
		    {
		      if(extrema[i])
			{
			  last = extrema[i];
			  lasti = i;
                          if(extrema[i]==MAX1D) *numberplus +=1;
                          else *numbermoins +=1;
			}
		      i++;
		    }
		  i--;
		}
	    }
	  else
	    {
	      if(extrema[i]==MAX1D) *numberplus +=1;
	      else *numbermoins +=1;
	      antelast = last;
	      antelasti = lasti;
	      last = extrema[i];
	      lasti = i;
	    }
	}
    }
  return(OK);
}
