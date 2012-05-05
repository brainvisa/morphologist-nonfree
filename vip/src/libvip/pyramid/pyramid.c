/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_pyramid.c        * TYPE     : Function
 * AUTHOR      : MANGIN J.-F.         * CREATION : 18/06/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Calul de la pyramide
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_util.h - Vip_alloc.h - Vip_pyramid.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 *            19/08/99| V Frouin     | Le borderwith forcé à 1 des volumes
 *                    |              | internes aux pyramides est remplace 
 *                    |              | par un heritage du borderw du haut
 *                    |              | ou du bas de la pyramide.
 *                    |              |     lignes   44 et 381
 *                    |              | Suppression du VipSetBorder( -1 )
 *                    |              | car peut se faire au niveau appelant
 ****************************************************************************/

#include <vip/util.h>
#include <vip/alloc.h>
#include <vip/pyramid.h>

PyramidVolume *VipCreateEmptyPyramidVolume()
{ 
  PyramidVolume *new;

  new = (PyramidVolume *)VipCalloc(1, sizeof(PyramidVolume), NULL);
  if (!new)
    {
      VipPrintfError("Error while allocating Pyramid structure");
      VipPrintfExit("(Vip_pyramid.c)VipCreateEmptyPyramidVolume");
      return((PyramidVolume *)NULL);
    }

  new->volume = (Volume *)VipDeclare3DVolumeStructure(1,1,1,1.0,1.0,1.0,
						      S16BIT,"",0);
  if (!new->volume)
    { 
      VipPrintfError("Error while allocating Pyramid volume");
      VipPrintfExit("(Vip_pyramid)VipCreateEmptyPyramidVolume");
      VipFree(new);
      return((PyramidVolume *)NULL);
    }

  new->nx = 0;
  new->ny = 0;
  new->nz = 0;
  new->rx = 0;
  new->ry = 0;
  new->rz = 0;
  new->winsize = 1;

  return(new);
}

PyramidVolume *VipCreatePyramidVolumeLikePyramid(PyramidVolume *pyr)
{
  PyramidVolume *new;

  new = VipCreateVoidPyramidVolumeLikePyramid(pyr);

  if (VipTransferVolumeData(pyr->volume, new->volume) == PB)
    { 
      VipPrintfError("Error while transfering volume data");
      VipPrintfExit("(Vip_pyramid.c)VipCreatePyramidVolumeLikePyramid");
      VipFreeVolume(new->volume);
      VipFree(new);
      return((PyramidVolume *)NULL);
    }

  return(new);
}

PyramidVolume *VipCreateVoidPyramidVolumeLikePyramid(PyramidVolume *pyr)
{ 
  PyramidVolume *new;

  new = (PyramidVolume *)VipCalloc(1, sizeof(PyramidVolume), NULL);
  if (!new)
    { 
      VipPrintfError("Error while allocating PyramidVolume structure");
      VipPrintfExit("(Vip_pyramid)VipCreatePyramidVolumeLikePyramid");
      return((PyramidVolume *)NULL);
    }

  *new = *pyr;
  new->volume = NULL;
  new->volume = (Volume *)VipDuplicateVolumeStructure(pyr->volume, "");
  if (!new->volume)
    { 
      VipPrintfError("Error while allocating volume");
      VipPrintfExit("(Vip_pyramid)VipCreatePyramidVolumeLikePyramid");
      VipFree(new);
      return((PyramidVolume *)NULL);
    }

  if (VipAllocateVolumeData(new->volume) == PB)
    { 
      VipPrintfError("Unable to allocate volume data");
      VipPrintfExit("(Vip_pyramid)VipCreatePyramidVolumeLikePyramid");
      VipFreeVolume(new->volume);
      VipFree(new);
      return((PyramidVolume *)NULL);
    }

  VipSet3DImageLevel(new->volume, 0, 0);

  return(new);
}

int VipAllocateDataSpacePyramidVolume(PyramidVolume *pyr)
{ 
  if (!VipAllocateVolumeData(pyr->volume))
    { 
      VipPrintfError("Error while allocating Pyramid volume");
      VipPrintfExit("(Vip_pyramid)VipAllocateDataSpacePyramidVolume");
      return(PB);
    }


  return(OK);
}

