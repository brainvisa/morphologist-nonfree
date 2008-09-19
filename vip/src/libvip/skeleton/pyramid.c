/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : skeleton             * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 27/07/99
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : Homotopic snake on pyramid.
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


#include <vip/math.h>
#include <vip/pyramid.h>
#include <vip/skeleton.h>
#include <vip/skeleton_static.h>

/*--------------------------------------------------------------------*/
static int  VipConditionalMerge(Volume *vol, Volume *merge, int allowed, int mergevalue);
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
static int PutBoundingBoxInRightVol(Volume *object, Volume *bbox, int val, int inside, int outside);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
Pyramid *VipCreateBoundingBoxLabelPyramid(Pyramid *pdata, int nlevel, int object, int inside, int outside)
/*--------------------------------------------------------------------*/
{
    Pyramid *pyrlab;
    int forbiden = -77;

    /*bidon pour warning*/
    nlevel = 3;

    if(!pdata) 
	{
	    VipPrintfError("Null pyramid");
	    VipPrintfExit("VipCreateBoundingBoxLabelPyramid");
	    return(PB);
	}

    printf("Creating label pyramid...\n");
    pyrlab = VipCreateEmptyPyramid(3); 
    if(pyrlab==PB) return(PB);

    printf("--------------\n");
    printf("Highest level:\n");
    pyrlab->image[2] = VipCreateVoidPyramidVolumeLikePyramid(pdata->image[2]);

    PutBoundingBoxInRightVol(pdata->image[2]->volume,pyrlab->image[2]->volume,object,inside,outside);
    /*
    VipWriteVolume(pyrlab->image[2]->volume,"bbox");
    */
    VipConditionalMerge(pyrlab->image[2]->volume,pdata->image[2]->volume,inside,forbiden);
    /*
    VipWriteVolume(pyrlab->image[2]->volume,"merge");
    */

    VipHomotopicGeodesicDilation(pyrlab->image[2]->volume, 100, object, inside, forbiden, outside, FRONT_RANDOM_ORDER);

    /*
    VipWriteVolume(pyrlab->image[2]->volume,"dilation");
    */

    VipHomotopicGeodesicErosionFromOutside(pyrlab->image[2]->volume, 100, object, forbiden, outside);

    /*
    VipWriteVolume(pyrlab->image[2]->volume,"erosion");
    */
    VipChangeIntLabel(pyrlab->image[2]->volume,forbiden,inside);

    /*second level*/ 

    printf("-------------\n");
    printf("Middle level:\n");

    pyrlab->image[1] = VipGetDownLevelPyramidVolume(pyrlab->image[2]);
    VipCopyPyramidImageInfoStructure( pdata,pyrlab,1); 

    VipConditionalMerge(pyrlab->image[1]->volume,pdata->image[1]->volume,inside,forbiden);

    VipHomotopicGeodesicDilation(pyrlab->image[1]->volume, 100, object, inside, forbiden, outside, FRONT_RANDOM_ORDER);

    VipHomotopicGeodesicErosionFromOutside(pyrlab->image[1]->volume, 100, object, forbiden, outside);

    VipChangeIntLabel(pyrlab->image[1]->volume,forbiden,inside);

   /*last level*/
    printf("----------------------------------\n");
    printf("Lowest level (highest resolution):\n");

    pyrlab->image[0] = VipGetDownLevelPyramidVolume(pyrlab->image[1]);
    VipCopyPyramidImageInfoStructure( pdata,pyrlab,0); 

    VipConditionalMerge(pyrlab->image[0]->volume,pdata->image[0]->volume,inside,forbiden);

    VipHomotopicGeodesicDilation(pyrlab->image[0]->volume, 100, object, inside, forbiden, outside, FRONT_RANDOM_ORDER);

    VipHomotopicGeodesicErosionFromOutside(pyrlab->image[0]->volume, 100, object, forbiden, outside);

    VipChangeIntLabel(pyrlab->image[0]->volume,forbiden,inside);

    return(pyrlab);
}

