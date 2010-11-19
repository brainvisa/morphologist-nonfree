/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_histo_analysis.c * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 27/04/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Scalespace analysis for tissue segmentation
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

/*---------------------------------------------------------------------------*/
static int PutUndersamplingRatio(VipT1HistoAnalysis *ana,int ratio);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
VipT1HistoAnalysis *VipReadT1HistoAnalysis(char *name)
/*---------------------------------------------------------------------------*/
{
    VipT1HistoAnalysis *ana;
    FILE *f;
    char filename[256];
    char error[300];
    char buf[512];
    int mean, sigma;
    char buf1[512],buf2[512],buf3[512];

    ana = VipCalloc(1,sizeof(VipT1HistoAnalysis),"VipAnalyseCascades");
    if(ana==PB)
	{
	    VipPrintfExit("VipReadT1HistoAnalysis");
	    return(PB);
	}

    strncpy(filename,name,245);
    if( strlen( filename ) < 4
        || strcmp( filename + strlen( filename ) - 4, ".han" ) != 0 )
      strcat(filename,".han");

    f = fopen(filename,"r");
    if(f==PB)
	{
	  strncpy(filename,name,245);
	  strcat(filename,".ana");

	  f = fopen(filename,"r");

	  if(f==PB)
	    {
	      sprintf(error,"Can not open file %s.han or %s.ana",name,name);
	      VipPrintfError(error);
	      VipPrintfExit("VipReadT1HistoAnalysis");
	      return(PB);
	    }
	}
    fgets(buf, 255, f);
    if(strstr(buf,"SPGR")!=NULL)
	ana->sequence = MRI_T1_SPGR;
    else if (strstr(buf,"inversion recovery")!=NULL)
	ana->sequence = MRI_T1_IR;
    else if (strstr(buf,"unknown")!=NULL)
	ana->sequence = MODE_RIDGE;
    else
	{
	    VipPrintfError("Unknown analyse format in file:");
	    fprintf(stderr,"%s\n",filename);
	    VipPrintfExit("VipReadT1HistoAnalysis");
	    return(PB);
	}
    for(fgets(buf, 255, f);!feof(f);fgets(buf, 255, f))
	{
	    if(strstr(buf,"background")!=NULL)
		{
		    ana->background = VipAllocSSObject();
		    if(ana->background==PB) return(PB);
		    sscanf(buf,"%s%s%d",buf1,buf2,&mean);
		    ana->background->mean = mean;
		}
	    if(strstr(buf,"csf")!=NULL)
		{
		    ana->csf = VipAllocSSObject();
		    if(ana->csf==PB) return(PB);
		    sscanf(buf,"%s%s%d",buf1,buf2,&mean);
		    ana->csf->mean = mean;
		}
	    if(strstr(buf,"gray")!=NULL)
		{
		    ana->gray = VipAllocSSObject();
		    if(ana->gray==PB) return(PB);
		    sscanf(buf,"%s%s%d%s%d",buf1,buf2,&mean,buf3,&sigma);
		    ana->gray->mean = mean;
		    ana->gray->sigma = sigma;
		}
	    if(strstr(buf,"white")!=NULL)
		{
		    ana->white = VipAllocSSObject();
		    if(ana->white==PB) return(PB);
		    sscanf(buf,"%s%s%d%s%d",buf1,buf2,&mean,buf3,&sigma);
		    ana->white->mean = mean;
		    ana->white->sigma = sigma;
		}
	    if(strstr(buf,"brain")!=NULL)
		{
		    ana->brain = VipAllocSSObject();
		    if(ana->brain==PB) return(PB);
		    sscanf(buf,"%s%s%d%s%d",buf1,buf2,&mean,buf3,&sigma);
		    ana->brain->mean = mean;
		    ana->brain->sigma = sigma;
		}
	}
    return(ana);

}


/*---------------------------------------------------------------------------*/
int VipWriteT1HistoAnalysis(VipT1HistoAnalysis *ana, char *name)
/*---------------------------------------------------------------------------*/
{
    FILE *f;
    char filename[256];
    char error[300];
    int i;

    if(ana==NULL)
	{
	    VipPrintfError("NULL analyse");
	    VipPrintfExit("VipWriteT1HistoAnalysis");
	    return(PB);
	}
    if(name==NULL)
	{
	    VipPrintfError("NULL name");
	    VipPrintfExit("VipWriteT1HistoAnalysis");
	    return(PB);
	}
    strncpy(filename,name,245);
    if( strlen( filename ) < 4
        || strcmp( filename + strlen( filename ) - 4, ".han" ) != 0 )
    strcat(filename,".han");

    f = fopen(filename,"w");
    if(f==PB)
 	{
	    sprintf(error,"Can not open file %s",filename);
	    VipPrintfError(error);
	    VipPrintfExit("VipWriteT1HistoAnalysis");
	    return(PB);
	}

    if (ana->sequence==MRI_T1_SPGR) fprintf(f,"sequence: standard T1-weighted SPGR\n");
    else if (ana->sequence==MRI_T1_IR) fprintf(f,"sequence: inversion recovery\n");
    else if (ana->sequence==MODES_2) fprintf(f,"sequence: 2 modes\n");
    else if (ana->sequence==MODES_3) fprintf(f,"sequence: 3 modes\n");
    else if (ana->sequence==MODES_4) fprintf(f,"sequence: 4 modes\n");
    else if (ana->sequence==MODE_HISTO) fprintf(f,"sequence: histology\n");
    else fprintf(f,"sequence: unknown\n");
    if(ana->background!=NULL) fprintf(f,"background: mean: %d\n",
				      ana->background->mean);
    if(ana->csf!=NULL)
      fprintf(f,"csf: mean: %d\n",
				      ana->csf->mean);
    else
      fprintf(f,"csf: mean: %d sigma: %d\n", -1, -1);
    if(ana->gray!=NULL) fprintf(f,"gray: mean: %d sigma: %d\n",
				      ana->gray->mean, ana->gray->sigma);
    else
      fprintf(f,"gray: mean: %d sigma: %d\n", -1, -1);
    if(ana->white!=NULL) fprintf(f,"white: mean: %d sigma: %d\n",
				      ana->white->mean, ana->white->sigma);
    else
      fprintf(f,"white: mean: %d sigma: %d\n", -1, -1);
    if(ana->brain!=NULL && ana->white==NULL) fprintf(f,"brain: mean: %d sigma: %d\n",
				      ana->brain->mean, ana->brain->sigma);
    if(ana->mode1!=NULL) fprintf(f,"mode1: mean: %d sigma: %d\n",
				      ana->mode1->mean, ana->mode1->sigma);
    if(ana->mode2!=NULL) fprintf(f,"mode2: mean: %d sigma: %d\n",
				      ana->mode2->mean, ana->mode2->sigma);
    if(ana->mode3!=NULL) fprintf(f,"mode3: mean: %d sigma: %d\n",
				      ana->mode3->mean, ana->mode3->sigma);
    if(ana->mode4!=NULL) fprintf(f,"mode4: mean: %d sigma: %d\n",
				      ana->mode4->mean, ana->mode4->sigma);
    for(i=0;i<6;i++)
      if(ana->candidate[i]!=NULL) fprintf(f,"candidate %d: mean: %d sigma: %d\n", i,
				      ana->candidate[i]->mean, ana->candidate[i]->sigma);
      else
        fprintf(f,"candidate %d: mean: %d sigma: %d\n", i, -1, -1);
    fclose(f);

    return(OK);
}


/*---------------------------------------------------------------------------*/
VipT1HistoAnalysis *VipGetT1HistoAnalysis(
Volume *vol)
/*---------------------------------------------------------------------------*/
{
  return(VipGetT1HistoAnalysisCustomized(vol,10,0.5));
}
/*---------------------------------------------------------------------------*/
VipT1HistoAnalysis *VipGetT1HistoAnalysisCustomized(
Volume *vol,
int track,
float dscale)
/*---------------------------------------------------------------------------*/
{
  Vip1DScaleSpaceStruct *volstruct;
  VipHisto *shorthisto;
  int nbiter;
  SSSingularity *slist;
  SSCascade *clist, *chigh;
  VipT1HistoAnalysis *ana;
  int offset = 0;
  int scalemax = 1000;
  int n;

  if(vol==PB)
    {
      VipPrintfError("Null volume as argument");
      VipPrintfExit("VipGetT1HistoAnalysisCustomized");
      return(PB);
    }
  if(track<=0 || track>100000)
    {
      VipPrintfError("Inconsistent threshold on scale for singularity tracking");
      VipPrintfExit("VipGetT1HistoAnalysisCustomized");
      return(PB);
    }
  if(dscale>0.5 || (dscale<0.001))
    {
      VipPrintfError("Inconsistent scale sampling");
      VipPrintfExit("VipGetT1HistoAnalysisCustomized");
      return(PB);
    }

  
  shorthisto = VipComputeVolumeHisto(vol);
  if(shorthisto==PB) return(PB);

  printf("Computing histogram scale space singularities up to order three...\n");
  VipSetHistoVal(shorthisto,0,0); /*outside field of view for square images*/
  nbiter = (int)(scalemax/dscale+0.5);
  
  volstruct = VipCompute1DScaleSpaceStructUntilLastCascade(shorthisto,dscale,offset,2,0);
  if(volstruct==PB) return(PB); 

  track = (int)(track/dscale+0.5);
  slist = VipComputeSSSingularityList(volstruct,track,VFALSE,VTRUE,VTRUE,VFALSE,VFALSE);
  if(slist==PB) return(PB);
  if(VipFindSSSingularityMates(slist)==PB) return(PB);

  chigh = NULL;
  VipCountSingularitiesStillAlive(slist,&n,volstruct->itermax);

 if((n<=5))
    chigh = VipCreateHighestCascade(slist,volstruct->itermax,n);
  
  clist = VipComputeOrderedCascadeList( slist, nbiter, volstruct->hcumul);
  if(chigh!=NULL)
    {
      chigh->next = clist;
      clist = chigh;
    }
  else
      {
	  VipPrintfError("Sorry, the analysis can not proceed further");
	  return(PB);
      }

  ana = VipAnalyseCascades( clist, volstruct, VFALSE);
  if(ana==PB) return(PB);

  if(volstruct->undersampling_ratio>=64)
      {
	  ana->sequence = MRI_T1_IR;
	  printf("This image seems to come from the Brucker 3T of SHFJ...good luck\n");
	  printf("NB: I hope you have used VipBiasCorrection before going further...\n");
      }

  if(ana->sequence == MRI_T1_SPGR)
    printf("This T1-MR image seems to come from a classic SPGR sequence (low gray/white contrast)\n");
  else if(ana->sequence == MRI_T1_IR)
    printf("This T1-MR image seems to come from an Inversion/Recovery sequence (high gray/white contrast)\n");
  else
    {
      VipPrintfError("Unknown sequence");
      return(PB);
    }

  VipFree1DScaleSpaceStruct(volstruct);
  VipFreeSSSingularityList(slist);
  VipFreeCascadeList(clist);
  return(ana);

}