PyramidVolume *VipConvertVolumeToPyramidVolume(Volume *vol)
{
  PyramidVolume *new;

  new = VipCreateEmptyPyramidVolume();
  if (!new)
    { 
      VipPrintfError("Error while allocating Empty Pyramid structure");
      VipPrintfExit("(Vip_pyramid.c)VipConvertVolumeToPyramidVolume");
      return((PyramidVolume *)NULL);
    }

  *new->volume = *vol;
  new->nx = mVipVolSizeX(new->volume);
  new->ny = mVipVolSizeY(new->volume);
  new->nz = mVipVolSizeZ(new->volume);
  new->rx = 0;
  new->ry = 0;
  new->rz = 0;
  new->winsize = 1;  
  VipCopyVolumeHeader( vol, new->volume );

  return(new);
}

Vip_S16BIT VipGetPyramidMean(Vip_S16BIT medtab[], int nb)
{
  int sum=0, i;
  int thecast;

  for (i=0; i<nb; i++)  sum += medtab[i];

  thecast = (int)((float)sum/(float)nb+0.5);
  return(thecast);
}

Vip_S16BIT VipGetPyramidMax(Vip_S16BIT medtab[], int nb)
{
  int max=0, i;

  for (i=0; i<nb; i++)  if(max<medtab[i]) max=medtab[i];
  return(max);
}

Vip_S16BIT VipGetPyramidMedian(Vip_S16BIT medtab[], int nb)
{ 
  int test1, test2, i=0, j=0, min, max;
 
  if ((nb <= 0) || (!medtab))
    {
      VipPrintfError("Error while computing Mediane...\n");
      VipPrintfExit("(Vip_pyramid)VipGetPyramidMediane");
      return((Vip_S16BIT)0);
    }
  
  min = medtab[0];
  max = medtab[0];
  
  while (nb > 2)
    { 
      min = medtab[0];
      max = medtab[0];
      for (i=0; i<nb; i++)
	{ 
	  if (medtab[i] < min)  min = medtab[i];
	  if (medtab[i] > max)  max = medtab[i];
	}
      j = 0;
      test1 = 0;
      test2 = 0;
      for (i=0; i<nb; i++)
	{ 
	  if (((medtab[i]!=min) || (test1==1)) && ((medtab[i]!=max)
						   || (test2==1)))
	    medtab[j++] = medtab[i];
	  else 
	    { 
	      if (medtab[i] == min)  test1 = 1;
	      if (medtab[i] == max)  test2 = 1;
	    }
	}
      nb = j;
    }

  return(medtab[j-1]);
}

