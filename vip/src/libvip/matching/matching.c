/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipMatching.c        * TYPE     : Function
 * AUTHOR      : MANGIN J.-F.         * CREATION : 
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : 
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            03/01/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#include <vip/matching/matching.h>
#include <vip/matching/recalage.h>
#include <vip/matching/dilatation.h>

/******************************************************/

char *VipGetTrans2MriFileName(char *mri, 
			      char *pet)
{
  char *goal;
  char *root1, *root2, *input;
  
  input = mri;
  root2 = input;
  root1 = input;
  while(root2!=NULL)
      {
	  root2 = strstr(root1,"/");
	  if(root2!=NULL) root1 = root2+1;
      }
  mri = root1;

  input = pet;
  root2 = input;
  root1 = input;
  while(root2!=NULL)
      {
	  root2 = strstr(root1,"/");
	  if(root2!=NULL) root1 = root2+1;
      }
  pet = root1;

  goal = (char *)VipMalloc(strlen(mri)+strlen(pet)+50, "");
  if (!goal) return((char *)NULL);

  strcpy(goal,pet);
  strcat(goal,"_TO_");
  strcat(goal,mri);
	
  return(goal);
}

/*****************************************************/
char *VipGetMri2TransFileName(char *mri, 
			      char *pet)
{
  char *goal;

  char *root1, *root2, *input;
  
  input = mri;
  root2 = input;
  root1 = input;
  while(root2!=NULL)
      {
	  root2 = strstr(root1,"/");
	  if(root2!=NULL) root1 = root2+1;
      }
  mri = root1;

  input = pet;
  root2 = input;
  root1 = input;
  while(root2!=NULL)
      {
	  root2 = strstr(root1,"/");
	  if(root2!=NULL) root1 = root2+1;
      }
  pet = root1;


  goal = (char *)VipMalloc(strlen(mri)+strlen(pet)+50, "");
  if (!goal) return((char *)NULL);

  strcpy(goal,mri);
  strcat(goal,"_TO_");
  strcat(goal,pet);
	
  return(goal);
}

/******************************************************/
int VipDumbMain2DOr3DRegistration(Surface *mobsurf, 
				  Volume *ref, 
				  VipDeplacement *dep, 
				  float increment, 
				  float precision, 
				  int rotation_2D)
{
  if (rotation_2D == VTRUE)
    {
      if ((VipDumbMain2DRegistration( mobsurf, ref, dep, increment, precision )) == PB)
	{
	  VipPrintfError("Error while processing 2D registration");
	  VipPrintfExit("(matching.c)VipDumbMain2DOr3DRegistration");
	  return(PB);
	}
    }
  else
    {
      if ((VipDumbMainRegistration( mobsurf, ref, dep, increment, precision )) == PB)
	{
	  VipPrintfError("Error while processing 3D registration");
	  VipPrintfExit("(matching.c)VipDumbMain2DOr3DRegistration");
	  return(PB);
	}
    }

  return(OK);
}

