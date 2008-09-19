/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_depla.c          * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 15/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne la manipulation de rotations affines
 *               jfmangin@cea.fr
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


/*-------------------------------------------------------------------*/
static int FillRotationAroundXAxe(
	float angle,
	VipRotation *rot);
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
static int FillRotationAroundYAxe(
	float angle,
	VipRotation *rot);
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
static int FillRotationAroundZAxe(
	float angle,
	VipRotation *rot);
/*-------------------------------------------------------------------*/

VipDeplacementVector *VipReadDeplacementVector(
char *name)
{
  VipDeplacementVector *depVector;
  VipDeplacement *dep;  
  int nombre;
  char buff[512];
 
  FILE *fp;

  if (name==NULL)
    {
      VipPrintfError("No filename in VipReadDeplacementVector");
      VipPrintfExit("(depla)VipReadDeplacementVector");
      return(PB);
    }
    fp = fopen( name, "r");

  if(!fp)
    {
      sprintf(buff,"Can not open file : %s\n",name);
      VipPrintfError(buff);
      VipPrintfExit("(depla)VipReadDeplacementVector");
      return(PB);
    }

  depVector = (VipDeplacementVector *)VipMalloc(sizeof(VipDeplacement),
					  "VipReadDeplacementVector");
  if(!depVector) return(PB);

  nombre = 0;
  while (fgets(buff, 512, fp) != NULL)
    {
      nombre+=1;
    }
  if ((nombre % 4) != 0)
    {
      VipPrintfError("Noumber of line in dep file must be multiple of 4");
      VipPrintfExit("(depla)VipReadDeplacementVector");
      return(PB);
    }
  nombre /=4;
  depVector->count = nombre;
  depVector->dep   = (VipDeplacement *)VipMalloc(sizeof(VipDeplacement)*nombre,
					  "VipReadDeplacementVector");
  if (depVector->dep == NULL) return(PB);
    
  fseek(fp, 0, SEEK_SET);
  
  dep = depVector->dep;
  while (fgets(buff, 512, fp) != NULL)
    {
    sscanf(buff, "%f %f %f",&(dep->t.x), &(dep->t.y), &(dep->t.z));
    fgets(buff, 512, fp);
    sscanf(buff, "%lf %lf %lf",&(dep->r.xx), &(dep->r.xy), &(dep->r.xz));
    fgets(buff, 512, fp);
    sscanf(buff, "%lf %lf %lf",&(dep->r.yx), &(dep->r.yy), &(dep->r.yz));
    fgets(buff, 512, fp);
    sscanf(buff, "%lf %lf %lf",&(dep->r.zx), &(dep->r.zy), &(dep->r.zz));
    dep++;
    }
  fclose(fp);
  
  return(depVector);
}

/*-------------------------------------------------------------------*/
VipDeplacement *VipReadDeplacement(
char *name)
/*-------------------------------------------------------------------*/