PyramidVolume *VipGetUpLevelPyramidVolume(PyramidVolume *down, int type)
{ 
  int dxsize, dslicesize, uxsize, uysize, uzsize;
  int sx, sy , sz, x, y, z, nb;
  int winsize, winsizex, winsizey, winsizez, xoffset, yoffset;
  float vx, vy, vz;
  Vip_S16BIT *upptr, *downptr, *imptr, *medtab;
  VipOffsetStruct *uvos, *dvos;
  PyramidVolume *up;

  winsize = 2;

  medtab = (Vip_S16BIT *)VipCalloc(mVipSq3(winsize), sizeof(Vip_S16BIT), NULL);
  if (!medtab)
    {
      VipPrintfError("Error while allocating median table");
      VipPrintfExit("(Vip_pyramid)VipGetUpLevelPyramidVolume");
      return((PyramidVolume *)NULL);
    }

  up = VipCreateEmptyPyramidVolume();
  if (up == NULL)
    { 
      VipPrintfError("Error while allocating Pyramid volume");
      VipPrintfExit("(Vip_pyramid)VipGetUpLevelPyramidVolume");
      VipFree(medtab);
      return((PyramidVolume *)NULL);
    }
  VipCopyVolumeHeader( down->volume, up->volume );

  vx = mVipVolVoxSizeX(down->volume)*(float)winsize;
  vy = mVipVolVoxSizeY(down->volume)*(float)winsize;
  vz = mVipVolVoxSizeZ(down->volume)*(float)winsize;
  VipSet3DVoxelSize(up->volume, vx, vy, vz);
  up->nx = mVipVolSizeX(down->volume)/winsize;  
  up->ny = mVipVolSizeY(down->volume)/winsize;  
  up->nz = mVipVolSizeZ(down->volume)/winsize;  
  up->rx = mVipVolSizeX(down->volume)%winsize;  
  up->ry = mVipVolSizeY(down->volume)%winsize;  
  up->rz = mVipVolSizeZ(down->volume)%winsize;
  VipSet3DSize(up->volume, up->nx+1*(up->rx > 0), up->ny+1*(up->ry > 0),
	       up->nz+1*(up->rz > 0));
  VipSetBorderWidth( up->volume, mVipVolBorderWidth(down->volume) );
  up->winsize = winsize;

  VipAllocateDataSpacePyramidVolume(up);
  uvos = VipGetOffsetStructure(up->volume);
  dvos = VipGetOffsetStructure(down->volume);

  dxsize = dvos->oLine;
  dslicesize = dvos->oSlice;
  uxsize = mVipVolSizeX(up->volume);
  uysize = mVipVolSizeY(up->volume);
  uzsize = mVipVolSizeZ(up->volume);

  upptr = VipGetDataPtr_S16BIT(up->volume)+uvos->oFirstPoint;
  imptr = VipGetDataPtr_S16BIT(down->volume)+dvos->oFirstPoint;

  for (sz=0; sz<uzsize; sz++, upptr+=uvos->oLineBetweenSlice)
    for (sy=0; sy<uysize; sy++, upptr+=uvos->oPointBetweenLine) 
      for (sx=0; sx<uxsize; sx++)
	{ 
	  downptr = imptr+winsize*(sx+sy*dxsize+sz*dslicesize);
	  winsizex = (sx != up->nx) ? winsize : up->rx;
	  winsizey = (sy != up->ny) ? winsize : up->ry;
	  winsizez = (sz != up->nz) ? winsize : up->rz;
	  xoffset = dxsize-winsizex;
	  yoffset = dslicesize-dxsize*winsizey;
	  for (nb=0, z=winsizez; z--; downptr+=yoffset)
	    for (y=winsizey; y--; downptr+=xoffset)
	      for (x=winsizex; x--;)  medtab[nb++] = *downptr++;
	  switch(type)
	      {
	      case PYR_MEDIAN: 
		  *upptr++ = VipGetPyramidMedian(medtab, nb);
		  break;
	      case PYR_MEAN:
		  *upptr++ = VipGetPyramidMean(medtab, nb);
		  break;
	      case PYR_MAX:
		  *upptr++ = VipGetPyramidMax(medtab, nb);
		  break;
	      default:
		  VipPrintfError("Unknown type");
		  VipPrintfExit("VipGetUpLevelPyramidVolume");
		  return(PB);
	      }
	}

  free(medtab);
  return(up);
}

void VipCopyPyramidImageInfoStructure(Pyramid *pyr1, Pyramid *pyr2, int n)
{
  float vx, vy, vz;

  vx = mVipVolVoxSizeX(pyr1->image[n]->volume);
  vy = mVipVolVoxSizeY(pyr1->image[n]->volume);
  vz = mVipVolVoxSizeZ(pyr1->image[n]->volume);
  VipSet3DVoxelSize(pyr2->image[n]->volume, vx, vy, vz);

  pyr2->image[n]->nx = pyr1->image[n]->nx;
  pyr2->image[n]->ny = pyr1->image[n]->ny;
  pyr2->image[n]->nz = pyr1->image[n]->nz;
  pyr2->image[n]->rx = pyr1->image[n]->rx;
  pyr2->image[n]->ry = pyr1->image[n]->ry;
  pyr2->image[n]->rz = pyr1->image[n]->rz;
  pyr2->image[n]->winsize = pyr1->image[n]->winsize;
}

