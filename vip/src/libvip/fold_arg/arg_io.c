/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 25/07/1999 
 * VERSION     : 1.5                 * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : recuperation du format de graphe relationnel
 *               attribue sur les plis corticaux  de la these de JF Mangin
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/


#include <vip/fold_arg.h>
#include <vip/fold_arg_static.h>
#include <vip/volume.h>
#include <vip/util/file.h>
#include <vip/util/shelltools.h>

/*-------------------------------------------------------------------------*/
int VipWriteGenericFoldArg(FoldArg *arg ,char *name)
/*-------------------------------------------------------------------------*/

{
    FILE *f;
    char filename[VIP_NAME_MAXLEN];
    char shortname[VIP_NAME_MAXLEN];
    char sep;


    if(arg==NULL)
	{
	    VipPrintfError("No Fold arg as input");
	    VipPrintfExit("VipWriteGenericFoldArg");
	    return(PB);
	}
    if(name==NULL)
	{
	    VipPrintfError("No name for Fold arg as input");
	    VipPrintfExit("VipWriteGenericFoldArg");
	    return(PB);
	}

    VipBasename( name, shortname );

    printf("------------------------------------\n");
    printf("Writing C++ generic library graph format\n");
    printf("------------------------------------\n");

    sep = VipFileSeparator();

    sprintf(filename,"%s.data",name);
    VipMkdir( filename );

    sprintf(filename,"%s.data%css", name, sep );
    VipMkdir( filename );

    sprintf(filename,"%s.data%cbottom", name, sep );
    VipMkdir( filename );

    sprintf(filename,"%s.data%cother", name, sep );
    VipMkdir( filename );

    sprintf(filename,"%s.data%cjunction", name, sep );
    VipMkdir( filename );

    sprintf(filename,"%s.data%cplidepassage", name, sep );
    VipMkdir( filename );

    sprintf(filename,"%s.data%ccortical", name, sep );
    VipMkdir( filename );

    sprintf(filename,"%s.data%cTmtk", name, sep );
    VipMkdir( filename );

    if(VipWriteGenericARGtoRef(arg, name, shortname)==PB) return(PB);

    if(arg->ss_list!=NULL)
	if(VipWriteGenericSSfile(arg->ss_list,name, shortname, arg->vox)==PB)
	return(PB);
    
    if(arg->jset!=NULL)
	if(VipWriteGenericJunctionSet(arg->jset,name, shortname,arg->vox)==PB) return(PB);
    
    if(arg->ppset!=NULL)
	if(VipWriteGenericPliDePassageSet(arg->ppset,name, shortname,arg->vox)==PB) return(PB);
    
    if(arg->cortex_jset!=NULL)
	if(VipWriteGenericCortexRelationSet(arg->cortex_jset,name, shortname,arg->vox)==PB) return(PB);
    
    strcpy(filename,name);
    strcat(filename,".arg");
    f=fopen(filename,"a");
    if(f==NULL)
	{
	    fprintf(stderr,"can not open %s\n",filename);
	    return(PB);
	}
    else
	{
	    fprintf(f,"*END\n");
	    fclose(f);
	}

    return(OK);
}

/*-------------------------------------------------------------------------*/
FoldArg *VipReadOldFoldArg(
char *name)
/*-------------------------------------------------------------------------*/
{
	FoldArg *new;

	if(name==NULL)
	  {
	    VipPrintfError("No name for Fold arg as input");
	    VipPrintfExit("VipReadOldFoldArg");
	    return(PB);
	  }
	new = CreateEmptyFoldArg(name);
	if(new==PB) return(PB); 

	new->ss_list = VipReadOldSSfile(name);
	if(new->ss_list==PB) 
	  {
	    return(PB);
	  }
	new->jset = VipReadOldJunctionSet(name, new->ss_list);
	if(new->jset==PB)
	  {
	    VipPrintfWarning("No junction file read");
	  }
	else
	  {
	    if(FillSSListJunctionPtr(new->ss_list,new->jset)==PB)
	      {
		VipPrintfError("Can not put junctions on SS");
		return(PB);
	      }
	  }
	new->ppset = VipReadOldPliDePassageSet(name, new->ss_list);
	if(new->ppset==PB)
	  {
	    VipPrintfWarning("No plidepassage file read");
	  }
	else
	  {
	    if(FillSSListPliDePassagePtr(new->ss_list,new->ppset)==PB)
	      {
		VipPrintfError("Can not put plis de passage on SS");
		return(PB);
	      }
	  }
	new->cortex_jset = VipReadOldCortexRelationSet( name, new->ss_list);
	if(new->jset==PB)
	  {
	    VipPrintfWarning("No cortical relation file read");
	  }
	else
	  {
	    if(FillSSListJunctionPtr(new->ss_list,new->jset)==PB)
	      {
		VipPrintfError("Can not put cortical relations on SS");
		return(PB);
	      }
	  }

	VipReadOldARGtoRef(new, name);
	return(new);
}

/*-------------------------------------------------------------------------*/
int VipReadOldARGtoRef(
FoldArg *arg,
char *name)
/*-------------------------------------------------------------------------*/
{
    char filename[VIP_NAME_MAXLEN];
    FILE *fp;
    char w[VIP_NAME_MAXLEN];
    int x, y, z;
    int CAx, CAy, CAz;
    int CPx, CPy, CPz;
    int IHx, IHy, IHz;
    int n;
    char buf1[VIP_NAME_MAXLEN],buf2[VIP_NAME_MAXLEN],
	buf3[VIP_NAME_MAXLEN], buf4[VIP_NAME_MAXLEN];

    if(!arg || !name)
	{
	    VipPrintfError("No name ot arg for Fold arg as input");
	    VipPrintfExit("VipReadOldARGtoRef");
	    return(PB);
	}

    strcpy(filename,name);
    strcat(filename,".ref.Vip");

    fp = fopen(filename,"r");
    if (!fp)
	{
	    fprintf(stderr,"Can not open file %s\n",filename);
	    VipPrintfExit("VipReadOldARGtoRef");
	    return(PB);
	}

    printf("Reading referentiel for %s...\n",name);
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if(*w!='r')
	{
	    fprintf(stderr,"Pb dans %s (rotation)\n",filename);
	    VipPrintfExit("VipReadOldARGtoRef");
	    return(PB);
	}
    if ( fscanf(fp,"%d%d",&(arg->gotoref_filled),&(arg->gonetoref)) != 2 )
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( fscanf(fp, "%f%f%f", &(arg->gotoref.t.x), &(arg->gotoref.t.y), &(arg->gotoref.t.z)) != 3)
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( fscanf(fp, "%lf%lf%lf", &(arg->gotoref.r.xx), &(arg->gotoref.r.xy), &(arg->gotoref.r.xz)) != 3)
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( fscanf(fp, "%lf%lf%lf", &(arg->gotoref.r.yx), &(arg->gotoref.r.yy), &(arg->gotoref.r.yz)) != 3)
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( fscanf(fp, "%lf%lf%lf", &(arg->gotoref.r.zx), &(arg->gotoref.r.zy), &(arg->gotoref.r.zz)) != 3)
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if(*w!='s')
	{
	    fprintf(stderr,"Pb dans %s (scale)\n",filename);
	    VipPrintfExit("VipReadOldARGtoRef");
	    return(PB);
	}
    if ( fscanf(fp, "%f%f%f", &(arg->scaleref.x), &(arg->scaleref.y), &(arg->scaleref.z)) != 3)
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if(*w!='b')
	{
	    fprintf(stderr,"Pb dans %s (box)\n",filename);
	    VipPrintfExit("VipReadOldARGtoRef");
	    return(PB);
	}
    if ( fscanf(fp, "%d%d%d", &x, &y, &z) != 3 )
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    arg->min_ima.x = x;
    arg->min_ima.y = y;
    arg->min_ima.z = z;
    if ( fscanf(fp, "%d%d%d", &x, &y, &z) != 3 )
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    arg->max_ima.x = x;
    arg->max_ima.y = y;
    arg->max_ima.z = z;
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if(*w!='r')
	{
	    fprintf(stderr,"Pb dans %s (refbox)\n",filename);
	    VipPrintfExit("VipReadOldARGtoRef");
	    return(PB);
	}
    if ( fscanf(fp, "%f%f%f", &(arg->min_ref.x), &(arg->min_ref.y), &(arg->min_ref.z)) != 3 )
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( fscanf(fp, "%f%f%f", &(arg->max_ref.x), &(arg->max_ref.y), &(arg->max_ref.z)) != 3)
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
		{
			VipPrintfExit("VipReadOldARGtoRef: File is corrupted");
	    return(PB);
		}
    if(*w!='v')
	{
	    fprintf(stderr,"Pb dans %s (voxel geometry)\n",filename);
	    VipPrintfExit("VipReadOldARGtoRef");
	    return(PB);
	}
    if ( fscanf(fp, "%f%f%f", &(arg->vox.x), &(arg->vox.y), &(arg->vox.z)) != 3 )
		{
	    VipPrintfExit("VipReadOldARGtoRef: Fail to read the items");
	    return(PB);
		}
    n = fscanf(fp,"%s%d%s%d%s%d%s%d",buf1,&(arg->CAfilled),
	       buf2,&CAx,buf3,&CAy,buf4,&CAz);
    if(n!=8)
	{
	    fprintf(stderr, "Warning: CAN not read CA...\n");
	    arg->CA.x = 0.;
	    arg->CA.y = 0.;
	    arg->CA.z = 0.;
	}
    else
	{
	    arg->CA.x = CAx;
	    arg->CA.y = CAy;
	    arg->CA.z = CAz;
	}
    n = fscanf(fp,"%s%d%s%d%s%d%s%d",buf1,&(arg->CPfilled),
	       buf2,&CPx,buf3,&CPy,buf4,&CPz);
    if(n!=8)
	{
	    fprintf(stderr, "Warning: CAN not read CP...\n");
	    arg->CP.x = 0.;
	    arg->CP.y = 0.;
	    arg->CP.z = 0.;
	}
    else
	{
	    arg->CP.x = CPx;
	    arg->CP.y = CPy;
	    arg->CP.z = CPz;
	}

    n = fscanf(fp,"%s%d%s%d%s%d%s%d",buf1,&(arg->IHfilled),
	       buf2,&IHx,buf3,&IHy,buf4,&IHz);
    if(n!=8)
	{
	    fprintf(stderr, "Warning: CAN not read IH...\n");
	    arg->IH.x = 0.;
	    arg->IH.y = 0.;
	    arg->IH.z = 0.;
	}
    else
	{
	    arg->IH.x = IHx;
	    arg->IH.y = IHy;
	    arg->IH.z = IHz;
	}

    fclose(fp);

    return(OK);
}

/*-------------------------------------------------------------------------*/
SurfaceSimpleList *VipReadOldSSfile(
char *name)
/*-------------------------------------------------------------------------*/

