/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_mat.c            * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 02/02/1997
 * VERSION     : 1.0                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : PC Linux
 *****************************************************************************
 *
 * DESCRIPTION : A few procedures for 3 by 3 matrix
 * They seems relatively funny today (they stem from a 6month period
 * in GE/CGR buc...)
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

#include <vip/depla.h>
#include <vip/depla_static.h>

/*---------------------------------------------------------------*/
static int StaticSyslin_3_3(
   double A[3][3],
   double b[3],
   double X[3]);     /* pivot total de Gauss  */
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
static int StaticEchange(
   double coef[3][4],
   int i,
   int j,
   char *mode,
   int memo[3]);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
 double StaticMax_piv(
   double coef[3][4],
   int n,
   int *lig,
   int *col);
                       /*  n indique recherche du (n+1)_eme pivot */
/*---------------------------------------------------------------*/


/*---------------------------------------------------------------*/
int StaticInverse(
   double Pin[3][3],
   double Q[3][3])
/*---------------------------------------------------------------*/

{
   double ID[3][3], P[3][3];
   double X[3];
   int i,j;

   for(i=0; i<3; i++) for(j=0; j<3; j++) P[i][j] = Pin[i][j];

   for( i=0; i<3; i++)
   {
      ID[i][i] = 1.;
      for( j=0; j<i; j++)
      {
         ID[i][j] = ID[j][i] = 0.;
      }
   }
   
   for( i=0; i<3; i++ )
   {
      if(StaticSyslin_3_3( P, &ID[i][0], X)==PB) return(PB);  /*ID symmetrique */
      for( j=0; j<3; j++)
      {
         Q[j][i] = X[j];
      }
   }

   return(OK);
}
/*---------------------------------------------------------------*/
static int StaticSyslin_3_3(
   double A[3][3],
   double b[3],
   double X[3])     /* pivot total de Gauss  */
/*---------------------------------------------------------------*/

{

   double pivot, num, max_piv();
   int i, j, k, lig, col;
   double coef[3][4];
   int memo[3];

   for (i=0; i<3; i++)
   {
      for( j=0; j<3; j++ ) coef[i][j] = A[i][j];
      coef[i][3] = b[i];
      memo[i] = i;
   }

   for (i=0; i<3; i++)
   {
      pivot = StaticMax_piv( coef, i, &lig, &col);
      if (fabs(pivot) <0.00000001)
      {
         VipPrintfExit("Singular matric in StaticSyslin\n");
         return(PB);
      }
      if (lig!=i) StaticEchange(coef, i, lig, "lig", memo);
      if (col!=i) StaticEchange(coef, i, col, "col", memo);
      for (k=0; k<3; k++) if (i!=k) 
      {    
         num = coef[k][i]/pivot;  
         for (j=i; j<4; j++) coef[k][j] -= num*coef[i][j];
      }
      for (j=0; j<4; j++) if (i!=j) coef[i][j] /= pivot;
   }
   
   for (i=0; i<3; i++) X[memo[i]]=coef[i][3];

   return(OK);
}

/*---------------------------------------------------------------*/
static int StaticEchange(
   double coef[3][4],
   int i,
   int j,
   char *mode,
   int memo[3])
/*---------------------------------------------------------------*/

{

   double fsauve;
   int k, isauve;

   if (!strcmp(mode,"lig"))
   {
     for (k=0; k<4; k++)
     {
        fsauve = coef[i][k];
        coef[i][k] = coef[j][k];
        coef[j][k] = fsauve;
     }
   }
   else
   {
      for (k=0; k<3; k++)
      {
         fsauve = coef[k][i];
         coef[k][i] = coef[k][j];
         coef[k][j] = fsauve;
      } 
      isauve = memo[i];
      memo[i] = memo[j];
      memo[j] = isauve;
   }
  return(OK);
}

/*---------------------------------------------------------------*/
 double StaticMax_piv(
   double coef[3][4],
   int n,
   int *lig,
   int *col)
                       /*  n indique recherche du (n+1)_eme pivot */
/*---------------------------------------------------------------*/

{

   double max;
   int i,j;

   max = 0;
   for (i=n; i<3; i++) for(j=n; j<3; j++)
      if (fabs(max)<fabs(coef[i][j])) 
      {
         max=coef[i][j];
         *lig = i;
         *col = j;
      }
   return(max);
}
