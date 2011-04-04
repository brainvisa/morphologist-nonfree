/****************************************************************************
# PROJECT     : V.I.P. Library
# MODULE NAME : topology_clas.c  * TYPE     : sources
# AUTHOR      : MANGIN JF            * CREATION : 14/04/1997
# VERSION     : 1.1                  * REVISION :
# LANGUAGE    : C                    * EXAMPLE  :
# DEVICE      : UltraSparc
#****************************************************************************
#
# DESCRIPTION : Topological classification of a whole volume
#
#****************************************************************************
#
# USED MODULES : 
#
#****************************************************************************
# REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
#--------------------|--------------|----------------------------------------
#              / /   |              |
#***************************************************************************/

#include <vip/topology.h>
#include <vip/topology_static.h>

/*----------------------------------------------------------------------------*/
int VipTopologicalClassificationForLabel(
Volume *vol,
int label)
/*----------------------------------------------------------------------------*/

{
  VipOffsetStruct *vos;
  Vip_S16BIT *imptr,*tempptr;
  int i, j, k;
  Topology26Neighborhood *topo26;
  Volume *temp;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(topology)VipTopologicalClassificationForLabel");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipTopologicalClassificationForLabel is only implemented for S16BIT volume");
      VipPrintfExit("(topology)VipTopologicalClassificationForLabel");
      return(PB);
    }
 if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipTopologicalClassificationForLabel is only implemented with border");
      VipPrintfExit("(topology)VipTopologicalClassificationForLabel");
      return(PB);
    }

 temp = VipCopyVolume(vol,"classif");
 if(temp==PB) return(PB);

 vos = VipGetOffsetStructure(vol);
 if(vos==PB) return(PB);

 imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
 tempptr = VipGetDataPtr_S16BIT(temp)+vos->oFirstPoint;
 
 VipSetBorderLevel( vol, -123 ); /*place object in empty area*/

 topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

 for (k=mVipVolSizeZ(vol); k--; imptr+=vos->oLineBetweenSlice, tempptr +=vos->oLineBetweenSlice)
   for (j=mVipVolSizeY(vol); j--; imptr+=vos->oPointBetweenLine,tempptr+=vos->oPointBetweenLine)
     for (i=mVipVolSizeX(vol); i--; imptr++, tempptr++)
       { 
	 if(*imptr==label)
	   *tempptr = VipComputeTopologicalClassificationForLabel_S16BIT(topo26,imptr,label);
       }

 if(VipTransferVolumeData(temp,vol)==PB) return(PB);

 VipFreeVolume(temp);
 return(OK);
}

/*----------------------------------------------------------------------------*/
int VipTopologicalClassificationForLabelComplement(
Volume *vol,
int label)
/*----------------------------------------------------------------------------*/

{
  VipOffsetStruct *vos;
  Vip_S16BIT *imptr, *tempptr;
  int i, j, k;
  Topology26Neighborhood *topo26;
  int empty;
  Volume *temp;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(topology)VipTopologicalClassificationForLabelComplement");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipTopologicalClassificationForLabelComplement is only implemented for S16BIT volume");
      VipPrintfExit("(topology)VipTopologicalClassificationForLabelComplement");
      return(PB);
    }
 if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipTopologicalClassificationForLabelComplement is only implemented with border");
      VipPrintfExit("(topology)VipTopologicalClassificationForLabelComplement");
      return(PB);
    }

 temp = VipCopyVolume(vol,"classif");
 if(temp==PB) return(PB);

 vos = VipGetOffsetStructure(vol);
 if(vos==PB) return(PB);

 imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
 tempptr = VipGetDataPtr_S16BIT(temp)+vos->oFirstPoint;

 empty = label -1;
 if(empty<0) empty = label+1;
 VipSetBorderLevel( vol, empty ); /*place object in non label area*/

 topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

 for (k=mVipVolSizeZ(vol); k--; imptr+=vos->oLineBetweenSlice, tempptr +=vos->oLineBetweenSlice)
   for (j=mVipVolSizeY(vol); j--; imptr+=vos->oPointBetweenLine,tempptr+=vos->oPointBetweenLine)
     for (i=mVipVolSizeX(vol); i--; imptr++, tempptr++)
       { 
	 if(*imptr!=label)
	   *tempptr = VipComputeTopologicalClassificationForLabelComplement_S16BIT(topo26,imptr,label);
       }

 if(VipTransferVolumeData(temp,vol)==PB) return(PB);

 VipFreeVolume(temp);
 return(OK);
}