{
  VipDeplacement *dep;
  FILE *f;
  char error[256];
  int n, protect;

  if(name==NULL)
    {
      VipPrintfError("No filename in VipReadDeplacement");
      VipPrintfExit("(depla)VipReadDeplacement");
      return(PB);
    }

  f = fopen( name, "r");
  if(!f)
    {
      sprintf(error,"Can not open file : %s\n",name);
      VipPrintfError(error);
      VipPrintfExit("(depla)VipReadDeplacement");
      return(PB);
    }

  dep = (VipDeplacement *)VipMalloc(sizeof(VipDeplacement),"VipReadDeplacement");
  if(!dep) return(PB);

  n = fscanf( f, "%f",&(dep->t.x));
  if( (n==0) || (dep->t.x < -1000.) || (dep->t.x > 1000.) )
    {
      sprintf(error,"Deplacement with comment lines? : %s\n",name);
      VipPrintfWarning(error);
      protect = 100;
      while( (n==0) || (dep->t.x < -1000.) || (dep->t.x > 1000.) )
	{
	  n = fscanf( f, "%f",&(dep->t.x));
	  if(protect-- == 0)
	    {
	      VipPrintfError("Can not recover the deplacement file format");
	      VipFree(dep);
	      VipPrintfExit("(depla)VipReadDeplacement");
	      return(PB);
	    }
	}
    }

  n = fscanf( f, "%f",&(dep->t.y));
  if( (n==0) || (dep->t.y < -1000.) || (dep->t.y > 1000.) )
    {
      sprintf(error,"Problematic deplacement file? : %s\n",name);
      VipPrintfError(error);
      VipFree(dep);
      VipPrintfExit("(depla)VipReadDeplacement");
      return(PB);
    }
  n = fscanf( f, "%f",&(dep->t.z));
  if( (n==0) || (dep->t.z < -1000.) || (dep->t.z > 1000.) )
    {
      sprintf(error,"Problematic deplacement file? : %s\n",name);
      VipPrintfError(error);
      VipFree(dep);
      VipPrintfExit("(depla)VipReadDeplacement");
      return(PB);
    }

  n = fscanf( f, "%lf%lf%lf",&(dep->r.xx),&(dep->r.xy),&(dep->r.xz));
  n += fscanf( f, "%lf%lf%lf",&(dep->r.yx),&(dep->r.yy),&(dep->r.yz));
  n += fscanf( f, "%lf%lf%lf",&(dep->r.zx),&(dep->r.zy),&(dep->r.zz));
  if(n!=9)
    {
      sprintf(error,"Problematic deplacement file? : %s\n",name);
      VipPrintfError(error);
      VipFree(dep);
      VipPrintfExit("(depla)VipReadDeplacement");
      return(PB);
    }
  fclose( f );
  return(dep);
}

/*-------------------------------------------------------------------*/
int VipWriteDeplacement(
	VipDeplacement *dep,
	char *name)
/*-------------------------------------------------------------------*/

{
  FILE *f;
  char error[256];
  int n;

  if(!dep)
    {
      VipPrintfError("No deplacement to write");
      VipPrintfExit("VipWriteDeplacement");
      return(PB);
    }
  if(!name)
    {
      VipPrintfError("No name for deplacement");
      VipPrintfExit("VipWriteDeplacement");
      return(PB);
    }
  f = fopen( name, "w");
  if(!f)
    {
      sprintf(error,"Can not open file for writing: %s\n",name);
      VipPrintfError(error);
      VipPrintfExit("VipWriteDeplacement");
      return(PB);
    }

  n = fprintf(f, "%f %f %f\n", dep->t.x, dep->t.y, dep->t.z);
  n += fprintf(f, "%f %f %f\n", dep->r.xx, dep->r.xy, dep->r.xz);
  n += fprintf(f, "%f %f %f\n", dep->r.yx, dep->r.yy, dep->r.yz);
  n += fprintf(f, "%f %f %f\n", dep->r.zx, dep->r.zy, dep->r.zz);

  /*if(n!=12)
    {
      sprintf(error,"Problematic deplacement writing? : %s\n",name);
      VipPrintfError(error);
      VipPrintfExit("(depla)VipWriteDeplacement");
      return(PB);
    }
  * 
  * Ce test n'est pas juste dans la mesure ou fprintf renvoie le nombre
  * de caracteres ecrits (apres conversion float->char) et non le nombre 
  * d'elements. 
  * En gros, 'n' n'est jamais egal a 12, mais plutot superieur a 100!
  * (F.P.)
  */

  fclose(f);

  return(OK);
}

/*-------------------------------------------------------------------*/
float VipGetRotationAngle(
	VipDeplacement *dep)
/*-------------------------------------------------------------------*/
{
  double trace;
  float  angle;
  
  trace = dep->r.xx + dep->r.yy + dep->r.zz;
  trace -= 1.;
  trace /= 2.;
  if((trace < -1.) || (trace > 1.))
    {
      VipPrintfWarning("Strange rotation trace in VipGetRotationAngle");
      return(0.);
    }
  angle = (float)(180./M_PI*acos(trace));
  return( angle);
}

/*-------------------------------------------------------------------*/
int VipApplyDeplacement(
	VipDeplacement *d,
	Vip3DPoint_VFLOAT *in,
        Vip3DPoint_VFLOAT *out)
/*-------------------------------------------------------------------*/

