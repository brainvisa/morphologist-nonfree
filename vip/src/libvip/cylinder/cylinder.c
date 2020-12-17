/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : cylinder.c          * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 21/01/2000
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : cree la triangulation d'un cylindre
 *               pour voir des aiguilles ou des electrodes
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

#include <vip/cylinder.h>

/*---------------------------------------------------------------------------*/
static int CreateCylinder(VipG3DPoint *p, int n, int l, float length, float radius,
    Vip3DBucket_VFLOAT *coord, Vip3DBucket_VFLOAT *normal,
    Vip3DBucket_S32BIT *triang); 
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static int CreateHatCylinder(VipG3DPoint *p, int n, int l, float length, float radius, float hatheight,
    Vip3DBucket_VFLOAT *coord, Vip3DBucket_VFLOAT *normal,
    Vip3DBucket_S32BIT *triang); 
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipWriteCylinder(float x1, float y1, float z1, 
		     float x2, float y2, float z2,
		     char *name,
		     float radius)
/*---------------------------------------------------------------------------*/
{
  char filename[512];
    FILE *f;
    char error[256];
    int i;
    VipG3DPoint G;
    Vip3DBucket_VFLOAT *coord, *normal;
    Vip3DBucket_S32BIT *triang;
    int n,l;
    int npoints, cpoints;
    int ntriangles, ctriangles;
    float clength;
    double norm;

    if(name==NULL)
	{
	    VipPrintfError("No electrod name");
	    VipPrintfExit("VipWriteCylinder");
	    return(PB);
	}

    strcpy(filename,name);
    strcat(filename,".tri");
    f = fopen(filename,"w");
    if(f==PB)
	{
	    sprintf(error,"Can not open file %s",filename);
	    VipPrintfError(error);
	    VipPrintfExit("VipWriteCylinder");
	    return(PB);
	}

    G.p.x = (x1+x2)/2.;
    G.p.y = (y1+y2)/2.;
    G.p.z = (z1+z2)/2.;
    norm = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1));
    if(norm<0.00001)
      {
	VipPrintfError("Equal extremities!");
	VipPrintfExit("VipWriteCylinder");
	return(PB);
      }
    G.g3D.x = (x2-x1)/norm;
    G.g3D.y = (y2-y1)/norm;
    G.g3D.z = (z2-z1)/norm;

    n = 20;
    l = 40;
    clength = norm;

    cpoints = (2+n*l+2*n);
    ctriangles = (2*n+(l-1)*n*2);
    npoints = cpoints;
    ntriangles = ctriangles;

    coord=VipAlloc3DBucket_VFLOAT(npoints);
    if(coord==PB) return(PB);
    normal=VipAlloc3DBucket_VFLOAT(npoints);
    if(normal==PB) return(PB);
    triang=VipAlloc3DBucket_S32BIT(ntriangles);
    if(triang==PB) return(PB);


    if(CreateCylinder(&G,n,l,clength,radius,coord,normal,triang)== PB)
      {
	return(PB);
      }

    fprintf(f,"- %d\n",coord->n_points);
    for(i=0;i<coord->n_points;i++)
	fprintf(f,"%f %f %f %f %f %f\n",coord->data[i].x, coord->data[i].y, coord->data[i].z,
		normal->data[i].x, normal->data[i].y, normal->data[i].z);
    fprintf(f,"- %d %d 0\n",triang->n_points, triang->n_points);
    for(i=0;i<triang->n_points;i++)
	fprintf(f,"%ld %ld %ld\n",triang->data[i].x,
		triang->data[i].y,triang->data[i].z);


    VipFree3DBucket_VFLOAT(coord);
    VipFree3DBucket_VFLOAT(normal);
    VipFree3DBucket_S32BIT(triang);

    fclose(f);

    return(OK);
}

/*---------------------------------------------------------------------------*/
int VipWriteHatCylinder(float x1, float y1, float z1, 
		     float x2, float y2, float z2,
		     char *name,
		     float radius,
		     float height)
