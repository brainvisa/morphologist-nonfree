/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : voi_protected.c      * TYPE     : Source
 * AUTHOR      : FROUIN V.            * CREATION : 27/09/1999
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Systeme de gestion de volume d'interet
 *               dans le contexte de Vip. Fonction 'Protected'
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

#include <vip/voi.h>
#define _PREF_ERROR (voi_protected.c)

int VipVoiTest( VipVoi *voi )
{
  if (! voi )
    {
      VipPrintfError("_PREF_ERRORVipVoiTest: Null arg VipVoi...\n");
      return(  PB );
    }
  if ( voi->NbVoi == 0 )
    {
      VipPrintfError("_PREF_ERRORVipVoiTest: 0 voi in VipVoi struct...\n");
      return(  PB );
    }
  return( OK );
}

int VipVoiSetDeplacement( VipVoi *voi, VipDeplacement *dep)
{

  voi->deplacement = dep;
  
  return( OK );
}

VipVoi *VipVoiAllocate( Volume *label_vol, int *label, int nb )
{
  VipVoi *voi;

  voi = VipCalloc(1, sizeof(VipVoi), "");
  if (!voi) 
    {
      VipPrintfError("_PREF_ERRORVipAllocate: Error allocating VipVoi...\n");
      return( (VipVoi *) PB);
    }
  voi->NbVoi = nb;
  voi->VoiImage = label_vol;
  voi->label = label; 
  voi->deplacement = NULL;

  if (VipVoiAllocateBucket( voi ) == PB) return ( PB );
  if (VipVoiAllocateDepla( voi ) == PB)  return ( PB );
  if (VipVoiAllocateOffset( voi ) == PB) return ( PB );
  if (VipVoiAllocateStat( voi ) == PB) return ( PB );
  if (VipVoiAllocateStatCorr( voi ) == PB) return ( PB );
  if (VipVoiAllocateMes( voi ) == PB) return ( PB );

  return( voi );
}


int VipVoiAllocateBucket(
    VipVoi    *voi
    )
{
  if ( VipVoiTest( voi ) == PB ) return(  PB );


  voi->ListeDePaquetDePoint = 
    (Vip3DBucket_S16BIT **) VipCalloc(voi->NbVoi,
                                      sizeof(Vip3DBucket_S16BIT *),"");
  if ( !voi->ListeDePaquetDePoint )
    {
      VipPrintfError("_PREF_ERRORVipVoiAllocateBucket: Error allocating VipVoi internal...\n");
      return(  PB );
    }


  return (OK);
}


int VipVoiAllocateDepla(
    VipVoi     *voi
    )
{

  if ( VipVoiTest( voi ) == PB ) return(  PB );

  voi->ListeDePaquetDeDeplaEntier =
    (Vip3DBucket_U16BIT **) VipCalloc(voi->NbVoi,
				      sizeof( Vip3DBucket_U16BIT * ), "");
  if (!(voi->ListeDePaquetDeDeplaEntier))
    {
      VipPrintfError("_PREF_ERRORVipVoiAllocateDepla: Error allocating VipVoi internal...\n");
      return(  PB );
    }

  return( OK );
}
int VipVoiAllocateDeplaData(
    VipVoi     *voi
    )
{
  int i,nb;
  
  for(i=0; i < voi->NbVoi; i++)
    {
      nb = (*(voi->ListeDePaquetDePoint + i))->n_points;
      *(voi->ListeDePaquetDeDeplaEntier + i) = (Vip3DBucket_U16BIT *) 
                                                VipAlloc3DBucket_U16BIT( nb );
      if ( !*(voi->ListeDePaquetDeDeplaEntier + i)) 
	{
	  return(PB);
	}
    }
  
  return( OK );
}

int VipVoiResetDeplaData(
    VipVoi     *voi
    )
{
  int i;
  
  for(i=0; i < voi->NbVoi; i++)
    {
      ( *(voi->ListeDePaquetDeDeplaEntier + i) )->n_points = 0;
    }
  
  return( OK );
}
int VipVoiAllocateOffset(
    VipVoi     *voi
    )
{
  if ( VipVoiTest( voi ) == PB ) return(  PB );

  voi->ListeDePaquetDoffset =
    (VipIntBucket **) VipCalloc(voi->NbVoi,sizeof(VipIntBucket *),NULL);
  if ( !voi->ListeDePaquetDoffset ) 
    {
      VipPrintfError("_PREF_ERRORVipVoiAllocateOffset: Error allocating VipVoi internal...\n");
      return(  PB );
    }

  return( OK );
}