/*--------------------------------------------------------------------*/
static int PutBoundingBoxInRightVol(Volume *object, Volume *bbox, int val, int inside, int outside)
/*--------------------------------------------------------------------*/
{
    Vip3DPoint_S16BIT bbmin, bbmax;
    VipOffsetStruct *vos;
    Vip_S16BIT *optr, *bbptr;
    int x,y,z;

    if(!object || !bbox)
	{
	    VipPrintfError("Null pyramid");
	    VipPrintfExit("PutBoundingBoxInRightVol");
	    return(PB);
	}


    if(VipTestEqualBorderWidth(object,bbox)==PB)
	{
	    VipPrintfExit("(skeleton)PutBoundingBoxInRightVol");
	    return(PB);
	}

    bbmin.x = 10000;
    bbmin.y = 10000;
    bbmin.z = 10000;
    bbmax.x = -10000;
    bbmax.y = -10000;
    bbmax.z = -10000;

    vos = VipGetOffsetStructure(object);
    if(!vos) return(PB);

    optr = VipGetDataPtr_S16BIT(object)+vos->oFirstPoint;
  
    for(z=0;z<mVipVolSizeZ(object);z++)
	{
	    for(y=0;y<mVipVolSizeY(object);y++)
		{
		    for(x=0;x<mVipVolSizeX(object);x++)
			{
			    if(*optr)
				{
				    if(x<bbmin.x) bbmin.x = x;
				    if(y<bbmin.y) bbmin.y = y;
				    if(z<bbmin.z) bbmin.z = z;
				    if(x>bbmax.x) bbmax.x = x;
				    if(y>bbmax.y) bbmax.y = y;
				    if(z>bbmax.z) bbmax.z = z;
				}
			    optr++;
			}
		    optr+=vos->oPointBetweenLine;
		}
	    optr += vos->oLineBetweenSlice;
	}
    if(bbmin.x>9999)
	{
	    VipPrintfError("Empty object");
	    VipPrintfExit("PutBoundingBoxInRightVol");
	    return(PB);
	}

    if(bbmin.x==0)
	{
	  /*VipPrintfWarning("Bounding box reaching x=0")*/;
	}
    else bbmin.x -=1;
    if(bbmin.y==0)
	{
	    /*VipPrintfWarning("Bounding box reaching y=0")*/;
	}
    else bbmin.y -=1;
    if(bbmin.z==0)
	{
	    /*VipPrintfWarning("Bounding box reaching z=0")*/;
	}
    else bbmin.z -=1;
    if(bbmax.x==(mVipVolSizeX(object)-1))
	{
	    /*VipPrintfWarning("Bounding box reaching max x")*/;
	}
    else bbmax.x +=1;
    if(bbmax.y==(mVipVolSizeY(object)-1))
	{
	    /*VipPrintfWarning("Bounding box reaching max y")*/;
	}
    else bbmax.y +=1;
    if(bbmax.z==(mVipVolSizeZ(object)-1))
	{
	    /*VipPrintfWarning("Bounding box reaching max z")*/;
	}
    else bbmax.z +=1;

    printf("Bounding box (top of the pyramid): x:[%d-%d], y:[%d-%d], z:[%d-%d]\n",
	   bbmin.x, bbmax.x, bbmin.y, bbmax.y, bbmin.z, bbmax.z); 

    bbptr = VipGetDataPtr_S16BIT(bbox)+vos->oFirstPoint;
  
    for(z=0;z<mVipVolSizeZ(object);z++)
	{
	    for(y=0;y<mVipVolSizeY(object);y++)
		{
		    for(x=0;x<mVipVolSizeX(object);x++)
			{
			    if((x>=bbmin.x && x<=bbmax.x)
			       &&(y>=bbmin.y && y<=bbmax.y) 
			       &&(z>=bbmin.z && z<=bbmax.z))
				{
				    if((x==bbmin.x)
				       ||(y==bbmin.y) 
				       ||(z==bbmin.z)
				       ||(x==bbmax.x)
				       ||(y==bbmax.y) 
				       ||(z==bbmax.z))
					{
					    *bbptr = val;
					}
				    else 
					{
					    *bbptr = inside;
					}
				}
			    else *bbptr=outside;			    
			    bbptr++;
			}
		    bbptr+=vos->oPointBetweenLine;
		}
	    bbptr += vos->oLineBetweenSlice;
	}

    return(OK);

 
}

/*--------------------------------------------------------------------*/
static int  VipConditionalMerge(Volume *vol, Volume *merge, int allowed, int mergevalue)
/*--------------------------------------------------------------------*/
{
    VipOffsetStruct *vos;
    int ix, iy, iz;
    Vip_S16BIT *ptr, *mptr;

    if(!vol || !merge || VipTestEqualBorderWidth(vol,merge)==PB)
	{
	    VipPrintfError("arg pb");
	    VipPrintfExit("VipConditionalMerge");
	    return(PB);
	}

    vos = VipGetOffsetStructure(vol);
    if(!vos) return(PB);

    ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
    mptr = VipGetDataPtr_S16BIT( merge ) + vos->oFirstPoint;

    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	{
	    for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
		{
		    for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
			{
			    if(*ptr==allowed)
				if(*mptr) *ptr = mergevalue;
			    
			    ptr++;
			    mptr++;
			}
		    ptr += vos->oPointBetweenLine;  /*skip border points*/
		    mptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	    ptr += vos->oLineBetweenSlice; /*skip border lines*/
	    mptr += vos->oLineBetweenSlice; /*skip border lines*/
	}

    return(OK);
}