{
  SurfaceSimpleList *sslist;
  char filename[VIP_NAME_MAXLEN];
  FILE *fp;
  SurfaceSimple *ssptr=NULL, first, *new=NULL;
  Vip3DPoint_S16BIT *sspptr=NULL, *edgepptr=NULL, *otherpptr=NULL;
  char w[VIP_NAME_MAXLEN], buf1[VIP_NAME_MAXLEN], buf2[VIP_NAME_MAXLEN];
  char poub1[VIP_NAME_MAXLEN], poub2[VIP_NAME_MAXLEN], poub3[VIP_NAME_MAXLEN];
  int nlu=0;
  int lab, npoints, sspoints, edgepoints,  otherpoints, index;
  int nscan;
  int x, y, z;
  float size;
  Vip3DPoint_VFLOAT normale, refnormale;
  Vip3DPoint_VFLOAT refboxmin, refboxmax;
  int boxminx, boxminy, boxminz, boxmaxx, boxmaxy, boxmaxz;
  Vip3DPoint_VFLOAT refg, g;
  float depth;
  int depth_filled;
  float mindepth;
  int mindepth_filled;
  int refnormale_filled;
  int normale_filled;
  int refbox_filled = VFALSE;
  int box_filled = VFALSE;
  int g_filled, refg_filled;
  int size_filled;
  int lookup_size, i;
  int rootsbassin, rootsbassin_filled;
  int talcovar_filled;
  float talcovar[3][3];

  if(name==NULL)
    {
      VipPrintfError("No name for Fold arg as input");
      VipPrintfExit("VipOldReadSSfile");
      return(PB);
    }
	
  strcpy(filename,name);
  strcat(filename,".ss.Vip");
	
  fp = fopen(filename,"r");
  if (!fp)
    {
      fprintf(stderr,"Can not open file %s\n",filename);
      VipPrintfExit("VipOldReadSSfile");
      return(PB);
    }

  printf("Reading %s simple surfaces...\n",name);

  first.next = NULL;

  sslist = (SurfaceSimpleList *)VipCalloc(1,sizeof(SurfaceSimpleList),"VipOldReadSSfile");
  if(!sslist) return(PB);
  sslist->n_ss = 0;

  if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
	{
      VipPrintfExit("VipOldReadSSfile : File is corrupted");
      return(PB);
	}

  while(!feof(fp))
    {
      if (*w != '#')
	{
	  if(*w == 'N')
	    {
	      if(sslist->n_ss!=0)
		{
		  fprintf(stderr,"incoherence dans %s\n",filename);
		  VipPrintfExit("VipOldReadSSfile");
		  return(PB);
		}
	      sscanf(&w[1], "%d", &(sslist->n_ss));
	      ssptr = &first;
	      new = NULL;
	      nlu = 0;
	      sslist->tab = (SurfaceSimple **)VipCalloc(sslist->n_ss+1,sizeof(SurfaceSimple *),
							 "VipOldReadSSfile");
	      if(sslist->tab==PB) return(PB);

	      lookup_size = HULL_SURFACE + (sslist->n_ss+1)*10; /*SS = label + range of length 9*/ 
	      sslist->labTOindex = (int *)VipCalloc(lookup_size,sizeof(int),"VipOldReadSSfile");
	      if(sslist->labTOindex==PB) return(PB);
	      for(i=0;i<lookup_size;i++) sslist->labTOindex[i] = HULL_SURFACE_INDEX;
	    }
	  else if ((*w=='$')&&(sslist->n_ss!=0))
	    {
	      if(*(w+1)=='l') sscanf(&w[1], "%s%d", buf1, &lab);
	      else if(*(w+1)=='i')
		{
		  if(*(w+2)=='d') sscanf(&w[1], "%s%s", buf1, buf2);
		  else if(*(w+2)=='n') sscanf(&w[1], "%s%d", buf1, &index);
		}
	      else if(!strncmp(w+1,"box",3))
		sscanf(&w[1],"%s%d%s%d%d%s%d%d%s%d%d",buf1, &box_filled,
		       poub1, &(boxminx), &(boxmaxx),
		       poub2, &(boxminy), &(boxmaxy),
		       poub3, &(boxminz), &(boxmaxz));
	      else if(!strncmp(w+1,"refbox",5))
		sscanf(&w[1],"%s%d%s%f%f%s%f%f%s%f%f",buf1, &refbox_filled,
		       poub1, &(refboxmin.x), &(refboxmax.x),
		       poub2, &(refboxmin.y), &(refboxmax.y),
		       poub3, &(refboxmin.z), &(refboxmax.z));
	      else if(!strncmp(w+1,"size",4)) sscanf(&w[1], "%s%d%f", buf1, &size_filled, &size);
	      else if(!strncmp(w+1,"roots",4)) sscanf(&w[1], "%s%d%d", buf1, &rootsbassin_filled, &rootsbassin);
	      else if(!strncmp(w+1,"normale",4)) sscanf(&w[1], "%s%d%f%f%f", buf1, &normale_filled,
							&(normale.x),&(normale.y),&(normale.z));
	      else if(!strncmp(w+1,"refnormale",4)) sscanf(&w[1], "%s%d%f%f%f", buf1, &refnormale_filled,
							   &(refnormale.x),&(refnormale.y),&(refnormale.z));
	      else if(!strncmp(w+1,"g",1)) sscanf(&w[1], "%s%d%f%f%f", buf1, &g_filled,&(g.x),&(g.y),&(g.z));
	      else if(!strncmp(w+1,"refg",4)) sscanf(&w[1], "%s%d%f%f%f", buf1, &refg_filled,
						     &(refg.x),&(refg.y),&(refg.z));
	      else if(!strncmp(w+1,"prof",2)) sscanf(&w[1], "%s%d%f", buf1, &depth_filled, &depth);
	      else if(!strncmp(w+1,"depth",4)) sscanf(&w[1], "%s%d%f", buf1, &depth_filled, &depth);
	      else if(!strncmp(w+1,"mindepth",7)) sscanf(&w[1], "%s%d%f", buf1, &mindepth_filled, &mindepth);
	      else if(!strncmp(w+1,"talcovar",8)) sscanf(&w[1], "%s%d%f%f%f%f%f%f%f%f%f", buf1, &talcovar_filled, &talcovar[0][0], &talcovar[0][1], &talcovar[0][2],
&talcovar[1][0], &talcovar[1][1], &talcovar[1][2],
&talcovar[2][0], &talcovar[2][1], &talcovar[2][2]);
	      else if(*(w+1)=='n') sscanf(&w[1], "%s%d", buf1, &npoints);
	      else if(*(w+1)=='s') sscanf(&w[1], "%s%d", buf1, &sspoints);
	      else if(*(w+1)=='e') sscanf(&w[1], "%s%d", buf1, &edgepoints);
	      else if(*(w+1)=='o') sscanf(&w[1], "%s%d", buf1, &otherpoints);
	      else
		{
		  fprintf(stderr,"I do not understand %s\n",w);
		  VipPrintfExit("VipOldReadSSfile");
		  return(PB);
		}
	    }
	  else if (*w=='*')
	    {
	      if(*(w+1)=='b')
		{
		  new = CreateEmptySurfaceSimple();
		  new->label = lab;
		  new->index = index;
		  sslist->tab[new->index] = new;
		  for(i=0;i<10;i++)
		    sslist->labTOindex[new->label+i] = new->index;
		  new->the_label = 0;
		  strcpy(new->the_name, buf2);
		  new->n_points = npoints;
		  new->surface_points = VipAlloc3DBucket_S16BIT(sspoints);
		  if(new->surface_points==NULL) return(PB);
		  new->edge_points = VipAlloc3DBucket_S16BIT(edgepoints);
		  if(new->edge_points==NULL) return(PB);
		  new->other_points = VipAlloc3DBucket_S16BIT(otherpoints);
		  if(new->other_points==NULL) return(PB);
		  new->next = NULL;
		  new->box_filled = box_filled;
		  new->boxmin.x = boxminx;
		  new->boxmin.y = boxminy;
		  new->boxmin.z = boxminz;
		  new->boxmax.x = boxmaxx;
		  new->boxmax.y = boxmaxy;
		  new->boxmax.z = boxmaxz;
		  new->refbox_filled = refbox_filled;
		  new->refboxmin.x = refboxmin.x;
		  new->refboxmin.y = refboxmin.y;
		  new->refboxmin.z = refboxmin.z;
		  new->refboxmax.x = refboxmax.x;
		  new->refboxmax.y = refboxmax.y;
		  new->refboxmax.z = refboxmax.z;
		  new->size = size;
		  new->size_filled = size_filled;
		  new->rootsbassin = rootsbassin;
		  new->rootsbassin_filled = rootsbassin_filled;
		  new->normale.x = normale.x;
		  new->normale.y = normale.y;
		  new->normale.z = normale.z;
		  new->normale_filled = normale_filled;
		  new->refnormale.x = refnormale.x;
		  new->refnormale.y = refnormale.y;
		  new->refnormale.z = refnormale.z;
		  new->refnormale_filled = refnormale_filled;
		  new->g.x = g.x;
		  new->g.y = g.y;
		  new->g.z = g.z;
		  new->g_filled = g_filled;
		  new->refg.x = refg.x;
		  new->refg.y = refg.y;
		  new->refg.z = refg.z;
		  new->refg_filled = refg_filled;
		  new->depth = depth;
		  new->depth_filled = depth_filled;
		  new->mindepth = mindepth;
		  new->mindepth_filled = mindepth_filled;
		  new->talcovar_filled = talcovar_filled;
		  new->talcovar[0][0] = talcovar[0][0];
		  new->talcovar[0][1] = talcovar[0][1];
		  new->talcovar[0][2] = talcovar[0][2];
		  new->talcovar[1][0] = talcovar[1][0];
		  new->talcovar[1][1] = talcovar[1][1];
		  new->talcovar[1][2] = talcovar[1][2];
		  new->talcovar[2][0] = talcovar[2][0];
		  new->talcovar[2][1] = talcovar[2][1];
		  new->talcovar[2][2] = talcovar[2][2];
		  sspptr = new->surface_points->data;
		  edgepptr = new->edge_points->data;
		  otherpptr = new->other_points->data;
		  box_filled = VFALSE;
		  refbox_filled = VFALSE;
		  size_filled = VFALSE;
		  rootsbassin_filled = VFALSE;
		  normale_filled = VFALSE;
		  refnormale_filled = VFALSE;
		  g_filled = VFALSE;
		  refg_filled = VFALSE;
		  talcovar_filled = VFALSE;

		}
	      else if (*(w+1)=='e')
		{
		  if((new->surface_points->size != new->surface_points->n_points)
		     || (new->edge_points->size != new->edge_points->n_points)
		     || (new->other_points->size != new->other_points->n_points))
		    {
		      fprintf(stderr,"surface simple incoherente label %d\n",new->label);
		      VipPrintfExit("VipOldReadSSfile");
		      return(PB);
		    }
		  ssptr->next = new;
		  ssptr= new;
		  new = NULL;
		  nlu++;
		}
	      else
		{
		  fprintf(stderr,"I do not understand %s\n",w);
		  VipPrintfExit("VipOldReadSSfile");
		  return(PB);
		}
	    }
	  else if(*w=='s')
	    {
	      nscan = sscanf(&w[1], "%d%d%d", &x, &y, &z);
	      if(nscan!=3)
		{
		  fprintf(stderr,"Strange 3D point: %s\n", w);
		  VipPrintfExit("VipOldReadSSfile");
		  return(PB);
		}
	      sspptr->x = x;
	      sspptr->y = y;
	      sspptr->z = z;
	      sspptr++;
	      new->surface_points->n_points++;
	    }
	  else if(*w=='e')
	    {
	      nscan = sscanf(&w[1], "%d%d%d", &x, &y, &z);
	      if(nscan!=3)
		{
		  fprintf(stderr,"Strange 3D point %s\n",w);
		  VipPrintfExit("VipOldReadSSfile");
		  return(PB);
		}
	      edgepptr->x = x;
	      edgepptr->y = y;
	      edgepptr->z = z;
	      edgepptr++;
	      new->edge_points->n_points++;
	    }
	  else if(*w=='o')
	    {
	      nscan = sscanf(&w[1], "%d%d%d", &x, &y, &z);
	      if(nscan!=3)
		{
		  fprintf(stderr,"Strange 3D point %s\n",w);
		  VipPrintfExit("VipOldReadSSfile");
		  return(PB);
		}
	      otherpptr->x = x;
	      otherpptr->y = y;
	      otherpptr->z = z;
	      otherpptr++;
	      new->other_points->n_points++;
	    }
	  else
	    {
	      fprintf(stderr,"Strange line in %s: %s\n",filename,w);
	      VipPrintfExit("VipOldReadSSfile");
	      return(PB);
	    }
	}
	  if ( !fgets(w, VIP_NAME_MAXLEN, fp) && !feof(fp) )
		{
	      VipPrintfExit("VipOldReadSSfile : File is corrputed");
	      return(PB);
		}

    }
  if(nlu!=sslist->n_ss)
    {
      VipPrintfError("Inconsistency in the number of simple surface read");
      VipPrintfExit("VipOldReadSSfile");
      return(PB);
    }
  sslist->first_ss = first.next;
  fclose(fp);
  return(sslist);
}