/*---------------------------------------------------------------------------*/
{
  char filename[512];
    FILE *f;
    char error[256];
    int i;
    VipG3DPoint G;
    Vip3DBucket_VFLOAT *coord, *normal;
    Vip3DBucket_S32BIT *triang;
    int n,l;
    int npoints, cpoints;
    int ntriangles, ctriangles;
    float clength;
    double norm;

    if(name==NULL)
	{
	    VipPrintfError("No electrod name");
	    VipPrintfExit("VipWriteHatCylinder");
	    return(PB);
	}

    strcpy(filename,name);
    strcat(filename,".tri");
    f = fopen(filename,"w");
    if(f==PB)
	{
	    sprintf(error,"Can not open file %s",filename);
	    VipPrintfError(error);
	    VipPrintfExit("VipWriteHatCylinder");
	    return(PB);
	}

    G.p.x = (x1+x2)/2.;
    G.p.y = (y1+y2)/2.;
    G.p.z = (z1+z2)/2.;
    norm = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1));
    if(norm<0.00001)
      {
	VipPrintfError("Equal extremities!");
	VipPrintfExit("VipWriteHatCylinder");
	return(PB);
      }
    G.g3D.x = (x2-x1)/norm;
    G.g3D.y = (y2-y1)/norm;
    G.g3D.z = (z2-z1)/norm;

    n = 20;
    l = 40;
    clength = norm;

    cpoints = (2+n*l+2*n + 2*n+2);
    ctriangles = (2*n+(l-1)*n*2 + 2*n+2);
    npoints = cpoints;
    ntriangles = ctriangles;

    coord=VipAlloc3DBucket_VFLOAT(npoints);
    if(coord==PB) return(PB);
    normal=VipAlloc3DBucket_VFLOAT(npoints);
    if(normal==PB) return(PB);
    triang=VipAlloc3DBucket_S32BIT(ntriangles);
    if(triang==PB) return(PB);


    if(CreateHatCylinder(&G,n,l,clength,radius,height,coord,normal,triang)== PB)
      {
	return(PB);
      }

    fprintf(f,"- %d\n",coord->n_points);
    for(i=0;i<coord->n_points;i++)
	fprintf(f,"%f %f %f %f %f %f\n",coord->data[i].x, coord->data[i].y, coord->data[i].z,
		normal->data[i].x, normal->data[i].y, normal->data[i].z);
    fprintf(f,"- %d %d 0\n",triang->n_points, triang->n_points);
    for(i=0;i<triang->n_points;i++)
	fprintf(f,"%ld %ld %ld\n",triang->data[i].x,
		triang->data[i].y,triang->data[i].z);


    VipFree3DBucket_VFLOAT(coord);
    VipFree3DBucket_VFLOAT(normal);
    VipFree3DBucket_S32BIT(triang);

    fclose(f);

    return(OK);
}

/*---------------------------------------------------------------------------*/
static int CreateCylinder(VipG3DPoint *p, int n, int l, float length, float radius,
    Vip3DBucket_VFLOAT *coord, Vip3DBucket_VFLOAT *normal,
    Vip3DBucket_S32BIT *triang) 
