/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : topology.c       * TYPE     : Function
 * AUTHOR      : MANGIN J.-F.         * CREATION : 22/01/1997
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Management of discrete topology (simple points, classification)
 *
 ****************************************************************************
 *
 * USED MODULES : math.h - alloc.h - topology.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              1/6/99| jeff         | + surfaces simples
 ****************************************************************************/

#include <assert.h>
#include <math.h>

#include <vip/alloc.h>
#include <vip/topology.h>
#include <vip/topology_static.h>

char TopologyTable[4][4][40] = 
{{{ "A=Volume point or B=Isolated point" },{ "B=Isolated point" },
  { "B=Isolated point" },{ "B=Isolated point" }},
 {{ "A=Volume point" },{ "C=Border point" },
  { "F=Surface point" },{ "H=Surfaces point" }},
 {{ "A=Volume point" },{ "D=Curve point" },
  { "G=Surface-curve(s) point" },{ "I=Surfaces-curve(s) point" }},
 {{ "A=Volume point" },{ "E=Curves point" },
  { "G=Surface-curve(s) point" },{ "I=Surfaces-curve(s) point" }}
};

/*for a long time, this tab war underdimensioned...!!!!!!!!!!,
  I hope now it is OK, a simple function would have been much better*/
int TopologicalClassification[10][7] =
{ { PB, TOPO_B, TOPO_B, TOPO_B, TOPO_B, TOPO_B, TOPO_B},
  { TOPO_A, TOPO_C, TOPO_F, TOPO_H, TOPO_H, TOPO_H, TOPO_H},
  { TOPO_A, TOPO_D, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I},
  { TOPO_A, TOPO_E, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I},
  { TOPO_A, TOPO_E, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I},
  { TOPO_A, TOPO_E, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I},
  { TOPO_A, TOPO_E, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I},
  { TOPO_A, TOPO_E, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I},
  { TOPO_A, TOPO_E, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I},
  { TOPO_A, TOPO_E, TOPO_G, TOPO_I , TOPO_I, TOPO_I, TOPO_I} };
/*-----------------------------------------------------*/

/*----------------------------------------------------------------------------*/
char *VipTopologicalStringFromDefine(int def)
/*----------------------------------------------------------------------------*/
{
  char *s;

  s = VipMalloc(sizeof(char)*100,"VipTopologicalStringFromDefine");
  if(!s)
    {
      VipPrintfExit("VipTopologicalStringFromDefine");
      return(NULL);
    }
  switch(def)
    {
    case TOPO_VOLUME_POINT: strcpy(s,"Volume point"); break;
    case TOPO_ISOLATED_POINT: strcpy(s,"Isolated point"); break;
    case TOPO_BORDER_POINT: strcpy(s,"Border (simple) point"); break;
    case TOPO_CURVE_POINT: strcpy(s,"Curve point"); break;
    case TOPO_JUNCTION_CURVES_POINT: strcpy(s,"Junction between curves point"); break;
    case TOPO_SURFACE_POINT: strcpy(s,"Surface point"); break;
    case TOPO_JUNCTION_SURFACE_CURVES_POINT: strcpy(s,"Junction between a surface and curves point"); break;
    case TOPO_JUNCTION_SURFACES_POINT: strcpy(s,"Junction between surfaces point"); break;
    case TOPO_JUNCTION_SURFACES_CURVES_POINT: strcpy(s,"Junction between surfaces and curves point"); break;
    default: strcpy(s,"Unknown topological classification value"); break;
    }
  return(s);
}

/*this test discard non simple point which do not really split the background*/
/*-----------------------------------------------------------------------*/
int IsRealSurfacePoint(
  Topology26Neighborhood *topo26)
/*-----------------------------------------------------------------------*/
{
    if(topo26->ordered_values[1] && topo26->ordered_values[6])
	if (topo26->ordered_values[1]!=topo26->ordered_values[6])
	    return(VTRUE);
   if(topo26->ordered_values[2] && topo26->ordered_values[5])
	if (topo26->ordered_values[2]!=topo26->ordered_values[5])
	    return(VTRUE);
   if(topo26->ordered_values[3] && topo26->ordered_values[4])
	if (topo26->ordered_values[3]!=topo26->ordered_values[4])
	    return(VTRUE);
   return(VFALSE);
}