PyramidVolume *VipGetDownLevelPyramidVolume(PyramidVolume *up)
{ 
  int dxsize, dslicesize, uxsize, uysize, uzsize, sx, sy , sz, x, y, z;
  int winsize, winsizex, winsizey, winsizez, xoffset, yoffset;
  float vx, vy, vz;
  Vip_S16BIT *downptr, *imptr, *dimptr;
  VipOffsetStruct *uvos, *dvos;
  PyramidVolume *down;

  winsize = 2;
  down = VipCreateEmptyPyramidVolume();
  VipSet3DSize(down->volume, up->nx*winsize+up->rx, up->ny*winsize+up->ry,
	       up->nz*winsize+up->rz);
  vx = mVipVolVoxSizeX(up->volume)/(float)winsize;
  vy = mVipVolVoxSizeY(up->volume)/(float)winsize;
  vz = mVipVolVoxSizeZ(up->volume)/(float)winsize;
  VipSet3DVoxelSize(down->volume, vx, vy, vz);
  VipSetBorderWidth( down->volume, mVipVolBorderWidth(up->volume) );
  VipCopyVolumeHeader( up->volume, down->volume );


  VipAllocateDataSpacePyramidVolume(down);
  uvos = VipGetOffsetStructure(up->volume);
  dvos = VipGetOffsetStructure(down->volume);

  dxsize = dvos->oLine;
  dslicesize = dvos->oSlice;
  uxsize = mVipVolSizeX(up->volume);
  uysize = mVipVolSizeY(up->volume);
  uzsize = mVipVolSizeZ(up->volume);
  
  dimptr = VipGetDataPtr_S16BIT(down->volume)+dvos->oFirstPoint;
  imptr =VipGetDataPtr_S16BIT(up->volume)+uvos->oFirstPoint;

  for (sz=0; sz<uzsize; sz++, imptr+=uvos->oLineBetweenSlice)
    for (sy=0; sy<uysize; sy++, imptr+=uvos->oPointBetweenLine)
      for (sx=0; sx<uxsize; sx++, imptr++)
	{
	  downptr = dimptr+winsize*(sx+sy*dxsize+sz*dslicesize);
	  winsizex = (sx != up->nx) ? winsize : up->rx;
	  winsizey = (sy != up->ny) ? winsize : up->ry;
	  winsizez = (sz != up->nz) ? winsize : up->rz;
	  xoffset = dxsize-winsizex;
	  yoffset = dslicesize-dxsize*winsizey;
	  for (z=winsizez; z--; downptr+=yoffset)
	    for (y=winsizey; y--; downptr+=xoffset)
	      for (x=winsizex; x--;)  *downptr++ = *imptr;
	}

  return(down);
}

Volume *VipCreateEmptyVolume()
{
  Volume *new;
  
  new = VipDeclare3DVolumeStructure(1, 1, 1, 1.0, 1.0, 1.0, S16BIT, "", 0);
  if (!new) 
    { 
      VipPrintfError("Error while allocating Volume structure");
      VipPrintfExit("(Vip_pyramid)VipCreateEmptyVolume");
      return((Volume *)NULL);
    }

  return(new);
}

Volume *VipConvertPyramidVolumeToVolume(PyramidVolume *pyr)
{
  Volume *new;

  new = VipCreateEmptyVolume();
  if (!new)
    { 
      VipPrintfError("Error while allocating empty Volume structure");
      VipPrintfExit("(Vip_pyramid.c)VipConvertPyramidVolumeToVolume");
      return((Volume *)NULL);
    }

  *new = *pyr->volume;
  VipCopyVolumeHeader( pyr->volume, new );

  return(new);
}

int VipAllocateDataSpacePyramid(Pyramid *pyr, int nb_level)
{ 
  pyr->image = (PyramidVolume **)VipCalloc(nb_level, sizeof(PyramidVolume *), 
					   NULL);
  if (!pyr->image)
    {
      VipPrintfError("Error while allocating Pyramid 'image' volume");
      VipPrintfExit("(Vip_pyramid)VipAllocateDataSpacePyramid");
      return(PB);
    }

  return(OK);
}

Pyramid *VipCreateEmptyPyramid(int NbLevel)
{
  Pyramid *pyr;

  pyr = (Pyramid *)VipCalloc(1, sizeof(Pyramid), "VipCreateEmptyPyramid");
  if (!pyr) 
    { 
      VipPrintfError("Error while allocating Pyramid structure");
      VipPrintfExit("(Vip_pyramid)VipCreateEmptyPyramid");
      return((Pyramid *)NULL);
    }

  pyr->NbLevel = NbLevel;

  if(VipAllocateDataSpacePyramid(pyr,NbLevel)==PB) return(PB);

  return(pyr);
}

