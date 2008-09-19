/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_distmap_mask.c   * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 05/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne les masques des transformations 
 *               de distances
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/distmap.h>
#include <vip/alloc.h>

#include <vip/distmap_static.h>


/*--------------------------------------------------------------------------*/
static void  OrderMaskVectorAfterEuclideanDistance(DistmapMask *newmask );
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
static DistmapMask *CreateMaskStructure 
(
 int xsize,
 int ysize,
 int zsize,
 int length
 );
/*--------------------------------------------------------------*/

/*---------------------------------------------------------------*/
static DistmapMask *AllocMaskStructure(
);
/*---------------------------------------------------------------*/

/*--------------------------------------------------------------*/
static void FillMaskEuclideanDistance 
(
 DistmapMask *mask,
 Volume *vol
);
/*--------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
static int CheckMasksizeCoherence (
   int xsize,
   int ysize,
   int zsize
);
/*-------------------------------------------------------------------*/
/*--------------------------------------------------------------*/
static DistmapMask *DuplicateMaskStructure (
   DistmapMask *modele,
   int length
);
/*--------------------------------------------------------------*/
/*---------------------------------------------------------------*/
static void ChangeMaskLength (
	DistmapMask **m,
	int newl
);
/*---------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
static void SplitMask (
   DistmapMask *in,
   DistmapMask **forward_ad,
   DistmapMask **backward_ad
);
/*--------------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
static void FillMaskOffset (
   DistmapMask *mask,
   Volume *vol
);
/*---------------------------------------------------------------------*/
/*----------------------------------------------------------------*/
static DistmapMask *DestroyMultiple (
   DistmapMask *m
);
/*----------------------------------------------------------------*/
/*-------------------------------------------------------------*/
/*static void ShowMask (
   DistmapMask *m
   );*/
/*-------------------------------------------------------------*/

/********************************************************************/

/*-----------------------------------------------------*/
int CreateAndSplitCubicMask (
   Volume *vol,
   int xsize,
   int ysize,
   int zsize,
   DistmapMask **forward,
   DistmapMask **backward
)
/*-----------------------------------------------------*/
{
  DistmapMask *fullmask;
  DistmapMask *f, *b;

  if ( !VipVerifyAll ( vol ) )
    {
      VipPrintfExit ( "(distmap_mask)CreateAndSplitCubicMask" );    
      return(PB);
    }

  fullmask = CreateSquareMask ( vol, xsize, ysize, zsize );
  if(fullmask==PB) return(PB);

  SplitMask ( fullmask, &f, &b );

  *forward = f;
  *backward = b;

  FreeMask(fullmask); /** rajoute le 26/03/1999  F.Poupon **/

  return(OK);
}

/*-----------------------------------------------------*/
int CreateAndSplitConnectivityMask (
   Volume *vol,
   int connectivity,
   DistmapMask **forward,
   DistmapMask **backward
)
/*-----------------------------------------------------*/
{
  DistmapMask *fullmask;
  DistmapMask *f, *b;

  if ( !VipVerifyAll ( vol ) )
    {
      VipPrintfExit ( "(distmap_mask)CreateAndSplitConnectivityMask" );     
      return(PB);
    }

  fullmask = CreateConnectivityMask ( vol, connectivity );
  if(fullmask==PB) return(PB);

  SplitMask ( fullmask, &f, &b );

  *forward = f;
  *backward = b;

  FreeMask(fullmask);

  return(OK);
}
/*-----------------------------------------------------*/
DistmapMask *CreateSquareMask (
   Volume *vol,
   int xsize,
   int ysize,
   int zsize
)
/*-----------------------------------------------------*/

   /* Cree la liste de points du masque munis de leur coordonnees,
		en eliminant les multiples
		Ceci se fait sur les vecteurs elementaires situes dans
		l'octant positif et est generalise au reste de l'espace
		a l'aide de 3 symmetries */

