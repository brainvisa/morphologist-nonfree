/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : statistiques.c       * TYPE     : Command line
 * AUTHOR      : FROUIN V.            * CREATION : 14/04/1999
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun  
 ****************************************************************************
 *
 * DESCRIPTION : Mesures 
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *           10/04/00 | V Frouin     |  Lecture automatique de tous les 
 *                    |              |  labels contenus dans voi
 ****************************************************************************/

#include <vip/voi.h>
#include <vip/volume.h>
#include <vip/bucket.h>
#include <vip/depla.h>
#include <vip/math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



static void Usage()
{ 
  (void)fprintf(stderr,"Usage : VipVoiStat\n");
  (void)fprintf(stderr,"\t\t-s[serie] {dynamic serie}\n");
  (void)fprintf(stderr,"\t\t-v[oi]    {voi volume}\n");
  (void)fprintf(stderr,"\t\t-d[epla]  {deplacement file (rotation/ affine}\n");
  (void)fprintf(stderr,"\t\t[-c[urve] {ASCII result file (def: stdout)}]\n");
  (void)fprintf(stderr,"\t\t[--dmc  {file with movement compensation}]\n");
  (void)fprintf(stderr,"\t\t[--pvec_matrix (pv cor y/n, default:y)]\n");
  (void)fprintf(stderr,"\t\t[--pvec_bkg_label (label of bkg, default:200)]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}




static void Help()
{ 
  (void)fprintf(stderr,"\t\t----------------------------------------------\n");
  (void)fprintf(stderr,"\t\tVipVoiStat performs statistics computation\n");
  (void)fprintf(stderr,"\t\t\tgiven a VOI file (label) a serie file\n");
  (void)fprintf(stderr,"\t\t\ta displacement file (and \n");
  (void)fprintf(stderr,"\t\t\tpossibly a movment compensations)\n");
  (void)fprintf(stderr,"\t\t\n");
  (void)fprintf(stderr,"\t\tFlag --pvec_matrix triggers pvec correction\n");
  (void)fprintf(stderr,"\t\t\tin that case the voi file must contain the \n");
  (void)fprintf(stderr,"\t\t\tbackground region (default label is 200 or\n");
  (void)fprintf(stderr,"\t\t\tgiven by --pvec_bkg_label\n");
  (void)fprintf(stderr,"\t\t\tthe -v[oi] parmater is the -l[abelout] file\n");
  (void)fprintf(stderr,"\t\t\toutput by the VipStriataGTM command\n");
  (void)fprintf(stderr,"\t\t----------------------------------------------\n");
  (void)fprintf(stderr,"\t\t\n");
  (void)fprintf(stderr,"Usage : VipVoiStat\n");
  (void)fprintf(stderr,"\t\t-s[serie] {dynamic serie}\n");
  (void)fprintf(stderr,"\t\t-v[oi]    {voi volume}\n");
  (void)fprintf(stderr,"\t\t-d[epla]  {deplacement file (rotation/ affine}\n");
  (void)fprintf(stderr,"\t\t[-c[urve] {ASCII result file (def: stdout)}]\n");
  (void)fprintf(stderr,"\t\t[--dmc  {file with movement compensation}]\n");
  (void)fprintf(stderr,"\t\t[--pvec_matrix (pv cor y/n, default:y)]\n");
  (void)fprintf(stderr,"\t\t[--pvec_bkg_label (label of bkg, default:200)]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}



int main(int argc, char *argv[])
{ 
  VIP_DEC_VOLUME(         Vserie);
  VIP_DEC_VOLUME(         Vmasque);
  VipDeplacement          *dep = NULL;
  VipDeplacementVector    *depVector = NULL;

  char              *masque         =NULL, *serie=NULL, 
                    *deplacement    =NULL, *courbe=NULL, *dmc=NULL;
  FILE              *fpcurve        =NULL;
  char              *gtmmatrix      =NULL;
  int               bkg_label       =200; /*supposed to be unaffected label*/

  int               i, j;
  VipNomenList      *nomenref=NULL, *nomenvol=NULL;
  VipMatrix_VDOUBLE *gtm = NULL;
  Vip_DOUBLE        **gtmcell = NULL;


  VipVoi            *roi = NULL; 


  for (i=1; i<argc; i++)
    {
      if (!strncmp(argv[i], "-serie", 2))
        {
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          serie = argv[i];
        }
      else if (!strncmp(argv[i], "-curve", 2))
        {
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          courbe = argv[i];
        }
      else if (!strncmp(argv[i], "-voi", 2))
	{
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
	  masque = argv[i];
	}
      else if (!strncmp (argv[i], "-depla", 2)) 
	{
	  if ((++i >= argc) || !strncmp(argv[i],"-",1)) Usage();
	  deplacement = argv[i];
	}
      else if (!strncmp (argv[i], "--dmc", 5)) 
	{
	  if ((++i >= argc) || !strncmp(argv[i],"-",1)) Usage();
	  dmc = argv[i];
	}
      else if (!strncmp (argv[i], "--pvec_matrix", 13)) 
	{
	  if ((++i >= argc) || !strncmp(argv[i],"-",1)) Usage();
	  gtmmatrix = argv[i];
	}
      else if (!strncmp (argv[i], "--pvec_bkg_label", 16)) 
	{
	  if ((++i >= argc) || !strncmp(argv[i],"-",1)) Usage();
	  bkg_label = atoi(argv[i]);
	}
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  /* check that all required arguments have been given   */
  if (serie == NULL)
    {
      VipPrintfError("serie arg is required by VipVoiStat");
      Usage();
    }
  if (masque == NULL)
    {
      VipPrintfError("roi arg is required by VipVoiStat");
      Usage();
    }
  if (deplacement==NULL && dmc==NULL)
    {
      (void)fprintf(stderr,"deplacement(dmc) arg is required by VipVoiStat\n");
      Usage();
    }

  /***** supprime les extensions des noms si elles existent  *****/
  if (strstr(serie, ".vimg") != NULL)  *strstr(serie, ".vimg") = '\0';
  if (strstr(masque, ".vimg") != NULL)  *strstr(masque, ".vimg") = '\0';

  /****** Lecture de l'image dynamique ************/
  fprintf(stderr,"\nReading dynamic image : %s ...\n", serie);
  if ((Vserie = VipReadVidaVolume(serie)) == (Volume *)PB)
    { 
      VipPrintfError("Error while reading volume.");
      VipPrintfExit("(commandline)VipStatistiques.");
      exit(EXIT_FAILURE);
    }

  /****** Lecture de l'image des labels et de la liste des labels   **********/
  /****      Si gtmmatrix est positionne : 1. Lire gtmmatrix **/
  /**                                      2. Re-ordonne la liste des labels**/
  fprintf(stderr,"\nReading label image : %s ...\n", masque);
  if ((Vmasque = VipReadVidaVolume(masque)) == (Volume *)PB)
    { 
      VipPrintfError("Error while reading volume.");
      VipPrintfExit("(commandline)VipVoiStat.");
      exit(EXIT_FAILURE);
    }
  if ( (nomenvol = VipCreateNomenListFromVolume( Vmasque ) ) == PB )
    {
      VipPrintfError("Can't get label from volume\n");
      exit( -1 );
    }
  if ( (nomenref = VipCreateRefNomenList()) == PB )
    {
      VipPrintfWarning("Can't use reference nomenclature\n");
    }
  else if ( VipMergeNomenList( nomenvol, nomenref ) == PB )
    {
      VipPrintfError("Can't merge nomenclature\n");
      exit ( - 1 );
    }
  if ( gtmmatrix )     /* cette operation n'est faite que si --pvec_matrix */
    {
      if (VipRemoveNomenListByLabel(nomenvol, bkg_label) == PB)
	{
	  VipPrintfError("Can't find the Background region label\n");
	  exit( -1 );
	}
      VipAddNomenList(nomenvol, bkg_label, "background");
      if (
          (gtm=VipReadMatrix_VDOUBLE(gtmmatrix,nomenvol->size, nomenvol->size))
          == NULL
          )
        {
          VipPrintfError("Error while reading gtmmatrix.");
          VipPrintfExit("(commandline)VipVoiStat.");
          return(PB);
        }
      if (
          (gtm=VipReadMatrix_VDOUBLE(gtmmatrix,nomenvol->size, nomenvol->size))
          == NULL
          )
        {
          VipPrintfError("Error while reading gtmmatrix.");
          exit( -1 );
        }
      /* Mise a 0.0 des termes <0 issus d'undershoot du filtrage recursif */
      /* la matrice gtm direct doit en effet etre positive et les termes  */
      /* tres faiblement negatifs peuvent etre annules sans risque        */
      gtmcell = VipGetMatrixPtr_VDOUBLE(gtm);
      for(i=0; i < VipGetMatrixSizeX_VDOUBLE(gtm); i++)
        for (j=0; j< VipGetMatrixSizeY_VDOUBLE(gtm); j++)
          {
            if (gtmcell[i][j] < (double) 0.0) gtmcell[i][j] = (double) 0.0;
          }

    }



  /****** lecture du fichier deplacement    *****/
  if (deplacement)
    {
      dep = VipReadDeplacement(deplacement);
      if (dep==PB)
	{
	  VipPrintfError("Displacement reading Problem...");
	  exit(EXIT_FAILURE);
	}
    }
  else
    {
      depVector = VipReadDeplacementVector(dmc);
      if (depVector == PB )
	{
	  VipPrintfError("Displacement reading Problem...");
	  exit(EXIT_FAILURE);
	}
    }


  /* Ouverture si necessaire du fichier courbe */
  if (courbe)
    {
      if ((fpcurve = fopen(courbe, "w")) == NULL)
	{
	  VipPrintfError("Error while while opening output curve file.");
	  VipPrintfExit("(commandline)VipVoiStat.");
	  exit(EXIT_FAILURE); 
	}
    }

  /* Allocation et init d'une structure de roi */
  roi = VipVoiReadFromVolume_S16BIT(Vmasque,
				    nomenvol->datalabel,
				    nomenvol->size);
  if (!roi)
    { 
      VipPrintfError("Error while reading rois from label image.");
      VipFreeVoi(roi);
      VipFreeVolume(Vmasque);
      VipFreeVolume(Vserie);
      VipPrintfExit("(commandline)VipVoiStat.");
      exit(EXIT_FAILURE);
    }



  if (deplacement)  /* Fichier deplacement unique */
    {
    if (VipVoiGetFunctionalValueFromVida(roi,dep, Vserie)  == PB)
      { 
	VipPrintfError("Error while gathering roi statistics.");
	VipFreeVoi(roi);
	VipFreeVolume(Vmasque);
	VipFreeVolume(Vserie);
	VipPrintfExit("(commandline)VipVoiStat.");
        exit(EXIT_FAILURE);
      }
    }
   else         /* Fichier deplacement contenant le mvt compensation */
     {
       if (VipVoiGetFunctionalValueFromVidaWithMC(roi,depVector,Vserie)  == PB)
      { 
	VipPrintfError("Error while gathering roi statistics.");
	VipFreeVoi(roi);
	VipFreeVolume(Vmasque);
	VipFreeVolume(Vserie);
	VipPrintfExit("(commandline)VipVoiStat.");
        exit(EXIT_FAILURE);
      }
     }


  /* Calcul de statistiques a partir des infos rassemblees */
  /* precedemment  et impression sur le moyen selectionne    */
  VipVoiComputeStat(roi);
  if ( gtmmatrix )
    {
      VipVoiGTMCorrection(roi, gtm);
      VipVoiStreamStat(roi,(fpcurve ? fpcurve : stdout), nomenvol, PVEC_CORR);
    }
  else
    {
      VipVoiStreamStat(roi, (fpcurve ? fpcurve : stdout), nomenvol, UN_CORR);
    }

  /* Liberation des ressources */
  if ( VipFreeVoi(roi) == PB) 
    { 
      VipPrintfExit("(commandline)VipVoiStat.");
      exit(EXIT_FAILURE);
    }

  if ( VipFreeVolume(Vmasque) == PB) 
    { 
      VipPrintfExit("(commandline)VipVoiStat.");
      exit(EXIT_FAILURE);
    }
  if ( VipFreeVolume(Vserie) == PB) 
    { 
      VipPrintfExit("(commandline)VipVoiStat.");
      exit(EXIT_FAILURE);
    }

  exit(EXIT_SUCCESS);

}