{
  float x, y, z;

  if (!d || !in || !out)
    {
      VipPrintfError("Bad arguments in VipApplyDeplacement");
      VipPrintfExit("VipApplyDeplacement");
      return(PB);
    }
  
  x = (float)d->r.xx*(in->x) + (float)d->r.xy*(in->y) + (float)d->r.xz*(in->z) + d->t.x;
  y = (float)d->r.yx*(in->x) + (float)d->r.yy*(in->y) + (float)d->r.yz*(in->z) + d->t.y;
  z = (float)d->r.zx*(in->x) + (float)d->r.zy*(in->y) + (float)d->r.zz*(in->z) + d->t.z;

  out->x = x;
  out->y = y;
  out->z = z;
  return(OK);
}

/*-------------------------------------------------------------------*/
int VipApplyRotation(
	VipRotation *r,
	Vip3DPoint_VFLOAT *in,
        Vip3DPoint_VFLOAT *out)
/*-------------------------------------------------------------------*/

  {
    float x, y, z;

    if (!r || !in || !out)
      {
	VipPrintfError("Bad arguments in VipApplyRotation");
	VipPrintfExit("VipApplyRotation");
	return(PB);
      }

    x = (float)r->xx*(in->x) + (float)r->xy*(in->y) + (float)r->xz*(in->z);
    y = (float)r->yx*(in->x) + (float)r->yy*(in->y) + (float)r->yz*(in->z);
    z = (float)r->zx*(in->x) + (float)r->zy*(in->y) + (float)r->zz*(in->z);

    out->x = x;
    out->y = y;
    out->z = z;
    return(OK);
  }


/*-------------------------------------------------------------------*/
VipDeplacement *VipInitialiseDeplacement(
	Vip3DPoint_VFLOAT *translat)
/*-------------------------------------------------------------------*/

{
	VipDeplacement *dep;

	dep = (VipDeplacement *)VipMalloc(sizeof(VipDeplacement),"VipInitialiseDeplacement");
	if(!dep) return(NULL);

	dep->r.xx = 1.0;
	dep->r.xy = 0.0;
	dep->r.xz = 0.0;
	
	dep->r.yx = 0.0;
	dep->r.yy = 1.0;
	dep->r.yz = 0.0;

	dep->r.zx = 0.0;
	dep->r.zy = 0.0;
	dep->r.zz = 1.0;

	dep->t.x = translat->x;
	dep->t.y = translat->y;
	dep->t.z = translat->z;

	return(dep);
}
/*-------------------------------------------------------------------*/
int VipDeplaFirstEgalDeplaSecond( 
	VipDeplacement *first,
	VipDeplacement *second)
/*-------------------------------------------------------------------*/
{
  if(!first || !second)
    {
      VipPrintfError("Bad Arguments in VipDeplaFirstEgalDeplaSecond");
      VipPrintfExit("VipDeplaFirstEgalDeplaSecond");
      return(PB);
    }

  first->r.xx = second->r.xx;
  first->r.xy = second->r.xy;
  first->r.xz = second->r.xz;
	
  first->r.yx = second->r.yx;
  first->r.yy = second->r.yy;
  first->r.yz = second->r.yz;

  first->r.zx = second->r.zx;
  first->r.zy = second->r.zy;
  first->r.zz = second->r.zz;

  first->t.x = second->t.x;
  first->t.y = second->t.y;
  first->t.z = second->t.z;
  return(OK);
}

/*-------------------------------------------------------------------*/
int VipFillDeplacement(
  Vip3DPoint_VFLOAT *tra,
  VipRotation *rot,
  VipDeplacement *dep)
