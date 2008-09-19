/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_label_contour.c  * TYPE     : Function
 * AUTHOR      : POUPON F.            * CREATION : 14/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Recupere le(s) contour(s) a partir d'un label ou d'une
 *               liste de labels
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_label.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/label.h>

int VipSetBorderLevel_S16BIT(Volume*,short);

Vip3DBucket_S16BIT *VipGetSingleLabelContour(Volume *vol, int label, int connexite)
{ int i, j, k, loop, cpt, xsize, ysize, zsize, bord;
  Vip3DBucket_S16BIT *bucket;
  Vip3DPoint_S16BIT *frontptr;
  Vip_S16BIT *imptr;
  VipConnectivityStruct *vcs;
  VipOffsetStruct *vos;

  vos = VipGetOffsetStructure(vol);
  imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  if ((bord = mVipVolBorderWidth(vol)) != 0)  VipSetBorderLevel_S16BIT(vol, -1);

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  bucket = VipAlloc3DBucket_S16BIT(100000);
  bucket->n_points = 0;

  vcs = VipGetConnectivityStruct(vol, connexite);
  frontptr = bucket->data;
 
  for (k=0; k<zsize; k++, imptr+=vos->oLineBetweenSlice)
    for (j=0; j<ysize; j++, imptr+=vos->oPointBetweenLine)
      for (i=0; i<xsize; i++, imptr++)
      { if (*imptr == label)
	{ for (cpt=0, loop=vcs->nb_neighbors; (loop--) && ((*(imptr+vcs->offset[loop]) == label) 
							|| (*(imptr+vcs->offset[loop]) == -1)); cpt++);
          if (cpt != vcs->nb_neighbors)
	  { if (bucket->n_points == bucket->size)  
            { VipIncrease3DBucket_S16BIT(bucket, 10000);
              frontptr = bucket->data+bucket->n_points;
	    }
            frontptr->x = i;
            frontptr->y = j;
            frontptr->z = k;
            frontptr++;
            bucket->n_points++;
	  }
	}
      }

  return(bucket);
}

Volume *VipGetMultipleLabelContourBucketsAndVolume(Volume *vol, Vip3DBucketVector_S16BIT *bucket,
			        		   int *label, int *label_col, int nb_label, int connexite)
{ VIP_DEC_VOLUME(volout);
  VipOffsetStruct *vos;
  Vip_S16BIT *imptr;
  Vip3DBucket_S16BIT **bucketptr;
  Vip3DPoint_S16BIT *frontptr;
  int loop, i, xsize, slicesize;

  if ((volout = VipDuplicateVolumeStructure(vol, "")) == NULL)
  { VipPrintfError("volume structure allocation error");
    VipPrintfExit("(Vip_label_contour.c)VipGetMultipleLabelContourBucketsAndVolume");
    return((Volume *)NULL);
  }
  VipAllocateVolumeData(volout);

  vos = VipGetOffsetStructure(volout);
  imptr = VipGetDataPtr_S16BIT(volout)+vos->oFirstPoint;
  bucketptr = VipGet3DBucketVectorPtr_S16BIT(bucket);
  xsize = vos->oLine;
  slicesize = vos->oSlice;

  printf("-> label %4d", label[0]);
  fflush(stdout);
  for (loop=0; loop<nb_label; loop++)
  { printf("\b\b\b\b%4d", label[loop]);
    fflush(stdout);
    bucketptr[loop] = VipGetSingleLabelContour(vol, label[loop], connexite);
    frontptr = bucketptr[loop]->data;
    for (i=bucketptr[loop]->n_points; i--; frontptr++)
      *(imptr+frontptr->x+frontptr->y*xsize+frontptr->z*slicesize) = label_col[loop];
  }
  printf("\n");

  return(volout);
}

Volume *VipGetMultipleLabelContourVolume(Volume *vol, int *label, int *label_col, int nb_label, int connexite)
{ VIP_DEC_VOLUME(volout);
  Vip3DBucketVector_S16BIT *bucket;

  bucket = VipDeclareVectorOf3DBucket_S16BIT(nb_label);
  if (!bucket)
  { VipPrintfExit("(Vip_label_contour.c)VipGetMultipleLabelContourVolume");
    return((Volume *)NULL);
  }
 
  volout = VipGetMultipleLabelContourBucketsAndVolume(vol, bucket, label, label_col, nb_label, connexite);
  if (volout == (Volume *)NULL)
  { VipPrintfExit("(Vip_label_contour.c)VipGetMultipleLabelContourVolume");
    return((Volume *)NULL);
  }  

  VipFreeVectorOf3DBucket_S16BIT(bucket);
  
  return(volout);
}