/*---------------------------------------------------------------------------*/
VipT1HistoAnalysis *VipAnalyseCascadesRidge(SSCascade *clist, Vip1DScaleSpaceStruct *volstruct, float mean)
/*---------------------------------------------------------------------------*/
{
  VipT1HistoAnalysis *ana;
  SSSingularity *slist=NULL;
  int ncascade;
  SSCascade *walker, *cascade[2];
  float dist, distmin;
  int scale;
  int i;
  int contrast;

  if(!clist || !volstruct )
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipAnalyseCascadesRidge");
      return(PB);
    }

  ana = VipCalloc(1,sizeof(VipT1HistoAnalysis),"VipAnalyseCascades");
  if(ana==PB) return(PB);

  ncascade = 3;
  walker = clist;
  distmin = 100000;
  cascade[1] = clist;
  while(ncascade-- && walker)
    {
      dist = fabs (mean-walker->D2ms->loc[0]*volstruct->undersampling_ratio);
      if(dist<distmin)
        {
          distmin = dist;
          cascade[1] = walker;
        }
      walker = walker->next;
    }
  printf("Closest cascade: %d/%f\n",cascade[1]->D2ms->loc[0] *volstruct->undersampling_ratio ,mean);

  cascade[0] = VipGetInsideRangeBiggestCascade(clist,5,cascade[1]->D2ms->loc[0]-1,1000000);

  for(i=0;i<2;i++)
    {
      if (cascade[i]!=NULL)
        {
  
          if(cascade[i]->D1ms!=NULL) VipInsertSSSingularity(cascade[i]->D1ms,&slist);
          if(cascade[i]->D1Ms!=NULL) VipInsertSSSingularity(cascade[i]->D1Ms,&slist);
          if(cascade[i]->D2ms!=NULL) VipInsertSSSingularity(cascade[i]->D2ms,&slist);
          if(cascade[i]->D2Ms!=NULL) VipInsertSSSingularity(cascade[i]->D2Ms,&slist);
        }

    }

  if(cascade[0])
    {
      ana->gray = VipAllocSSObject();
      if(ana->gray==PB) return(PB);  
      ana->gray->D2ms = cascade[0]->D2ms;
      VipExpandSSObjectSingularities( ana->gray, volstruct ); 
    }

  if(cascade[1])
    {
      ana->white = VipAllocSSObject();
      if(ana->white==PB) return(PB);  
      ana->white->D2ms = cascade[1]->D2ms;
      VipExpandSSObjectSingularities( ana->white, volstruct ); 
    }
  /*
  VipComputeAnalysedLoc( ana->white, 100000 );
  VipComputeAnalysedLoc( ana->gray, 100000 );
  scale = mVipMin(ana->gray->good_scale,ana->white->good_scale);
  scale = mVipMax(100,scale);*/
  scale=50;
  VipComputeAnalysedLoc( ana->white, scale+1 );
  VipComputeAnalysedLoc( ana->gray, scale+1 );
  PutUndersamplingRatio(ana,volstruct->undersampling_ratio);

  contrast = ana->white->mean - ana->gray->mean;
  
  if(ana->white)
    {
      ana->white->sigma = mVipMax(ana->white->right_sigma,ana->white->left_sigma);
      ana->white->sigma = mVipMax(ana->white->sigma,(int)(0.167*contrast));
      printf("white: mean: %d, sigma:%d\n",ana->white->mean,ana->white->sigma);
    }
  if(ana->gray)
    {
      ana->gray->sigma = mVipMin(ana->gray->right_sigma,ana->gray->left_sigma);
      if (ana->white) ana->gray->sigma = mVipMax(ana->gray->sigma,1.5*ana->white->sigma);
      printf("gray: mean: %d, sigma:%d\n",ana->gray->mean,ana->gray->sigma);
    }

  ana->sequence = MODE_RIDGE; 
  return(ana);
}
/*---------------------------------------------------------------------------*/
VipT1HistoAnalysis *VipAnalyseCascades(SSCascade *clist, Vip1DScaleSpaceStruct *volstruct, int sequence)
/*---------------------------------------------------------------------------*/
{
  SSCascade *c1, *c2, *c3;
  SSCascade *cmax;
  float r1, r2;
  VipT1HistoAnalysis *ana;
  int SPGRana;
  int maxvolume = 0;

  if(!clist || !volstruct)
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipAnalyseCascades");
      return(PB);
    }

  ana = VipCalloc(1,sizeof(VipT1HistoAnalysis),"VipAnalyseCascades");
  if(ana==PB) return(PB);

  if (sequence==MODE_HISTO)
	{
	  ana->sequence = sequence;
          if(VipAnalyseCascadesHistology(ana, clist, volstruct)==PB) return(PB);
          else return(ana);
	}
  if (sequence==MRI_T1_SEGMENTED_BRAIN)
	{
	  ana->sequence = sequence;
          if(VipAnalyseCascadesSegmentedSequence(ana, clist, volstruct)==PB) return(PB);
          else return(ana);
	}
  if (sequence==MODES_2 || sequence==MODES_3 || sequence==MODES_4)
	{
	  ana->sequence = sequence;
          if(VipAnalyseCascadesModes(ana, clist, volstruct, sequence)==PB) return(PB);
          else return(ana);
	}
  c1 = clist->next;
  if(c1==NULL)
    {
      VipPrintfError("Null First Cascade");
      VipPrintfExit("VipAnalyseCascades");
      return(PB);
    }
  c2 = c1->next;
  if(c2==NULL)
    {
      VipPrintfError("Null Second Cascade");
      VipPrintfExit("VipAnalyseCascades");
      return(PB);
    }
  c3 = c2->next;
  if(c3==NULL)
    {
      VipPrintfError("Null Third Cascade");
      VipPrintfExit("VipAnalyseCascades");
      return(PB);
    }

  if( (c1->scale_event < c2->scale_event))
    {
      if( c2->scale_event < c3->scale_event )
	{
	  cmax = c3;
	}
      else
	{
	  cmax = c2;
	}
    }
  else
      if( c1->scale_event < c3->scale_event )
	{
	  cmax = c3;
	}
      else
	{
	  cmax = c1;
	}

  if(cmax->scale_event<=0)
   {
      VipPrintfError("Inconsistent event scale");
      VipPrintfExit("VipAnalyseCascades");
      return(PB);
    }
    
  if( (c1->volume < c2->volume))
    {
      if( c2->volume < c3->volume )
	{
	  maxvolume = c3->volume;
	}
      else
	{
	  maxvolume = c2->volume;
	}
    }
  else
      if( c1->volume < c3->volume )
	{
	  maxvolume = c3->volume;
	}
      else
	{
	  maxvolume = c1->volume;
	}
  
  printf("Main event scales: %d %d %d\n", c1->scale_event, c2->scale_event, c3->scale_event);
  printf("Main cascade volumes: %d %d %d\n", c1->volume, c2->volume, c3->volume);

  if(c1!=cmax)
    {
      r1 = (float)c1->scale_event / (float)cmax->scale_event;
      if(c2!=cmax)
	r2 = (float)c2->scale_event / (float)cmax->scale_event;
      else 
	r2 = (float)c3->scale_event / (float)cmax->scale_event;
    }
  else
    {     
      r1 = (float)c2->scale_event / (float)cmax->scale_event;
      r2 = (float)c3->scale_event / (float)cmax->scale_event;
    }

  if((mVipMax(r1,r2)> SPGR_IR_RATIO) /*|| ((mVipMax(r1,r2)>0.10) && (volstruct->undersampling_ratio>=32))*/) ana->sequence = MRI_T1_IR;
  else ana->sequence = MRI_T1_SPGR;


  if(sequence!=VFALSE)
    {
      if(sequence!=ana->sequence)
	{
	  VipPrintfWarning("Specified and automatic contrast detection are different!");
	  ana->sequence = sequence;
	}
    }
  /*ana->sequence = MRI_T1_SPGR;*/
printf("avant %d\n",ana->sequence);fflush(stdout);
  if(ana->sequence==MRI_T1_SPGR)
    {
      SPGRana=VipAnalyseCascadesSPGRSequence(ana, clist, cmax, volstruct);
      if(SPGRana==PB) return(PB);
      else if (SPGRana==MRI_T1_IR)
	{
	  ana->sequence = MRI_T1_IR;
	  if(VipAnalyseCascadesIRSequence(ana, clist, volstruct)==PB) return(PB);
	}
    }
  else if(ana->sequence==MRI_T1_IR)
    {
      if(VipAnalyseCascadesIRSequence(ana, clist, volstruct)==PB) return(PB);
    }

  return(ana);
}