/*---------------------------------------------------------------------------*/
{
    float x,y,z;
    float ux,uy,uz;
    float vx,vy,vz;
    float wx,wy,wz;
    double norm;
    float dlength;
    double teta, teta0;
    int i,j;
    int debut,fin;

    if(n<3 || l<2 || radius<0.0001)
	{
	    VipPrintfError("Bad cylinder sampling");
	    VipPrintfExit("WriteCreateCylinder");
	    return(PB);
	}

    debut = coord->n_points;
    fin = coord->n_points + (2+n*l+2*n);


    ux = p->g3D.x;
    uy = p->g3D.y;
    uz = p->g3D.z;
    x = (float)(p->p.x) - ux/2.*length;
    y = (float)(p->p.y) -uy/2.*length;
    z = (float)(p->p.z) -uz/2.*length;


 

    dlength = length / (l-1);

    norm = ux*ux+uy*uy+uz*uz;
    if(norm<0.00001)
	{
	    VipPrintfWarning("NULL vector in WriteCylinder");
	    printf("Coord: %d %d %d\n",p->p.x,p->p.y,p->p.z);
	    return(OK);
	}
    if(length<0.01)
	{
	    return(OK);
	}

    coord->data[coord->n_points].x = x;
    coord->data[coord->n_points].y = y;
    coord->data[coord->n_points++].z = z;
    normal->data[normal->n_points].x = -ux;
    normal->data[normal->n_points].y = -uy;
    normal->data[normal->n_points++].z = -uz;
    
    vx = uy;
    vy = -ux;
    vz = 0.;
    norm = ux*ux+uy*uy;
    if(norm<0.00001) /*unlucky...*/
	{
	    vx = uz;
	    vy = 0.;
	    vz = -ux;
	    norm = ux*ux+uz*uz;
	    norm = sqrt(norm);
	    vx /= norm;
	    vz /= norm;
	}
    norm = sqrt(norm);
    vx /= norm;
    vy /= norm;

    wx = uy*vz-uz*vy;
    wy = vx*uz-ux*vz;
    wz = ux*vy-uy*vx;

    teta0 = 3.14*2 / n;


    for(i=0;i<n;i++)
	{
	    teta = i*teta0;
	    coord->data[coord->n_points].x = x+(cos(teta)*vx+sin(teta)*wx)*radius;
	    coord->data[coord->n_points].y = y+(cos(teta)*vy+sin(teta)*wy)*radius;
	    coord->data[coord->n_points++].z = z+(cos(teta)*vz+sin(teta)*wz)*radius;
	    normal->data[normal->n_points].x = -ux;
	    normal->data[normal->n_points].y = -uy;
	    normal->data[normal->n_points++].z = -uz;

	}
    for(i=0;i<(n-1);i++)
	{
	    triang->data[triang->n_points].x = debut+i+2;
	    triang->data[triang->n_points].y = debut+(i+1);
	    triang->data[triang->n_points++].z = debut+0;
	}
    triang->data[triang->n_points].x = debut+1;
    triang->data[triang->n_points].y = debut+n;
    triang->data[triang->n_points++].z = debut+0;

    for(i=0;i<n;i++)
	{
	    teta = i*teta0;
	    coord->data[coord->n_points].x = x+(cos(teta)*vx+sin(teta)*wx)*radius;
	    coord->data[coord->n_points].y = y+(cos(teta)*vy+sin(teta)*wy)*radius;
	    coord->data[coord->n_points++].z = z+(cos(teta)*vz+sin(teta)*wz)*radius;
	    normal->data[normal->n_points].x = (cos(teta)*vx+sin(teta)*wx);
	    normal->data[normal->n_points].y = (cos(teta)*vy+sin(teta)*wy);
	    normal->data[normal->n_points++].z = (cos(teta)*vz+sin(teta)*wz);

	}

    for(j=1;j<l;j++)
	{
	    for(i=1;i<=n;i++)
		{
		    teta = i*teta0;
		    coord->data[coord->n_points].x = coord->data[debut+i+n].x+ux*j*dlength;
		    coord->data[coord->n_points].y = coord->data[debut+i+n].y+uy*j*dlength;
		    coord->data[coord->n_points++].z = coord->data[debut+i+n].z + uz*j*dlength;
		    normal->data[normal->n_points].x = (cos(teta)*vx+sin(teta)*wx);
		    normal->data[normal->n_points].y = (cos(teta)*vy+sin(teta)*wy);
		    normal->data[normal->n_points++].z = (cos(teta)*vz+sin(teta)*wz);    
		}
	    for(i=1;i<=(n-1);i++)
		{
		    triang->data[triang->n_points].z = debut+i+n*(j-1)+n;
		    triang->data[triang->n_points].y = debut+i+n*j+n;
		    triang->data[triang->n_points++].x = debut+i+1+n*j+n;
		}
	    triang->data[triang->n_points].z = debut+ n+n*(j-1)+n;
	    triang->data[triang->n_points].y =debut+ n*j+n+n;
	    triang->data[triang->n_points++].x =debut+ n*j+1+n;
	    for(i=1;i<=(n-1);i++)
		{
		    triang->data[triang->n_points].z = debut+i+n*(j-1)+n;
		    triang->data[triang->n_points].y = debut+i+1+n*j+n;
		    triang->data[triang->n_points++].x = debut+i+1+n*(j-1)+n;
		}
	    triang->data[triang->n_points].z = debut+n+n*(j-1)+n;
	    triang->data[triang->n_points].y = debut+n*j+1+n;
	    triang->data[triang->n_points++].x =debut+n*(j-1)+1+n;
	}

    j = l-1;
    for(i=1;i<=n;i++)
	{
	    teta = i*teta0;
	    coord->data[coord->n_points].x = coord->data[debut+i+n].x+ux*j*dlength;
	    coord->data[coord->n_points].y = coord->data[debut+i+n].y+uy*j*dlength;
	    coord->data[coord->n_points++].z = coord->data[debut+i+n].z + uz*j*dlength;
	    normal->data[normal->n_points].x = ux;
	    normal->data[normal->n_points].y = uy;
	    normal->data[normal->n_points++].z = uz;    
	}

    coord->data[coord->n_points].x = x+ux*length;
    coord->data[coord->n_points].y = y+uy*length;
    coord->data[coord->n_points++].z = z+uz*length;
    normal->data[normal->n_points].x = ux;
    normal->data[normal->n_points].y = uy;
    normal->data[normal->n_points++].z = uz;

    for(i=1;i<=(n-1);i++)
	{
	    triang->data[triang->n_points].x = fin-1-i-1;
	    triang->data[triang->n_points].y = fin-1-i;
	    triang->data[triang->n_points++].z = fin-1;
	}
    triang->data[triang->n_points].x = fin-1-1;
    triang->data[triang->n_points].y = fin-1-n;
    triang->data[triang->n_points++].z = fin-1;

  


    return(OK);

}