/*-----------------------------------------------------*/
Topology26Neighborhood *VipCreateTopology26Neighborhood(Volume *vol)
{ int i, j, x, y, z, val, compteur=0;
  int *rel, *nb6, *nb18, *nb26, *offsetptr;
  int X[27], Y[27], Z[27];
  Topology26Neighborhood *topo;
  int linesize, slicesize;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(topology)VipCreateTopology26Neighborhood");
      return(PB);
    }

  topo = (Topology26Neighborhood *)VipCalloc(1, sizeof(Topology26Neighborhood), NULL);
  if (!topo)
  { VipPrintfError("Error while allocating Topology structure");
    VipPrintfExit("(topology)VipCreateTopology26Neighborhood");
    return((Topology26Neighborhood *)NULL);
  }


  /* neighbors ordering: 6-neighbors, 18-neighbors, 26-neighbors for genericity*/

  X[compteur] = Y[compteur] = Z[compteur] = 0;
  ++compteur;

  for (x=-1; x<=1; x++)
    for (y=-1; y<=1; y++)
      for (z=-1; z<=1; z++)
        if ((abs(x)+abs(y)+abs(z)) == 1)
        { X[compteur] = x;
          Y[compteur] = y;
          Z[compteur++] = z;
        }

  for (x=-1; x<=1; x++)
    for (y=-1; y<=1; y++)
      for (z=-1; z<=1; z++)
        if ((abs(x)+abs(y)+abs(z)) == 2)
        { X[compteur] = x;
          Y[compteur] = y;
          Z[compteur++] = z;
        }

  for (x=-1; x<=1; x++)
    for (y=-1; y<=1; y++)
      for (z=-1; z<=1; z++)
        if ((abs(x)+abs(y)+abs(z)) == 3)
        { X[compteur] = x;
          Y[compteur] = y;
          Z[compteur++] = z;
        }

  /* offset to filled the 1D buffer with 26 neighbors values */
  /* WARNING: volume dependent and borderwidth dependent */

  linesize = VipOffsetLine(vol);
  slicesize = VipOffsetSlice(vol);
  offsetptr = topo->offset;
  for (i=0; i<27; i++)
    *offsetptr++ = X[i]+linesize*Y[i]+slicesize*Z[i];

  rel = topo->relation_store;
  for (i=0; i<27; i++)
  {
    topo->relation[i] = rel;
    nb6 = topo->nb6neighbors+i;
    nb18 = topo->nb18neighbors+i;
    nb26 = topo->nb26neighbors+i;
    x = X[i];
    y = Y[i];
    z = Z[i];
    *nb6 = 0;
    for (j=0; j<27; j++)
      if ((abs(x-X[j]) <= 1) && ((abs(y-Y[j])) <= 1) && ((abs(z-Z[j])) <= 1))
      { val = abs(x-X[j])+abs(y-Y[j])+abs(z-Z[j]);
        if (val == 1)
        { *rel++ = j;
          (*nb6)++;
        }
      }
    *nb18 = *nb6;
    for (j=0; j<27; j++)
      if ((abs(x-X[j]) <= 1) && ((abs(y-Y[j])) <= 1) && ((abs(z-Z[j])) <= 1))
      { val = abs(x-X[j])+abs(y-Y[j])+abs(z-Z[j]);
        if (val == 2)
        { *rel++ = j;
          (*nb18)++;
        }
      }
    *nb26 = *nb18;
    for (j=0; j<27; j++)
      if ((abs(x-X[j]) <= 1) && ((abs(y-Y[j])) <= 1) && ((abs(z-Z[j])) <= 1))
      { val = abs(x-X[j])+abs(y-Y[j])+abs(z-Z[j]);
        if (val == 3)
        { *rel++ = j;
          (*nb26)++;
        }
      }
  }

  /* Verify that the store has the correct size */
  assert((void*)rel == (void*)topo->relation_store + sizeof(topo->relation_store));

  return(topo);
}

/*------------------------------------------------------------------*/

void VipFreeTopology26Neighborhood(Topology26Neighborhood *topo)
{
  VipFree(topo);
}

/*------------------------------------------------------------------*/