/*---------------------------------------------------------------------------*/
static int PutUndersamplingRatio(VipT1HistoAnalysis *ana,int ratio)
/*---------------------------------------------------------------------------*/
{
  int i;

  if(ratio<=0)
    {
      VipPrintfError("Bad undersampling ratio");
      return(PB);
    }
  if(ana==NULL)
   {
      VipPrintfError("NULL arg in PutUndersamplingRatio");
      return(PB);
    }

  for(i=0;i<6;i++)
  if(ana->candidate[i]!=NULL)
    {
      if((ana->candidate[i]!=ana->white) && (ana->candidate[i]!=ana->gray) && (ana->candidate[i]!=ana->csf))
        {
          ana->candidate[i]->mean*=ratio;
          if(ana->candidate[i]->left_sigma<4) ana->candidate[i]->left_sigma=4;
          if(ana->candidate[i]->right_sigma<4) ana->candidate[i]->right_sigma=4;
          ana->candidate[i]->left_sigma*=ratio;
          ana->candidate[i]->right_sigma*=ratio;
        }
    }

  if(ana->background!=NULL)
    {
      ana->background->mean*=ratio;
      if(ana->background->left_sigma<4) ana->background->left_sigma=4;
      if(ana->background->right_sigma<4) ana->background->right_sigma=4;
      ana->background->left_sigma*=ratio;
      ana->background->right_sigma*=ratio;
    }
  if(ana->brain!=NULL)
    {
      ana->brain->mean*=ratio;
     if(ana->brain->left_sigma<4) ana->brain->left_sigma=4;
      if(ana->brain->right_sigma<4) ana->brain->right_sigma=4;
      ana->brain->left_sigma*=ratio;
      ana->brain->right_sigma*=ratio;
    }
  if(ana->white!=NULL)
    {
      ana->white->mean*=ratio;
     if(ana->white->left_sigma<4) ana->white->left_sigma=4;
      if(ana->white->right_sigma<4) ana->white->right_sigma=4;
      ana->white->left_sigma*=ratio;
      ana->white->right_sigma*=ratio;
    }
  if(ana->gray!=NULL)
    {
      ana->gray->mean*=ratio;
     if(ana->gray->left_sigma<4) ana->gray->left_sigma=4;
      if(ana->gray->right_sigma<4) ana->gray->right_sigma=4;
      ana->gray->left_sigma*=ratio;
      ana->gray->right_sigma*=ratio;
      if(ana->white!=NULL && ana->gray->left_sigma<1.2*mVipMax(ana->white->left_sigma,ana->white->right_sigma))
	{
	  /*VipPrintfWarning("Trigger gray/white protection for gray left standard deviation");*/
	  ana->gray->left_sigma=(int)(1.2*mVipMax(ana->white->left_sigma,ana->white->right_sigma));
	}
      if(ana->white!=NULL && ana->gray->right_sigma<1.2*mVipMax(ana->white->left_sigma,ana->white->right_sigma))
	{
	  ana->gray->right_sigma=(int)(1.2*mVipMax(ana->white->left_sigma,ana->white->right_sigma));
	  /*VipPrintfWarning("Trigger gray/white protection for gray left standard deviation");*/
	}
      /* protection, after bias correction, gray varaibility is higher because of partial volume effect.*/
    }
  if(ana->csf!=NULL)
    {
      ana->csf->mean*=ratio;
     if(ana->csf->left_sigma<4) ana->csf->left_sigma=4;
      if(ana->csf->right_sigma<4) ana->csf->right_sigma=4;
      ana->csf->left_sigma*=ratio;
      ana->csf->right_sigma*=ratio;
    }
  if(ana->skull!=NULL)
    {
      ana->skull->mean*=ratio;
      if(ana->skull->left_sigma<4) ana->skull->left_sigma=4;
      if(ana->skull->right_sigma<4) ana->skull->right_sigma=4;
      ana->skull->left_sigma*=ratio;
      ana->skull->right_sigma*=ratio;
    }
  return(OK);

}

/*---------------------------------------------------------------------------*/
int VipAnalyseCascadesModes(VipT1HistoAnalysis *ana,
SSCascade *clist, Vip1DScaleSpaceStruct *volstruct, int sequence)
/*---------------------------------------------------------------------------*/
{
  SSSingularity *slist, *walker;
  SSCascade *cwalker;
  int ncascade;
  int loc;
  SSCascade *cascade;

  if(!clist || !ana || !volstruct )
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipAnalyseCascadesSegmentedSequence");
      return(PB);
    }
  
  slist = NULL;
  ncascade = 0;
  loc = 100000;

  if(clist->D1ms!=NULL) VipInsertSSSingularity(clist->D1ms,&slist);
  if(clist->D1Ms!=NULL) VipInsertSSSingularity(clist->D1Ms,&slist);
  if(clist->D2ms!=NULL)
    {
      if (loc>(clist->D2ms->loc[0])) loc = clist->D2ms->loc[0];
      VipInsertSSSingularity(clist->D2ms,&slist);
    }
  if(clist->D2Ms!=NULL) VipInsertSSSingularity(clist->D2Ms,&slist);
  ana->mode1 = VipAllocSSObject();
  if(ana->mode1==PB) return(PB);
  ncascade +=1;

  cwalker = clist->next;
  /* it is the biggest volume cascade, if it is not complet, it is either
     a terribly compressed image, or this is a background additional mode which will be discarded further
  while(cwalker!=NULL && (cwalker->complet!=VTRUE) cwalker = cwalker->next;*/
  if(cwalker==NULL)
    {
      VipPrintfError("NULL cascade 2");
      VipPrintfExit("VipAnalyseCascadesSegmentedSequence");
      return(PB);
    }
  if(cwalker!=NULL)
    {
      if(cwalker->D1ms!=NULL) VipInsertSSSingularity(cwalker->D1ms,&slist);
      if(cwalker->D1Ms!=NULL) VipInsertSSSingularity(cwalker->D1Ms,&slist);
      if(cwalker->D2ms!=NULL)
        {
          if (loc>(cwalker->D2ms->loc[0])) loc = cwalker->D2ms->loc[0];
          VipInsertSSSingularity(cwalker->D2ms,&slist);
        }
      if(cwalker->D2Ms!=NULL) VipInsertSSSingularity(cwalker->D2Ms,&slist);
      ana->mode2 = VipAllocSSObject();
      if(ana->mode2==PB) return(PB);
      ncascade +=1;
    }



  if (sequence != MODES_2)
    {
      cwalker = cwalker->next;
      if(cwalker==NULL)
        {
          VipPrintfError("NULL cascade 3");
          VipPrintfExit("VipAnalyseCascadesSegmentedSequence");
          return(PB);
        } 
      if(cwalker!=NULL)
        {
          if(cwalker->D1ms!=NULL) VipInsertSSSingularity(cwalker->D1ms,&slist);
          if(cwalker->D1Ms!=NULL) VipInsertSSSingularity(cwalker->D1Ms,&slist);
          if(cwalker->D2ms!=NULL)
            {
              if (loc>(cwalker->D2ms->loc[0])) loc = cwalker->D2ms->loc[0];
              VipInsertSSSingularity(cwalker->D2ms,&slist);
            }
          if(cwalker->D2Ms!=NULL) VipInsertSSSingularity(cwalker->D2Ms,&slist);
          ncascade +=1;
          ana->mode3 = VipAllocSSObject();
          if(ana->mode3==PB) return(PB);
        }
    }

  if (sequence != MODES_2 && sequence != MODES_3)
    {
      printf("Looking for forth mode in range[%d,%d] \n",15,loc-1); fflush(stdout);

      cascade = VipGetInsideRangeCascade(clist, 15, loc-1, 100000);
      if (cascade!=NULL && cascade->volume>100)
        {
          ncascade++;
          ana->mode4 = VipAllocSSObject();
          if(ana->mode4==PB) return(PB);
          if(cascade->D1ms!=NULL) VipInsertSSSingularity(cascade->D1ms,&slist);
          if(cascade->D1Ms!=NULL) VipInsertSSSingularity(cascade->D1Ms,&slist);
          if(cascade->D2ms!=NULL) VipInsertSSSingularity(cascade->D2ms,&slist);
          if(cascade->D2Ms!=NULL) VipInsertSSSingularity(cascade->D2Ms,&slist);          
        }
      else
        {
          cwalker = cwalker->next;
          if(cwalker==NULL)
            {
              VipPrintfError("NULL cascade 4");
              VipPrintfExit("VipAnalyseCascadesSegmentedSequence");
              return(PB);
            } 
          if(cwalker!=NULL)
            {
              if(cwalker->D1ms!=NULL) VipInsertSSSingularity(cwalker->D1ms,&slist);
              if(cwalker->D1Ms!=NULL) VipInsertSSSingularity(cwalker->D1Ms,&slist);
              if(cwalker->D2ms!=NULL)
                {
                  if (loc>(cwalker->D2ms->loc[0])) loc = cwalker->D2ms->loc[0];
                  VipInsertSSSingularity(cwalker->D2ms,&slist);
                }
              if(cwalker->D2Ms!=NULL) VipInsertSSSingularity(cwalker->D2Ms,&slist);
              ncascade +=1;
              ana->mode4 = VipAllocSSObject();
              if(ana->mode4==PB) return(PB);
            }
        }
    }

  /*analyse*/
  walker = slist;
  walker = VipGetNextSingularityOnRight( walker, D2m );
  if(walker==PB) return(PB);
  if (ncascade>1)
    {
      ana->mode1->D2ms = walker;
      walker = VipGetNextSingularityOnRight( walker->right, D2m );
      if(walker==PB) return(PB);
      else ana->mode2->D2ms = walker;
      if (ncascade>2)
        {
          walker = VipGetNextSingularityOnRight( walker->right, D2m );
          if(walker==PB) return(PB);
          else ana->mode3->D2ms = walker;
          if (ncascade>3)
            {
              walker = VipGetNextSingularityOnRight( walker->right, D2m );
              if(walker==PB) return(PB);
              else ana->mode4->D2ms = walker;
            }
        }
    }
    
  if (ana->mode1) VipExpandSSObjectSingularities( ana->mode1, volstruct );
  if (ana->mode2) VipExpandSSObjectSingularities( ana->mode2, volstruct );
  if (ana->mode3) VipExpandSSObjectSingularities( ana->mode3, volstruct );
  if (ana->mode4) VipExpandSSObjectSingularities( ana->mode4, volstruct );

  /*
  printf("Scale events: %d, %d\n",ana->gray->scale_event, ana->white->scale_event);
  */

  printf("Scale event: %d\n",ana->mode1->scale_event);
  if (ana->mode2)
    {
      loc = mVipMin(ana->mode1->scale_event, ana->mode2->scale_event);
      printf("Scale event: %d\n",ana->mode2->scale_event);
    }
  else loc = ana->mode1->scale_event;
  /*
  if (ana->mode3) 
    {
      loc = mVipMin(ana->mode3->scale_event, loc);
      printf("Scale event: %d\n",ana->mode3->scale_event);
    }
  if (ana->mode4)
    {
SSObject      loc = mVipMin(ana->mode4->scale_event, loc);
      printf("Scale event: %d\n",ana->mode4->scale_event);
    }
  */
  printf("Localization speed minima search under %d scale\n",loc); 
  if (ana->mode1) VipComputeAnalysedLoc( ana->mode1, loc );
  if (ana->mode2) VipComputeAnalysedLoc( ana->mode2, loc );
  if (ana->mode3) VipComputeAnalysedLoc( ana->mode3, loc );
  if (ana->mode4) VipComputeAnalysedLoc( ana->mode4, loc );

  PutUndersamplingRatio(ana,volstruct->undersampling_ratio);
  if (ana->mode1) ana->mode1->sigma = mVipMax(ana->mode1->right_sigma,ana->mode1->left_sigma);
  if (ana->mode2) ana->mode2->sigma = mVipMax(ana->mode2->right_sigma,ana->mode2->left_sigma);
  if (ana->mode3) ana->mode3->sigma = mVipMax(ana->mode3->right_sigma,ana->mode3->left_sigma);
  if (ana->mode4) ana->mode4->sigma = mVipMax(ana->mode4->right_sigma,ana->mode4->left_sigma);


  if (ana->mode1)  printf("mode1 found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->mode1->left_sigma,
	 ana->mode1->mean, ana->mode1->right_sigma);
  if (ana->mode2)  printf("mode2 found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->mode2->left_sigma,
	 ana->mode2->mean, ana->mode2->right_sigma);
  if (ana->mode3)  printf("mode3 found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->mode3->left_sigma,
	 ana->mode3->mean, ana->mode3->right_sigma);
  if (ana->mode4)  printf("mode4 found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->mode4->left_sigma,
	 ana->mode4->mean, ana->mode4->right_sigma);

  return(OK);
}