{
  DistmapMask_point *mpptr, *fptr;
  DistmapMask *newmask, *finalmask;
  int x, y, z, newl;
  int sym[24], lsym;
  int i;
  int maxsum, m;

  if(CheckMasksizeCoherence( xsize, ysize, zsize)==PB) return(PB);

  newmask = CreateMaskStructure( xsize/2, ysize/2, zsize/2,
				 ((xsize/2+1)*(ysize/2+1))*(zsize/2+1));
   
  mpptr = newmask->first_point;
  newl = 0;

  maxsum = xsize/2 + ysize/2 + zsize/2;

  /*order vector to optimize front propagation memory*/
  for(m=1;m<=maxsum;m++)
    {
      for(x= 0; x<=xsize/2; x++)
	for(y= 0; y<=ysize/2; y++)
	  for(z= 0; z<=zsize/2; z++)
	    {
	      if( (x+y+z)==m )
		{
		  mpptr->x = x;
		  mpptr->y = y;
		  mpptr->z = z;
		  newl++;
		  mpptr++;
		}
	    }
    }

  newmask->length = newl;

  newmask = DestroyMultiple( newmask);
  FillMaskEuclideanDistance( newmask, vol);
  OrderMaskVectorAfterEuclideanDistance( newmask );

  finalmask = DuplicateMaskStructure( newmask, 8*(newmask->length) );

  mpptr = newmask->first_point;
  fptr = finalmask->first_point;
  finalmask->length = 0;

  /* depliement */
  for(newl=newmask->length;newl>0;newl--)
    {
      sym[0] = mpptr->x;
      sym[1] = mpptr->y;
      sym[2] = mpptr->z;
      lsym = 1;
      if(sym[0])
	{
	  sym[3] = -sym[0];
	  for(i=1;i<3;i++) sym[i+3]=sym[i];
	  lsym=2;
	}
      if(sym[1])
	{
	  for(i= 0;i<lsym;i++) sym[3*(lsym+i)+1]= -sym[1];
	  for(i= 0;i<lsym;i++) sym[3*(lsym+i)]=sym[3*i];
	  for(i= 0;i<lsym;i++) sym[3*(lsym+i)+2]=sym[2];
	  lsym *=2;
	}
      if(sym[2])
	{
	  for(i= 0;i<lsym;i++) sym[3*(lsym+i)+2]= -sym[2];
	  for(i= 0;i<lsym;i++) sym[3*(lsym+i)]=sym[3*i];
	  for(i= 0;i<lsym;i++) sym[3*(lsym+i)+1]=sym[3*i+1];
	  lsym *=2;
	}
      for(i=0;i<lsym;i++)
	{
	  fptr->x = sym[3*i];
	  fptr->y = sym[3*i+1];
	  fptr->z = sym[3*i+2];
	  fptr->dist = mpptr->dist;
	  fptr++;
	}
      (finalmask->length)+=lsym;
      mpptr++;
    }

  ChangeMaskLength( &finalmask, finalmask->length );
  FillMaskOffset( finalmask, vol);	

  FreeMask(newmask);

  /*ShowMask( finalmask );*/

  return( finalmask);
}

/*-----------------------------------------------------*/
DistmapMask *CreateConnectivityMask (
   Volume *vol,
   int connectivity
)
/*-----------------------------------------------------*/

{
  DistmapMask_point *mpptr;
  DistmapMask *newmask;
  int x, y, z;
  int i;
  VipConnectivityStruct *vcs;
  Vip3DPoint_S16BIT *xyzptr;
  int *offsetptr;

  vcs = VipGetConnectivityStruct(vol,connectivity);
  if(vcs==NULL) return(NULL);
  /*NB not freed...*/

  x = 0;
  y = 0;
  z = 0;
  /*compute cubesize for borderwidth checking*/
  xyzptr = vcs->xyz_offset;
  for(i=vcs->nb_neighbors;i--;)
    {
      if (abs((int)xyzptr->x)>x) x = abs((int)xyzptr->x);
      if (abs((int)xyzptr->y)>y) y = abs((int)xyzptr->y);
      if (abs((int)xyzptr->z)>z) z = abs((int)xyzptr->z);
      xyzptr++;
    }

  newmask = CreateMaskStructure( x, y, z, vcs->nb_neighbors);

  mpptr = newmask->first_point;
  xyzptr = vcs->xyz_offset;
  offsetptr = vcs->offset;
  for(i=vcs->nb_neighbors;i--;)
    {
      mpptr->x = xyzptr->x;
      mpptr->y = xyzptr->y;
      mpptr->z = xyzptr->z;
      mpptr->offset = *offsetptr++;
      mpptr->dist = 1;
      mpptr++;
      xyzptr++;
    }

  VipFreeConnectivityStruct(vcs);

  return( newmask);
}