/*---------------------------------------------------------------------------*/
static int CreateHatCylinder(VipG3DPoint *p, int n, int l, float length, float radius, float hatheight,
    Vip3DBucket_VFLOAT *coord, Vip3DBucket_VFLOAT *normal,
    Vip3DBucket_S32BIT *triang) 
/*---------------------------------------------------------------------------*/
{
    float x,y,z;
    float ux,uy,uz;
    float vx,vy,vz;
    float wx,wy,wz;
    double norm;
    float dlength;
    double teta, teta0;
    int i,j;
    int debut,fin;

    if(n<3 || l<2 || radius<0.0001)
	{
	    VipPrintfError("Bad cylinder sampling");
	    VipPrintfExit("WriteCreateCylinder");
	    return(PB);
	}

    debut = coord->n_points;
    fin = coord->n_points + (2+n*l+2*n);


    ux = p->g3D.x;
    uy = p->g3D.y;
    uz = p->g3D.z;
    x = (float)(p->p.x) - ux/2.*length;
    y = (float)(p->p.y) -uy/2.*length;
    z = (float)(p->p.z) -uz/2.*length;

    dlength = length / (l-1);

    norm = ux*ux+uy*uy+uz*uz;
    if(norm<0.00001)
	{
	    VipPrintfWarning("NULL vector in CreateHatCylinder");
	    printf("Coord: %d %d %d\n",p->p.x,p->p.y,p->p.z);
	    return(OK);
	}
    if(length<0.01)
	{
	    return(OK);
	}

    coord->data[coord->n_points].x = x;
    coord->data[coord->n_points].y = y;
    coord->data[coord->n_points++].z = z;
    normal->data[normal->n_points].x = -ux;
    normal->data[normal->n_points].y = -uy;
    normal->data[normal->n_points++].z = -uz;
    
    vx = uy;
    vy = -ux;
    vz = 0.;
    norm = ux*ux+uy*uy;
    if(norm<0.00001) /*unlucky...*/
	{
	    vx = uz;
	    vy = 0.;
	    vz = -ux;
	    norm = ux*ux+uz*uz;
	    norm = sqrt(norm);
	    vx /= norm;
	    vz /= norm;
	}
    norm = sqrt(norm);
    vx /= norm;
    vy /= norm;

    wx = uy*vz-uz*vy;
    wy = vx*uz-ux*vz;
    wz = ux*vy-uy*vx;

    teta0 = 3.14*2 / n;


    for(i=0;i<n;i++)
	{
	    teta = i*teta0;
	    coord->data[coord->n_points].x = x+(cos(teta)*vx+sin(teta)*wx)*radius;
	    coord->data[coord->n_points].y = y+(cos(teta)*vy+sin(teta)*wy)*radius;
	    coord->data[coord->n_points++].z = z+(cos(teta)*vz+sin(teta)*wz)*radius;
	    normal->data[normal->n_points].x = -ux;
	    normal->data[normal->n_points].y = -uy;
	    normal->data[normal->n_points++].z = -uz;

	}
    for(i=0;i<(n-1);i++)
	{
	    triang->data[triang->n_points].x = debut+i+2;
	    triang->data[triang->n_points].y = debut+(i+1);
	    triang->data[triang->n_points++].z = debut+0;
	}
    triang->data[triang->n_points].x = debut+1;
    triang->data[triang->n_points].y = debut+n;
    triang->data[triang->n_points++].z = debut+0;

    for(i=0;i<n;i++)
	{
	    teta = i*teta0;
	    coord->data[coord->n_points].x = x+(cos(teta)*vx+sin(teta)*wx)*radius;
	    coord->data[coord->n_points].y = y+(cos(teta)*vy+sin(teta)*wy)*radius;
	    coord->data[coord->n_points++].z = z+(cos(teta)*vz+sin(teta)*wz)*radius;
	    normal->data[normal->n_points].x = (cos(teta)*vx+sin(teta)*wx);
	    normal->data[normal->n_points].y = (cos(teta)*vy+sin(teta)*wy);
	    normal->data[normal->n_points++].z = (cos(teta)*vz+sin(teta)*wz);

	}

    for(j=1;j<l;j++)
	{
	    for(i=1;i<=n;i++)
		{
		    teta = i*teta0;
		    coord->data[coord->n_points].x = coord->data[debut+i+n].x+ux*j*dlength;
		    coord->data[coord->n_points].y = coord->data[debut+i+n].y+uy*j*dlength;
		    coord->data[coord->n_points++].z = coord->data[debut+i+n].z + uz*j*dlength;
		    normal->data[normal->n_points].x = (cos(teta)*vx+sin(teta)*wx);
		    normal->data[normal->n_points].y = (cos(teta)*vy+sin(teta)*wy);
		    normal->data[normal->n_points++].z = (cos(teta)*vz+sin(teta)*wz);    
		}
	    for(i=1;i<=(n-1);i++)
		{
		    triang->data[triang->n_points].z = debut+i+n*(j-1)+n;
		    triang->data[triang->n_points].y = debut+i+n*j+n;
		    triang->data[triang->n_points++].x = debut+i+1+n*j+n;
		}
	    triang->data[triang->n_points].z = debut+ n+n*(j-1)+n;
	    triang->data[triang->n_points].y =debut+ n*j+n+n;
	    triang->data[triang->n_points++].x =debut+ n*j+1+n;
	    for(i=1;i<=(n-1);i++)
		{
		    triang->data[triang->n_points].z = debut+i+n*(j-1)+n;
		    triang->data[triang->n_points].y = debut+i+1+n*j+n;
		    triang->data[triang->n_points++].x = debut+i+1+n*(j-1)+n;
		}
	    triang->data[triang->n_points].z = debut+n+n*(j-1)+n;
	    triang->data[triang->n_points].y = debut+n*j+1+n;
	    triang->data[triang->n_points++].x =debut+n*(j-1)+1+n;
	}

    j = l-1;
    for(i=1;i<=n;i++)
	{
	    teta = i*teta0;
	    coord->data[coord->n_points].x = coord->data[debut+i+n].x+ux*j*dlength;
	    coord->data[coord->n_points].y = coord->data[debut+i+n].y+uy*j*dlength;
	    coord->data[coord->n_points++].z = coord->data[debut+i+n].z + uz*j*dlength;
	    normal->data[normal->n_points].x = ux;
	    normal->data[normal->n_points].y = uy;
	    normal->data[normal->n_points++].z = uz;    
	}

    coord->data[coord->n_points].x = x+ux*length;
    coord->data[coord->n_points].y = y+uy*length;
    coord->data[coord->n_points++].z = z+uz*length;
    normal->data[normal->n_points].x = ux;
    normal->data[normal->n_points].y = uy;
    normal->data[normal->n_points++].z = uz;

    for(i=1;i<=(n-1);i++)
	{
	    triang->data[triang->n_points].x = fin-1-i-1;
	    triang->data[triang->n_points].y = fin-1-i;
	    triang->data[triang->n_points++].z = fin-1;
	}
    triang->data[triang->n_points].x = fin-1-1;
    triang->data[triang->n_points].y = fin-1-n;
    triang->data[triang->n_points++].z = fin-1;

    /*hat*/

    debut = coord->n_points;
    coord->data[coord->n_points].x = x+ux*length;
    coord->data[coord->n_points].y = y+uy*length;
    coord->data[coord->n_points++].z = z+uz*length;
    normal->data[normal->n_points].x = -ux;
    normal->data[normal->n_points].y = -uy;
    normal->data[normal->n_points++].z = -uz;

    for(i=0;i<n;i++)
      {
	teta = i*teta0;
	coord->data[coord->n_points].x = x+ux*length+(cos(teta)*vx+sin(teta)*wx)*radius*2;
	coord->data[coord->n_points].y = y+uy*length+(cos(teta)*vy+sin(teta)*wy)*radius*2;
	coord->data[coord->n_points++].z = z+uz*length+(cos(teta)*vz+sin(teta)*wz)*radius*2;
	normal->data[normal->n_points].x = -ux;
	normal->data[normal->n_points].y = -uy;
	normal->data[normal->n_points++].z = -uz;

	}
    for(i=0;i<(n-1);i++)
	{
	    triang->data[triang->n_points].x = debut+i+2;
	    triang->data[triang->n_points].y = debut+(i+1);
	    triang->data[triang->n_points++].z = debut+0;
	}
    triang->data[triang->n_points].x = debut+1;
    triang->data[triang->n_points].y = debut+n;
    triang->data[triang->n_points++].z = debut+0;

    debut = coord->n_points;
    coord->data[coord->n_points].x = x+ux*(length+hatheight);
    coord->data[coord->n_points].y = y+uy*(length+hatheight);
    coord->data[coord->n_points++].z = z+uz*(length+hatheight);
    normal->data[normal->n_points].x = ux;
    normal->data[normal->n_points].y = uy;
    normal->data[normal->n_points++].z = uz;

    for(i=0;i<n;i++)
      {
	teta = i*teta0;
	coord->data[coord->n_points].x = x+ux*length+(cos(teta)*vx+sin(teta)*wx)*radius*2;
	coord->data[coord->n_points].y = y+uy*length+(cos(teta)*vy+sin(teta)*wy)*radius*2;
	coord->data[coord->n_points++].z = z+uz*length+(cos(teta)*vz+sin(teta)*wz)*radius*2;
	normal->data[normal->n_points].x = (cos(teta)*vx+sin(teta)*wx);
	normal->data[normal->n_points].y = (cos(teta)*vy+sin(teta)*wy);
	normal->data[normal->n_points++].z = (cos(teta)*vz+sin(teta)*wz);

	}
    for(i=0;i<(n-1);i++)
	{
	    triang->data[triang->n_points].x = debut+i+1;
	    triang->data[triang->n_points].y = debut+(i+2);
	    triang->data[triang->n_points++].z = debut+0;
	}
    triang->data[triang->n_points].x = debut+n;
    triang->data[triang->n_points].y = debut+1;
    triang->data[triang->n_points++].z = debut+0;

    return(OK);

}