int VipVoiAllocateOffsetData(VipVoi     *voi)
{
  int i,nb;


  for(i=0; i < voi->NbVoi; i++)
    {
      nb = (*(voi->ListeDePaquetDePoint+i))->n_points;
      *(voi->ListeDePaquetDoffset + i)=(VipIntBucket *)VipAllocIntBucket( nb );
      if ( !*(voi->ListeDePaquetDoffset + i) ) 
	{
	  return(PB);
	}
    }
  return( OK );
}

int VipVoiResetOffsetData(VipVoi     *voi)
{
  int i;


  for(i=0; i < voi->NbVoi; i++)
    {
      (*(voi->ListeDePaquetDoffset + i) )->n_points = 0;
    }
  return( OK );
}

int VipVoiAllocateStat(
    VipVoi     *voi
    )
{
  if ( VipVoiTest( voi ) == PB ) return(  PB );

  voi->ListeDeStat = 
    (Statistics ***) VipCalloc(voi->NbVoi, sizeof(Statistics **), NULL);
  if ( !voi->ListeDeStat ) 
    {
      VipPrintfError("_PREF_ERRORVipVoiAllocateStat: Error allocating VipVoi internal...\n");
      return(  PB );
    }

  return( OK );
}
   

int VipVoiAllocateStatData(VipVoi     *voi)
{
  int i,j;
  Statistics **tmp;

  
  for(i=0; i < voi->NbVoi; i++)
    {
      if (voi->ReferenceImage == NULL) return ( PB );
      *(voi->ListeDeStat + i) = (Statistics **) VipCalloc(
					mVipVolSizeT(voi->ReferenceImage),
					sizeof(Statistics *), "");
      tmp = *(voi->ListeDeStat + i);
      if (!tmp) 
	{
	  return( PB );
	}
      for(j=0; j < mVipVolSizeT(voi->ReferenceImage); j++)
	{
	  *(tmp+j) = (Statistics *) VipCalloc(1, sizeof(Statistics), "");
	  if ( !(*(tmp+j)) ) 
	    {
	      return( PB );
	    }
	}
    }
  return( OK );
}


int VipVoiAllocateStatCorr(
    VipVoi     *voi
    )
{
  if ( VipVoiTest( voi ) == PB ) return(  PB );

  voi->ListeDeStatCorr = 
    (Statistics ***) VipCalloc(voi->NbVoi, sizeof(Statistics **), NULL);
  if ( !voi->ListeDeStatCorr ) 
    {
      VipPrintfError("_PREF_ERRORVipVoiAllocateStatCorr: Error allocating VipVoi internal...\n");
      return(  PB );
    }

  return( OK );
}

int VipVoiAllocateStatCorrData(VipVoi     *voi)
{
  int i,j;
  Statistics **tmp;

  
  for(i=0; i < voi->NbVoi; i++)
    {
      if (voi->ReferenceImage == NULL) return ( PB );
      *(voi->ListeDeStatCorr + i) = (Statistics **) VipCalloc(
					mVipVolSizeT(voi->ReferenceImage),
					sizeof(Statistics *), "");
      tmp = *(voi->ListeDeStatCorr + i);
      if (!tmp) 
	{
	  return( PB );
	}
      for(j=0; j < mVipVolSizeT(voi->ReferenceImage); j++)
	{
	  *(tmp+j) = (Statistics *) VipCalloc(1, sizeof(Statistics), "");
	  if ( !(*(tmp+j)) ) 
	    {
	      return( PB );
	    }
	}
    }
  return( OK );
}

int VipVoiAllocateMes(
    VipVoi     *voi
    )
{
  if ( VipVoiTest( voi ) == PB ) return(  PB );

  voi->ListeDePaquetDeMesure = 
    (float ***) VipCalloc(voi->NbVoi, sizeof(float **), NULL);
  if ( !voi->ListeDePaquetDeMesure ) 
    {
      VipPrintfError("_PREF_ERRORVipVoiAllocateStatCorr: Error allocating VipVoi internal...\n");
      return(  PB );
    }

  return( OK );
}