/*--------------------------------------------------------------------------*/
void  OrderMaskVectorAfterEuclideanDistance(DistmapMask *mask )
/*--------------------------------------------------------------------------*/
{
  int i;
  int xtemp;
  int ytemp;
  int ztemp;
  int offsettemp;
  int disttemp;
  int flag;
  DistmapMask_point *p1, *p2;

  flag = VTRUE;
  while(flag==VTRUE)
    {
      flag = VFALSE;
      p1 = mask->first_point;
      p2 = p1+1;
      for(i=1;i<mask->length;i++)
	{
	  if(p1->dist>p2->dist)
	    {
	      xtemp = p1->x;
	      ytemp = p1->y;
	      ztemp = p1->z;
	      offsettemp = p1->offset;
	      disttemp = p1->dist;
	      p1->x = p2->x;
	      p1->y = p2->y;
	      p1->z = p2->z;
	      p1->dist = p2->dist;
	      p1->offset = p2->offset;
	      p2->x = xtemp;
	      p2->y = ytemp;
	      p2->z = ztemp;
	      p2->offset = offsettemp;
	      p2->dist = disttemp;
	      flag = VTRUE;
	    }
	  p1++;
	  p2++;
	}
    }      
}

/*--------------------------------------------------------------------------*/
void SplitMask (
		       DistmapMask *in,
		       DistmapMask **forward_ad,
		       DistmapMask **backward_ad
		       )
/*--------------------------------------------------------------------------*/

	/* Partitionne le masque en deux, chaque sous-masque
           correspondant a un des deux balayages */
{
  DistmapMask *forward, *backward;
  DistmapMask_point *reader, *fwriter, *bwriter;
  int i;

  if( in==NULL )
    {
      VipPrintfError("nothing to feed SplitMask");
      VipPrintfExit("(distmap_mask)SplitMask");
    }

  forward  = AllocMaskStructure();
  backward = AllocMaskStructure();

  forward->xcubesize = backward->xcubesize = in->xcubesize;
  forward->ycubesize = backward->ycubesize = in->ycubesize;
  forward->zcubesize = backward->zcubesize = in->zcubesize;

  forward->length = 0;
  backward->length = 0;
  reader = in->first_point;
  for(i=in->length;i>0;i--)
    {
      if( (reader->z < 0) ||
          ((reader->z==0)&&(reader->y<0)) ||
          ((reader->z==0) && (reader->y==0) && (reader->x<0)) )
	forward->length++;
      else backward->length++;
      reader++;
    }

  /* Tout ceci est pour des masques symetriques sans interet */

  if( in->length != forward->length+backward->length )
    {
      VipPrintfError("Big malaise au niveau de Splitmask");
      VipPrintfExit("(distmap_mask)SplitMask");
    }

  forward->first_point =
    (DistmapMask_point *)VipMalloc(sizeof(DistmapMask_point)
				   *forward->length,
				   "(distmap_mask)SplitMask");
  backward->first_point =
    (DistmapMask_point *)VipMalloc(sizeof(DistmapMask_point)
				   *backward->length,
				   "(distmap_mask)SplitMask");

  reader = in->first_point;
  fwriter = forward->first_point;
  bwriter = backward->first_point;
  for(i=in->length;i>0;i--)
    {
      if( (reader->z < 0) ||
          ((reader->z==0)&&(reader->y<0)) ||
          ((reader->z==0) && (reader->y==0) && (reader->x<0)) )

	/* NB: une autre facon de faire consiste a utiliser l'offset
	   calcule plus loin */

	{
	  fwriter->x = reader->x;
	  fwriter->y = reader->y;
	  fwriter->z = reader->z;
	  fwriter->offset = reader->offset;
	  fwriter->dist = reader->dist;
	  fwriter++;
	  reader++;
	}
      else
	{
	  bwriter->x = reader->x;
	  bwriter->y = reader->y;
	  bwriter->z = reader->z;
	  bwriter->offset = reader->offset;
	  bwriter->dist = reader->dist;
	  bwriter++;
	  reader++;
	}
    }
  *forward_ad = forward;
  *backward_ad = backward;
}