/*-------------------------------------------------------------------*/

	/* NB: Comme toutes mes procedures commencant par Fill,
		et contrairement aux procedures Get,
		la structure resultat doit etre prealablement allouee,
		ce qui evite le pb penible de liberer les zones
		retournees */
{
  if(!tra || !rot || !dep)
    {
      VipPrintfError("Bad arguments in VipFillDeplacement");
      VipPrintfExit("ViFillDeplacement");
      return(PB);	  
    }
  dep->t.x = tra->x;
  dep->t.y = tra->y;
  dep->t.z = tra->z;

  dep->r.xx = rot->xx;
  dep->r.xy = rot->xy;
  dep->r.xz = rot->xz;

  dep->r.yx = rot->yx;
  dep->r.yy = rot->yy;
  dep->r.yz = rot->yz;

  dep->r.zx = rot->zx;
  dep->r.zy = rot->zy;
  dep->r.zz = rot->zz;
  return(OK);
}


	
/*-------------------------------------------------------------------*/
int VipFillRotation(
	float xangle,
	float yangle,
	float zangle,
	VipRotation *rot) 
	/*Remplis la structure rotation (qui doit etre allouee prealablement)
	 avec une rotation combinaison de 3 rotations autour des axes*/
/*-------------------------------------------------------------------*/

{
  VipRotation xrot, yrot, zrot;

  if(!rot) 
    {
      VipPrintfError("Bad argument in VipFillRotation");
      VipPrintfExit("VipFillRotation");
      return(PB);
    }
	
  FillRotationAroundXAxe( xangle, &xrot);
  FillRotationAroundYAxe( yangle, &yrot);
  FillRotationAroundZAxe( zangle, &zrot);

  VipMultiplyRotations( &xrot, &yrot, rot);
  VipMultiplyRotations( rot, &zrot, rot);
  return(OK);
}

/*-------------------------------------------------------------------*/
int VipMultiplyRotations( 
	VipRotation *g,
	VipRotation *d,
	VipRotation *out) /* g=left, d=right ...*/
/*-------------------------------------------------------------------*/

{
  VipRotation result;

  if(!g || !d || !out)
    {
      VipPrintfError("Bad argument in VipMultiplyRotations");
      VipPrintfExit("VipMultiplyRotations");
      return(PB);
    }

  result.xx = g->xx * d->xx + g->xy * d->yx + g->xz * d->zx;
  result.xy = g->xx * d->xy + g->xy * d->yy + g->xz * d->zy;
  result.xz = g->xx * d->xz + g->xy * d->yz + g->xz * d->zz;

  result.yx = g->yx * d->xx + g->yy * d->yx + g->yz * d->zx;
  result.yy = g->yx * d->xy + g->yy * d->yy + g->yz * d->zy;
  result.yz = g->yx * d->xz + g->yy * d->yz + g->yz * d->zz;

  result.zx = g->zx * d->xx + g->zy * d->yx + g->zz * d->zx;
  result.zy = g->zx * d->xy + g->zy * d->yy + g->zz * d->zy;
  result.zz = g->zx * d->xz + g->zy * d->yz + g->zz * d->zz;

  out->xx = result.xx;
  out->xy = result.xy;
  out->xz = result.xz;

  out->yx = result.yx;
  out->yy = result.yy;
  out->yz = result.yz;
	
  out->zx = result.zx;
  out->zy = result.zy;
  out->zz = result.zz;

  return(OK);

}

/*-------------------------------------------------------------------*/
int VipPrintRotation( 
	VipRotation *rot)
/*-------------------------------------------------------------------*/
{
  if (!rot)
    {
      VipPrintfError("No rotation as argument");
      VipPrintfExit("VipPrintRotation");
      return(PB);
    }
  printf("Rotation:\n");
  printf("xx:%f xy:%f xz:%f\n",rot->xx, rot->xy, rot->xz);
  printf("yx:%f yy:%f yz:%f\n",rot->yx, rot->yy, rot->yz);
  printf("zx:%f zy:%f zz:%f\n",rot->zx, rot->zy, rot->zz);
  printf("........................................\n");

  return(OK);
}


/*-------------------------------------------------------------------*/
static int FillRotationAroundXAxe(
	float angle,
	VipRotation *rot)