Pyramid *VipGetPyramid(Volume *vol, int nlevel, int type)
{
  int i;
  Pyramid *pyr;
  char message[256];

  pyr = VipCreateEmptyPyramid(nlevel);
  if (!pyr)
    { 
      VipPrintfError("Error while allocating Pyramid");
      VipPrintfExit("(Vip_pyramid)VipGetPyramid");
      return((Pyramid *)NULL);
    }

  pyr->NbLevel = nlevel;
  if ((VipAllocateDataSpacePyramid(pyr, nlevel)) == PB)
    {
      VipPrintfError("Error in Pyramid structure fields allocation");
      VipPrintfExit("(Vip_pyramid)VipGetPyramid");
      return((Pyramid *)NULL);
    }

  printf("level : %1d/%1d", 0, nlevel-1);
  fflush(stdout);
  pyr->image[0] = VipConvertVolumeToPyramidVolume(vol);
  if (!pyr->image[0])
    { 
      VipPrintfError("Error while converting volume to Pyramid volume");
      VipPrintfExit("(Vip_pyramid.c)VipGetPyramid");
      VipFree(pyr->image);
      VipFree(pyr);
      return((Pyramid *)NULL);
    }

  for (i=1; i<nlevel; i++)
    {
      printf("\b\b\b%1d/%1d", i, nlevel-1);
      fflush(stdout);
      pyr->image[i] = VipGetUpLevelPyramidVolume(pyr->image[i-1], type);
      if (!pyr->image[i])
	{ 
	  sprintf(message, "Error while getting Pyramid level %d", i);
	  VipPrintfError(message);
	  VipPrintfExit("(Vip_pyramid.c)VipGetPyramid");
	  for (;i--;)  
	    {
	      VipFreeVolume(pyr->image[i]->volume);
	      VipFree(pyr->image[i]);
	    }
	  VipFree(pyr->image);
	  VipFree(pyr);
	  return((Pyramid *)NULL);
	}
    }
  printf("\n");

  return(pyr);
}

void VipFreePyramid(Pyramid *pyr)
{
  int i;

  if (pyr)
    { 
      if (pyr->image)
	{
	  for (i=pyr->NbLevel; i--;)
	    if (pyr->image[i])
	      { 
		if (pyr->image[i]->volume) 
		  VipFreeVolume(pyr->image[i]->volume);
		VipFree(pyr->image[i]);
	      }
	  VipFree(pyr->image);
	}
      VipFree(pyr);
    }
}

int VipConstructPyramid(Pyramid *pyr, int type)
{ 
  int i;
  char message[256];

  if (!pyr->image[0])
    { 
      VipPrintfError("Level 0 volume must be present in Pyramid");
      VipPrintfExit("(Vip_pyramid.c)VipConstructPyramid");
      if (pyr->image)  VipFree(pyr->image);
      if (pyr)  VipFree(pyr);
      return(PB);
    }

  printf("level : %1d/%1d", 0, pyr->NbLevel-1);
  fflush(stdout);
  for (i=1; i<pyr->NbLevel; i++)
    { 
      printf("\b\b\b%1d/%1d", i, pyr->NbLevel-1);
      fflush(stdout);
      pyr->image[i] = VipGetUpLevelPyramidVolume(pyr->image[i-1], type);
      if (!pyr->image[i])
	{
	  sprintf(message, "Error while getting Pyramid level %d", i);
	  VipPrintfError(message);
	  VipPrintfExit("(Vip_pyramid.c)VipConstructPyramid");
	  for (;i--;)  
	    { 
	      VipFreeVolume(pyr->image[i]->volume);
	      VipFree(pyr->image[i]);
	    }
	  VipFree(pyr->image);
	  VipFree(pyr);
	  return(PB);
	}
    }
  printf("\n");

  return(OK);
}