/*---------------------------------------------------------------------*/
void FillMaskOffset (
   DistmapMask *mask,
   Volume *vol
)
/*---------------------------------------------------------------------*/

{
  int l;
  DistmapMask_point *ptr;
  int xsize, slicesize;

  ptr = mask->first_point;

  xsize = VipOffsetLine( vol);
  slicesize = VipOffsetSlice( vol );

  for(l=mask->length; l>0; l--)
    {
      ptr->offset = ptr->x + xsize * ptr->y
	+  slicesize * ptr->z;

      ptr++;
    }
}

/*----------------------------------------------------------------*/
DistmapMask *DestroyMultiple (
   DistmapMask *m
)
/*----------------------------------------------------------------*/
{
  DistmapMask_point *ptr_base, *ptr_search;
  int base, search, new_l;
  DistmapMask *newmask;
  float qx, qy, qz, bingo;

  ptr_base = m->first_point;
  for( base=m->length; base>0; base-- )
    {
      ptr_base->offset = 0;
      ptr_base++;
    }

  bingo=1000.;

  ptr_base = m->first_point;
  for( base=m->length; base>0; base-- )
    {
      if( ptr_base->offset==0)
	{
	  ptr_search = m->first_point;
	  for( search=m->length; search>0; search-- )
	    {
	      if( (ptr_search != ptr_base) && (ptr_search->offset==0) )
		{
		  if (ptr_base->z==0)
		    {
		      if (ptr_search->z==0) qz = bingo;
		      else qz = -bingo;
		    }
		  else  qz = (float)(ptr_search->z) / (float)(ptr_base->z);
		  if (ptr_base->y==0)
		    {
		      if (ptr_search->y==0) qy = bingo;
		      else qy = -bingo;
		    }
		  else  qy = (float)(ptr_search->y) / (float)(ptr_base->y);
		  if (ptr_base->x==0)
		    {
		      if (ptr_search->x==0) qx = bingo;
		      else qx = -bingo;
		    }
		  else  qx = (float)(ptr_search->x) / (float)(ptr_base->x);
		  if( ( (qz==qy) || (qz==bingo) || (qy==bingo) )
		      && ( (qx==qy) || (qx==bingo) || (qy==bingo) )
		      && ( (qz==qx) || (qz==bingo) || (qx==bingo) ) )
		    ptr_search->offset= -1;
		}
	      ptr_search++;
	    }
	}
      ptr_base++;
    }

  new_l=0;
  ptr_base = m->first_point;
  for(base = m->length; base>0; base--)
    {
      if(ptr_base->offset==0) new_l++;
      ptr_base++;
    }

  newmask = DuplicateMaskStructure( m, new_l );
  ptr_base = m->first_point;
  ptr_search = newmask->first_point;
  for(base = m->length; base>0; base--)
    {
      if(ptr_base->offset==0)
	{
	  ptr_search->x = ptr_base->x;
	  ptr_search->y = ptr_base->y;
	  ptr_search->z = ptr_base->z;
	  ptr_search++;
	}
      ptr_base++;
    }

  FreeMask(m);

  return(newmask);
}

/*	approximation par defaut des distances euclidiennes elementaires,
	multipliees au prealable par VIP_DISTMAP_MULTFACT */
/*--------------------------------------------------------------*/
void FillMaskEuclideanDistance (
   DistmapMask *mask,
   Volume *vol
)
/*--------------------------------------------------------------*/
{
  int l;
  DistmapMask_point *ptr;
  float euclid, x, y, z;

  ptr = mask->first_point;

  for(l=mask->length; l>0; l--)
    {
      x = ptr->x * mVipVolVoxSizeX( vol);
      y = ptr->y * mVipVolVoxSizeY( vol);
      z = ptr->z * mVipVolVoxSizeZ( vol);
      euclid = (float)sqrt((double)(x*x + y*y + z*z));
      ptr->dist = (int)((double)(VIP_USED_DISTMAP_MULTFACT*euclid)+0.5);
      ptr++;
    }
}

/*-------------------------------------------------------------*/
/*debug
  void ShowMask (
   DistmapMask *m
   )*/