/*-------------------------------------------------------------------*/

	/*Remplis la structure rotation (qui doit etre allouee prealablement)
	 avec une rotation d'angle angle autour de l'axe des X)
	NB l'angle est specifie en degre*/
{
	if(!rot) 
	{
	  VipPrintfError("Bad argument in FillRotationAroundXAxe");
	  VipPrintfExit("FillRotationAroundXAxe");
	  return(PB);		
	}

	if((angle>360.0f) || (angle<-360.0f))
	{
	   VipPrintfWarning("Strange angle in FillRotationAroundXAxe");
	}

	angle = angle / 180.0f;

	rot->xx = 1.0;
	rot->xy = 0.0;
	rot->xz = 0.0;

	rot->yx = 0.0;
	rot->yy = cos(M_PI*(double)angle);
	rot->yz = -sin(M_PI*(double)angle);

	rot->zx = 0.0;
	rot->zy = sin(M_PI*(double)angle);
	rot->zz = cos(M_PI*(double)angle);
  return(OK);
}
/*-------------------------------------------------------------------*/
static int FillRotationAroundYAxe(
	float angle,
	VipRotation *rot)
/*-------------------------------------------------------------------*/

	/*Remplis la structure rotation (qui doit etre allouee prealablement)
	 avec une rotation d'angle angle autour de l'axe des Y)
	NB l'angle est specifie en degre*/
{
	if(!rot) 
	{
	  VipPrintfError("Bad argument in FillRotationAroundYAxe");
	  VipPrintfExit("FillRotationAroundYAxe");
	  return(PB);		
	}

	if((angle>360.0f) || (angle<-360.0f))
	{
	   VipPrintfWarning("Strange angle in FillRotationAroundYAxe");
	}

	angle = angle / 180.0f;


	rot->xx = cos(M_PI*(double)angle);
	rot->xy = 0.0;
	rot->xz = sin(M_PI*(double)angle);

	rot->yx = 0.0;
	rot->yy = 1.0;
	rot->yz = 0.0;

	rot->zx = -sin(M_PI*(double)angle);
	rot->zy = 0.0;
	rot->zz = cos(M_PI*(double)angle);
  return(OK);
}


/*-------------------------------------------------------------------*/
static int FillRotationAroundZAxe(
	float angle,
	VipRotation *rot)
/*-------------------------------------------------------------------*/

	/*Remplis la structure rotation (qui doit etre allouee prealablement)
	 avec une rotation d'angle angle autour de l'axe des Z)
	NB l'angle est specifie en degre*/
{
	if(!rot) 
	{
	  VipPrintfError("Bad argument in FillRotationAroundZAxe");
	  VipPrintfExit("FillRotationAroundZAxe");
	  return(PB);		
	}

	if((angle>360.0f) || (angle<-360.0f))
	{
	   VipPrintfWarning("Strange angle in FillRotationAroundZAxe");
	}
	angle = angle / 180.0f;

	rot->xx = cos(M_PI*(double)angle);
	rot->xy = -sin(M_PI*(double)angle);
	rot->xz = 0.0;

	rot->yx = sin(M_PI*(double)angle);
	rot->yy = cos(M_PI*(double)angle);
	rot->yz = 0.0;

	rot->zx = 0.0;
	rot->zy = 0.0;
	rot->zz = 1.0;
  return(OK);
}

/*----------------------------------------------------------------------*/
int VipInverseDeplacement(
VipDeplacement *dep,
VipDeplacement *invdep)
/*----------------------------------------------------------------------*/

{
	double rot[3][3], invrot[3][3];

	if(!dep || !invdep)
	{
		VipPrintfError("Arguments in VipInverseDeplacement!\n");
		return(PB);
	}

	rot[0][0] = (dep->r.xx);
	rot[0][1] = (dep->r.xy);
	rot[0][2] = (dep->r.xz);
	rot[1][0] = (dep->r.yx);
	rot[1][1] = (dep->r.yy);
	rot[1][2] = (dep->r.yz);
	rot[2][0] = (dep->r.zx);
	rot[2][1] = (dep->r.zy);
	rot[2][2] = (dep->r.zz);

	StaticInverse( rot, invrot );

	invdep->r.xx=invrot[0][0];
	invdep->r.xy=invrot[0][1];	
	invdep->r.xz=invrot[0][2];
	invdep->r.yx=invrot[1][0];
	invdep->r.yy=invrot[1][1];
	invdep->r.yz=invrot[1][2];
	invdep->r.zx=invrot[2][0];
	invdep->r.zy=invrot[2][1];
	invdep->r.zz=invrot[2][2];

	invdep->t.x = -dep->t.x;
	invdep->t.y = -dep->t.y;
	invdep->t.z = -dep->t.z;

	VipApplyRotation( &(invdep->r), &(invdep->t), &(invdep->t) );

	return(OK);

}