Pyramid *VipReadVolumeInPyramid(char *name, int borderWidth, int NbLevel)
{
  Pyramid *pyr;
  VIP_DEC_VOLUME(vol);

  if (borderWidth < 1)
  {
    VipPrintfError("borderWidth must not be null");
    VipPrintfExit("(Vip_pyramid.c)VipReadVolumeInPyramid");
    return((Pyramid *)NULL);
  }

  vol = VipReadVolumeWithBorder(name, borderWidth);
  if (!vol)
  { 
    VipPrintfError("Error while reading volume file");
    VipPrintfExit("(Vip_pyramid.c)VipReadVolumeInPyramid");
    return((Pyramid *)NULL);
  }
  

  pyr = VipCreateEmptyPyramid(NbLevel);
  if (!pyr)
  { 
    VipPrintfError("Error while allocating Pyramid structure");
    VipPrintfExit("(Vip_pyramid.c)VipReadVolumeInPyramid");
    VipFreeVolume(vol);
    return((Pyramid *)NULL);
  }

  /*jeff, clearer stuff in inside previous function...
  if ((VipAllocateDataSpacePyramid(pyr, NbLevel)) == PB)
  { 
    VipPrintfError("Error in Pyramid structure fields allocation");
    VipPrintfExit("(Vip_pyramid)VipReadVolumeInPyramid");
    VipFreeVolume(vol);
    return((Pyramid *)NULL);
  }
  */
  pyr->image[0] = VipConvertVolumeToPyramidVolume(vol);
  if (!pyr->image[0])
  { 
    VipPrintfError("Error while converting volume to Pyramid volume");
    VipPrintfExit("(Vip_pyramid.c)VipReadVolumeInPyramid");
    VipFree(pyr->image);
    VipFree(pyr);
    return((Pyramid *)NULL);
  }

  return(pyr);
}

Pyramid *VipReadTopLevelVolumeInPyramid(char *name, int borderWidth, 
					int NbLevel)
{ Pyramid *pyr;
 VIP_DEC_VOLUME(vol);

 if (borderWidth < 1)
   { 
     VipPrintfError("borderWidth must not be null");
     VipPrintfExit("(Vip_pyramid.c)VipReadTopLevelVolumeInPyramid");
     return((Pyramid *)NULL);
   }

 vol = VipReadVolumeWithBorder(name, borderWidth);
 if (!vol)
   { 
     VipPrintfError("Error while reading volume file");
     VipPrintfExit("(Vip_pyramid.c)VipReadTopLevelVolumeInPyramid");
     return((Pyramid *)NULL);
   }
  

 pyr = VipCreateEmptyPyramid(NbLevel);
 if (!pyr)
   { 
     VipPrintfError("Error while allocating Pyramid structure");
     VipPrintfExit("(Vip_pyramid.c)VipReadTopLevelVolumeInPyramid");
     VipFreeVolume(vol);
     return((Pyramid *)NULL);
   }

 if ((VipAllocateDataSpacePyramid(pyr, NbLevel)) == PB)
   { 
     VipPrintfError("Error in Pyramid structure fields allocation");
     VipPrintfExit("(Vip_pyramid)VipReadTopLevelVolumeInPyramid");
     VipFreeVolume(vol);
     return((Pyramid *)NULL);
   }

 pyr->image[NbLevel-1] = VipConvertVolumeToPyramidVolume(vol);
 if (!pyr->image[NbLevel-1])
   { 
     VipPrintfError("Error while converting volume to Pyramid volume");
     VipPrintfExit("(Vip_pyramid.c)VipReadTopLevelVolumeInPyramid");
     VipFree(pyr->image);
     VipFree(pyr);
     return((Pyramid *)NULL);
   }

 return(pyr);
}

Pyramid *VipReadVolumeInPyramidAndCreatePyramid(char *name, int border, 
						int NbLevel, int type)
{ 
  Pyramid *pyr;

  if (border < 1)
    { 
      VipPrintfError("borderWidth must not be null");
      VipPrintfExit("(Vip_pyramid.c)VipReadVolumeInPyramidAndCreatePyramid");
      return((Pyramid *)NULL);
    }

  pyr = VipReadVolumeInPyramid(name, border, NbLevel);
  if (!pyr)
    { 
      VipPrintfError("Error while reading volume in Pyramid");
      VipPrintfExit("(Vip_pyramid.c)VipReadVolumeInPyramidAndCreatePyramid");
      return((Pyramid *)NULL);
    }

  if (VipConstructPyramid(pyr, type) == PB)
    { 
      VipPrintfError("Error while constructing Pyramid");
      VipPrintfExit("(Vip_pyramid.c)VipReadVolumePyramidAndCreatePyramid");
      VipFreeVolume(pyr->image[0]->volume);
      VipFree(pyr->image[0]);
      VipFree(pyr->image);
      VipFree(pyr);
      return((Pyramid *)NULL);
    }

  return(pyr);
}