/*---------------------------------------------------------------------------*/
int VipAnalyseCascadesHistology(VipT1HistoAnalysis *ana,
SSCascade *clist, Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{
  SSSingularity *slist;
  SSCascade *cascade[6], *ccsf, *cwhite;
  int ncascade;
  int i;
  SSObject *temp;
  int left = 0;
  int missing_cascade5 = VFALSE;
  int contrastGB;
  int sizewhite=0;

  if(!clist || !ana || !volstruct )
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipAnalyseCascadesHistology");
      return(PB);
    }
  
  slist = NULL;
  ncascade = 0;
  
  for(i=0;i<6;i++) cascade[i]=NULL;

  /*NB: the algorithm assummes cascades are ordered in descending scale order*/
  
  /*La plus haute = cascade4*/
  printf("Select last trajectory as grey matter\n");
  cascade[3] = clist;


  /*Maintenant, les deux plus hautes a droite sont selectionnees*/
  printf("Looking for the two highest trajectories in [%d;%d]\n",clist->D2ms->loc[0],mVipHistoRangeMax(volstruct->histo)-1);
  cascade[4] = VipGetInsideRangeCascade(clist,clist->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1,1000000);
  if (cascade[4]!=NULL && cascade[4]->volume<1500)
    {
      printf("highest too small for white, Looking for biggest trajectory in [%d;%d]\n",clist->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1);
      cascade[5] = VipGetInsideRangeBiggestCascade(clist,clist->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1,1000000);
      if (cascade[5]==cascade[4])
        cascade[5] = VipGetInsideRangeCascade(cascade[4]->next,clist->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1,1000000);
    }
  if (cascade[4]!=NULL) sizewhite = cascade[4]->volume;
  if (cascade[5]!=NULL) sizewhite = mVipMax(cascade[5]->volume,sizewhite);

  if(cascade[4]==NULL || (cascade[4]!=NULL && cascade[4]->volume<1500 && cascade[5]==NULL) || ((cascade[4]!=NULL) && (cascade[4]->volume<1500) && (cascade[5]!=NULL) && (cascade[5]->volume<1500)))
    {
      VipPrintfWarning("No good trajectory for white!, move grey matter to biggest cascade on the left");
      printf("Looking for biggest trajectory in [%d;%d]\n",5,clist->D2ms->loc[0]-1);
      cascade[3] = VipGetInsideRangeBiggestCascade(clist,5,clist->D2ms->loc[0]-1,1000000);
      if(cascade[3]==NULL)
        {
          VipPrintfWarning("No more big trajectory!");
          return(PB);
        }
      if(cascade[3]->volume>2500 || (sizewhite<500))
        {
          cascade[4] = clist;
          printf("Looking for highest trajectory in [%d;%d]\n",cascade[4]->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1);
          cascade[5] = VipGetInsideRangeCascade(clist,cascade[4]->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1,1000000);
          if (cascade[5]!=NULL && cascade[5]->volume<100)
            {
              printf("Finally Looking for biggest trajectory in [%d;%d]\n",cascade[4]->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1);
              cascade[5] = VipGetInsideRangeCascade(clist,cascade[4]->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1,1000000);
            }
        }
      else
        {
          printf("Nothing big enough on the left...\n");
          cascade[3] = clist;
          cascade[4] = VipGetInsideRangeCascade(clist,clist->D2ms->loc[0]+1,mVipHistoRangeMax(volstruct->histo)-1,1000000);
          if(cascade[4]!=NULL)
            cascade[5] = VipGetInsideRangeCascade(cascade[4]->next,clist->D2ms->loc[0],mVipHistoRangeMax(volstruct->histo),1000000);
        }
    }
  else if (cascade[5]==NULL)
    {
      cascade[5] = VipGetInsideRangeCascade(cascade[4]->next,clist->D2ms->loc[0],mVipHistoRangeMax(volstruct->histo),1000000);
    }
  if(cascade[5]==NULL)
    {
      VipPrintfWarning("Only one trajectory!");
      missing_cascade5 = VTRUE;
    }


 /*Maintenant, les trois plus hautes a gauche sont selectionnees*/
  printf("Looking for the three highest trajectories in [5;%d]\n",cascade[3]->D2ms->loc[0]-1);
  cascade[0] = VipGetInsideRangeCascade(clist,5,cascade[3]->D2ms->loc[0]-1,1000000);
  if(cascade[0]==NULL)
    {
      VipPrintfWarning("No trajectory!");
    }
  else
    {      
      cascade[1] = VipGetInsideRangeCascade(cascade[0]->next,5,cascade[3]->D2ms->loc[0]-1,1000000);
    }
  if(cascade[1]==NULL)
    {
      VipPrintfWarning("Only one trajectory!");
    }
  else
    {
      left = 5;
      /*
      if (cascade[0]!=NULL) left = mVipMax(left,cascade[0]->D2ms->loc[0]);
      if (cascade[1]!=NULL) left = mVipMax(left,cascade[1]->D2ms->loc[0]);
      printf("Looking for the highest trajectory in [%d;%d]\n",left,cascade[3]->D2ms->loc[0]-1);
      */
      cascade[2] = VipGetInsideRangeCascade(cascade[1]->next,left,cascade[3]->D2ms->loc[0]-1,1000000);
    }
  if(cascade[2]==NULL)
    {
      VipPrintfWarning("No more trajectory");
    }

  if (missing_cascade5==VTRUE)
    {
      left = 5;
      if (cascade[0]!=NULL) left = mVipMax(left,cascade[0]->D2ms->loc[0]);
      if (cascade[1]!=NULL) left = mVipMax(left,cascade[1]->D2ms->loc[0]);
      if (cascade[2]!=NULL) left = mVipMax(left,cascade[2]->D2ms->loc[0]);
      printf("Recover additional grey matter candidate...\n");
      printf("Looking for the biggest trajectory in [%d;%d]\n",left,cascade[3]->D2ms->loc[0]-1);
      cascade[5] = VipGetInsideRangeBiggestCascade(cascade[3]->next,left,cascade[3]->D2ms->loc[0]-1,1000000);
      if(cascade[5]==NULL)
        {
          VipPrintfWarning("No more trajectory");
          missing_cascade5=VFALSE;
        }
    }  

  for(i=0;i<6;i++)
    {
      if (cascade[i]!=NULL)
        {
  
          if(cascade[i]->D1ms!=NULL) VipInsertSSSingularity(cascade[i]->D1ms,&slist);
          if(cascade[i]->D1Ms!=NULL) VipInsertSSSingularity(cascade[i]->D1Ms,&slist);
          if(cascade[i]->D2ms!=NULL) VipInsertSSSingularity(cascade[i]->D2ms,&slist);
          if(cascade[i]->D2Ms!=NULL) VipInsertSSSingularity(cascade[i]->D2Ms,&slist);
        }
    }

  for(i=0;i<6;i++)
    {
      if (cascade[i]!=NULL)
        {
          ana->candidate[i] = VipAllocSSObject();
          if(ana->candidate[i]==PB) return(PB);  
          if(cascade[i]->D2ms==NULL)
            {
              VipPrintfError("Strange cascade without root");
              return(PB);
            }
          ana->candidate[i]->D2ms = cascade[i]->D2ms;

          VipExpandSSObjectSingularities( ana->candidate[i], volstruct ); 
        }
    }

  ana->gray = ana->candidate[3];

  if(cascade[4]!=NULL && missing_cascade5==VFALSE && cascade[5]!=NULL)
   {
     if (cascade[4]->volume<200 && (cascade[5]->volume>cascade[4]->volume))
       {
         ana->white = ana->candidate[5];
         cwhite = cascade[5];
         printf("switch white mode to second candidate\n");
       }
     else
       {
         ana->white = ana->candidate[4];
         cwhite = cascade[4];
       }
   }
  else
    {
      ana->white = ana->candidate[4];
      cwhite = cascade[4];
    }

  if (cwhite!=NULL)
    contrastGB = cwhite->D2ms->loc[0]-cascade[3]->D2ms->loc[0];
  else contrastGB=10;

  if(cascade[0]!=NULL && cascade[0]->volume<100)
    {
      printf("No marker in this slice\n");
      ccsf=NULL;
    }
  else
    {
      ana->csf = ana->candidate[0];
      ccsf = cascade[0];
    }

  /*
  if (ccsf!=NULL && (contrastGB*1 >(cascade[3]->D2ms->loc[0]-ccsf->D2ms->loc[0]))) 
    {
      ana->csf = NULL;
      ccsf=NULL;
    }
  */

  if( cwhite!=NULL && cwhite->volume<100)
    {
      cwhite=NULL;
      ana->white=NULL;
    }


  VipComputeAnalysedLoc( ana->gray, 100000 );
  for (i=0;i<6;i++)
    if (ana->candidate[i] && ana->candidate[i]!=ana->gray) VipComputeAnalysedLoc( ana->candidate[i], 
                                                                                  mVipMin(ana->gray->good_scale,200) ); 
  VipComputeAnalysedLoc( ana->gray, mVipMin(ana->gray->good_scale,200) );


  printf("Here are the selected cascades (before ordering from loc[0]):\n");
  PutUndersamplingRatio(ana,volstruct->undersampling_ratio);

 for(i=0;i<6;i++)
    {
      if (cascade[i]!=NULL)
        {
          ana->candidate[i]->sigma = mVipMax(ana->candidate[i]->right_sigma,ana->candidate[i]->left_sigma);
          printf("loc[0]: %d, sigma:%d, volume: %d\n",cascade[i]->D2ms->loc[0],ana->candidate[i]->sigma,cascade[i]->volume);
        }
    }

  if(ccsf && ana->csf)
    {
      printf("MARKER CANDIDATE = scale: %d, vol: %d, loc[scale=0]: %d, loc[%d]:%d\n",ccsf->scale_event, ccsf->volume, ccsf->D2ms->loc[0], ana->csf->good_scale, ccsf->D2ms->loc[ana->csf->good_scale]);
    }
 if(cascade[3] && ana->gray)
    {
      printf("GREY MATTER CANDIDATE = scale: %d, vol: %d, loc[scale=0]:%d, loc[%d]:%d\n",cascade[3]->scale_event, cascade[3]->volume, cascade[3]->D2ms->loc[0], ana->gray->good_scale, cascade[3]->D2ms->loc[ana->gray->good_scale]);
    }
   if(cwhite && ana->white)
    {
      printf("WHITE MATTER CANDIDATE = scale: %d, vol: %d, loc[scale=0]:%d, loc[%d]:%d\n",cwhite->scale_event, cwhite->volume, cwhite->D2ms->loc[0], ana->white->good_scale, cwhite->D2ms->loc[ana->white->good_scale]);
    }

  /* reordonne par loc0 croissant */

  if(ana->candidate[0] && ana->candidate[1])
    if (ana->candidate[0]->D2ms->loc[0]>ana->candidate[1]->D2ms->loc[0])
      {
        temp = ana->candidate[1];
        ana->candidate[1] = ana->candidate[0];
        ana->candidate[0] = temp;
      }
  if(ana->candidate[1] && ana->candidate[2])
    if (ana->candidate[1]->D2ms->loc[0]>ana->candidate[2]->D2ms->loc[0])
      {
        temp = ana->candidate[2];
        ana->candidate[2] = ana->candidate[1];
        ana->candidate[1] = temp;
      }
  if(ana->candidate[0] && ana->candidate[1])
    if (ana->candidate[0]->D2ms->loc[0]>ana->candidate[1]->D2ms->loc[0])
      {
        temp = ana->candidate[1];
        ana->candidate[1] = ana->candidate[0];
        ana->candidate[0] = temp;
      } 
  if(ana->candidate[4] && ana->candidate[5] && missing_cascade5==VFALSE)
   if (ana->candidate[4]->D2ms->loc[0]>ana->candidate[5]->D2ms->loc[0])
     {
       temp = ana->candidate[4];
       ana->candidate[4] = ana->candidate[5];
       ana->candidate[5] = temp;
     } 
  if(missing_cascade5==VTRUE)
    {
      temp=ana->candidate[5];
      ana->candidate[5] = ana->candidate[4];
      ana->candidate[4] = ana->candidate[3];
      ana->candidate[3] = temp;
    }

  return(OK);
}