int VipGetNumberOfComponentsIn26Neighborhood(Topology26Neighborhood *topo, int connectivity, int adjacency_to_central)
{ 
  int point, nbcomp=0, point_courant, voisin, voisin_courant;
  int *listptr, *Xptr, *rel, list[27];
  int debut, fin;
  int **relation;
  int *nbvoisins;
  int *X;

  relation = topo->relation;
  X = topo->ordered_values;
  switch (connectivity)
    {
    case CONNECTIVITY_6:
      nbvoisins = topo->nb6neighbors;
      break;
    case CONNECTIVITY_18:
      nbvoisins = topo->nb18neighbors;
      break;  
    case CONNECTIVITY_26:
      nbvoisins = topo->nb26neighbors;
      break;
    default:
      VipPrintfError("Unknown connectivity in VipGetNumberOfComponentsIn26Neighborhood");
      return(TOPO_PB); 
    }

  switch (adjacency_to_central)
    {
    case CC_6_ADJACENT:
      debut = 1;
      fin = 6;
      break;
    case CC_18_ADJACENT:
      debut = 1;
      fin = 18;
      break;  
    case CC_26_ADJACENT:
      debut = 1;
      fin = 26;
      break;
    default:
      VipPrintfError("Unknown adjacency mode in VipGetNumberOfComponentsIn26Neighborhood");
      return(TOPO_PB); 
    }

  Xptr = X;

  for (point=27; point--; Xptr++)
    if (*Xptr)  *Xptr = VIP_IL_EXIST;

  Xptr = X+debut;
  listptr = list;

  for (point=debut; point<=fin; point++)
    if ((*Xptr++) == VIP_IL_EXIST)
      {
	nbcomp++;
	*listptr++ = point;
	/*	X[point] = VIP_IN_LIST;*/
	X[point] = nbcomp;
	do
	  {
	    point_courant = *(--listptr);
	    rel = relation[point_courant];
	    for (voisin=0; voisin<nbvoisins[point_courant]; voisin++)
	      {
		voisin_courant = rel[voisin];
		if (X[voisin_courant] == VIP_IL_EXIST)
		  {
		    *listptr++ = voisin_courant;
		    /*X[voisin_courant] = VIP_IN_LIST;*/
		    X[voisin_courant] = nbcomp;
		  }
	      } 
	  }
	while (listptr != list);
      }

  return(nbcomp);
}




/*----------------------------------------------------------*/

int VipGetNumberOfComponentsIn18Neighborhood(Topology26Neighborhood *topo, int connectivity, int adjacency_to_central)
{ 
  int point, nbcomp=0, point_courant, voisin, voisin_courant;
  int *listptr, *Xptr, *rel, list[27];
  int debut, fin;
  int **relation;
  int *nbvoisins;
  int *X;

  relation = topo->relation;
  X = topo->ordered_values;
  switch (connectivity)
    {
    case CONNECTIVITY_6:
      nbvoisins = topo->nb6neighbors;
      break;
    case CONNECTIVITY_18:
      nbvoisins = topo->nb18neighbors;
      break;  
    case CONNECTIVITY_26:
      nbvoisins = topo->nb26neighbors;
      break;
    default:
      VipPrintfError("Unknown connectivity in VipGetNumberOfComponentsIn18Neighborhood");
      return(TOPO_PB); 
    }

  switch (adjacency_to_central)
    {
    case CC_6_ADJACENT:
      debut = 1;
      fin = 6;
      break;
    case CC_18_ADJACENT:
      debut = 1;
      fin = 18;
      break;  
    case CC_26_ADJACENT:
      debut = 1;
      fin = 26;
      break;
    default:
      VipPrintfError("Unknown adjacency mode in VipGetNumberOfComponentsIn18Neighborhood");
      return(TOPO_PB); 
    }

  Xptr = X;

  for (point=19; point>0; point--, Xptr++)
    if (*Xptr)  *Xptr = VIP_IL_EXIST;

  for (point=8; point>0; point--, Xptr++)
    if (*Xptr)  *Xptr = VIP_IL_EST_TROP_LOIN;

  listptr = list;
  Xptr = X+debut;

  for (point=debut; point<=fin; point++)
    if ((*Xptr++) == VIP_IL_EXIST)
    { nbcomp++;
      *listptr++ = point;
      /* X[point] = VIP_IN_LIST;*/
	X[point] = nbcomp;
      do
      { point_courant = *(--listptr);
        rel = relation[point_courant];
        for (voisin=0; voisin<nbvoisins[point_courant]; voisin++)
        { voisin_courant = rel[voisin];
          if (X[voisin_courant] == VIP_IL_EXIST)
          { *listptr++ = voisin_courant;
	  /*X[voisin_courant] = VIP_IN_LIST;*/
	  X[voisin_courant] = nbcomp;
          }
        } 
      }
      while (listptr != list);
    }

  return(nbcomp);
}

/*----------------------------------------------------------------------------*/

int VipFlipTopology26NeighborhoodValueTable(
  Topology26Neighborhood *topo
)
{ int i, *Xptr;

  if(topo==NULL)
    {
      VipPrintfError("arguments in VipFlipTopology26NeighborhoodValueTable");
      VipPrintfExit("VipFlipTopology26NeighborhoodValueTable");
      return(PB);
    }

  Xptr=topo->ordered_values+1;
  for (i=26; i--; Xptr++)  *Xptr = !(*Xptr);

  return(OK);
}

/*----------------------------------------------------------------------------*/