/*----------------------------------------------------------------------*/
int VipMultDeplacement(
	VipDeplacement *dep,
	VipDeplacement *left,
	VipDeplacement *right)
/*----------------------------------------------------------------------*/
{
	float l[4][4], r[4][4], o[4][4];
	int i,j,k;

	if(!dep || !left || !right )	
	{
                VipPrintfExit("Arguments in  VipScaleDeplacement ???\n");
                return(PB);
        }
	l[0][0]=left->r.xx;l[0][1]=left->r.xy;l[0][2]=left->r.xz;
	l[0][3]=left->t.x;	
	l[1][0]=left->r.yx;l[1][1]=left->r.yy;l[1][2]=left->r.yz;
	l[1][3]=left->t.y;	
	l[2][0]=left->r.zx;l[2][1]=left->r.zy;l[2][2]=left->r.zz;
	l[2][3]=left->t.z;	
	l[3][0]=l[3][1]=l[3][2]=0.0;l[3][3]=1.0;

	r[0][0]=right->r.xx;r[0][1]=right->r.xy;r[0][2]=right->r.xz;
	r[0][3]=right->t.x;	
	r[1][0]=right->r.yx;r[1][1]=right->r.yy;r[1][2]=right->r.yz;
	r[1][3]=right->t.y;	
	r[2][0]=right->r.zx;r[2][1]=right->r.zy;r[2][2]=right->r.zz;
	r[2][3]=right->t.z;	
	r[3][0]=r[3][1]=r[3][2]=0.0;r[3][3]=1.0;
	
	for(i=0;i<4;i++)
	  for(j=0;j<4;j++)
	    {
	    o[i][j] = 0.0;
	    for(k=0;k<4;k++) o[i][j] += l[i][k] * r[k][j];
	    }
     
	dep->r.xx=o[0][0];dep->r.xy=o[0][1];dep->r.xz=o[0][2];
	dep->r.yx=o[1][0];dep->r.yy=o[1][1];dep->r.yz=o[1][2];
	dep->r.zx=o[2][0];dep->r.zy=o[2][1];dep->r.zz=o[2][2];
	dep->t.x=o[0][3] ;dep->t.y=o[1][3] ;dep->t.z=o[2][3];


	return(OK);
}
/*----------------------------------------------------------------------*/
int VipScaleDeplacement(
	VipDeplacement *dep,
	Volume *from,
	Volume *to)
/*----------------------------------------------------------------------*/

{
	if(!dep || !from || !to)
	{
		VipPrintfExit("Arguments in  VipScaleDeplacement ???\n");
		return(PB);
	}
	if(VipVerifyAll(from)==PB || VipVerifyAll(to)==PB)
	{
		VipPrintfExit("Arguments in  VipScaleDeplacement ???\n");
		return(PB);
	}
	dep->r.xx *= mVipVolVoxSizeX(from) / mVipVolVoxSizeX(to);
	dep->r.xy *= mVipVolVoxSizeY(from) / mVipVolVoxSizeX(to);
	dep->r.xz *= mVipVolVoxSizeZ(from) / mVipVolVoxSizeX(to);

	dep->r.yx *= mVipVolVoxSizeX(from) / mVipVolVoxSizeY(to);
	dep->r.yy *= mVipVolVoxSizeY(from) / mVipVolVoxSizeY(to);
	dep->r.yz *= mVipVolVoxSizeZ(from) / mVipVolVoxSizeY(to);

	dep->r.zx *= mVipVolVoxSizeX(from) / mVipVolVoxSizeZ(to);
	dep->r.zy *= mVipVolVoxSizeY(from) / mVipVolVoxSizeZ(to);
	dep->r.zz *= mVipVolVoxSizeZ(from) / mVipVolVoxSizeZ(to);

	dep->t.x /= mVipVolVoxSizeX(to);
	dep->t.y /= mVipVolVoxSizeY(to);
	dep->t.z /= mVipVolVoxSizeZ(to);
  return(OK);
}
/************************************************/