/*---------------------------------------------------------------------------*/
int VipAnalyseCascadesSegmentedSequence(VipT1HistoAnalysis *ana,
SSCascade *clist, Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{
  SSSingularity *slist, *walker;
  SSCascade *cascade2, *cascade3;
  /*, *cascade4, *cascade5, *cascade6;*/
  int ncascade;
  int left, right;
 
  if(!clist || !ana || !volstruct )
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipAnalyseCascadesSegmentedSequence");
      return(PB);
    }
  
  slist = NULL;
  ncascade = 0;
  
  if(clist->D1ms!=NULL) VipInsertSSSingularity(clist->D1ms,&slist);
  if(clist->D1Ms!=NULL) VipInsertSSSingularity(clist->D1Ms,&slist);
  if(clist->D2ms!=NULL) VipInsertSSSingularity(clist->D2ms,&slist);
  if(clist->D2Ms!=NULL) VipInsertSSSingularity(clist->D2Ms,&slist);
  ana->brain = VipAllocSSObject();
  if(ana->brain==PB) return(PB);
  ncascade +=1;

  /*recherche du cerveau*/
  walker = slist;
  walker = VipGetNextSingularityOnRight( walker, D2m );
  if(walker==PB) return(PB);
  ana->brain->D2ms = walker;
  VipExpandSSObjectSingularities( ana->brain, volstruct );
  VipComputeAnalysedLoc( ana->brain, ana->brain->scale_event );    
  printf("brain scale: %d\n",ana->brain->good_scale);

  cascade2 = PB;
  if (ana->brain->D1Ms != NULL)
    {
      left = mVipMax(ana->brain->D1Ms->loc[ana->brain->good_scale],ana->brain->D2ms->loc[ana->brain->good_scale]-40);
    }
  else
    left = ana->brain->D2ms->loc[ana->brain->good_scale]-50;
  printf("Looking for white/gray cascade in range[%d,%d] under scale %d\n",left,mVipHistoRangeMax(volstruct->histo),(int)((ana->brain->scale_event-1)/2));
  cascade2 = VipGetInsideRangeBiggestCascade(clist,left, mVipHistoRangeMax(volstruct->histo), ana->brain->scale_event-1);
  if(cascade2==PB)
    {
      VipPrintfWarning("Found only one tissue type, assume it is gray matter of extreme histological slice");
      printf("It may also be a brain without any contrast, because of a huge spatial bias\n");
    }
  else
    {
      printf("scale: %d, loc: %d\n",cascade2->scale_event, cascade2->D2ms->loc[0]);
      printf("Found gray and white matter modes!\n");
      if(cascade2->D1ms!=NULL) VipInsertSSSingularity(cascade2->D1ms,&slist);
      if(cascade2->D1Ms!=NULL) VipInsertSSSingularity(cascade2->D1Ms,&slist);
      if(cascade2->D2ms!=NULL) VipInsertSSSingularity(cascade2->D2ms,&slist);
      if(cascade2->D2Ms!=NULL) VipInsertSSSingularity(cascade2->D2Ms,&slist);
      ncascade +=1;
      ana->white = VipAllocSSObject();
      if(ana->white==PB) return(PB);
      ana->gray = VipAllocSSObject();
      if(ana->gray==PB) return(PB);
      ana->brain=NULL;

      walker = slist;
      walker = VipGetNextSingularityOnRight( walker, D2m );
      if(ana->gray!=NULL)
        {
          ana->gray->D2ms = walker;
          walker = VipGetNextSingularityOnRight( walker->right, D2m );
          if(walker==PB) return(PB);
          ana->white->D2ms = walker;
          cascade3 = VipGetInsideRangeCascade(clist, 15, 100000, 100000);
        }
      
      if (ana->gray) VipExpandSSObjectSingularities( ana->gray, volstruct );
      if (ana->white) VipExpandSSObjectSingularities( ana->white, volstruct );
      VipComputeAnalysedLoc( ana->gray, mVipMin(mVipMin(ana->gray->scale_event, ana->white->scale_event),1000) );    
      VipComputeAnalysedLoc( ana->white, mVipMin(mVipMin(ana->gray->scale_event, ana->white->scale_event),1000) );
 
      if (ana->gray->D1Ms != NULL)
        {
          right = mVipMax(-2*(ana->gray->D2ms->loc[ana->gray->good_scale] - ana->gray->D1Ms->loc[ana->gray->good_scale])
                 + ana->gray->D2ms->loc[ana->gray->good_scale],ana->gray->D2ms->loc[ana->gray->good_scale]-50);
        }
      else
        right = ana->gray->D2ms->loc[ana->gray->good_scale]-50;
      printf("Looking for CSF/marker cascade in range[15,%d] under scale %d\n",right,100000);
    
      cascade3=PB;
      cascade3 = VipGetInsideRangeCascade(clist, 15, right, 100000);
      if(cascade3!=PB && cascade3->volume>100)
        {
          printf("Found CSF/histological marker mode!\n");
          if(cascade3->D1ms!=NULL) VipInsertSSSingularity(cascade3->D1ms,&slist);

          if(cascade3->D1Ms!=NULL) VipInsertSSSingularity(cascade3->D1Ms,&slist);

          if(cascade3->D2ms!=NULL) VipInsertSSSingularity(cascade3->D2ms,&slist);

          if(cascade3->D2Ms!=NULL) VipInsertSSSingularity(cascade3->D2Ms,&slist);

          ncascade +=1;
          ana->csf = VipAllocSSObject();
          if(ana->csf==PB) return(PB);
        }
      
      walker = slist;
      walker = VipGetNextSingularityOnRight( walker, D2m );
      if(ana->csf!=NULL)
        {
          ana->csf->D2ms = walker;
          VipExpandSSObjectSingularities( ana->csf, volstruct );
          if (ana->csf->D1Ms)
            {
              VipComputeAnalysedLoc( ana->csf, mVipMax(2*ana->csf->D1Ms->scale_event - ana->csf->D2ms->scale_event,50)  );
              printf("marker/CSF loc under %d\n",(int)(mVipMax(2*ana->csf->D1Ms->scale_event - ana->csf->D2ms->scale_event,50)/2));
            }
          else
            VipComputeAnalysedLoc( ana->csf, mVipMin(ana->white->scale_event,ana->gray->scale_event) );
        }


      /*
      cascade4=PB;
      cascade4 = VipGetInsideRangeBiggestCascade(cascade3, 20, right, clist->scale_event);
      if(cascade4!=PB && cascade4->volume>1000)
        {
          if(cascade4->D1ms!=NULL) VipInsertSSSingularity(cascade4->D1ms,&slist);
          if(cascade4->D1Ms!=NULL) VipInsertSSSingularity(cascade4->D1Ms,&slist);
          if(cascade4->D2ms!=NULL) VipInsertSSSingularity(cascade4->D2ms,&slist);
          if(cascade4->D2Ms!=NULL) VipInsertSSSingularity(cascade4->D2Ms,&slist);
          ncascade +=1;
          ana->mode2 = VipAllocSSObject();
          if(ana->mode2==PB) return(PB);
        }
      
      if(ana->mode2!=NULL)
        {
          ana->mode2->D2ms = cascade4->D2ms;
          VipExpandSSObjectSingularities( ana->mode2, volstruct );
          VipComputeAnalysedLoc( ana->mode2, mVipMin(ana->white->scale_event,ana->gray->scale_event) );
        }
      */

    }

   if(ana->brain)
     {
       VipComputeAnalysedLoc( ana->brain, ana->brain->scale_event );
        ana->gray = ana->brain;
        ana->brain=NULL;
     }    

   PutUndersamplingRatio(ana,volstruct->undersampling_ratio);

  if( ana->csf!=NULL) 
    {
      printf("CSF (or marker) found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->csf->left_sigma,
             ana->csf->mean, ana->csf->right_sigma);
      if(ana->csf->D2ms->n_scale_extrema>=1)
        {
          ana->csf->D2ms->n_scale_extrema = 1;
          ana->csf->D2ms->scale_extrema[0] = ana->csf->good_scale;
        }
    }
  else
    printf("No CSF/marker found...\n");
  if( ana->gray!=NULL) 
    {
      ana->gray->sigma = mVipMax(ana->gray->left_sigma,ana->gray->right_sigma);
      printf("Gray matter found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->gray->left_sigma,
	 ana->gray->mean, ana->gray->right_sigma);
      if(ana->gray->D2ms->n_scale_extrema>=1)
        {
          ana->gray->D2ms->n_scale_extrema = 1;
          ana->gray->D2ms->scale_extrema[0] = ana->gray->good_scale;
        }
    }
  if( ana->white!=NULL)
    {
      ana->white->sigma = mVipMax(ana->white->right_sigma,ana->white->left_sigma);

      printf("White matter found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->white->left_sigma,
	 ana->white->mean, ana->white->right_sigma);
      if(ana->white->D2ms->n_scale_extrema>=1)
        {
          ana->white->D2ms->n_scale_extrema = 1;
          ana->white->D2ms->scale_extrema[0] = ana->white->good_scale;
        }
    }
  return(OK);
}




