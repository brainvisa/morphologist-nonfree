/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 02/03/1999
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : recuperation du format de graphe relationnel
 *               attribue sur les plis corticaux  de la these de JF Mangin
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_SIMPLESURFACE_H
#define VIP_SIMPLESURFACE_H

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct surfacepoint {
    short x;
    short y;
    short z;
  } SurfacePoint;

  typedef struct surfacesimple {
    int label;
    /*in skeleton*/
    int index;
    /*for some tabs*/
    int the_label; /*identification*/

    /* identification de la surface simple*/
    char the_name[VIP_NAME_MAXLEN];
    float confiance; /*pourcentage identification*/
    float dist_moy;
    /* nombre total de points*/
    int n_points;
    /* bucket des points de surface*/
    Vip3DBucket_S16BIT *surface_points;
    /* bucket des points frontieres*/
    Vip3DBucket_S16BIT *edge_points;
    /* ss junction points and small ss dispatched to this ss*/
    Vip3DBucket_S16BIT *other_points;
    /* surface simple suivante dans la liste chainee*/
    struct surfacesimple *next;
    /*jonction avec la surface externe */
    BiJunction *hull_bijunction;
    /*jonctions topologiques:
      nombre: n_bijunction;
      liste de pointeurs de jonctions topologiques
      liste de pointeurs des surfaces simples correspondantes*/
    int n_bijunction;
    BiJunction **bijunction;
    struct surfacesimple **bijunction_neighbor;
    /*plidepassage:
      nombre: n_bijunction;
      liste de pointeurs de jonctions topologiques
      liste de pointeurs des surfaces simples correspondantes*/
    int n_plidepassage;
    PliDePassage **plidepassage;
    struct surfacesimple **plidepassage_neighbor; 
    int rootsbassin_filled;
    int rootsbassin;
    int npotentialpp;
    int *pppotentialneighbor_index;
    /*Jonctions Corticales...
      nombre: n_cortex_birel;
      liste de pointeurs de relations corticales;
      liste de pointeurs des surfaces simples correspondantes;*/
    int n_cortex_birel;
    CortexBiRelation **cortex_birel;
    struct surfacesimple **cortex_neighbor;
    /* taille de la ss  */
    float size;
    int size_filled;
    /*boite englobante de la ss dans le referentiel image*/
    int box_filled;
    Vip3DPoint_S16BIT boxmin;
    Vip3DPoint_S16BIT boxmax;
    /*boite englobante de la ss dans le referentiel commun*/
    int refbox_filled;
    Vip3DPoint_VFLOAT refboxmin;
    Vip3DPoint_VFLOAT refboxmax;
    /* normale de la ss par rapport aux axes de l'image*/
    Vip3DPoint_VFLOAT normale;
    int normale_filled;
    /* normale dans le referentiel commun*/
    Vip3DPoint_VFLOAT refnormale;
    int refnormale_filled;
    /* centre de gravite dans le ref de l'image*/
    Vip3DPoint_VFLOAT g;
    int g_filled;
    /* centre de gravite dans le ref commun*/
    Vip3DPoint_VFLOAT refg;
    int refg_filled;
    /* distance max et min a la surface externe*/
    float depth;
    int depth_filled;
    float mindepth;
    int mindepth_filled;
      /*matrice d'inertie dans Talairach*/
      float talcovar[3][3];
      int talcovar_filled;

  } SurfaceSimple;

  /* liste chainee de surfaces simples
   */
  typedef struct surfacesimplelist {
    SurfaceSimple *first_ss;
    int n_ss;
    SurfaceSimple **tab; /*WARNING, tab is of size n_ss + 1, 0 = HULL*/
    int *labTOindex;

  } SurfaceSimpleList;

/*-------------------------------------------------------------------------*/
    extern SurfaceSimpleList *CreateSurfaceSimpleList(
						      Vip3DBucket_S16BIT *sp_list);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
  extern SurfaceSimpleList *VipComputeFoldArgSSList( Volume *squel , int inside, int outside,
						     int limitsize, char *rootsvoronoi_name);   
/*-------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------*/
  extern SurfaceSimple *CreateEmptySurfaceSimple();
  /*-------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------*/
  extern int FreeSurfaceSimple( SurfaceSimple *dead );
  /*-------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------*/
  extern int FreeSurfaceSimpleList(SurfaceSimpleList *deadlist);
  /*-------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------*/
  extern Vip3DBucket_S16BIT *GetSurfaceSimple(
					      Volume *vol,
					      int labelSS,
					      int inside,
					      int outside);
  /*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int FillSSListJunctionPtr( SurfaceSimpleList *ss_list,
				  JunctionSet *jset);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int FillSSListCortexBirelationPtr(
				 SurfaceSimpleList *ss_list,
				 CortexRelationSet *jset);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
    extern int FillSurfaceSimpleEdgeBucket(
					   SurfaceSimpleList *ss_list,
					   Volume *vol,
					   int labelFRONTIERE);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int FillSurfaceSimpleOtherBucket(
SurfaceSimpleList *ss_list,
Volume *squel,
Volume *voronoi);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern BiJunction *CreateBiJunction( int index1,
				     int index2,
				     Vip3DBucket_S16BIT *buck,
				     SurfaceSimpleList *sslist);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern CortexBiRelation *CreateCortexBiRelation(
			     int index1,
			     int index2,
			     Vip3DBucket_S16BIT *buck,
			     SurfaceSimpleList *sslist);
/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