/*-------------------------------------------------------------------------*/
JunctionSet *VipReadOldJunctionSet( 
char *name,
SurfaceSimpleList *sslist)
/*-------------------------------------------------------------------------*/
{
  JunctionSet *jset;

  char filename[VIP_NAME_MAXLEN];
  FILE *fp;
  BiJunction *bijptr=NULL, first, *new;
  Vip3DPoint_S16BIT *pptr=NULL;
  char w[VIP_NAME_MAXLEN], buf1[VIP_NAME_MAXLEN];
  int nlu=0;
  int lab1, lab2, npoints;
  int nscan;
  int x, y, z;
  Vip3DBucket_S16BIT *newbuck=NULL;
  int size_filled = VFALSE;
  float size = 0.;
  int e1x, e1y, e1z, e2x, e2y, e2z;
  float mindepth, maxdepth;
  float dx, dy, dz, rdx, rdy, rdz;
  float re1x, re1y, re1z, re2x, re2y, re2z;
  int mindepth_filled, maxdepth_filled;
  int d_filled, e1_filled, e2_filled, re1_filled, re2_filled, rd_filled;
  

  if(name==NULL)
    {
      VipPrintfError("No name for Fold arg as input");
      VipPrintfExit("VipReadOldJunctionSet");
      return(PB);
    }

  strcpy(filename,name);
  strcat(filename,".bij.Vip");
	
  fp = fopen(filename,"r");
  if (!fp)
    {
      fprintf(stderr,"Can not open file %s\n",filename);
      VipPrintfExit("VipReadOldJunctionSet");
      return(PB);
    }
  printf("Reading %s topological relations...\n",name);

  jset = (JunctionSet *)VipCalloc(1,sizeof(JunctionSet),"VipReadOldJunctionSet");
  if (!jset) return(PB);

  jset->n_bi = 0;
  jset->first_bi = NULL;

  first.next = NULL;

  if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
	{
		VipPrintfExit("VipReadOldJunctionSet : Corrupted file");
    return(PB);
	}

  while(!feof(fp))
    {
      if (*w != '#')
	{
	  if(*w == 'N')
	    {
	      if(jset->n_bi!=0)
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldJunctionSet");
		  return(PB);
		}
	      sscanf(&w[1], "%d", &(jset->n_bi));
	      bijptr = &first;
	      new = NULL;
	      nlu = 0;
	    }
	  else if ((*w=='$')&&(jset->n_bi!=0))
	    {
	      if(*(w+1)=='i')
		{
		  if(*(w+6)=='1') sscanf(&w[1], "%s%d", buf1, &lab1);
		  else if (*(w+6)=='2') sscanf(&w[1], "%s%d", buf1, &lab2);
		  else
		    {
		      fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		      VipPrintfExit("VipReadOldJunctionSet");
		      return(PB);
		    }
		}
	      else if(*(w+1)=='n') sscanf(&w[1], "%s%d", buf1, &npoints);
	      else if(*(w+1)=='s') sscanf(&w[1], "%s%d%f",buf1, &size_filled,
					  &size);
	      else if(*(w+1)=='m' && *(w+2)=='i') sscanf(&w[1], "%s%d%f",buf1, &mindepth_filled,
					  &mindepth);
	      else if(*(w+1)=='m' && *(w+2)=='a') sscanf(&w[1], "%s%d%f",buf1, &maxdepth_filled,
					  &maxdepth);
	      else if(*(w+1)=='d') sscanf(&w[1], "%s%d%f%f%f",buf1, &d_filled,
					  &dx, &dy, &dz);
	      else if(*(w+1)=='r'&& *(w+4)=='d') sscanf(&w[1], "%s%d%f%f%f",buf1, &rd_filled,
					  &rdx, &rdy, &rdz);
	      else if(*(w+1)=='e' && *(w+10)=='1') sscanf(&w[1], "%s%d%d%d%d",buf1, &e1_filled,
					  &e1x, &e1y, &e1z);
	      else if(*(w+1)=='e' && *(w+10)=='2') sscanf(&w[1], "%s%d%d%d%d",buf1, &e2_filled,
					  &e2x, &e2y, &e2z);
	      else if(*(w+1)=='r' && *(w+13)=='1') sscanf(&w[1], "%s%d%f%f%f",buf1, &re1_filled,
					  &re1x, &re1y, &re1z);
	      else if(*(w+1)=='r' && *(w+13)=='2') sscanf(&w[1], "%s%d%f%f%f",buf1, &re2_filled,
					  &re2x, &re2y, &re2z);
	      else
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldJunctionSet");
		  return(PB);
		}
	    }
	  else if (*w=='*')
	    {
	      if(*(w+1)=='b')
		{
		  newbuck = VipAlloc3DBucket_S16BIT(npoints);
		  if(newbuck==NULL) return(PB);
		  pptr = newbuck->data;
		}
	      else if (*(w+1)=='e')
		{
		  if(newbuck->size != newbuck->n_points)
		    {
		      fprintf(stderr,"strange junction index %d %d\n",lab1, lab2);
		      VipPrintfExit("VipReadOldJunctionSet");
		      return(PB);
		    }
		  new = CreateBiJunction(lab1, lab2, newbuck, sslist);
		  bijptr->next = new;
		  bijptr= new;
		  bijptr->size_filled = size_filled;
		  bijptr->size = size;
		  bijptr->mindepth_filled = mindepth_filled;
		  bijptr->mindepth = mindepth;
		  bijptr->maxdepth_filled = maxdepth_filled;
		  bijptr->maxdepth = maxdepth;
		  bijptr->direction_filled = d_filled;
		  bijptr->direction.x = dx;
		  bijptr->direction.y = dy;
		  bijptr->direction.z = dz;
		  bijptr->refdirection_filled = rd_filled;
		  bijptr->refdirection.x = rdx;
		  bijptr->refdirection.y = rdy;
		  bijptr->refdirection.z = rdz;
		  bijptr->extremity1_filled = e1_filled;
		  bijptr->extremity1.x = e1x;
		  bijptr->extremity1.y = e1y;
		  bijptr->extremity1.z = e1z;
		  bijptr->extremity2_filled = e2_filled;
		  bijptr->extremity2.x = e2x;
		  bijptr->extremity2.y = e2y;
		  bijptr->extremity2.z = e2z;
		  bijptr->refextremity1_filled = re1_filled;
		  bijptr->refextremity1.x = re1x;
		  bijptr->refextremity1.y = re1y;
		  bijptr->refextremity1.z = re1z;
		  bijptr->refextremity2_filled = re2_filled;
		  bijptr->refextremity2.x = re2x;
		  bijptr->refextremity2.y = re2y;
		  bijptr->refextremity2.z = re2z;
		  size = 0.;
		  size_filled = VFALSE;
		  mindepth_filled=VFALSE;
		  maxdepth_filled=VFALSE;
		  d_filled=VFALSE;
		  rd_filled = VFALSE;
		  e1_filled=VFALSE;
		  e2_filled=VFALSE;
		  re1_filled=VFALSE;
		  re2_filled=VFALSE;
		  new = NULL;
		  nlu++;
		}
	      else
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldJunctionSet");
		  return(PB);
		}
	    }
	  else if (*w=='j')
	    {
	      nscan = sscanf(&w[1], "%d%d%d", &x, &y, &z);
	      if(nscan!=3)
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldJunctionSet");
		  return(PB);
		}
	      pptr->x = x;
	      pptr->y = y;
	      pptr->z = z;
	      pptr++;
	      newbuck->n_points++;
	    }
	  else 
	    {
	      fprintf(stderr,"Strange line in %s: %s\n",filename,w);
	      VipPrintfExit("VipReadOldJunctionSet");
	      return(PB);
	    }				
	}
	  if (!fgets(w, VIP_NAME_MAXLEN, fp) && !feof(fp) )
		{
			VipPrintfExit("VipReadOldJunctionSet : Corrupted file");
	    return(PB);
		}
    }
  if(nlu!=jset->n_bi)
    {
      VipPrintfError("Inconsistent junction number during reading");
      VipPrintfExit("VipReadOldJunctionSet");
      return(PB);
    }
  jset->first_bi = first.next;

  fclose(fp);
  return(jset);
}