/*---------------------------------------------------------------------------*/
int VipAnalyseCascadesSPGRSequence(VipT1HistoAnalysis *ana,
SSCascade *clist, SSCascade *cbrain, Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{
  SSSingularity *slist=NULL, *walker=NULL;
  SSCascade *cgraywhite=NULL, *cgraycsf=NULL;
  int ncascade;
  int NO_CSF_CLUE = VFALSE;

  if(!clist || !ana || !cbrain || !volstruct)
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipAnalyseCascadesSPGRSequence");
      return(PB);
    }
  
  slist = NULL;
  ncascade = 0;
  printf("1\n"),fflush(stdout);
  
  if(clist->D1ms!=NULL) VipInsertSSSingularity(clist->D1ms,&slist);
  if(clist->D1Ms!=NULL) VipInsertSSSingularity(clist->D1Ms,&slist);
  if(clist->D2ms!=NULL) VipInsertSSSingularity(clist->D2ms,&slist);
  if(clist->D2Ms!=NULL) VipInsertSSSingularity(clist->D2Ms,&slist);
  ana->background = VipAllocSSObject();
  if(ana->background==PB) return(PB);
  ncascade +=1;

  if(cbrain->complet==VFALSE)
    {
      VipPrintfError("Strange brain cascade for SPGR sequence");
      VipPrintfExit("VipAnalyseCascadesSPGRSequence");
      return(PB);
    }
  VipInsertSSSingularity(cbrain->D1ms,&slist);
  VipInsertSSSingularity(cbrain->D1Ms,&slist);
  VipInsertSSSingularity(cbrain->D2ms,&slist);
  VipInsertSSSingularity(cbrain->D2Ms,&slist);
  ncascade +=1;
  ana->brain = VipAllocSSObject();
  if(ana->brain==PB) return(PB);
  printf("2\n"),fflush(stdout);

  /*first analyse*/
  walker = slist;
  walker = VipGetNextSingularityOnRight( walker, D2m );
  if(walker==PB) return(PB);
  ana->background->D2ms = walker;
  walker = VipGetNextSingularityOnRight( walker->right, D2m );
  if(walker==PB) return(PB);
  ana->brain->D2ms = walker;
  VipExpandSSObjectSingularities( ana->background, volstruct );
  VipExpandSSObjectSingularities( ana->brain, volstruct );

  printf("3\n"),fflush(stdout);
  printf("%d\n",ana),fflush(stdout);
  printf("%d\n",ana->brain),fflush(stdout);
  printf("%d\n",ana->brain->D2Mrs),fflush(stdout);
  printf("%d\n",ana->brain->D1Ms),fflush(stdout);
  printf("%d\n",ana->brain->D1ms),fflush(stdout);

  if (ana->brain->D2Mrs!=NULL && ana->brain->D1Ms!=NULL)
      {
	  cgraywhite = VipGetInsideRangeCascade(clist, ana->brain->D1Ms->loc[0], ana->brain->D2Mrs->loc[0], ana->brain->scale_event);
      }
  else if (ana->brain->D1Ms!=NULL && ana->brain->D1ms!=NULL)
      {
	  cgraywhite = VipGetInsideRangeCascade(clist, ana->brain->D1Ms->loc[0], ana->brain->D1ms->loc[0], ana->brain->scale_event);
      }
  else
      {
      VipPrintfWarning("Pb discovered in 2010 by C. Fischer");
      return(PB);
      }
  printf("31\n"),fflush(stdout);

  if(cgraywhite==PB)
    {
      VipPrintfWarning("Impossible to distinguish gray and white matter: insufficient grey white contrast, you have to increase field regularization in VipBiasCorrection)");
      return(MRI_T1_IR);
 
    }
  else
    {
      if(cgraywhite->D1ms!=NULL) VipInsertSSSingularity(cgraywhite->D1ms,&slist);
      if(cgraywhite->D1Ms!=NULL) VipInsertSSSingularity(cgraywhite->D1Ms,&slist);
      if(cgraywhite->D2ms!=NULL) VipInsertSSSingularity(cgraywhite->D2ms,&slist);
      if(cgraywhite->D2Ms!=NULL) VipInsertSSSingularity(cgraywhite->D2Ms,&slist);
      ncascade +=1;
      ana->white = VipAllocSSObject();
      if(ana->white==PB) return(PB);
      ana->gray = VipAllocSSObject();
      if(ana->gray==PB) return(PB);
    }
  fflush(stdout);
  printf("4\n"),fflush(stdout);

  if(ana->brain->D1Ms==NULL)
    {
      VipPrintfError("ana->brain->D1Ms Analyse problem");
      VipPrintfExit("VipAnalyseCascadesSPGRSequence");
      return(PB);
    }
  if(ana->background->D1ms==NULL)
    {
      cgraycsf = VipGetInsideRangeCascade(clist, 0, ana->brain->D1Ms->loc[0], ana->brain->scale_event);
    }
  else
    {
      cgraycsf = VipGetInsideRangeCascade(clist, ana->background->D1ms->loc[0], ana->brain->D1Ms->loc[0], ana->brain->scale_event);
    }
  if(cgraycsf==PB)
    {
      VipPrintfWarning("Unusual pattern: No CSF clue (perhaps this sequence comes from brucker 3T Inv/Recup with nonstationarities: You can force another kind of grey white contrast in VipHistoAnalysis.");
      NO_CSF_CLUE = VTRUE;
      /*ana->sequence=MRI_T1_IR;*/
    }
  else
    {
      if(cgraycsf->D1ms!=NULL) VipInsertSSSingularity(cgraycsf->D1ms,&slist);
      if(cgraycsf->D1Ms!=NULL) VipInsertSSSingularity(cgraycsf->D1Ms,&slist);
      if(cgraycsf->D2ms!=NULL) VipInsertSSSingularity(cgraycsf->D2ms,&slist);
      if(cgraycsf->D2Ms!=NULL) VipInsertSSSingularity(cgraycsf->D2Ms,&slist);
      ncascade +=1;
      ana->csf = VipAllocSSObject();
      if(ana->csf==PB) return(PB);
    }
  printf("5\n"),fflush(stdout);

  walker = ana->background->D2ms;
  walker = VipGetNextSingularityOnRight( walker->right, D2m );
  if(walker==PB) return(PB);
  if(NO_CSF_CLUE==VFALSE)
      {
	  ana->csf->D2ms = walker;


	  walker = VipGetNextSingularityOnRight( walker->right, D2m );
	  if(walker==PB) return(PB);
      }
  if(ncascade>=3)
      {
	  ana->gray->D2ms = walker;
	  walker = VipGetNextSingularityOnRight( walker->right, D2m );
	  if(walker==PB) return(PB);
	  ana->white->D2ms = walker;
      }
  else
      ana->brain->D2ms = walker;

  if(NO_CSF_CLUE==VFALSE)
      VipExpandSSObjectSingularities( ana->csf, volstruct );
  printf("6\n"),fflush(stdout);

  if(ncascade>=3)
    {
      VipExpandSSObjectSingularities( ana->white, volstruct );
      VipExpandSSObjectSingularities( ana->gray, volstruct );

      VipComputeAnalysedLoc( ana->gray, mVipMin(ana->gray->scale_event, ana->white->scale_event) );    
      VipComputeAnalysedLoc( ana->white, mVipMin(ana->gray->scale_event, ana->white->scale_event) );

      PutUndersamplingRatio(ana,volstruct->undersampling_ratio);
      ana->gray->sigma = ana->gray->left_sigma;
      /*      ana->gray->sigma = mVipMax(ana->gray->left_sigma,ana->gray->right_sigma);*/
      ana->white->sigma = mVipMax(ana->white->right_sigma,ana->white->left_sigma);
      printf("Gray matter found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->gray->left_sigma,
	     ana->gray->mean, ana->gray->right_sigma);
      printf("White matter found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->white->left_sigma,
		    ana->white->mean, ana->white->right_sigma);
      if(ana->gray->D2Mls!=NULL)
	if(ana->gray->D2Mls->cascade!=NULL)
   {
	  if(ana->gray->D2Mls->cascade->complet==VTRUE)
	    ana->partial_volume_effect = VFALSE;
      else
	{
	  ana->partial_volume_effect = VTRUE;
	  /*
	  printf("this image seems to be endowed with a large partial volume effect,\n");
	  printf("which can stem from larger voxel dimensions than usual (slice thickness...)\n");
	  */
	}
   }
      if(ana->csf!=NULL)
	  {
	      VipComputeAnalysedLoc( ana->csf, 50 );    
	      ana->csf->sigma = mVipMax(ana->csf->left_sigma, ana->csf->right_sigma);
	  }
    }
  else
  {
    VipExpandSSObjectSingularities( ana->brain, volstruct );
    VipComputeAnalysedLoc( ana->brain, ana->brain->scale_event );
    PutUndersamplingRatio(ana,volstruct->undersampling_ratio);
    ana->brain->sigma = mVipMax(ana->brain->left_sigma,ana->brain->right_sigma);

    printf("Brain found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->brain->left_sigma,
	     ana->brain->mean, ana->brain->right_sigma);
  }
  if(ana->background!=NULL)
      {
	  VipComputeAnalysedLoc( ana->background,50 );    
	  ana->background->sigma = ana->background->right_sigma;
      }
  return(OK);
}