/*----------------------------------------------------------------------------*/
int VipTopologicalClassificationForTwoLabelComplement(
Volume *vol,
int inside,
int outside)
/*----------------------------------------------------------------------------*/

{
  VipOffsetStruct *vos;
  Vip_S16BIT *imptr, *tempptr;
  int i, j, k;
  Topology26Neighborhood *topo26;
  Volume *temp;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(topology)VipTopologicalClassificationForTwoLabelComplement");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipTopologicalClassificationForTwoLabelComplement is only implemented for S16BIT volume");
      VipPrintfExit("(topology)VipTopologicalClassificationForTwoLabelComplement");
      return(PB);
    }
 if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipTopologicalClassificationForTwoLabelComplement is only implemented with border");
      VipPrintfExit("(topology)VipTopologicalClassificationForTwoLabelComplement");
      return(PB);
    }

 temp = VipCopyVolume(vol,"classif");
 if(temp==PB) return(PB);

 vos = VipGetOffsetStructure(vol);
 if(vos==PB) return(PB);

 imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
 tempptr = VipGetDataPtr_S16BIT(temp)+vos->oFirstPoint;

 VipSetBorderLevel( vol, outside ); /*place object in non label area*/

 topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

 for (k=mVipVolSizeZ(vol); k--; imptr+=vos->oLineBetweenSlice, tempptr +=vos->oLineBetweenSlice)
   for (j=mVipVolSizeY(vol); j--; imptr+=vos->oPointBetweenLine,tempptr+=vos->oPointBetweenLine)
     for (i=mVipVolSizeX(vol); i--; imptr++, tempptr++)
       { 
	 if(*imptr!=inside && *imptr!=outside)
	   *tempptr = VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(topo26,imptr,inside,outside);
       } 

 if(VipTransferVolumeData(temp,vol)==PB) return(PB);

 VipFreeVolume(temp);
 return(OK);
}

/*----------------------------------------------------------------------------*/
int VipTopologicalClassificationForTwoLabel(
Volume *vol,
int inside,
int outside)
/*----------------------------------------------------------------------------*/

{
  VipOffsetStruct *vos;
  Vip_S16BIT *imptr, *tempptr;
  int i, j, k;
  Topology26Neighborhood *topo26;
  Volume *temp;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(topology)VipTopologicalClassificationForTwoLabel");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipTopologicalClassificationForTwoLabel is only implemented for S16BIT volume");
      VipPrintfExit("(topology)VipTopologicalClassificationForTwoLabel");
      return(PB);
    }
 if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipTopologicalClassificationForTwoLabel is only implemented with border");
      VipPrintfExit("(topology)VipTopologicalClassificationForTwoLabel");
      return(PB);
    }

 temp = VipCopyVolume(vol,"classif");
 if(temp==PB) return(PB);

 vos = VipGetOffsetStructure(vol);
 if(vos==PB) return(PB);

 imptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
 tempptr = VipGetDataPtr_S16BIT(temp)+vos->oFirstPoint;

 VipSetBorderLevel( vol, outside ); /*place object in non label area*/

 topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

 for (k=mVipVolSizeZ(vol); k--; imptr+=vos->oLineBetweenSlice, tempptr +=vos->oLineBetweenSlice)
   for (j=mVipVolSizeY(vol); j--; imptr+=vos->oPointBetweenLine,tempptr+=vos->oPointBetweenLine)
     for (i=mVipVolSizeX(vol); i--; imptr++, tempptr++)
       { 
	 if(*imptr!=inside && *imptr!=outside)
	   *tempptr = VipComputeTopologicalClassificationForTwoLabel_S16BIT(topo26,imptr,inside,outside);
       } 

 if(VipTransferVolumeData(temp,vol)==PB) return(PB);

 VipFreeVolume(temp);
 return(OK);
}