/*-------------------------------------------------------------------------*/
CortexRelationSet *VipReadOldCortexRelationSet( 
char *name,
SurfaceSimpleList *sslist)
/*-------------------------------------------------------------------------*/
{
  CortexRelationSet *jset;

  char filename[VIP_NAME_MAXLEN];
  FILE *fp;
  CortexBiRelation *bijptr=NULL, first, *new;
  Vip3DPoint_S16BIT *pptr=NULL;
  int SS1nearestx, SS2nearestx;
  int SS1nearesty, SS2nearesty;
  int SS1nearestz, SS2nearestz;
  int SS1_filled=VFALSE, SS2_filled=VFALSE;
  float refSS1nearestx, refSS2nearestx;
  float refSS1nearesty, refSS2nearesty;
  float refSS1nearestz, refSS2nearestz;
  int refSS1_filled=VFALSE, refSS2_filled=VFALSE;
  char w[VIP_NAME_MAXLEN], buf1[VIP_NAME_MAXLEN];
  int nlu=0;
  int lab1, lab2, npoints;
  int nscan;
  int x, y, z;
  Vip3DBucket_S16BIT *newbuck=NULL;
  int size_filled = VFALSE;
  float size = 0.;
  int dist_filled = VFALSE;
  float dist = 0.;

  if(name==NULL)
    {
      VipPrintfError("No name for Fold arg as input");
      VipPrintfExit("VipReadOldCortexRelationSet");
      return(PB);
    }

  strcpy(filename,name);
  strcat(filename,".bic.Vip");
	
  fp = fopen(filename,"r");
  if (!fp)
    {
      fprintf(stderr,"Can not open file %s\n",filename);
      VipPrintfExit("VipReadOldCortexRelationSet");
      return(PB);
    }
  printf("Reading %s cortical relations...\n",name);

  jset = (CortexRelationSet *)VipCalloc(1,sizeof(CortexRelationSet),"VipReadOldCortexRelationSet");
  if (!jset) return(PB);

  jset->n_bi = 0;
  jset->first_bi = NULL;

  first.next = NULL;

  if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
	{
		VipPrintfExit("VipReadOldCortexRelationSet : Corrupted file");
    return(PB);
	}

  while(!feof(fp))
    {
      if (*w != '#')
	{
	  if(*w == 'N')
	    {
	      if(jset->n_bi!=0)
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldCortexRelationSet");
		  return(PB);
		}
	      sscanf(&w[1], "%d", &(jset->n_bi));
	      bijptr = &first;
	      new = NULL;
	      nlu = 0;
	    }
	  else if ((*w=='$')&&(jset->n_bi!=0))
	    {
	      if(*(w+1)=='i')
		{
		  if(*(w+6)=='1') sscanf(&w[1], "%s%d", buf1, &lab1);
		  else if (*(w+6)=='2') sscanf(&w[1], "%s%d", buf1, &lab2);
		  else
		    {
		      fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		      VipPrintfExit("VipReadOldCortexRelationSet");
		      return(PB);
		    }
		}
	      else if(*(w+1)=='n') sscanf(&w[1], "%s%d", buf1, &npoints);
	      else if(*(w+1)=='s') sscanf(&w[1], "%s%d%f",buf1, &size_filled,
					  &size);
	      else if(*(w+1)=='d') sscanf(&w[1], "%s%d%f",buf1, &dist_filled,
					  &dist);
	      else if(strstr(w,"$SS1nearest") )
		      sscanf(&w[1], "%s%d%d%d%d",buf1, &SS1_filled,&(SS1nearestx),
			     &(SS1nearesty),&(SS1nearestz));
	      else if(strstr(w+1,"refSS1nearest") )
		      sscanf(&w[1], "%s%d%f%f%f",buf1, &refSS1_filled,&(refSS1nearestx),
			     &(refSS1nearesty),&(refSS1nearestz));
	      else if(strstr(w,"$SS2nearest") )
		      sscanf(&w[1], "%s%d%d%d%d",buf1, &SS2_filled,&(SS2nearestx),
			     &(SS2nearesty),&(SS2nearestz));
	      else if(strstr(w+1,"refSS2nearest") )
		      sscanf(&w[1], "%s%d%f%f%f",buf1, &refSS2_filled,&(refSS2nearestx),
			     &(refSS2nearesty),&(refSS2nearestz));
	      else
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldCortexRelationSet");
		  return(PB);
		}
	    }
	  else if (*w=='*')
	    {
	      if(*(w+1)=='b')
		{
		  newbuck = VipAlloc3DBucket_S16BIT(npoints);
		  if(newbuck==NULL) return(PB);
		  pptr = newbuck->data;
		}
	      else if (*(w+1)=='e')
		{
		  if(newbuck->size != newbuck->n_points)
		    {
		      fprintf(stderr,"strange junction index %d %d\n",lab1, lab2);
		      VipPrintfExit("VipReadOldCortexRelationSet");
		      return(PB);
		    }
		  new = CreateCortexBiRelation(lab1, lab2, newbuck, sslist);
		  bijptr->next = new;
		  bijptr= new;
		  bijptr->size_filled = size_filled;
		  bijptr->size = size;
		  bijptr->hulljunctiondist_filled = dist_filled;
		  bijptr->hulljunctiondist = dist;
		  bijptr->SS1nearest_filled= SS1_filled;
		  bijptr->SS1nearest.x = SS1nearestx;
		  bijptr->SS1nearest.y = SS1nearesty;
		  bijptr->SS1nearest.z = SS1nearestz;
		  bijptr->SS2nearest_filled= SS2_filled;
		  bijptr->SS2nearest.x = SS2nearestx;
		  bijptr->SS2nearest.y = SS2nearesty;
		  bijptr->SS2nearest.z = SS2nearestz;
		  bijptr->refSS1nearest_filled= refSS1_filled;
		  bijptr->refSS1nearest.x = refSS1nearestx;
		  bijptr->refSS1nearest.y = refSS1nearesty;
		  bijptr->refSS1nearest.z = refSS1nearestz;
		  bijptr->refSS2nearest_filled= refSS2_filled;
		  bijptr->refSS2nearest.x = refSS2nearestx;
		  bijptr->refSS2nearest.y = refSS2nearesty;
		  bijptr->refSS2nearest.z = refSS2nearestz;
		  size = 0.;
		  size_filled = VFALSE;
		  dist_filled = VFALSE;
		  SS1_filled = VFALSE;
		  SS2_filled = VFALSE;
		  refSS1_filled = VFALSE;
		  refSS2_filled = VFALSE;
		  new = NULL;
		  nlu++;
		}
	      else
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldCortexRelationSet");
		  return(PB);
		}
	    }
	  else if (*w=='j')
	    {
	      nscan = sscanf(&w[1], "%d%d%d", &x, &y, &z);
	      if(nscan!=3)
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldCortexRelationSet");
		  return(PB);
		}
	      pptr->x = x;
	      pptr->y = y;
	      pptr->z = z;
	      pptr++;
	      newbuck->n_points++;
	    }
	  else 
	    {
	      fprintf(stderr,"Strange line in %s: %s\n",filename,w);
	      VipPrintfExit("VipReadOldCortexRelationSet");
	      return(PB);
	    }				
	}
	  if ( !fgets(w, VIP_NAME_MAXLEN, fp) && !feof(fp) )
		{
			VipPrintfExit("VipReadOldCortexRelationSet : Corrupted file");
	    return(PB);
		}
    }
  if(nlu!=jset->n_bi)
    {
      VipPrintfError("Inconsistent junction number during reading");
      VipPrintfExit("VipReadOldCortexRelationSet");
      return(PB);
    }
  jset->first_bi = first.next;

  fclose(fp);
  return(jset);
}

/*-------------------------------------------------------------------------*/
PliDePassageSet *VipReadOldPliDePassageSet( 
char *name,
SurfaceSimpleList *sslist)
/*-------------------------------------------------------------------------*/
{
  PliDePassageSet *ppset;

  char filename[VIP_NAME_MAXLEN];
  FILE *fp;
  PliDePassage *ppptr=NULL, first, *new;
  Vip3DPoint_S16BIT *pptr=NULL;
  char w[VIP_NAME_MAXLEN], buf1[VIP_NAME_MAXLEN];
  int nlu=0;
  int lab1, lab2, npoints;
  int nscan;
  int x, y, z;
  Vip3DBucket_S16BIT *newbuck=NULL;
  int size_filled = VFALSE;
  float size = 0.;
  int depth_filled = VFALSE;
  float depth = 0.;
  Vip3DPoint_VFLOAT reflocation;
  int location_filled = VFALSE, reflocation_filled=VFALSE;
  int locationx, locationy, locationz;;

  if(name==NULL)
    {
      VipPrintfError("No name for Fold arg as input");
      VipPrintfExit("VipReadOldPliDePassageSet");
      return(PB);
    }

  strcpy(filename,name);
  strcat(filename,".pp.Vip");
	
  fp = fopen(filename,"r");
  if (!fp)
    {
      fprintf(stderr,"Can not open file %s\n",filename);
      VipPrintfExit("VipReadOldPliDePassageSet");
      return(PB);
    }
  printf("Reading %s plis de passage...\n",name);

  ppset = (PliDePassageSet *)VipCalloc(1,sizeof(PliDePassageSet),"VipReadOldPliDePassageSet");
  if (!ppset) return(PB);

  ppset->n_pp = 0;
  ppset->first_pp = NULL;

  first.next = NULL;

  if ( !fgets(w, VIP_NAME_MAXLEN, fp) )
	{
	  VipPrintfExit("VipReadOldPliDePassageSet : Corrupted file");
    return(PB);
	}

  while(!feof(fp))
    {
      if (*w != '#')
	{
	  if(*w == 'N')
	    {
	      if(ppset->n_pp!=0)
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldPliDePassageSet");
		  return(PB);
		}
	      sscanf(&w[1], "%d", &(ppset->n_pp));
	      ppptr = &first;
	      new = NULL;
	      nlu = 0;
	    }
	  else if ((*w=='$')&&(ppset->n_pp!=0))
	    {
	      if(*(w+1)=='i')
		{
		  if(*(w+6)=='1') sscanf(&w[1], "%s%d", buf1, &lab1);
		  else if (*(w+6)=='2') sscanf(&w[1], "%s%d", buf1, &lab2);
		  else
		    {
		      fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		      VipPrintfExit("VipReadOldPliDePassageSet");
		      return(PB);
		    }
		}
	      else if(*(w+1)=='n') sscanf(&w[1], "%s%d", buf1, &npoints);
	      else if(*(w+1)=='s') sscanf(&w[1], "%s%d%f",buf1, &size_filled,
					  &size);
	      else if(*(w+1)=='d') sscanf(&w[1], "%s%d%f",buf1, &depth_filled,
					  &depth);
	      else if(*(w+1)=='l') sscanf(&w[1], "%s%d%d%d%d",buf1, &location_filled,
					  &(locationx),&(locationy),&(locationz));
	      else if(*(w+1)=='r') sscanf(&w[1], "%s%d%f%f%f",buf1, &reflocation_filled,
					  &(reflocation.x),&(reflocation.y),&(reflocation.z));
	      else
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldPliDePassageSet");
		  return(PB);
		}
	    }
	  else if (*w=='*')
	    {
	      if(*(w+1)=='b')
		{
		  newbuck = VipAlloc3DBucket_S16BIT(npoints);
		  if(newbuck==NULL) return(PB);
		  pptr = newbuck->data;
		}
	      else if (*(w+1)=='e')
		{
		  if(newbuck->size != newbuck->n_points)
		    {
		      fprintf(stderr,"strange junction index %d %d\n",lab1, lab2);
		      VipPrintfExit("VipReadOldPliDePassageSet");
		      return(PB);
		    }
		  new = CreatePliDePassage(lab1, lab2, newbuck, sslist);
		  ppptr->next = new;
		  ppptr= new;
		  ppptr->size_filled = size_filled;
		  ppptr->size = size;
		  ppptr->depth_filled = depth_filled;
		  ppptr->depth = depth;
		  ppptr->location_filled = location_filled;
		  ppptr->location.x = locationx;
		  ppptr->location.y = locationy;
		  ppptr->location.z = locationz;
		  ppptr->reflocation.x = reflocation.x;
		  ppptr->reflocation.y = reflocation.y;
		  ppptr->reflocation.z = reflocation.z;
		  size = 0.;
		  size_filled = VFALSE;
		  depth_filled = VFALSE;
		  location_filled = VFALSE;
		  reflocation_filled = VFALSE;
		  new = NULL;
		  nlu++;
		}
	      else
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldPliDePassageSet");
		  return(PB);
		}
	    }
	  else if (*w=='j')
	    {
	      nscan = sscanf(&w[1], "%d%d%d", &x, &y, &z);
	      if(nscan!=3)
		{
		  fprintf(stderr,"Strange line in %s: %s\n",filename,w);
		  VipPrintfExit("VipReadOldPliDePassageSet");
		  return(PB);
		}
	      pptr->x = x;
	      pptr->y = y;
	      pptr->z = z;
	      pptr++;
	      newbuck->n_points++;
	    }
	  else 
	    {
	      fprintf(stderr,"Strange line in %s: %s\n",filename,w);
	      VipPrintfExit("VipReadOldPliDePassageSet");
	      return(PB);
	    }				
	}
	  if (!fgets(w, VIP_NAME_MAXLEN, fp) && !feof(fp) )
		{
			VipPrintfExit("VipReadOldPliDePassageSet : Corrupted file");
	    return(PB);
		}
    }
  if(nlu!=ppset->n_pp)
    {
      VipPrintfError("Inconsistent junction number during reading");
      VipPrintfExit("VipReadOldPliDePassageSet");
      return(PB);
    }
  ppset->first_pp = first.next;

  fclose(fp);
  return(ppset);
}

/*-------------------------------------------------------------------------*/
int VipWriteOldFoldArg(FoldArg *arg ,char *name)
/*-------------------------------------------------------------------------*/