/*---------------------------------------------------------------------------*/
int VipAnalyseCascadesIRSequence(VipT1HistoAnalysis *ana,
SSCascade *clist, Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{
  SSSingularity *slist, *walker;
  SSCascade *cwalker, *temp;
  int ncascade;
  int loc1, loc2; /*marseille PB*/
  int vol1, vol2;

  if(!clist || !ana || !volstruct )
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipAnalyseCascadesIRSequence");
      return(PB);
    }
  
  slist = NULL;
  ncascade = 0;
  
  if(clist->D1ms!=NULL) VipInsertSSSingularity(clist->D1ms,&slist);
  if(clist->D1Ms!=NULL) VipInsertSSSingularity(clist->D1Ms,&slist);
  if(clist->D2ms!=NULL) VipInsertSSSingularity(clist->D2ms,&slist);
  if(clist->D2Ms!=NULL) VipInsertSSSingularity(clist->D2Ms,&slist);
  ana->background = VipAllocSSObject();
  if(ana->background==PB) return(PB);
  ncascade +=1;

  cwalker = clist->next;
  /* it is the biggest volume cascade, if it is not complet, it is either
     a terribly compressed image, or this is a background additional mode which will be discarded further
  while(cwalker!=NULL && (cwalker->complet!=VTRUE) cwalker = cwalker->next;*/
  if(cwalker==NULL)
    {
      VipPrintfError("NULL cascade 1");
      VipPrintfExit("VipAnalyseCascadesIRSequence");
      return(PB);
    }
  /*PB with Marseille sequence where there is a huge noise mode near background*/
  loc1 = cwalker->D2ms->loc[0];
  if(cwalker->next!=NULL) loc2 = cwalker->next->D2ms->loc[0];
  else loc2 = 1;
  if(loc2<loc1) /*grey has to be largest*/
    {
      if((cwalker->next->next->volume)>(0.03*cwalker->volume)
	 && (cwalker->next->next->D2ms->loc[0])>loc2) /*additional check for noise mode*/
	{
	  printf("1\n");
	  temp = cwalker->next;
	  cwalker->next = cwalker->next->next;
	  /*cwalker->next->next = temp;*/
	  /*	  printf("Additional background mode test: %d/%d\n",loc1,loc2);
		  VipPrintfWarning("Trigger first left noise mode protection!");*/
	}
    }
  else if(   ((float) loc1/ (float)loc2)<0.4) /*usually gray is closer to white than to background*/
    {

      if(cwalker->next->next!=NULL)
	{
	  if((float)(cwalker->next->next->volume)>(0.2*cwalker->volume)) /*additional check for noise mode*/
	    {
	      cwalker = cwalker->next;
	      /*
	      printf("Additional background mode test: %d/%d\n",loc1,loc2);
	      VipPrintfWarning("Trigger second left noise mode protection!");*/
	      if(cwalker==NULL)
		{
		  VipPrintfError("NULL cascade 2 (left noise mode protection)");
		  VipPrintfExit("VipAnalyseCascadesIRSequence");
		  return(PB);
		} 
	    }
	}
    }

  if(cwalker->D1ms!=NULL) VipInsertSSSingularity(cwalker->D1ms,&slist);
  if(cwalker->D1Ms!=NULL) VipInsertSSSingularity(cwalker->D1Ms,&slist);
  if(cwalker->D2ms!=NULL) VipInsertSSSingularity(cwalker->D2ms,&slist);
  if(cwalker->D2Ms!=NULL) VipInsertSSSingularity(cwalker->D2Ms,&slist);
  ncascade +=1;

  vol1 = cwalker->volume; /*white or grey mode*/

  cwalker = cwalker->next;

  while(cwalker!=NULL && (cwalker->complet!=VTRUE && (cwalker->next!=NULL && ((float)(cwalker->next->volume)/(float)(cwalker->volume))>0.5) ) ) cwalker = cwalker->next;
  if(cwalker==NULL)
    {
      VipPrintfError("NULL cascade 2");
      VipPrintfExit("VipAnalyseCascadesIRSequence");
      return(PB);
    } 
  if(cwalker!=NULL) vol2 = cwalker->volume;
  else vol2 = 0.000001;
  if(cwalker!=NULL)
    {
      if(   ((float) vol2/ (float)vol1)<0.01) /*we get a long life noise mode*/
	{
	  cwalker = cwalker->next;
	  printf("Additional mode volume test: %d/%d\n",vol1,vol2);
	  VipPrintfWarning("Trigger right noise mode protection!");
	  if(cwalker==NULL)
	    {
	      VipPrintfError("NULL cascade 2 (right noise mode protection)");
	      VipPrintfExit("VipAnalyseCascadesIRSequence");
	      return(PB);
	    } 
	}
    }
  if(cwalker!=NULL)
    {
      if(cwalker->D1ms!=NULL) VipInsertSSSingularity(cwalker->D1ms,&slist);
      if(cwalker->D1Ms!=NULL) VipInsertSSSingularity(cwalker->D1Ms,&slist);
      if(cwalker->D2ms!=NULL) VipInsertSSSingularity(cwalker->D2ms,&slist);
      if(cwalker->D2Ms!=NULL) VipInsertSSSingularity(cwalker->D2Ms,&slist);
      ncascade +=1;
    }

  ana->white = VipAllocSSObject();
  if(ana->white==PB) return(PB);
  ana->gray = VipAllocSSObject();
  if(ana->gray==PB) return(PB);

  /*analyse*/
  walker = slist;
  walker = VipGetNextSingularityOnRight( walker, D2m );
  if(walker==PB) return(PB);
  ana->background->D2ms = walker;
  walker = VipGetNextSingularityOnRight( walker->right, D2m );
  if(walker==PB) return(PB);
  ana->gray->D2ms = walker;
  walker = VipGetNextSingularityOnRight( walker->right, D2m );
  if(walker==PB) return(PB);
  ana->white->D2ms = walker;

  VipExpandSSObjectSingularities( ana->background, volstruct );
  VipExpandSSObjectSingularities( ana->white, volstruct );
  VipExpandSSObjectSingularities( ana->gray, volstruct );

  /*
  printf("Scale events: %d, %d\n",ana->gray->scale_event, ana->white->scale_event);
  */
  VipComputeAnalysedLoc( ana->gray, mVipMin(ana->gray->scale_event, ana->white->scale_event) );
  VipComputeAnalysedLoc( ana->white, mVipMin(ana->gray->scale_event, ana->white->scale_event) );

  PutUndersamplingRatio(ana,volstruct->undersampling_ratio);
  ana->gray->sigma = ana->gray->left_sigma;
  /*  ana->gray->sigma = mVipMax(ana->gray->left_sigma,ana->gray->right_sigma);*/
  ana->white->sigma = mVipMax(ana->white->right_sigma,ana->white->left_sigma);

  printf("Gray matter found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->gray->left_sigma,
	 ana->gray->mean, ana->gray->right_sigma);
  printf("White matter found at: [%d-%d-%d] (sigma/mean/sigma)\n",ana->white->left_sigma,
	 ana->white->mean, ana->white->right_sigma);

  VipComputeAnalysedLoc( ana->background, 50 );
  ana->background->sigma = ana->background->right_sigma;
  return(OK);
}