int VipIsSimplePoint(int Cbar, int Cstar)
{
  if((Cbar==1)&&(Cstar==1)) return(VTRUE);
  else return(VFALSE);
}


/*----------------------------------------------------------------------------*/

int VipIsCurvesPoint(int Cbar, int Cstar)
{
  if((Cbar==1)&&(Cstar>1)) return(VTRUE);
  else return(VFALSE);
}


/*----------------------------------------------------------------------------*/
int VipFillSSComponent(
VipS16BITPtrBucket *comp1,
VipS16BITPtrBucket *comp2,
Topology26Neighborhood *top,
Vip_S16BIT *adresse)
{
	int *X_ptr, *offset_ptr, i;
	Vip_S16BIT **buf1_ptr, **buf2_ptr;

	buf1_ptr = comp1->data;
	buf2_ptr = comp2->data;
	comp1->n_points = 0;
	comp2->n_points = 0;

	X_ptr = top->ordered_values + 1;
	offset_ptr = top->offset + 1;

	for(i=18;i--;)
	{	
		if((*X_ptr)==1)
		{
			*buf1_ptr++ = adresse + *offset_ptr;
			(comp1->n_points)++;
		}
		else if ((*X_ptr)==2)
		{ 
			*buf2_ptr++ = adresse + *offset_ptr;
			(comp2->n_points)++;
		}
		X_ptr++;
		offset_ptr++;
	}

	if(comp1->n_points<=0 || comp2->n_points<=0)
	    {
		VipPrintfError("empty 6-connected component");
		VipPrintfExit("VipFillSSComponent");
		return(PB);
	    }

	return(OK);
}
/*----------------------------------------------------------------------------*/
int VipSurfaceSimpleEquivalence(
VipS16BITPtrBucket *comp1,
VipS16BITPtrBucket *comp2,
VipS16BITPtrBucket *comp1bis,
VipS16BITPtrBucket *comp2bis)
/*----------------------------------------------------------------------------*/
{
	Vip_S16BIT **buf1_ptr, **buf2_ptr;
	Vip_S16BIT **buf1bis_ptr, **buf2bis_ptr;
	int i, j;
	int c1_c1bis, c2_c2bis, c1_c2bis, c2_c1bis;


/* comp1 versus comp1bis */

	c1_c1bis = VFALSE;
	buf1bis_ptr = comp1bis->data;
	for(i=comp1bis->n_points;i--;)
	{	
		buf1_ptr = comp1->data;
		for(j=comp1->n_points;j--;)
		{
			if(*buf1bis_ptr == *buf1_ptr)
			{
				c1_c1bis = VTRUE;
				break;
			}
			buf1_ptr++;
		}
		if (c1_c1bis == VTRUE) break;
		else buf1bis_ptr++;
	}

/* comp2 versus comp2bis */

	if(c1_c1bis==VTRUE)
	{
		c2_c2bis = VFALSE;
		buf2bis_ptr = comp2bis->data;
		for(i=comp2bis->n_points;i--;)
		{	
			buf2_ptr = comp2->data;
			for(j=comp2->n_points;j--;)
			{
				if(*buf2bis_ptr == *buf2_ptr)
				{
					c2_c2bis = VTRUE;
					break;
				}
				buf2_ptr++;
			}
			if (c2_c2bis == VTRUE) break;
			else buf2bis_ptr++;
		}
		if (c2_c2bis == VTRUE) return(VTRUE);
	}

/* c1 versus c2bis */

	c1_c2bis = VFALSE;
	buf2bis_ptr = comp2bis->data;
	for(i=comp2bis->n_points;i--;)
	{	
		buf1_ptr = comp1->data;
		for(j=comp1->n_points;j--;)
		{
			if(*buf2bis_ptr == *buf1_ptr)
			{
				c1_c2bis = VTRUE;
				break;
			}
			buf1_ptr++;
		}
		if (c1_c2bis == VTRUE) break;
		else buf2bis_ptr++;
	}

/* c2 versus c1bis */

	if(c1_c2bis==VTRUE)
	{
		c2_c1bis = VFALSE;
		buf1bis_ptr = comp1bis->data;
		for(i=comp1bis->n_points;i--;)
		{	
			buf2_ptr = comp2->data;
			for(j=comp2->n_points;j--;)
			{
				if(*buf1bis_ptr == *buf2_ptr)
				{
					c2_c1bis = VTRUE;
					break;
				}
				buf2_ptr++;
			}
			if (c2_c1bis == VTRUE) break;
			else buf1bis_ptr++;
		}
		if (c2_c1bis == VTRUE) return(VTRUE);
	}

	return(VFALSE);
}