{

  if(arg==NULL)
    {
      VipPrintfError("No Fold arg as input");
      VipPrintfExit("VipWriteOldFoldArg");
      return(PB);
    }
  if(name==NULL)
    {
      VipPrintfError("No name for Fold arg as input");
      VipPrintfExit("VipWriteOldFoldArg");
      return(PB);
    }

  if(arg->ss_list!=NULL) if(VipWriteOldSSfile(arg->ss_list,name)==PB) return(PB);
  if(arg->jset!=NULL) if(VipWriteOldJunctionSet(arg->jset,name)==PB) return(PB);
  if(arg->ppset!=NULL) if(VipWriteOldPliDePassageSet(arg->ppset,name)==PB) return(PB);
  if(arg->cortex_jset!=NULL) if(VipWriteOldCortexRelationSet(arg->cortex_jset,name)==PB) return(PB);
  if(VipWriteOldARGtoRef(arg, name)==PB) return(PB);

  return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteGenericSSfile(
		    SurfaceSimpleList *sslist,
		    char *name,
		    char * shortname,
		    Vip3DPoint_VFLOAT voxsize)
/*-------------------------------------------------------------------------*/
{
    char filename[VIP_NAME_MAXLEN], buckfilename[VIP_NAME_MAXLEN];
    char dstfilename[VIP_NAME_MAXLEN];
    FILE *f, *buckf, *testf;
    SurfaceSimple *ssptr;
    Vip3DPoint_S16BIT *pptr;
    int i;
    int count;
    char sep;

    sep = VipFileSeparator();

    if (!sslist || !name)
	{
	    VipPrintfError("Null arg");
	    VipPrintfExit("VipWriteGenericSSfile");
	    return(PB);
	}

    strcpy(filename,name);
    strcat(filename,".arg");
	
     f = fopen(filename,"a");
    if (!f)
	{
	    fprintf(stderr,"Can not open file %s\n",filename);
	    VipPrintfExit("VipWriteGenericSSfile");
	    return(PB);
	}

    printf("Writing %s simple surfaces...\n",shortname);
	
    ssptr = sslist->first_ss;
    count = 1;

    fprintf(f,"*BEGIN NODE hull %d\n",10000); /*The generic library does not accept 0 index*/    
    fprintf(f,"name brain_hull\n");
    fprintf(f,"# external hull of the brain used to compute some Voronoi diagrams\n");
    fprintf(f,"# this object is implied in hull_junctions\n");
    fprintf(f,"index %d\n", HULL_SURFACE_INDEX);
    fprintf(f,"*END\n\n");

    
    while(ssptr!=NULL)
	{

	    fprintf(f,"*BEGIN NODE fold %d\n",ssptr->index);
	    fprintf(f,"index %d\n", ssptr->index);
	    fprintf(f,"skeleton_label %d\n", ssptr->label);
	    fprintf(f,"name %s\n", ssptr->the_name);

	    if(ssptr->box_filled==VTRUE)
		{
		    fprintf(f, "boundingbox_min ");
		    fprintf(f, "%d %d %d\n", ssptr->boxmin.x, ssptr->boxmin.y, ssptr->boxmin.z);
		    fprintf(f, "boundingbox_max ");
		    fprintf(f, "%d %d %d\n", ssptr->boxmax.x, ssptr->boxmax.y, ssptr->boxmax.z);
		}
	    if(ssptr->refbox_filled==VTRUE)
		{
		    fprintf(f, "Tal_boundingbox_min ");
		    fprintf(f, "%f %f %f\n", ssptr->refboxmin.x, ssptr->refboxmin.y, ssptr->refboxmin.z);
		    fprintf(f, "Tal_boundingbox_max ");
		    fprintf(f, "%f %f %f\n", ssptr->refboxmax.x, ssptr->refboxmax.y, ssptr->refboxmax.z);
		}	    	
	    if(ssptr->size_filled==VTRUE)
		{
		    fprintf(f,"size %.3f\n",ssptr->size);
		}
	    else
		{
		    fprintf(f,"size %.3f\n",ssptr->n_points*voxsize.x
			    *voxsize.y*voxsize.z);
		}
	    if(ssptr->normale_filled==VTRUE)
		{
		    fprintf(f,"normal %.3f %.3f %.3f\n",
			    ssptr->normale.x,
			    ssptr->normale.y, 
			    ssptr->normale.z);
		}
	    if(ssptr->refnormale_filled==VTRUE)
		{
		    fprintf(f,"refnormal %.3f %.3f %.3f\n",
			    ssptr->refnormale.x,
			    ssptr->refnormale.y, 
			    ssptr->refnormale.z);
		}
	    if(ssptr->g_filled==VTRUE)
		{
		    fprintf(f,"gravity_center %.3f %.3f %.3f\n",
			    ssptr->g.x,
			    ssptr->g.y, ssptr->g.z);
		}
	    if(ssptr->refg_filled==VTRUE)
		{
		    fprintf(f,"refgravity_center %.3f %.3f %.3f\n",
			    ssptr->refg.x,
			    ssptr->refg.y, ssptr->refg.z);
		}
	    if(ssptr->depth_filled==VTRUE)
		{
		    fprintf(f,"maxdepth %.3f\n",
			    ssptr->depth);
		}
	    if(ssptr->mindepth_filled==VTRUE)
		{
		    fprintf(f,"mindepth %.3f\n",
			    ssptr->mindepth);
		}
	    if(ssptr->rootsbassin_filled==VTRUE)
		{
		    fprintf(f,"rootsbassin %d\n",
			    ssptr->rootsbassin);
		}
	    if(ssptr->talcovar_filled ==VTRUE)
		{
		    fprintf(f,"talcovar %f %f %f %f %f %f %f %f %f\n",
			    ssptr->talcovar[0][0], ssptr->talcovar[0][1], ssptr->talcovar[0][2],
			    ssptr->talcovar[1][0], ssptr->talcovar[1][1], ssptr->talcovar[1][2],
			    ssptr->talcovar[2][0], ssptr->talcovar[2][1], ssptr->talcovar[2][2]);
		}
	    fprintf(f,"point_number %d\n",ssptr->n_points);

	    fprintf(f,"ss_point_number %d\n",ssptr->surface_points->n_points);

	    fprintf(f,"ss_filename ss/%s-ss%d.bck\n",shortname,ssptr->index);

	    /*Je sais, c'est pas beau et c'est idiot, mais ca me demande trop d'energie
de le faire bien en ce moment. Je voudrai que le pipeline soit operationnel. On risque
de jamais reprendre ce genre de chose, mais il est probable que si on reprend
qqchose, ce soit l'ensemble de la construction du graphe en Vip, ca serait vraiment
sain que le format intermediaire Vip disparaisse*/
	    sprintf( buckfilename, "%s.data/Tmtk/*tmt%d.mesh", name, 
                     ssptr->index );
            sprintf( dstfilename, "%s.data/Tmtk/%s-tmt%d.mesh", name, 
                     shortname, ssptr->index );
            VipMv( buckfilename, dstfilename );

            sprintf( buckfilename, "%s.data/Tmtk/*tmt%d.gii", name,
                     ssptr->index );
            sprintf( dstfilename, "%s.data/Tmtk/%s-tmt%d.gii", name,
                     shortname, ssptr->index );
            VipMv( buckfilename, dstfilename );

            sprintf( buckfilename, "%s.data/Tmtk/*tmt%d.tri", name,
                     ssptr->index );
            sprintf( dstfilename, "%s.data/Tmtk/%s-tmt%d.tri", name,
                     shortname, ssptr->index );
            VipMv( buckfilename, dstfilename );

	    sprintf( buckfilename, "%s.data/Tmtk/%s-tmt%d.tri", name, 
                     shortname, ssptr->index );
	    testf = fopen(buckfilename,"r");
	    if(testf)
            {
              fclose(testf);
              sprintf(buckfilename,"Tmtk/%s-tmt%d.tri",shortname,ssptr->index);
              fprintf(f,"Tmtktri_filename %s\n",buckfilename);
            }
	    else
            {
              sprintf( buckfilename, "%s.data/Tmtk/%s-tmt%d.gii", name,
                        shortname, ssptr->index );
              testf = fopen(buckfilename,"r");
              if(testf)
              {
                fclose(testf);
                sprintf(buckfilename,"Tmtk/%s-tmt%d.gii",shortname,
                        ssptr->index);
                fprintf(f,"Tmtktri_filename %s\n",buckfilename);
              }
              else
              {
                sprintf(buckfilename,"%s.data/Tmtk/%s-tmt%d.mesh",name,
                        shortname,ssptr->index);
                testf = fopen(buckfilename,"r");
                if(testf)
                {
                  fclose(testf);
                  sprintf(buckfilename,"Tmtk/%s-tmt%d.mesh",shortname,
                          ssptr->index);
                  fprintf(f,"Tmtktri_filename %s\n",buckfilename);
                }
              }
            }

	    sprintf(buckfilename,"%s.data%css%c%s-ss%d.bck",name,sep,sep,
                    shortname,ssptr->index);
	    fflush(stdout);
	    buckf = fopen(buckfilename,"w");
	    if (!buckf)
		{
		    fprintf(stderr,"Can not open file %s\n",buckfilename);
		    VipPrintfExit("VipWriteGenericSSfile");
		    return(PB);
		}
	    fprintf(buckf,"ascii\n");
	    fprintf(buckf,"-type VOID\n");
	    fprintf(buckf,"-dx %f\n",voxsize.x);
	    fprintf(buckf,"-dy %f\n",voxsize.y);
	    fprintf(buckf,"-dz %f\n",voxsize.z);
	    fprintf(buckf,"-dt 1.\n");
	    fprintf(buckf,"-dimt 1\n");
	    fprintf(buckf,"-time 0\n");
	    fprintf(buckf,"-dim %d\n",ssptr->surface_points->n_points);
	    pptr = ssptr->surface_points->data;
	    for(i=ssptr->surface_points->n_points;i--;)
		{
		    fprintf(buckf,"(%d,%d,%d)\n",pptr->x,pptr->y,pptr->z);
		    pptr++;
		}
	    fclose(buckf);

	    fprintf(f,"bottom_point_number %d\n",ssptr->edge_points->n_points);

	    fprintf(f,"bottom_filename bottom/%s-bot%d.bck\n",shortname,ssptr->index);

	    sprintf(buckfilename,"%s.data%cbottom%c%s-bot%d.bck",name,sep,sep,shortname,ssptr->index);
	
	    buckf = fopen(buckfilename,"w");
	    if (!buckf)
		{
		    fprintf(stderr,"Can not open file %s\n",buckfilename);
		    VipPrintfExit("VipWriteGenericSSfile");
		    return(PB);
		}
	    fprintf(buckf,"ascii\n");
	    fprintf(buckf,"-type VOID\n");
	    fprintf(buckf,"-dx %f\n",voxsize.x);
	    fprintf(buckf,"-dy %f\n",voxsize.y);
	    fprintf(buckf,"-dz %f\n",voxsize.z);
	    fprintf(buckf,"-dt 1.\n");
	    fprintf(buckf,"-dimt 1\n");
	    fprintf(buckf,"-time 0\n");
	    fprintf(buckf,"-dim %d\n",ssptr->edge_points->n_points);
	    pptr = ssptr->edge_points->data;
	    for(i=ssptr->edge_points->n_points;i--;)
		{
		    fprintf(buckf,"(%d,%d,%d)\n",pptr->x,pptr->y,pptr->z);
		    pptr++;
		}
	    fclose(buckf);

	    fprintf(f,"other_point_number %d\n",ssptr->other_points->n_points);

	    fprintf(f,"other_filename other/%s-o%d.bck\n",shortname,ssptr->index);

	    sprintf(buckfilename,"%s.data%cother%c%s-o%d.bck",name,sep,sep,shortname,ssptr->index);
	
	    buckf = fopen(buckfilename,"w");
	    if (!buckf)
		{
		    fprintf(stderr,"Can not open file %s\n",buckfilename);
		    VipPrintfExit("VipWriteGenericSSfile");
		    return(PB);
		}
	    fprintf(buckf,"ascii\n");
	    fprintf(buckf,"-type VOID\n");
	    fprintf(buckf,"-dx %f\n",voxsize.x);
	    fprintf(buckf,"-dy %f\n",voxsize.y);
	    fprintf(buckf,"-dz %f\n",voxsize.z);
	    fprintf(buckf,"-dt 1.\n");
	    fprintf(buckf,"-dimt 1\n");
	    fprintf(buckf,"-time 0\n");
	    fprintf(buckf,"-dim %d\n",ssptr->other_points->n_points);
	    pptr = ssptr->other_points->data;
	    for(i=ssptr->other_points->n_points;i--;)
		{
		    fprintf(buckf,"(%d,%d,%d)\n",pptr->x,pptr->y,pptr->z);
		    pptr++;
		}
	    fclose(buckf);

	    fprintf(f,"*END\n\n");
	    ssptr = ssptr->next;

	    fclose(f);
	    f = fopen(filename,"a");
	    if (!f)
	      {
		fprintf(stderr,"Can not open file %s\n",filename);
		VipPrintfExit("VipWriteGenericSSfile");
		return(PB);
	      }
	}
    fclose(f);

    return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteOldSSfile(
		    SurfaceSimpleList *sslist,
		    char *name)
/*-------------------------------------------------------------------------*/
{
  char filename[1024];
  FILE *f;
  SurfaceSimple *ssptr;
  Vip3DPoint_S16BIT *pptr;
  int i;

  if (!sslist || !name)
    {
      VipPrintfError("Null arg");
      VipPrintfExit("VipWriteOldSSfile");
      return(PB);
    }

  strcpy(filename,name);
  strcat(filename,".ss.Vip");
	
  f = fopen(filename,"w");
  if (!f)
    {
      fprintf(stderr,"Can not open file %s\n",filename);
      VipPrintfExit("VipWriteOldSSfile");
      return(PB);
    }

  printf("Writing %s simple surfaces...\n",name);
	
  fprintf(f,"N %d\n",sslist->n_ss);

  ssptr = sslist->first_ss;

  while(ssptr!=NULL)
    {
      fprintf(f,"$index %d\n",ssptr->index);

      fprintf(f,"$lab %d\n",ssptr->label);

      fprintf(f,"$identification %s\n", ssptr->the_name);

      fprintf(f,"$box %d x: %d %d y: %d %d z: %d %d\n",
	      ssptr->box_filled,
	      ssptr->boxmin.x, ssptr->boxmax.x,
	      ssptr->boxmin.y, ssptr->boxmax.y,
	      ssptr->boxmin.z, ssptr->boxmax.z);		

      fprintf(f,"$refbox %d x: %.3f %.3f y: %.3f %.3f z: %.3f %.3f\n",
	      ssptr->refbox_filled,
	      ssptr->refboxmin.x, ssptr->refboxmax.x,
	      ssptr->refboxmin.y, ssptr->refboxmax.y,
	      ssptr->refboxmin.z, ssptr->refboxmax.z);													
      fprintf(f,"$size %d %.3f\n",ssptr->size_filled,ssptr->size);
      fprintf(f,"$rootsbassin %d %d\n",ssptr->rootsbassin_filled,ssptr->rootsbassin);
      fprintf(f,"$normale: %d %.3f %.3f %.3f\n",ssptr->normale_filled,
	      ssptr->normale.x,
	      ssptr->normale.y, 
	      ssptr->normale.z);
      fprintf(f,"$refnormale: %d %.3f %.3f %.3f\n",ssptr->refnormale_filled,
	      ssptr->refnormale.x,
	      ssptr->refnormale.y, 
	      ssptr->refnormale.z);
      fprintf(f,"$g: %d %.3f %.3f %.3f\n",ssptr->g_filled,ssptr->g.x,
	      ssptr->g.y, ssptr->g.z);
      fprintf(f,"$refg: %d %.3f %.3f %.3f\n",ssptr->refg_filled,ssptr->refg.x,
	      ssptr->refg.y, ssptr->refg.z);
      fprintf(f,"$depth %d %.3f\n",ssptr->depth_filled,
	      ssptr->depth);
      fprintf(f,"$mindepth %d %.3f\n",ssptr->mindepth_filled,
	      ssptr->mindepth);
      fprintf(f,"$talcovar %d %f %f %f %f %f %f %f %f %f\n",ssptr->talcovar_filled,
	      ssptr->talcovar[0][0],
	      ssptr->talcovar[0][1],
	      ssptr->talcovar[0][2],
	      ssptr->talcovar[1][0],
	      ssptr->talcovar[1][1],
	      ssptr->talcovar[1][2],
	      ssptr->talcovar[2][0],
	      ssptr->talcovar[2][1],
	      ssptr->talcovar[2][2]);
      fprintf(f,"$npoints %d\n",ssptr->n_points);
      fprintf(f,"$sspoints %d\n",ssptr->surface_points->n_points);
      fprintf(f,"$edgepoints %d\n",ssptr->edge_points->n_points);
      fprintf(f,"$otherpoints %d\n",ssptr->other_points->n_points);
      fprintf(f,"*begin\n");
      pptr = ssptr->surface_points->data;
      for(i=ssptr->surface_points->n_points;i--;)
	{
	  fprintf(f,"s %d %d %d\n",pptr->x,pptr->y,pptr->z);
	  pptr++;
	}
      pptr = ssptr->edge_points->data;
      for(i=ssptr->edge_points->n_points;i--;)
	{
	  fprintf(f,"e %d %d %d\n",pptr->x,pptr->y,pptr->z);
	  pptr++;
	}
      pptr = ssptr->other_points->data;
      for(i=ssptr->other_points->n_points;i--;)
	{
	  fprintf(f,"o %d %d %d\n",pptr->x,pptr->y,pptr->z);
	  pptr++;
	}
      fprintf(f,"*end\n");
      ssptr = ssptr->next;
    }
  fclose(f);

  return(OK);
}


/*-------------------------------------------------------------------------*/
int VipWriteGenericJunctionSet(
		      JunctionSet *jset,
		      char *name,
		      char * shortname,
		      Vip3DPoint_VFLOAT voxsize)
/*-------------------------------------------------------------------------*/
{
    char filename[VIP_NAME_MAXLEN];
    char buckfilename[VIP_NAME_MAXLEN];
    FILE *f, *buckf;
    BiJunction *bijptr;
    Vip3DPoint_S16BIT *pptr;
    int i;
    int hull;

    if (!jset || !name)
	{
	    VipPrintfError("Null arg");
	    VipPrintfExit("VipWriteGenericJunctionSet");
	    return(PB);
	}

    if(jset->first_bi==NULL) return(OK);
	
    strcpy(filename,name);
    strcat(filename,".arg");
	
    f = fopen(filename,"a");
    if (!f)
	{
	    fprintf(stderr,"Can not open file %s\n",filename);
	    VipPrintfExit("VipWriteGenericJunctionSet");
	    return(PB);
	}

    printf("Writing %s topological relations...\n",shortname);

    bijptr = jset->first_bi;
    while(bijptr!=NULL)
	{
	    /*The generic library does not accept 0 index*/
	    if(bijptr->index1==HULL_SURFACE_INDEX)
		{
		    fprintf(f,"*BEGIN UBEDGE hull_junction %d %d\n",10000,bijptr->index2);
		    hull = VTRUE;
		}
	    else if(bijptr->index2==HULL_SURFACE_INDEX)
		{
		    fprintf(f,"*BEGIN UBEDGE hull_junction %d %d\n",bijptr->index1,10000);
		    hull = VTRUE;
		}
	    else
		{
		    fprintf(f,"*BEGIN UBEDGE junction %d %d\n",bijptr->index1,bijptr->index2);
		    hull = VFALSE;
		}
	    fprintf(f,"point_number %d\n",bijptr->points->n_points);
	    if(bijptr->size_filled==VTRUE)
		{
		    fprintf(f,"size %.3f\n",bijptr->size);
		}
	    else
		{
		    fprintf(f,"size %.3f\n",bijptr->points->n_points*voxsize.x
			    *voxsize.y*voxsize.z);
		}

	    if(bijptr->direction_filled==VTRUE)
		fprintf(f,"direction %.3f %.3f %.3f\n",bijptr->direction.x,
			bijptr->direction.y,bijptr->direction.z);
	    if(bijptr->refdirection_filled==VTRUE)
		fprintf(f,"refdirection %.3f %.3f %.3f\n",bijptr->refdirection.x,
		    bijptr->refdirection.y,bijptr->refdirection.z);
	    if(bijptr->extremity1_filled==VTRUE)
		fprintf(f,"extremity1 %d %d %d\n",bijptr->extremity1.x,
			bijptr->extremity1.y,bijptr->extremity1.z);
	    if(bijptr->extremity2_filled==VTRUE)
		fprintf(f,"extremity2 %d %d %d\n",bijptr->extremity2.x,
			bijptr->extremity2.y,bijptr->extremity2.z);
	    if(bijptr->refextremity1_filled==VTRUE)
		fprintf(f,"refextremity1 %.3f %.3f %.3f\n",bijptr->refextremity1.x,
			bijptr->refextremity1.y,bijptr->refextremity1.z);
	    if(bijptr->refextremity2_filled==VTRUE)
		fprintf(f,"refextremity2 %.3f %.3f %.3f\n",bijptr->refextremity2.x,
			bijptr->refextremity2.y,bijptr->refextremity2.z);

	    if(hull==VFALSE)
		{
		    if(bijptr->mindepth_filled==VTRUE)
			fprintf(f,"mindepth %.3f\n",bijptr->mindepth);
		    if(bijptr->maxdepth_filled==VTRUE)
			fprintf(f,"maxdepth %.3f\n",bijptr->maxdepth);
		}
	    fprintf(f,"junction_filename junction/%s-j%d-%d.bck\n",shortname,bijptr->index1,bijptr->index2);

	    sprintf(buckfilename,"%s.data/junction/%s-j%d-%d.bck",name,shortname,bijptr->index1,bijptr->index2);
	
	    buckf = fopen(buckfilename,"w");
	    if (!buckf)
		{
		    fprintf(stderr,"Can not open %s\n",buckfilename);
		    VipPrintfExit("VipWriteGenericJunctionSet");
		    return(PB);
		}
	    fprintf(buckf,"ascii\n");
	    fprintf(buckf,"-type VOID\n");
	    fprintf(buckf,"-dx %f\n",voxsize.x);
	    fprintf(buckf,"-dy %f\n",voxsize.y);
	    fprintf(buckf,"-dz %f\n",voxsize.z);
	    fprintf(buckf,"-dt 1.\n");
	    fprintf(buckf,"-dimt 1\n");
	    fprintf(buckf,"-time 0\n");
	    fprintf(buckf,"-dim %d\n",bijptr->points->n_points);
	    pptr = bijptr->points->data;
	    for(i=bijptr->points->n_points;i--;)
		{
		    fprintf(buckf,"(%d,%d,%d)\n",pptr->x,pptr->y,pptr->z);
		    pptr++;
		}
	    fclose(buckf);
	    fprintf(f,"*END\n\n");
	    bijptr = bijptr->next;
	}
    fclose(f);

    return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteOldPliDePassageSet(
		      PliDePassageSet *ppset,
		      char *name)
/*-------------------------------------------------------------------------*/
{
  char filename[VIP_NAME_MAXLEN];
  FILE *f;
  PliDePassage *ppptr;
  Vip3DPoint_S16BIT *pptr;
  int i;

  if (!ppset || !name)
    {
      VipPrintfError("Null arg");
      VipPrintfExit("VipWriteOldPliDePassageSet");
      return(PB);
    }

  if(ppset->first_pp!=NULL)
    {
      strcpy(filename,name);
      strcat(filename,".pp.Vip");
	
      f = fopen(filename,"w");
      if (!f)
	{
	  fprintf(stderr,"Can not open file %s\n",filename);
	  VipPrintfExit("VipWriteOldPliDePassageSet");
	  return(PB);
	}

      printf("Writing %s plis de passage...\n",name);
      fprintf(f,"N %d\n",ppset->n_pp);

      ppptr = ppset->first_pp;

      while(ppptr!=NULL)
	{
	  fprintf(f,"$index1 %d\n",ppptr->index1);
	  fprintf(f,"$index2 %d\n",ppptr->index2);
	  fprintf(f,"$npoints %d\n",ppptr->points->n_points);
	  fprintf(f,"$size %d %.3f\n",ppptr->size_filled,ppptr->size);
	  fprintf(f,"$depth %d %.3f\n",ppptr->depth_filled,ppptr->depth);
	  fprintf(f,"$location %d %d %d %d\n",ppptr->location_filled,ppptr->location.x,
		  ppptr->location.y,ppptr->location.z);
	  fprintf(f,"$reflocation %d %.3f %.3f %.3f\n",ppptr->reflocation_filled,ppptr->reflocation.x,
		  ppptr->reflocation.y,ppptr->reflocation.z);
	  fprintf(f,"*begin\n");
	  pptr = ppptr->points->data;
	  for(i=ppptr->points->n_points;i--;)
	    {
	      fprintf(f,"j %d %d %d\n",pptr->x,pptr->y,pptr->z);
	      pptr++;
	    }
	  fprintf(f,"*end\n");
	  ppptr = ppptr->next;
	}
      fclose(f);
    }

  return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteGenericPliDePassageSet(
		      PliDePassageSet *ppset,
		      char *name,
		      char * shortname,
		      Vip3DPoint_VFLOAT voxsize)
/*-------------------------------------------------------------------------*/
{
    char filename[VIP_NAME_MAXLEN];
    char buckfilename[VIP_NAME_MAXLEN];
    FILE *f, *buckf;
    PliDePassage *ppptr;
    Vip3DPoint_S16BIT *pptr;
    int i;

    if (!ppset || !name)
	{
	    VipPrintfError("Null arg");
	    VipPrintfExit("VipWriteGenericPliDePassageSet");
	    return(PB);
	}

    if(ppset->first_pp==NULL) return(OK);
	
    strcpy(filename,name);
    strcat(filename,".arg");
	
    f = fopen(filename,"a");
    if (!f)
	{
	    fprintf(stderr,"Can not open file %s\n",filename);
	    VipPrintfExit("VipWriteGenericPliDePassageSet");
	    return(PB);
	}

    printf("Writing %s plis de passage...\n",shortname);

    ppptr = ppset->first_pp;
    while(ppptr!=NULL)
	{
	    /*The generic library does not accept 0 index*/

	    fprintf(f,"*BEGIN UBEDGE plidepassage %d %d\n",ppptr->index1,ppptr->index2);
		    
	    fprintf(f,"point_number %d\n",ppptr->points->n_points);
	    if(ppptr->size_filled==VTRUE)
		{
		    fprintf(f,"size %.3f\n",ppptr->size);
		}
	    else
		{
		    fprintf(f,"size %.3f\n",ppptr->points->n_points*voxsize.x
			    *voxsize.y*voxsize.z);
		}
	    if(ppptr->depth_filled==VTRUE)
		{
		    fprintf(f,"depth %.3f\n",ppptr->depth);
		}
	    if(ppptr->location_filled==VTRUE)
		{
		    fprintf(f,"location %d %d %d\n",
			    ppptr->location.x,
			    ppptr->location.y, ppptr->location.z);
		}
	    if(ppptr->reflocation_filled==VTRUE)
		{
		    fprintf(f,"reflocation %.3f %.3f %.3f\n",
			    ppptr->reflocation.x,
			    ppptr->reflocation.y, ppptr->reflocation.z);
		}
	    fprintf(f,"plidepassage_filename plidepassage/%s-pp%d-%d.bck\n",shortname,ppptr->index1,ppptr->index2);

	    sprintf(buckfilename,"%s.data/plidepassage/%s-pp%d-%d.bck",name,shortname,ppptr->index1,ppptr->index2);
	
	    buckf = fopen(buckfilename,"w");
	    if (!buckf)
		{
		    fprintf(stderr,"Can not open %s\n",buckfilename);
		    VipPrintfExit("VipWriteGenericPliDePassage");
		    return(PB);
		}
	    fprintf(buckf,"ascii\n");
	    fprintf(buckf,"-type VOID\n");
	    fprintf(buckf,"-dx %f\n",voxsize.x);
	    fprintf(buckf,"-dy %f\n",voxsize.y);
	    fprintf(buckf,"-dz %f\n",voxsize.z);
	    fprintf(buckf,"-dt 1.\n");
	    fprintf(buckf,"-dimt 1\n");
	    fprintf(buckf,"-time 0\n");
	    fprintf(buckf,"-dim %d\n",ppptr->points->n_points);
	    pptr = ppptr->points->data;
	    for(i=ppptr->points->n_points;i--;)
		{
		    fprintf(buckf,"(%d,%d,%d)\n",pptr->x,pptr->y,pptr->z);
		    pptr++;
		}
	    fclose(buckf);
	    fprintf(f,"*END\n\n");
	    ppptr = ppptr->next;
	}
    fclose(f);

    return(OK);
}
/*-------------------------------------------------------------------------*/
int VipWriteOldJunctionSet(
		      JunctionSet *jset,
		      char *name)
/*-------------------------------------------------------------------------*/
{
  char filename[VIP_NAME_MAXLEN];
  FILE *f;
  BiJunction *bijptr;
  Vip3DPoint_S16BIT *pptr;
  int i;

  if (!jset || !name)
    {
      VipPrintfError("Null arg");
      VipPrintfExit("VipWriteJunctionSet");
      return(PB);
    }

  if(jset->first_bi!=NULL)
    {
      strcpy(filename,name);
      strcat(filename,".bij.Vip");
	
      f = fopen(filename,"w");
      if (!f)
	{
	  fprintf(stderr,"Can not open file %s\n",filename);
	  VipPrintfExit("VipWriteJunctionSet");
	  return(PB);
	}

      printf("Writing %s topological relations...\n",name);
      fprintf(f,"N %d\n",jset->n_bi);

      bijptr = jset->first_bi;

      while(bijptr!=NULL)
	{
	  fprintf(f,"$index1 %d\n",bijptr->index1);
	  fprintf(f,"$index2 %d\n",bijptr->index2);
	  fprintf(f,"$npoints %d\n",bijptr->points->n_points);
	  fprintf(f,"$size %d %.3f\n",bijptr->size_filled,bijptr->size);
	  fprintf(f,"$direction %d %.3f %.3f %.3f\n",bijptr->direction_filled,bijptr->direction.x,
		  bijptr->direction.y,bijptr->direction.z);
	  fprintf(f,"$refdirection %d %.3f %.3f %.3f\n",bijptr->refdirection_filled,bijptr->refdirection.x,
		  bijptr->refdirection.y,bijptr->refdirection.z);
	  fprintf(f,"$mindepth %d %.3f\n",bijptr->mindepth_filled,bijptr->mindepth);
	  fprintf(f,"$maxdepth %d %.3f\n",bijptr->maxdepth_filled,bijptr->maxdepth);
	  fprintf(f,"$extremity1 %d %d %d %d\n",bijptr->extremity1_filled,bijptr->extremity1.x,
		  bijptr->extremity1.y,bijptr->extremity1.z);
	  fprintf(f,"$extremity2 %d %d %d %d\n",bijptr->extremity2_filled,bijptr->extremity2.x,
		  bijptr->extremity2.y,bijptr->extremity2.z);
	  fprintf(f,"$refextremity1 %d %.3f %.3f %.3f\n",bijptr->refextremity1_filled,bijptr->refextremity1.x,
		  bijptr->refextremity1.y,bijptr->refextremity1.z);
	  fprintf(f,"$refextremity2 %d %.3f %.3f %.3f\n",bijptr->refextremity2_filled,bijptr->refextremity2.x,
		  bijptr->refextremity2.y,bijptr->refextremity2.z);
	  fprintf(f,"*begin\n");
	  pptr = bijptr->points->data;
	  for(i=bijptr->points->n_points;i--;)
	    {
	      fprintf(f,"j %d %d %d\n",pptr->x,pptr->y,pptr->z);
	      pptr++;
	    }
	  fprintf(f,"*end\n");
	  bijptr = bijptr->next;
	}
      fclose(f);
    }

  return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteGenericCortexRelationSet(
		      CortexRelationSet *jset,
		      char *name,
		      char * shortname,
		      Vip3DPoint_VFLOAT voxsize)
/*-------------------------------------------------------------------------*/
{
    char filename[VIP_NAME_MAXLEN];
    char buckfilename[VIP_NAME_MAXLEN];
    FILE *f, *buckf;
    CortexBiRelation *bijptr;
    Vip3DPoint_S16BIT *pptr=NULL;
    int i;

    if (!jset || !name)
	{
	    VipPrintfError("Null arg");
	    VipPrintfExit("VipWriteGenericCortexRelationSet");
	    return(PB);
	}

    if(jset->first_bi==NULL) return(OK);
	
    strcpy(filename,name);
    strcat(filename,".arg");
	
    f = fopen(filename,"a");
    if (!f)
	{
	    fprintf(stderr,"Can not open file %s\n",filename);
	    VipPrintfExit("VipWriteGenericCortexRelationSet");
	    return(PB);
	}

    printf("Writing %s cortical relations...\n",shortname);

    bijptr = jset->first_bi;
    while(bijptr!=NULL)
	{

	    fprintf(f,"*BEGIN UBEDGE cortical %d %d\n",bijptr->index1,bijptr->index2);
		    
	    fprintf(f,"point_number %d\n",bijptr->skiz->n_points);
	    if(bijptr->size_filled==VTRUE)
		{
		    fprintf(f,"size %.3f\n",bijptr->size);
		}
	    else
		{
		    fprintf(f,"size %.3f\n",bijptr->skiz->n_points*voxsize.x
			    *voxsize.y*voxsize.z);
		}
	    if(bijptr->hulljunctiondist_filled==VTRUE)
		{
		    fprintf(f,"dist %.3f\n",bijptr->hulljunctiondist);
		}
	    if(bijptr->SS1nearest_filled==VTRUE)
		{
		    fprintf(f,"SS1nearest %d %d %d\n",bijptr->SS1nearest.x,
			    bijptr->SS1nearest.y,bijptr->SS1nearest.z);
		}
	    if(bijptr->refSS1nearest_filled==VTRUE)
		{
		    fprintf(f,"refSS1nearest %.3f %.3f %.3f\n",bijptr->refSS1nearest.x,
			    bijptr->refSS1nearest.y,bijptr->refSS1nearest.z);
		}
	    if(bijptr->SS2nearest_filled==VTRUE)
		{
		    fprintf(f,"SS2nearest %d %d %d\n",bijptr->SS2nearest.x,
			    bijptr->SS2nearest.y,bijptr->SS2nearest.z);
		}
	    if(bijptr->refSS2nearest_filled==VTRUE)
		{
		    fprintf(f,"refSS2nearest %.3f %.3f %.3f\n",bijptr->refSS2nearest.x,
			    bijptr->refSS2nearest.y,bijptr->refSS2nearest.z);
		}
	    fprintf(f,"cortical_filename cortical/%s-c%d-%d.bck\n",shortname,bijptr->index1,bijptr->index2);

	    sprintf(buckfilename,"%s.data/cortical/%s-c%d-%d.bck",name,shortname,bijptr->index1,bijptr->index2);
	
	    buckf = fopen(buckfilename,"w");
	    if (!buckf)
		{
		    fprintf(stderr,"Can not open %s\n",buckfilename);
		    VipPrintfExit("VipWriteGenericCortexRelationSet");
		    return(PB);
		}
	    fprintf(buckf,"ascii\n");
	    fprintf(buckf,"-type VOID\n");
	    fprintf(buckf,"-dx %f\n",voxsize.x);
	    fprintf(buckf,"-dy %f\n",voxsize.y);
	    fprintf(buckf,"-dz %f\n",voxsize.z);
	    fprintf(buckf,"-dt 1.\n");
	    fprintf(buckf,"-dimt 1\n");
	    fprintf(buckf,"-time 0\n");
	    fprintf(buckf,"-dim %d\n",bijptr->skiz->n_points);
	    pptr = bijptr->skiz->data;
	    for(i=bijptr->skiz->n_points;i--;)
		{
		    fprintf(buckf,"(%d,%d,%d)\n",pptr->x,pptr->y,pptr->z);
		    pptr++;
		}
	    fclose(buckf);
	    fprintf(f,"*END\n\n");
	    bijptr = bijptr->next;
	}
    fclose(f);
	
    return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteOldCortexRelationSet(
		      CortexRelationSet *jset,
		      char *name)
/*-------------------------------------------------------------------------*/
{
  char filename[VIP_NAME_MAXLEN];
  FILE *f;
  CortexBiRelation *bijptr;
  Vip3DPoint_S16BIT *pptr;
  int i;

  if (!jset || !name)
    {
      VipPrintfError("Null arg");
      VipPrintfExit("VipWriteOldCortexRelationSet");
      return(PB);
    }

  if(jset->first_bi!=NULL)
    {
      strcpy(filename,name);
      strcat(filename,".bic.Vip");
	
      f = fopen(filename,"w");
      if (!f)
	{
	  fprintf(stderr,"Can not open file %s\n",filename);
	  VipPrintfExit("VipWriteOldCortexRelationSet");
	  return(PB);
	}

      printf("Writing %s cortical relations...\n",name);
      fprintf(f,"N %d\n",jset->n_bi);

      bijptr = jset->first_bi;

      while(bijptr!=NULL)
	{
	  fprintf(f,"$index1 %d\n",bijptr->index1);
	  fprintf(f,"$index2 %d\n",bijptr->index2);
	  fprintf(f,"$npoints %d\n",bijptr->skiz->n_points);
	  fprintf(f,"$size %d %.3f\n",bijptr->size_filled,bijptr->size);
	  fprintf(f,"$dist %d %.3f\n",bijptr->hulljunctiondist_filled,bijptr->hulljunctiondist);
	  fprintf(f,"$SS1nearest %d %d %d %d\n",bijptr->SS1nearest_filled,bijptr->SS1nearest.x,
 		  bijptr->SS1nearest.y,bijptr->SS1nearest.z);
	  fprintf(f,"$SS2nearest %d %d %d %d\n",bijptr->SS2nearest_filled,bijptr->SS2nearest.x,
		  bijptr->SS2nearest.y,bijptr->SS2nearest.z);
	  fprintf(f,"$refSS1nearest %d %.3f %.3f %.3f\n",bijptr->refSS1nearest_filled,bijptr->refSS1nearest.x,
		  bijptr->refSS1nearest.y,bijptr->refSS1nearest.z);
	  fprintf(f,"$refSS2nearest %d %.3f %.3f %.3f\n",bijptr->refSS2nearest_filled,bijptr->refSS2nearest.x,
		  bijptr->refSS2nearest.y,bijptr->refSS2nearest.z);
	  fprintf(f,"*begin\n");
	  pptr = bijptr->skiz->data;
	  for(i=bijptr->skiz->n_points;i--;)
	    {
	      fprintf(f,"j %d %d %d\n",pptr->x,pptr->y,pptr->z);
	      pptr++;
	    }
	  fprintf(f,"*end\n");
	  bijptr = bijptr->next;
	}
      fclose(f);
    }

  return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteOldARGtoRef(
	      FoldArg *arg,
	      char *name)
/*-------------------------------------------------------------------------*/
{
  char filename[VIP_NAME_MAXLEN];
  FILE *fp;

  if(arg==NULL)
    {
      VipPrintfError("No Fold arg as input");
      VipPrintfExit("VipWriteOldARGtoRef");
      return(PB);
    }
  if(name==NULL)
    {
      VipPrintfError("No name for Fold arg as input");
      VipPrintfExit("VipWriteOldARGtoRef");
      return(PB);
    }

  strcpy(filename,name);
  strcat(filename,".ref.Vip");

  fp = fopen(filename,"w");
  if (!fp)
    {
      printf("Can not open file %s\n",filename);
      VipPrintfExit("VipWriteOldARGtoRef");
      return(PB);
    }

  printf("Writing referentiel for %s...\n",name);	
  fprintf(fp,"rotation 3D:\n");
  fprintf(fp,"%d %d\n",arg->gotoref_filled,arg->gonetoref);
  fprintf(fp, "%f %f %f\n", arg->gotoref.t.x, arg->gotoref.t.y, arg->gotoref.t.z);
  fprintf(fp, "%f %f %f\n", arg->gotoref.r.xx, arg->gotoref.r.xy, arg->gotoref.r.xz);
  fprintf(fp, "%f %f %f\n", arg->gotoref.r.yx, arg->gotoref.r.yy, arg->gotoref.r.yz);
  fprintf(fp, "%f %f %f\n", arg->gotoref.r.zx, arg->gotoref.r.zy, arg->gotoref.r.zz);
  fprintf(fp, "scale factors:\n");
  fprintf(fp, "%f %f %f\n", arg->scaleref.x, arg->scaleref.y, arg->scaleref.z);
  fprintf(fp, "box:\n");
  fprintf(fp, "%d %d %d\n", arg->min_ima.x, arg->min_ima.y, arg->min_ima.z);
  fprintf(fp, "%d %d %d\n", arg->max_ima.x, arg->max_ima.y, arg->max_ima.z);
  fprintf(fp, "refbox:\n");
  fprintf(fp, "%f %f %f\n", arg->min_ref.x, arg->min_ref.y, arg->min_ref.z);
  fprintf(fp, "%f %f %f\n", arg->max_ref.x, arg->max_ref.y, arg->max_ref.z);
  fprintf(fp,"voxel geometry (mm)\n");
  fprintf(fp, "%f %f %f\n", arg->vox.x, arg->vox.y, arg->vox.z);
  fprintf(fp, "CA: %d x: %d y: %d z: %d\n", arg->CAfilled, arg->CA.x, arg->CA.y, arg->CA.z);
  fprintf(fp, "CP: %d x: %d y: %d z: %d\n", arg->CPfilled, arg->CP.x, arg->CP.y, arg->CP.z);
  fprintf(fp, "IH: %d x: %d y: %d z: %d\n", arg->IHfilled, arg->IH.x, arg->IH.y, arg->IH.z);

  fclose(fp);

  return(OK);
}

/*-------------------------------------------------------------------------*/
int VipWriteGenericARGtoRef(
	      FoldArg *arg,
	      char *name,
	      char * shortname
)
/*-------------------------------------------------------------------------*/
{
    char filename[VIP_NAME_MAXLEN];
    FILE *fp;

    if(arg==NULL)
	{
	    VipPrintfError("No Fold arg as input");
	    VipPrintfExit("VipWriteGenericARGtoRef");
	    return(PB);
	}
    if(name==NULL)
	{
	    VipPrintfError("No name for Fold arg as input");
	    VipPrintfExit("VipWriteGenericARGtoRef");
	    return(PB);
	}

    strcpy(filename,name);
    strcat(filename,".arg");

    fp = fopen(filename,"w");
    if (!fp)
	{
	    printf("Can not open file %s\n",filename);
	    VipPrintfExit("VipWriteGenericARGtoRef");
	    return(PB);
	}

    printf("Writing referentiel for %s...\n",shortname);
	
    fprintf(fp,"# graph 1.0\n\n");
    fprintf(fp,"*BEGIN GRAPH CorticalFoldArg\n\n");
    fprintf(fp,"CorticalFoldArg_VERSION 2.1\n");
    if(arg->gotoref_filled==VTRUE)
	{
	    fprintf(fp,"Talairach_rotation ");
	    fprintf(fp, "%f %f %f ", arg->gotoref.r.xx, arg->gotoref.r.xy, arg->gotoref.r.xz);
	    fprintf(fp, "%f %f %f ", arg->gotoref.r.yx, arg->gotoref.r.yy, arg->gotoref.r.yz);
	    fprintf(fp, "%f %f %f\n", arg->gotoref.r.zx, arg->gotoref.r.zy, arg->gotoref.r.zz);
	    fprintf(fp, "Talairach_translation ");
	    fprintf(fp, "%f %f %f\n", arg->gotoref.t.x, arg->gotoref.t.y, arg->gotoref.t.z);
	    fprintf(fp, "Talairach_scale ");
	    fprintf(fp, "%f %f %f\n", arg->scaleref.x, arg->scaleref.y, arg->scaleref.z);
	    fprintf(fp, "Tal_boundingbox_min ");
	    fprintf(fp, "%f %f %f\n", arg->min_ref.x, arg->min_ref.y, arg->min_ref.z);
	    fprintf(fp, "Tal_boundingbox_max ");
	    fprintf(fp, "%f %f %f\n", arg->max_ref.x, arg->max_ref.y, arg->max_ref.z);
	}
    fprintf(fp, "boundingbox_min ");
    fprintf(fp, "%d %d %d\n", arg->min_ima.x, arg->min_ima.y, arg->min_ima.z);
    fprintf(fp, "boundingbox_max ");
    fprintf(fp, "%d %d %d\n", arg->max_ima.x, arg->max_ima.y, arg->max_ima.z);
    fprintf(fp,"voxel_size ");
    fprintf(fp, "%f %f %f\n", arg->vox.x, arg->vox.y, arg->vox.z);
    if(arg->CAfilled==VTRUE)
	{
	    fprintf(fp,"anterior_commissure ");
	    fprintf(fp, "%d %d %d\n", arg->CA.x, arg->CA.y, arg->CA.z);
	}
    if(arg->CPfilled==VTRUE)
	{
	    fprintf(fp,"posterior_commissure ");
	    fprintf(fp, "%d %d %d\n", arg->CP.x, arg->CP.y, arg->CP.z);
	}
    if(arg->IHfilled==VTRUE)
	{
	    fprintf(fp,"interhemi_point ");
	    fprintf(fp, "%d %d %d\n", arg->IH.x, arg->IH.y, arg->IH.z);
	}
    fprintf(fp, "filename_base %s.data\n\n",shortname);
    fprintf(fp,"#filenames semantic\n");
    fprintf(fp,"type.tri fold.tri\n");
    fprintf(fp,"fold.tri fold Tmtktri_filename\n");
    fprintf(fp,"type.bck fold.bck junction.bck hull_junction.bck cortical.bck plidepassage.bck\n");
    fprintf(fp,"fold.bck fold ss_filename bottom_filename other_filename\n");
    fprintf(fp,"junction.bck junction junction_filename\n");
    fprintf(fp,"plidepassage.bck plidepassage plidepassage_filename\n");
    fprintf(fp,"hull_junction.bck hull_junction junction_filename\n");
    fprintf(fp,"cortical.bck cortical cortical_filename\n");
    fprintf(fp,"Tmtktri_filename 0 255 0\n");
    fprintf(fp,"ss_filename 0 255 0\n");
    fprintf(fp,"bottom_filename 255 0 100\n");
    fprintf(fp,"other_filename 0 255 255\n\n");
    fclose(fp);

    return(OK);
}