/*---------------------------------------------------------------------------*/
SSSingularity *VipGetNextSingularityOnRight( SSSingularity *s, int nature )
/*---------------------------------------------------------------------------*/
{
  if(s==NULL)
    {
      VipPrintfError("Null Arg");
      VipPrintfExit("VipGetNextSingularityOnRight");
      return(PB);
    }

  while((s!=NULL)&&(s->nature!=nature)) s = s->right;

  if(s==NULL)
    {      
      VipPrintfError("Can not find such Singularity");
      VipPrintfExit("VipGetNextSingularityOnRight");
      return(PB);
    }

  return(s);
}

/*---------------------------------------------------------------------------*/
SSObject *VipAllocSSObject()
/*---------------------------------------------------------------------------*/
{
  SSObject *o;

  o = VipCalloc(1, sizeof(SSObject), "VipAllocSSObject");
  return(o);  
}


/*---------------------------------------------------------------------------*/
int VipComputeAnalysedLoc (SSObject *o, int scale)
/*---------------------------------------------------------------------------*/
{
  int i;
  int good_scale = -1;
  int found = VFALSE;
  int VISU=VFALSE;
  

  if(!o || !(o->D2ms) || (scale<0) || (scale>1000000))
    {
      VipPrintfError("Bad argument");
      VipPrintfExit("VipComputeAnalysedLoc");
      return(PB);
    }
  
  scale = mVipMin(scale,5000); /*Je change ca en fevrier 2004, c'etait surement une protection,
                                    mais je ne sais pas vraiment pourquoi, je passe a 5000*/

  VipComputeSSSingularitySimpleSpeedMinima( o->D2ms );

  if(VISU==VTRUE) 
    {
      printf("------------------");
      printf("Scale lim: %d\n",scale);
    }
  for(i=o->D2ms->n_scale_extrema-1;i>=0;i--)
    {
	if(VISU==VTRUE) printf("speed min: %d\n",o->D2ms->scale_extrema[i]);
      if(o->D2ms->scale_extrema[i]<scale) 
	{
	  found = VTRUE;
	  if(good_scale<o->D2ms->scale_extrema[i]) good_scale = o->D2ms->scale_extrema[i];
	}
    }
  if(found==VFALSE)
    {/*
       VipPrintfWarning("Can not found speed minimum under limit scale, assuming scale 5 is OK\n");*/
      good_scale = mVipMin(10,o->scale_event);
    }

  o->good_scale = good_scale;

  o->D2ms->analysed_loc = o->D2ms->loc[good_scale];
  if(VISU==VTRUE) printf("D2: %d\n",o->D2ms->analysed_loc);
  if(o->D1Ms!=NULL)
    {
      if(o->D1Ms->scale_event > good_scale)
	  {
	      o->D1Ms->analysed_loc = o->D1Ms->loc[good_scale];
	      if(VISU==VTRUE) printf("scale_event:%d, good_scale: %d, D1M: %d\n",o->D1Ms->scale_event,good_scale,o->D1Ms->analysed_loc);
	  }
      else
	   {/*
	      VipPrintfWarning("The optimal scale for this object is not compatible with its left edge, assuming scale 10 is OK\n");*/
	     o->D1Ms->analysed_loc = mVipMin(o->D1Ms->loc[10],o->D1Ms->scale_event);
	   }
    }
  if(o->D1ms!=NULL)    
    {
      if(o->D1ms->scale_event > good_scale)
	  {
	      o->D1ms->analysed_loc = o->D1ms->loc[good_scale];
	      if(VISU==VTRUE) printf("scale_event:%d, good_scale: %d, D1m: %d\n",o->D1ms->scale_event,good_scale,o->D1ms->analysed_loc);
	  }
      else
	{/*
	   VipPrintfWarning("The optimal scale for this object is not compatible with its right edge, assuming scale 10 is OK\n");*/
	  o->D1ms->analysed_loc = mVipMin(o->D1ms->loc[10],o->D1ms->scale_event);
	}
    }

  o->mean = o->D2ms->analysed_loc;
  
  if(o->D1Ms!=NULL) o->left_sigma = o->mean - o->D1Ms->analysed_loc;
  if(o->D1ms!=NULL) o->right_sigma = o->D1ms->analysed_loc - o->mean;
  /*
  if(o->D1Ms!=NULL) o->left_sigma = (int)(sqrt((double)(o->mean - o->D1Ms->analysed_loc)*(o->mean - o->D1Ms->analysed_loc)-good_scale/2)+0.5);
  if(o->D1ms!=NULL) o->right_sigma =(int)(sqrt((double)(o->mean - o->D1ms->analysed_loc)*(o->mean - o->D1ms->analysed_loc)-good_scale/2)+0.5);
  */
  if(o->left_sigma==0 && o->right_sigma!=0) o->left_sigma = o->right_sigma;
  if(o->left_sigma!=0 && o->right_sigma==0) o->right_sigma = o->left_sigma;

  return(OK);

}

/*---------------------------------------------------------------------------*/
int VipExpandSSObjectSingularities (SSObject *o, Vip1DScaleSpaceStruct *volstruct)
/*---------------------------------------------------------------------------*/
{
  int sigma;
  

  if(!o || o->D2ms==NULL || !volstruct)
    {
      VipPrintfError("Bad argument");
      VipPrintfExit("VipExpandSingularities");
      return(PB);
    }
 
  o->scale_event = o->D2ms->scale_event;

  if(o->D2ms->left!=NULL)
    {
      if(o->D2ms->left->nature==D1M)
	{
	  o->D1Ms = o->D2ms->left;	  	  
	  if(o->D1Ms->left!=NULL)
	    {
	      if(o->D1Ms->left->nature==D2M)
		{
		  sigma = o->D2ms->loc[0] - o->D1Ms->loc[0];
		  if(o->D1Ms->left->loc[0]>(o->D2ms->loc[0]-20*sigma))
		    {
		      o->D2Mls = o->D1Ms->left;
		     
		    }
		}
	    }
	}
    }

  if(o->D2ms->right!=NULL)
    {
      if(o->D2ms->right->nature==D1m)
	{
	  o->D1ms = o->D2ms->right;
	  if(o->D1ms->right!=NULL)
	    {
	      if(o->D1ms->right->nature==D2M)
		{
		  sigma = o->D1ms->loc[0] - o->D2ms->loc[0];
		  if(o->D1ms->right->loc[0]<(o->D2ms->loc[0]+20*sigma))
		    {
		      o->D2Mrs = o->D1ms->right;
		      
		    }
		}
	    }
	}
    }

  return(OK);

}

/*---------------------------------------------------------------------------*/
SSCascade *VipGetInsideRangeCascade( SSCascade *clist, int left, int right, int scalemax)
/*---------------------------------------------------------------------------*/
{
  SSCascade *cgw=NULL, *walker=NULL;
  int highest;

  if(!clist)
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipGetInsideD1Cascade");
      return(PB);
    }
  if(left>=right)
    {
      VipPrintfError("Unconsistent range");
      VipPrintfExit("VipGetInsideD1Cascade");
      return(PB);
    }    

  highest = 0;
  walker = clist;
  cgw = NULL;

       printf("outside");fflush(stdout);

  while(walker!=NULL)
    {
      printf("O");fflush(stdout);
      if((walker->D2ms) && ((walker->D2ms->loc[0]) > left )
	 &&((walker->D2ms->loc[0]) < right )
	 &&((walker->scale_event)<scalemax))
        {
          if(walker->scale_event>highest)
            {
              highest = walker->scale_event;
              cgw = walker;
            }
        }
      walker = walker->next;
    }

  return(cgw);
}


/*---------------------------------------------------------------------------*/
SSCascade *VipGetInsideRangeBiggestCascade( SSCascade *clist, int left, int right, int scalemax)
/*---------------------------------------------------------------------------*/
{
  SSCascade *cgw=NULL, *walker=NULL;
  int biggest;

  if(!clist)
    {
      VipPrintfError("Null argument");
      VipPrintfExit("VipGetInsideD1Cascade");
      return(PB);
    }
  if(left>=right)
    {
      VipPrintfError("Unconsistent range");
      VipPrintfExit("VipGetInsideD1Cascade");
      return(PB);
    }    

  biggest = 0;
  walker = clist;
  cgw = NULL;

  while(walker!=NULL)
    {
      if((walker->D2ms) && ((walker->D2ms->loc[0]) > left )
	 &&((walker->D2ms->loc[0]) < right )
	 &&((walker->scale_event)<scalemax))
        {
          if(walker->volume>biggest)
            {
              biggest = walker->volume;
              cgw = walker;
            }
        }
      walker = walker->next;
    }

  return(cgw);
}