/*-------------------------------------------------------------*/
/*
{
  DistmapMask_point *ptr;
  int i;

  if( m==NULL)
    {
      VipPrintfError("Nothing to peer at!!!\n");
      VipPrintfExit("(distmap_mask)ShowMask");
    }
  ptr = m->first_point;
  (void)printf("\n");
  for(i=m->length;i>0;i--)
    {
      (void)printf("%d %d %d, offset: %d, dist: %d\n",ptr->x, ptr->y, ptr->z,
		   ptr->offset, ptr->dist);
      ptr++;
    }
  (void)printf("\n");
  (void)printf("Mask length: %d\n", m->length);
}
*/
/*--------------------------------------------------------------*/
DistmapMask *DuplicateMaskStructure (
   DistmapMask *modele,
   int length
)
/*--------------------------------------------------------------*/
{
  DistmapMask *newmask;

  newmask = CreateMaskStructure( modele->xcubesize,
				 modele->ycubesize,modele->zcubesize,length );
  if (!newmask) return((DistmapMask *)NULL);

  return( newmask );
}

/*--------------------------------------------------------------*/
DistmapMask *CreateMaskStructure (
   int xsize,
   int ysize,
   int zsize,
   int length
)
/*--------------------------------------------------------------*/

{
  DistmapMask *newmask;

  newmask = AllocMaskStructure();
  if (!newmask) return((DistmapMask *)NULL);

  newmask->xcubesize = xsize;
  newmask->ycubesize = ysize;
  newmask->zcubesize = zsize;
  newmask->length = length;

  newmask->first_point =
    (DistmapMask_point *)VipCalloc(length, sizeof(DistmapMask_point),
				   "(distmap_mask)CreateMaskStructure");
  if (!newmask->first_point)
    {
      if (newmask) VipFree(newmask);
      return((DistmapMask *)NULL);
    }

  return(newmask);
}

/*-------------------------------------------------------------------*/
int CheckMasksizeCoherence (
   int xsize,
   int ysize,
   int zsize
)
/*-------------------------------------------------------------------*/

{
  int masksize[3];
  int i;

  masksize[0] = xsize;
  masksize[1] = ysize;
  masksize[2] = zsize;

  for(i=0;i<3;i++)
    if((masksize[i]>CHAMFER_MASK_MAXSIZE) || (masksize[i]<1) )
      {
	VipPrintfError("Non consistent or too large chamfer mask size");
	VipPrintfExit("(distmap_mask)CheckMasksizeCoherence");
	return(PB);
      }
  for(i=0;i<3;i++)
    if((masksize[i])==(2*(masksize[i]/2)))
      {
	VipPrintfError("Chamfer masks are symmetrical (==>odd sizes)");
	VipPrintfExit("(distmap_mask)CheckMasksizeCoherence");
	return(PB);
      }
  return(OK);
}

/*---------------------------------------------------------------*/
DistmapMask *AllocMaskStructure(
)
/*---------------------------------------------------------------*/

{

  DistmapMask *newmask;

  newmask = (DistmapMask *)VipMalloc(sizeof(DistmapMask),"(distmap_mask)AllocMaskStructure");
  if (!newmask) return((DistmapMask *)NULL);

  return(newmask);
}

/*---------------------------------------------------------------*/
void ChangeMaskLength (
	DistmapMask **m,
	int newl
)
/*---------------------------------------------------------------*/

{
  DistmapMask *newmask;
  DistmapMask_point *in, *out;
  int i;

  if (!((*m)->xcubesize) && !((*m)->length) )
    {
      VipPrintfError(" That Mask does not exist");
      VipPrintfExit("(distmap_mask)ChangeMaskLength");
    }

  newmask = DuplicateMaskStructure( *m, newl );
  in = (*m)->first_point;
  out = newmask->first_point;
  for (i=newl;i--;)
    {
      out->x = in->x;
      out->y = in->y;
      out->z = in->z;
      out->dist = in->dist;
      in++;
      out++;
    }

  FreeMask( *m );

  *m = newmask;
}

/*----------------------------------------------------------------*/
void FreeMask (
   DistmapMask *m
)
/*----------------------------------------------------------------*/
{
  if (m)
    {
      if (m->first_point) VipFree(m->first_point);
      VipFree(m);
    }
}
