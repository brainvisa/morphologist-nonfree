/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_linresamp.c      * TYPE     : Source
 * AUTHOR      : FROUIN V.           * CREATION : 28/06/2000
 * VERSION     : 1.0                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : PC Linux
 *****************************************************************************
 *
 * DESCRIPTION : Interface call
 * 
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

#include <vip/linresamp.h>
#include <vip/linresamp_static.h>

/*------------------------------------------------------------------*/
Volume *VipLinearResamp(
	Volume *vol,
	Volume *templatev,
	VipDeplacement *dep)
/*------------------------------------------------------------------*/  
{
   return( VipLinearResampCustom( vol, templatev, dep, VIP_LINRESAMP_ZERO) );
}