Pyramid *VipPutVolumeInPyramidAndCreatePyramid(Volume *vol, 
						int NbLevel, int type)
{ 
  Pyramid *pyr;

  if(!vol)
      {
	  VipPrintfError("empty arg");
	  VipPrintfExit("(Vip_pyramid.c)VipPutVolumeInPyramidAndCreatePyramid");
	  return((Pyramid *)NULL);
    }

  if (mVipVolBorderWidth(vol) < 1)
    { 
      /*

      VipPrintfError("borderWidth must not be null");
      VipPrintfExit("(Vip_pyramid.c)VipPutVolumeInPyramidAndCreatePyramid");
      return((Pyramid *)NULL);

      a priori ce notion de borderwidth peut etre nulle VF 19/08/99 */
      VipPrintfWarning("(Vip_pyramid.c)VipPutVolumeInPyramidAndCreatePyramid:borderwidth is 0");
    }


  pyr = VipCreateEmptyPyramid(NbLevel);
  if (!pyr)
  { 
    VipPrintfError("Error while allocating Pyramid structure");
    VipPrintfExit("(Vip_pyramid.c)VipPutVolumeInPyramidAndCreatePyramid");
    return((Pyramid *)NULL);
  }

  pyr->image[0] = VipConvertVolumeToPyramidVolume(vol);
  if (!pyr->image[0])
  { 
    VipPrintfError("Error while converting volume to Pyramid volume");
    VipPrintfExit("(Vip_pyramid.c)VipPutVolumeInPyramidAndCreatePyramid");
    VipFree(pyr->image);
    VipFree(pyr);
    return((Pyramid *)NULL);
  }


  if (VipConstructPyramid(pyr, type) == PB)
    { 
      VipPrintfError("Error while constructing Pyramid");
      VipPrintfExit("(Vip_pyramid.c)VipPutVolumeInPyramidAndCreatePyramid");
      VipFreeVolume(pyr->image[0]->volume);
      VipFree(pyr->image[0]);
      VipFree(pyr->image);
      VipFree(pyr);
      return((Pyramid *)NULL);
    }

  return(pyr);
}

int VipWritePyramidImageVolume(char *name, Pyramid *pyr, int level)
{
  char filename[VIP_NAME_MAXLEN];

  if ((level < 0) || (level >= pyr->NbLevel))
    { 
      VipPrintfError("Wrong level value");
      VipPrintfExit("(Vip_pyramid.c)VipWritePyramidImageVolume");
      return(PB);
    }

  if (name ==  NULL)
    { 
      VipPrintfError("No file name specified");
      VipPrintfExit("(Vip_pyramid.c)VipWritePyramidImageVolume");
      return(PB);
    }

  if (pyr == NULL)
    { 
      VipPrintfError("Pyramid does not exist");
      VipPrintfExit("(Vip_pyramid.c)VipWritePyramidImageVolume");
      return(PB);
    }

  if (!pyr->image[level] || !pyr->image[level]->volume)
    { 
      sprintf(filename, "Level %d Pyramid image volume does not exist", level);
      VipPrintfError(filename);
      VipPrintfExit("(Vip_pyramid.c)VipWritePyramidImageVolume");
      return(PB);
    }

  sprintf(filename, "%s_%d", name, level);
  printf("Writing level %d image volume : %s\n", level, filename);
  if (VipWriteVolume(pyr->image[level]->volume, filename) == PB)
    { 
      VipPrintfError("Error while writing Pyramid image file");
      VipPrintfExit("(Vip_pyramid.c)VipWritePyramidImageVolume");
      return(PB);
    }

  return(OK);
}

int VipWritePyramidVolumes(char *name, Pyramid *pyr)
{
  int i=pyr->NbLevel;
  char message[256];

  if (name == NULL)
    { 
      VipPrintfError("No file name specified");
      VipPrintfExit("(Vip_pyramid.c)VipWritePyramidVolumes");
      return(PB);
    }

  if (pyr == NULL)
    { 
      VipPrintfError("Pyramid does not exist");
      VipPrintfExit("(Vip_pyramid.c)VipWritePyramidVolumes");
      return(PB);
    }

  for (;i--;)
    if (pyr->image[i]!=NULL && pyr->image[i]->volume != NULL)  
      { 
	if (VipWritePyramidImageVolume(name, pyr, i) == PB)
	  { 
	    sprintf(message, "Error while writing level %d Image volume", i);
	    VipPrintfError(message);
	    VipPrintfExit("(Vip_pyramid.c)VipWritePyramidVolumes");
	    return(PB);
	  }
      }

  return(OK);
}