int VipVoiAllocateMesData(VipVoi     *voi)
{
  int i,j,nb;
  float **tmp;

  
  for(i=0; i < voi->NbVoi; i++)
    {
      nb = (*(voi->ListeDePaquetDePoint+i))->n_points;
      if (voi->ReferenceImage == NULL) return ( PB );
      *(voi->ListeDePaquetDeMesure + i) = (float **) VipCalloc(
					mVipVolSizeT(voi->ReferenceImage),
					sizeof(float *), "");
      tmp = *(voi->ListeDePaquetDeMesure + i);
      if (!tmp) 
	{
	  return( PB );
	}
      for(j=0; j < mVipVolSizeT(voi->ReferenceImage); j++)
	{
	  *(tmp+j) = (float *) VipCalloc(nb, sizeof(float), "");
	  if ( !(*(tmp+j)) ) 
	    {
	      return( PB );
	    }
	}
    }
  return( OK );
}


Vip3DBucket_S16BIT *VipVoiFirstPtrBucket(
    VipVoi     *voi
    )
{
  voi->traLDPDP = voi->ListeDePaquetDePoint;
  return( *(voi->traLDPDP) );
}

Vip3DBucket_S16BIT *VipVoiNextPtrBucket(VipVoi *voi)
{
  voi->traLDPDP +=1;
  return( *(voi->traLDPDP) );
}

Vip3DBucket_U16BIT *VipVoiFirstPtrDepla(VipVoi *voi)
{
  voi->traLDPDDE = voi->ListeDePaquetDeDeplaEntier;
  return( *(voi->traLDPDDE) );
}
Vip3DBucket_U16BIT *VipVoiNextPtrDepla(VipVoi *voi)
{
  voi->traLDPDDE +=1;
  return( *(voi->traLDPDDE) );
}

VipIntBucket *VipVoiFirstPtrOffset(VipVoi *voi)
{
  voi->traLDPDO = voi->ListeDePaquetDoffset;
  return( *(voi->traLDPDO) );
}
VipIntBucket *VipVoiNextPtrOffset(VipVoi *voi)
{
  voi->traLDPDO +=1;
  return( *(voi->traLDPDO) );
}


Statistics **VipVoiFirstPtrStat(VipVoi *voi)
{
  voi->traLDS = voi->ListeDeStat;
  return( *(voi->traLDS) );
}

Statistics **VipVoiNextPtrStat(VipVoi *voi)
{
  voi->traLDS +=1;
  return( *(voi->traLDS) );
}

Statistics **VipVoiFirstPtrStatCorr(VipVoi *voi)
{
  voi->traLDSC = voi->ListeDeStatCorr;
  return( *(voi->traLDSC) );
}

Statistics **VipVoiNextPtrStatCorr(VipVoi *voi)
{
  voi->traLDSC +=1;
  return( *(voi->traLDSC) );
}

float **VipVoiFirstPtrMes(VipVoi *voi)
{
  voi->traLDPDM = voi->ListeDePaquetDeMesure;
  return( *(voi->traLDPDM) );
}

float **VipVoiNextPtrMes( VipVoi *voi)
{
  voi->traLDPDM +=1;
  return( *(voi->traLDPDM) );
}


float *VipVoiFirstPtrMesVector( VipVoi *voi)
{
  voi->t_traLDPDM = *(voi->traLDPDM);
  return ( *(voi->t_traLDPDM) );
}

float *VipVoiNextPtrMesVector(VipVoi *voi)
{
  voi->t_traLDPDM +=1;
  return ( *(voi->t_traLDPDM) );
}

Statistics *VipVoiFirstPtrStatVector(VipVoi *voi)
{
  voi->t_traLDS = *(voi->traLDS);
  return ( *(voi->t_traLDS) );
}

Statistics *VipVoiNextPtrStatVector(VipVoi *voi)
{
  voi->t_traLDS +=1;
  return ( *(voi->t_traLDS) );
}
Statistics *VipVoiFirstPtrStatCorrVector(VipVoi *voi)
{
  voi->t_traLDSC = *(voi->traLDSC);
  return ( *(voi->t_traLDSC) );
}

Statistics *VipVoiNextPtrStatCorrVector(VipVoi *voi)
{
  voi->t_traLDSC +=1;
  return ( *(voi->t_traLDSC) );
}