/******************************************************/
VipDeplacement *VipMatching(Volume *volref, Volume *volmob, Volume *voldist, 
                            char *gradFile, int gradDim, int undersampling,
                            VipDeplacement *initDep, int x_mask_size,
                            int y_mask_size, int z_mask_size, int dodilation,
                            int rot2D, float threshold, float M1_init_angle, 
                            float M1_final_angle, float M2_init_angle, 
                            float M2_final_angle, float M3_init_angle, 
                            float M3_final_angle)
{
  Surface *mobsurf;
  Gravity volgrav;
  Translation *initial_tra=NULL;
  float dilatation=0.0;
  double best;
  int pet_head_reduce;
  VipDeplacement *dep;

  pet_head_reduce = 100/undersampling;

  /* extraction de la surface mobile */
  mobsurf = VipVolumeToLittleSurface( volmob, pet_head_reduce, gradFile,
				      gradDim );
  if (!mobsurf)
    {
      VipPrintfError("Error while generating Little Surface\n");
      VipPrintfExit("(matching.c)VipMatching");
      return((VipDeplacement *)NULL);
    }

  /* centres de gravite */
  printf("Computing initial optimal translation...\n");
  if ( initDep == NULL)
    {
      printf("Initialization using the surface centers of mass.\n");
      initial_tra = VipGetInitialTranslation( mobsurf, volref, &volgrav);
      if (!initial_tra)
	{
	  VipPrintfError("Error while getting initial translation\n");
	  VipPrintfExit("(matching.c)VipMatching");
	  if (mobsurf) VipFreeSurface(mobsurf);
          return((VipDeplacement *)NULL);
	}

      dep = VipInitialiseDeplacement( initial_tra );
      if (!dep)
	{
	  VipPrintfError("Error while initializing deplacement\n");
	  VipPrintfExit("(matching.c)VipMatching");
	  if (mobsurf) VipFreeSurface(mobsurf);
	  if (initial_tra) VipFree(initial_tra);
          return((VipDeplacement *)NULL);
	}
      VipFree(initial_tra);
    }
  else
    {
      dep = (VipDeplacement *)VipMalloc(sizeof(VipDeplacement), "");
      if (!dep) return((VipDeplacement *)NULL);

      VipDeplaFirstEgalDeplaSecond( dep, initDep );

      if ((VipFillSurfG( mobsurf )) == PB)
	{
	  VipPrintfError("Error while getting surface gravity center\n");
	  VipPrintfExit("(matching.c)VipMatching");
	  if (mobsurf) VipFreeSurface(mobsurf);
          if (dep) VipFree(dep);
          return((VipDeplacement *)NULL);
	}
    }

  /* carte de distance */
  if ( voldist == NULL )
    {
      printf("Computation of the distance map to the reference surface...\n");
      voldist = VipCopyVolume( volref, "" );
      if ((VipCreateDistanceMapWithLut( voldist, x_mask_size, y_mask_size, 
					z_mask_size, VIP_MASK_AUTO, 
					SCALP_LUT_DISTANCE_CORRECTION)) == PB)
	{
	  VipPrintfError("Error while computing distance map");
	  VipPrintfExit("(matching.c)VipMatching");
	  if (mobsurf) VipFreeSurface(mobsurf);
          if (dep) VipFree(dep);
          return((VipDeplacement *)NULL);
	}
    }

  /*Recalage initial en translation*/
  if ((VipFillDilatedBucket( mobsurf, dilatation )) == PB)
    {
      VipPrintfError("Error while filling bucket");
      VipPrintfExit("(matching.c)VipMatching");
      if (mobsurf) VipFreeSurface(mobsurf);
      if (dep) VipFree(dep);
      return((VipDeplacement *)NULL);
    }

  best = VipGetMinDistTrilinAndFillBestTransForRot( mobsurf, voldist, dep);
  if (best < 0.0)
    {
      VipPrintfError("Error while searching minimum distance");
      VipPrintfExit("(matching.c)VipMatching");
      if (mobsurf) VipFreeSurface(mobsurf);
      if (dep) VipFree(dep);
      return((VipDeplacement *)NULL);
    }
  printf("Quadratric distance average: %.4f\n",(float)best);

  /*Premiere Mise en correspondance*/
  printf("Surface matching...\n");
  if ((VipDumbMain2DOr3DRegistration( mobsurf, voldist, dep, M1_init_angle, 
				      M1_final_angle, rot2D )) == PB)
    {
      VipPrintfError("Error while processing registration");
      VipPrintfExit("(matching.c)VipMatching");
      if (mobsurf) VipFreeSurface(mobsurf);
      if (dep) VipFree(dep);
      return((VipDeplacement *)NULL);
    }

  /* dilatation et deuxieme mise en correspondance */
  if ( dodilation == VTRUE )
    {
      printf("With inflation: ");

      if (mobsurf->gradFilled == VFALSE)
	{
	  printf("Warning: Simulated gradient\n");
	  if (mobsurf->gradDim == GRAD_2D)
	    {
	      if ((VipFillSurfaceGSlice(mobsurf)) == PB)
		{
		  VipPrintfError("Error while filling surface slice gravity");
                  VipPrintfExit("(matching.c)VipMatching");
		  if (mobsurf) VipFreeSurface(mobsurf);
                  if (dep) VipFree(dep);
                  return((VipDeplacement *)NULL);
		}
	    }
	}

      dilatation = VipFindBestDilationForDep( mobsurf, voldist, dep, 
					      dilatation, 1.0 );
      if (dilatation == NO_BEST_DILATION)
	{
	  VipPrintfError("Error while finding best dilation for deplacement");
          VipPrintfExit("(matching.c)VipMatching");
	  if (mobsurf) VipFreeSurface(mobsurf);
          if (dep) VipFree(dep);
          return((VipDeplacement *)NULL);
	}
      printf("Optimal dilation in the gradient direction: %.3f mm\n", 
	     dilatation);

      if ((VipDumbMain2DOr3DRegistration( mobsurf, voldist, dep, M2_init_angle, 
                                          M2_final_angle, rot2D )) == PB)
	{
	  VipPrintfError("Error while processing registration");
          VipPrintfExit("(matching.c)VipMatching");
	  if (mobsurf) VipFreeSurface(mobsurf);
          if (dep) VipFree(dep);
          return((VipDeplacement *)NULL);
	}
    }

  /* elimination des outliers */
  printf("With outlier elimination...\n");
  if ((VipDestroyOutliersParam( mobsurf, dep, voldist, threshold )) == PB)
    {
      VipPrintfError("Error while destroying outliers");
      VipPrintfExit("(matching.c)VipMatching");
      if (mobsurf) VipFreeSurface(mobsurf);
      if (dep) VipFree(dep);
      return((VipDeplacement *)NULL);
    }

  if ((VipFillDilatedBucket( mobsurf, dilatation )) == PB)
    {
      VipPrintfError("Error while filling dilated bucket");
      VipPrintfExit("(matching.c)VipMatching");
      if (mobsurf) VipFreeSurface(mobsurf);
      if (dep) VipFree(dep);
      return((VipDeplacement *)NULL);
    }

  /* Troisieme mise en correspondance */
  if ((VipDumbMain2DOr3DRegistration( mobsurf, voldist, dep, M3_init_angle, 
				      M3_final_angle, rot2D )) == PB)
    {
      VipPrintfError("Error while processing registration");
      VipPrintfExit("(matching.c)VipMatching");
      if (mobsurf) VipFreeSurface(mobsurf);
      if (dep) VipFree(dep);
      return((VipDeplacement *)NULL);
    }

  VipFreeSurface(mobsurf);

  return( dep );
}
