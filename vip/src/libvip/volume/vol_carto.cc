/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME :                      * TYPE     : Source
 * AUTHOR      : RIVIERE D.           * CREATION : 05/12/2004
 * VERSION     : 2.6                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : 
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
 *****************************************************************************/

#include <vip/volume/vol_carto.h>

#ifdef VIP_CARTO_VOLUME_WRAPPING

#include <vip/volume/vol_carto_cpp.h>
#include <vip/volume.h>
#include <vip/alloc.h>
#include <vip/util.h>
#include <cartodata/volume/volume.h>
#include <aims/data/data.h>
#include <aims/io/process.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/io/finder.h>
#include <iostream>

// #define VIP_CARTOVOL_DEBUG

using namespace std;
using namespace aims;
using namespace carto;


struct VipVolumeCartoPrivate
{
  VipVolumeCartoPrivate() : vol() {}

  Object	vol;
};


void VipVolumeCartoAllocStruct( ::Volume *volume )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoAllocStruct\n";
#endif
  volume->carto = new VipVolumeCartoPrivate;
}


int VipVolumeCartoFreeStruct( ::Volume *volume )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoFreeStruct " << volume->name << endl;
#endif
  if( volume->carto && volume->carto->vol.get() )
    volume->data = 0;
  delete volume->carto;
  volume->carto = 0;
  return YES;
}


int VipVolumeCartoFreeData( ::Volume *volume )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoFreeData " << volume->name << endl;
#endif
  int	res = NO;
  if( volume->carto && volume->carto->vol.get() )
    {
      volume->data = 0;
      volume->carto->vol = Object();
      volume->state = STATE_DECLARED;
      volume->size3Db = 0;
      res = YES;
    }
  return res;
}


namespace
{

  template <typename T>
  PropertySet* getCartoHeader( ::Volume *volume )
  {
    return &volume->carto->vol->
      GenericObject::value<rc_ptr<carto::Volume<T> > >()->header();
  }


  void copyHeader2( const PropertySet & psr, PropertySet & psw )
  {
    Object              i = psr.objectIterator();
    static set<string>  forbidden;
    set<string>::const_iterator allowed = forbidden.end();
    string              key;

    if( forbidden.empty() )
      {
        forbidden.insert( "sizeX" );
        forbidden.insert( "sizeY" );
        forbidden.insert( "sizeZ" );
        forbidden.insert( "sizeT" );
        forbidden.insert( "_borderWidth" );
        forbidden.insert( "volume_dimension" );
      }

    for( ; i->isValid(); i->next() )
      {
        key = i->key();
        if( forbidden.find( key ) == allowed )
          psw.setProperty( key, i->currentValue() );
      }
  }

  template<typename T>
  void copyHeader( const carto::Volume<T> & volR, carto::Volume<T> & volW )
  {
    const PropertySet	& psr = volR.header();
    PropertySet		& psw = volW.header();
    copyHeader2( psr, psw );
  }


  // retreive volume header even after a type change on Vip side
  PropertySet *getAlienHeader( ::Volume *volume )
  {
    Object ovol = volume->carto->vol;
    if( ovol.get() )
    {
      string dt = ovol->type();
      if( dt == DataTypeCode<rc_ptr<carto::Volume<int8_t> > >::name() )
        return getCartoHeader<int8_t>( volume );
      if( dt == DataTypeCode<rc_ptr<carto::Volume<uint8_t> > >::name() )
        return getCartoHeader<uint8_t>( volume );
      if( dt == DataTypeCode<rc_ptr<carto::Volume<int16_t> > >::name() )
        return getCartoHeader<int16_t>( volume );
      if( dt == DataTypeCode<rc_ptr<carto::Volume<uint16_t> > >::name() )
        return getCartoHeader<uint16_t>( volume );
      if( dt == DataTypeCode<rc_ptr<carto::Volume<int32_t> > >::name() )
        return getCartoHeader<int32_t>( volume );
      if( dt == DataTypeCode<rc_ptr<carto::Volume<uint32_t> > >::name() )
        return getCartoHeader<uint32_t>( volume );
      if( dt == DataTypeCode<rc_ptr<carto::Volume<float> > >::name() )
        return getCartoHeader<float>( volume );
      if( dt == DataTypeCode<rc_ptr<carto::Volume<double> > >::name() )
        return getCartoHeader<double>( volume );
    }
    return 0;
  }


  template<typename T>
  int cartoAllocData( ::Volume* volume )
  {
#ifdef VIP_CARTOVOL_DEBUG
    cout << "cartoAllocData " << DataTypeCode<T>::name() << ", " 
         << volume->name << endl;
#endif

    if( !volume->carto )
      return PB;

    rc_ptr<carto::Volume<T> > 
      vol( new carto::Volume<T>( volume->size.x + volume->borderWidth * 2, 
                                 volume->size.y + volume->borderWidth * 2, 
                                 volume->size.z + volume->borderWidth * 2, 
                                 volume->size.t ) );
    if( volume->borderWidth != 0 )
    {
      vol.reset( new carto::Volume<T>(
        vol,
        typename carto::Volume<T>::Position4Di( volume->borderWidth,
                                       volume->borderWidth,
                                       volume->borderWidth, 0 ),
        typename carto::Volume<T>::Position4Di( volume->size.x, volume->size.y,
                                       volume->size.z, volume->size.t ) ) );
    }

    // keep previous header
    if( volume->carto->vol.get() )
      try
        {
          copyHeader2( *getAlienHeader( volume ), vol->header() );
        }
      catch( ... )
        {
        }

    volume->carto->vol = Object::value( vol );
    if( volume->borderWidth != 0 )
    {
      /* with a border, the data pointer is relative to the bigger borderd 
         volume. */
      volume->data = (char *) &*vol->refVolume()->begin();
    }
    else
      volume->data = (char *) &*vol->begin();
    volume->tab = NULL;
    VipSetState( volume, STATE_ALLOCATED );
    return OK;
  }


  template<typename T>
  void cartoCopyHdr( ::Volume *volumeR, ::Volume *volumeW )
  {
    if( !volumeR->carto || !volumeR->carto->vol.get()
      || !volumeW->carto || !volumeW->carto->vol.get() )
      return;
    try
    {
      rc_ptr<carto::Volume<T> >	volR = volumeR->carto->vol
        ->GenericObject::value<rc_ptr<carto::Volume<T> > >();
      rc_ptr<carto::Volume<T> >	volW = volumeW->carto->vol
        ->GenericObject::value<rc_ptr<carto::Volume<T> > >();
      copyHeader( *volR, *volW );
    }
    catch( ... )
    {
    }
  }


  template<typename T>
  void cartoCopyStruct( ::Volume *volumeR, ::Volume *volumeW )
  {
    if( !volumeR->carto || !volumeR->carto->vol.get() )
      return;
    try
    {
      rc_ptr<carto::Volume<T> >       volR = volumeR->carto->vol
        ->GenericObject::value<rc_ptr<carto::Volume<T> > >();
      rc_ptr<carto::Volume<T> >       volW( new carto::Volume<T> );
      copyHeader( *volR, *volW );
      volumeW->carto->vol = Object::value( volW );
    }
    catch( ... )
    {
    }
  }


  template <typename T>
  void cartoResizeBorder( ::Volume *volume, int borderWidth )
  {
    // resize border while keeping the global (large) volume identical:
    // no reallocation
    rc_ptr<carto::Volume<T> >   vol1 
      = volume->carto->vol->GenericObject::value<rc_ptr<carto::Volume<T> > >();

    // take large volume
    rc_ptr<carto::Volume<T> >   vollarge = vol1;
    if( vol1->refVolume() )
      vollarge = vol1->refVolume();
    rc_ptr<carto::Volume<T> > vol2;
    if( borderWidth == 0 )
      // no border: the new vol is the older large one
      vol2 = vollarge;
    else
    {
      vol2.reset( new carto::Volume<T>( vollarge,
        typename carto::Volume<T>::Position4Di(
          borderWidth, borderWidth, borderWidth, 0 ),
        typename carto::Volume<T>::Position4Di(
          vollarge->getSizeX() - borderWidth * 2,
          vollarge->getSizeY() - borderWidth * 2,
          vollarge->getSizeZ() - borderWidth * 2,
          vollarge->getSizeT() ) ) );
      copyHeader2( vollarge->header(), vol2->header() );
    }
    // no need to copy data

    volume->carto->vol = Object::value( vol2 );
  }

}


int VipVolumeCartoResizeBorder( ::Volume* volume, int borderWidth )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoResizeBorder " << volume->name << endl;
#endif

  if( volume->carto && volume->carto->vol.get() )
  {
    switch( volume->type )
    {
    case U8BIT:
      cartoResizeBorder<uint8_t>( volume, borderWidth );
      return OK;
    case S8BIT:
      cartoResizeBorder<int8_t>( volume, borderWidth );
      return OK;
    case U16BIT:
      cartoResizeBorder<uint16_t>( volume, borderWidth );
      return OK;
    case S16BIT:
      cartoResizeBorder<int16_t>( volume, borderWidth );
      return OK;
    case U32BIT:
      cartoResizeBorder<uint32_t>( volume, borderWidth );
      return OK;
    case S32BIT:
      cartoResizeBorder<int32_t>( volume, borderWidth );
      return OK;
    case VFLOAT:
      cartoResizeBorder<float>( volume, borderWidth );
      return OK;
    case VDOUBLE:
      cartoResizeBorder<double>( volume, borderWidth );
      return OK;
    default:
      cerr << "VipVolumeCartoResizeHeader: unknown type " 
           << volume->type << endl;
    }
  }
  return PB;
}


int VipVolumeCartoAllocData( ::Volume* volume )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoAllocData " << volume->name << endl;
#endif
  switch( volume->type )
    {
    case U8BIT:
      return cartoAllocData<uint8_t>( volume );
    case S8BIT:
      return cartoAllocData<int8_t>( volume );
    case U16BIT:
      return cartoAllocData<uint16_t>( volume );
    case S16BIT:
      return cartoAllocData<int16_t>( volume );
    case U32BIT:
      return cartoAllocData<uint32_t>( volume );
    case S32BIT:
      return cartoAllocData<int32_t>( volume );
    case VFLOAT:
      return cartoAllocData<float>( volume );
    case VDOUBLE:
      return cartoAllocData<double>( volume );
    default:
      cerr << "VipVolumeCartoAllocData: unknown type " << volume->type << endl;
      break;
    }
  return PB;
}


void VipVolumeCartoCopyStruct( ::Volume * volumeR, ::Volume *volumeW )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoCopyStruct " << volumeR->name << " -> " 
       << volumeW->name << endl;
#endif

  switch( volumeR->type )
    {
    case U8BIT:
      cartoCopyStruct<uint8_t>( volumeR, volumeW );
      break;
    case S8BIT:
      cartoCopyStruct<int8_t>( volumeR, volumeW );
      break;
    case U16BIT:
      cartoCopyStruct<uint16_t>( volumeR, volumeW );
      break;
    case S16BIT:
      cartoCopyStruct<int16_t>( volumeR, volumeW );
      break;
    case U32BIT:
      cartoCopyStruct<uint32_t>( volumeR, volumeW );
      break;
    case S32BIT:
      cartoCopyStruct<int32_t>( volumeR, volumeW );
      break;
    case VFLOAT:
      cartoCopyStruct<float>( volumeR, volumeW );
      break;
    case VDOUBLE:
      cartoCopyStruct<double>( volumeR, volumeW );
      break;
    default:
      cerr << "VipVolumeCartoCopyStruct: unknown type " << volumeR->type 
           << endl;
      volumeW->carto->vol = Object();
    }
}


void VipVolumeCartoCopyHeader( ::Volume * volumeR, ::Volume *volumeW )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoCopyStruct " << volumeR->name << " -> "
       << volumeW->name << endl;
#endif

  switch( volumeR->type )
    {
    case U8BIT:
      cartoCopyHdr<uint8_t>( volumeR, volumeW );
      break;
    case S8BIT:
      cartoCopyHdr<int8_t>( volumeR, volumeW );
      break;
    case U16BIT:
      cartoCopyHdr<uint16_t>( volumeR, volumeW );
      break;
    case S16BIT:
      cartoCopyHdr<int16_t>( volumeR, volumeW );
      break;
    case U32BIT:
      cartoCopyHdr<uint32_t>( volumeR, volumeW );
      break;
    case S32BIT:
      cartoCopyHdr<int32_t>( volumeR, volumeW );
      break;
    case VFLOAT:
      cartoCopyHdr<float>( volumeR, volumeW );
      break;
    case VDOUBLE:
      cartoCopyHdr<double>( volumeR, volumeW );
      break;
    default:
      cerr << "VipVolumeCartoCopyStruct: unknown type " << volumeR->type
           << endl;
      volumeW->carto->vol = Object();
    }
}


namespace
{

  template<typename T> int 
  copyVolumeData( ::Volume *volumeR, ::Volume *volumeW )
  {
    if( volumeR->size.x == volumeW->size.x 
        && volumeR->size.y == volumeW->size.y 
        && volumeR->size.z == volumeW->size.z 
        && volumeR->size.t == volumeW->size.t 
        && volumeR->borderWidth == volumeW->borderWidth )
      {
        /*
        cout << "carto copy direct: vs: " << volumeW->voxelSize.x << ", " 
            << volumeW->voxelSize.y << ", " << volumeW->voxelSize.z << endl;
        cout << "orig: " << volumeR->voxelSize.x << ", " 
            << volumeR->voxelSize.y << ", " << volumeR->voxelSize.z << endl;
        */
        try
        {
          rc_ptr<carto::Volume<T> >	volR 
            = volumeR->carto->vol
            ->GenericObject::value<rc_ptr<carto::Volume<T> > >();
          rc_ptr<carto::Volume<T> >	volW;
          if( !volR->refVolume().isNull() )
          {
            // volume with border: copy the larger one
            volW.reset( new carto::Volume<T>( *volR->refVolume() ) );
            volumeW->carto->vol = Object::value( volW );
            // with border, point to the larger volume
            volumeW->data = (char *) &*volW->begin();
            volW.reset( new carto::Volume<T>( volW, volR->posInRefVolume(), 
              typename carto::Volume<T>::Position4Di( volR->getSizeX(),
                volR->getSizeY(), volR->getSizeZ(), volR->getSizeT() ) ) );
            volumeW->carto->vol = Object::value( volW );
            if( volR->allocatorContext().allocatorType() 
                != AllocatorStrategy::Unallocated )
            {
              cout << "WARNING: copied volume in duplicated as border + volume !\n";
              *volW = *volR; // this should NOT happen...
            }
          }
          else
          {
            volW.reset( new carto::Volume<T>( *volR ) );
            volumeW->carto->vol = Object::value( volW );
            volumeW->data = (char *) &*volW->begin();
          }
        }
        catch( exception & e )
        {
          cerr << e.what() << endl;
          return PB;
        }
        volumeW->state = STATE_FILLED;
        volumeW->tab = NULL;
        return OK;
      }

     // differing sizes. Copy manually

     if( cartoAllocData<T>( volumeW ) == PB )
       return PB;
     cout << "carto copy full: vs: " << volumeW->voxelSize.x << ", " 
          << volumeW->voxelSize.y << ", " << volumeW->voxelSize.z << endl;

     if( volumeW->size.x > volumeR->size.x ||
         volumeW->size.y > volumeR->size.y ||
         volumeW->size.z > volumeR->size.z )
       {
         VipPrintfWarning ( "Destination volume is over-dimensioned" );
         cerr << "\tThe destination volume will not be completely filled."
              << "\n\tIn VipTransferVolumeData." << endl;
       }
        
     if( volumeW->size.x < volumeR->size.x ||
         volumeW->size.y < volumeR->size.y ||
         volumeW->size.z < volumeR->size.z )
       {
         VipPrintfWarning ( "Source volume is over-dimensioned." );
         cerr << "\tThe source volume will not be completely copied." 
              << "\n\tIn VipTransferVolumeData." << endl;
       }

     copyHeader( *volumeR->carto->vol
                 ->GenericObject::value<rc_ptr<carto::Volume<T> > >(), 
                 *volumeW->carto->vol
                 ->GenericObject::value<rc_ptr<carto::Volume<T> > >() );

     int	nx,
       iy, ny,
       iz, nz,
       bytesPerVoxel,
       oLR, oLW,
       oCR, oCW;
     char	*ptrDataR,
       *ptrDataW;

     bytesPerVoxel = sizeof( T );

     nx = mVipMin ( volumeR->size.x, volumeW->size.x );
     ny = mVipMin ( volumeR->size.y, volumeW->size.y );
     nz = mVipMin ( volumeR->size.z, volumeW->size.z );

     oCR = ( volumeR->size.x - nx + VipOffsetPointBetweenLine ( volumeR ) )
       * bytesPerVoxel;
     oCW = ( volumeW->size.x - nx + VipOffsetPointBetweenLine ( volumeW ) )
       * bytesPerVoxel;
     oLR = ( ( volumeR->size.y - ny ) * VipOffsetLine ( volumeR ) +
             VipOffsetLineBetweenSlice ( volumeR ) ) * bytesPerVoxel;
     oLW = ( ( volumeW->size.y - ny ) * VipOffsetLine ( volumeW ) +
             VipOffsetLineBetweenSlice ( volumeW ) ) * bytesPerVoxel;

     ptrDataR = volumeR->data 
       + VipOffsetFirstPoint ( volumeR ) * bytesPerVoxel;
     ptrDataW = volumeW->data 
       + VipOffsetFirstPoint ( volumeW ) * bytesPerVoxel;

     nx *= bytesPerVoxel;
     oCR += nx;
     oCW += nx;

     for ( iz = 0; iz < nz; iz++ )
       {
         for ( iy = 0; iy < ny; iy++ )
           {
             memcpy ( (void*)ptrDataW, (void*)ptrDataR, nx );
             ptrDataR += oCR;
             ptrDataW += oCW;
           }
         ptrDataR += oLR;
         ptrDataW += oLW;
       }

     VipSetState ( volumeW, STATE_FILLED );
	
     return OK;
   }

}


int VipVolumeCartoTransferVolumeData( ::Volume *volumeR, ::Volume *volumeW )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoTransferVolumeData " << volumeR->name << " -> " 
       << volumeW->name << endl;
#endif

  if( !volumeR->carto || !volumeR->carto->vol.get() )
    return PB;

  if( ( !volumeW->carto || !volumeW->carto->vol.get() )
      && ( volumeW->state == STATE_ALLOCATED 
           || volumeW->state == STATE_FILLED ) )
    VipFreeVolumeData( volumeW );

  switch( volumeR->type )
    {
    case U8BIT:
      return copyVolumeData<uint8_t>( volumeR, volumeW );
    case S8BIT:
      return copyVolumeData<int8_t>( volumeR, volumeW );
    case U16BIT:
      return copyVolumeData<uint16_t>( volumeR, volumeW );
    case S16BIT:
      return copyVolumeData<int16_t>( volumeR, volumeW );
    case U32BIT:
      return copyVolumeData<uint32_t>( volumeR, volumeW );
    case S32BIT:
      return copyVolumeData<int32_t>( volumeR, volumeW );
    case VFLOAT:
      return copyVolumeData<float>( volumeR, volumeW );
    case VDOUBLE:
      return copyVolumeData<double>( volumeR, volumeW );
    default:
      cerr << "VipVolumeCartoTransferVolumeData: unknown type " 
           << volumeR->type << endl;
    }
  return PB;
}


namespace
{

  template<typename T> int mytype();

  template<> int mytype<uint8_t>()
  {
    return U8BIT;
  }

  template<> int mytype<int8_t>()
  {
    return S8BIT;
  }

  template<> int mytype<uint16_t>()
  {
    return U16BIT;
  }

  template<> int mytype<int16_t>()
  {
    return S16BIT;
  }

  template<> int mytype<uint32_t>()
  {
    return U32BIT;
  }

  template<> int mytype<int32_t>()
  {
    return S32BIT;
  }

  template<> int mytype<float>()
  {
    return VFLOAT;
  }

  template<> int mytype<double>()
  {
    return VDOUBLE;
  }


  template<typename T>
  ::Volume* VipVolumeCartoCreate( rc_ptr<carto::Volume<T> > vol, 
                                  const string & fname, int border )
  {
    vector<float>	vs( 4 );
    vs[0] = vs[1] = vs[2] = vs[3] = 1.f;
    try
      {
        Object	vs1 = vol->header().getProperty( "voxel_size" );
        if( vs1.get() )
          {
            vector<float> & vs2 = vs1->GenericObject::value<vector<float> >();
            if( vs2.size() >= 4 )
              vs[3] = vs2[3];
            if( vs2.size() >= 3 )
              vs[2] = vs2[2];
            if( vs2.size() >= 2 )
              vs[1] = vs2[1];
            if( vs2.size() >= 1 )
              vs[0] = vs2[0];
          }
      }
    catch( ... )
      {
        // oooh well, never mind...
      }

    /*
    cout << "VipVolumeCartoCreate " << fname << ", vol: " << vol.get() 
         << ", vs : " << vs[0] << ", " << vs[1] << ", " << vs[2] << ", " 
         << vs[3] << endl;
    cout << "dim: " << vol->getSizeX() << ", " << vol->getSizeY() << ", " 
         << vol->getSizeZ() << ", " << vol->getSizeT() << endl;
    cout << "vs: " << vs[0] << ", "<< vs[1] << ", " << vs[2] << ", "
         << vs[3] << endl;
    */

    ::Volume *v 
        = VipDeclare4DVolumeStructure( vol->getSizeX(),
                                       vol->getSizeY(),
                                       vol->getSizeZ(),
                                       vol->getSizeT(),
                                       vs[0], vs[1], vs[2], vs[3],
                                       mytype<T>(),
                                       const_cast<char *>( fname.c_str() ),
                                       border );
    if( !v )
      return 0;
    VipVolumeCartoAllocStruct( v );
    v->carto->vol = Object::value( vol );
    if( !vol->refVolume().isNull() )
      // with border, point to the bigger volume
      v->data = (char *) &*vol->refVolume()->begin();
    else
      v->data = (char *) &*vol->begin();
    v->tab = NULL;
    VipSetState( v, STATE_ALLOCATED );

    /*
    if( !VipAllocateVolumeData( v ) )
      {
        VipFreeVolume( v );
        return 0;
      }
    */
    return v;
  }


  class VipReader : public Process
  {
  public:
    VipReader( int borderWidth, int fr );
    virtual ~VipReader();

    template<typename T> static 
    bool doit( Process &, const string &, Finder & );

    int 	border;
    int		frame;
    ::Volume	*volume;
  };


  VipReader::VipReader( int borderWidth, int fr )
    : border( borderWidth ), frame( fr ), volume( 0 )
  {
  }


  VipReader::~VipReader()
  {
  }


  template<typename T>
  bool VipReader::doit( Process & p, const string & fname, Finder & f )
  {
    // cout << "VipReader::doit<" << DataTypeCode<T>::name() << ">" << endl;

    string			format = f.format();
    VipReader			&vr = (VipReader &) p;

    /* */
    Reader<carto::Volume<T> >	r( fname );
    rc_ptr<carto::Volume<T> >	vol;

    vol.reset( r.read( vr.border, &format, vr.frame ) );
    if( !vol.get() )
      return false;

    vr.volume 
      = VipVolumeCartoCreate( vol, fname, vr.border );
    if( !vr.volume )
      return false;

    VipSetState ( vr.volume, STATE_FILLED );

    return true;
  }


  template<typename T>
  int writeCartoVolume( const carto::Volume<T> & vol, const char * fname, 
                        int format )
  {
#ifdef VIP_CARTOVOL_DEBUG
    cout << "writeCartoVolume " << fname << endl;
#endif

    try
      {
        Writer<carto::Volume<T> >	w( fname );
        string	fmt, *fmt2 = &fmt;
        switch( format )
          {
          case VIDA:
            fmt = "VIDA";
            break;
          case TIVOLI:
            fmt = "GIS";
            break;
          case SPM:
            fmt = "SPM";
            break;
          case MINC:
            fmt = "MINC";
            break;
          default:
            fmt2 = 0;
          }
        w.write( vol, 0, fmt2 );
        return OK;
      }
    catch( exception & e )
      {
        cerr << e.what() << endl;
      }
    return PB;
  }


  template<typename T>
  int writeCartoVipVolume( ::Volume* volume, const char * fname, int format )
  {
    /*
    cout << "writeObject " << DataTypeCode<T>::name() << ", file: " 
         << fname << ", format: " << format << endl;

    cout << "size: " << volume->size.x << ", border: " << volume->borderWidth 
         << endl;
    cout << "Vip vs: " << volume->voxelSize.x << ", " << volume->voxelSize.y 
         << ", " << volume->voxelSize.z << ", " << volume->voxelSize.t << endl;
    */

    vector<float>	vs(4);
    vs[0] = volume->voxelSize.x;
    vs[1] = volume->voxelSize.y;
    vs[2] = volume->voxelSize.z;
    vs[3] = volume->voxelSize.t;

    if( volume->carto && volume->carto->vol.get() )
      {
        rc_ptr<carto::Volume<T> >	ovol = volume->carto->vol
          ->GenericObject::value<rc_ptr<carto::Volume<T> > >();

        ovol->header().setProperty( "voxel_size", vs );
        return writeCartoVolume( *ovol, fname, format );
      }

    // old Vip allocation way (should not exist anymore)

    carto::rc_ptr<carto::Volume<T> > vol(
      new carto::Volume<T>( volume->size.x + volume->borderWidth * 2,
                            volume->size.y + volume->borderWidth * 2, 
                            volume->size.z + volume->borderWidth * 2, 
                            volume->size.t, 
                            (T *) volume->data ) );
    if( volume->borderWidth != 0 )
    {
      // cout << "vol with border: " << volume->borderWidth << endl;
      vol.reset( new carto::Volume<T>(
        vol,
        typename carto::Volume<T>::Position4Di( volume->borderWidth,
                                       volume->borderWidth,
                                       volume->borderWidth, 0 ),
        typename carto::Volume<T>::Position4Di( volume->size.x, volume->size.y,
                                       volume->size.z, volume->size.t ) ) );
    }
    vol->header().setProperty( "voxel_size", vs );
    return writeCartoVolume( *vol, fname, format );
  }

}


::Volume* VipVolumeCartoRead( const char* ima_name, int borderWidth, 
                              int dtype, int frame )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoRead " << ima_name << ", border: " << borderWidth 
       << endl;
#endif

  try
    {
      // cout << "VipVolumeCartoRead " << ima_name << endl;
      VipReader	vr( borderWidth, frame );

      if( dtype == U8BIT || dtype == ANY_TYPE || dtype == ANY_INT )
        vr.registerProcessType( "Volume", "U8", &VipReader::doit<uint8_t> );
      if( dtype == S8BIT || dtype == ANY_TYPE || dtype == ANY_INT )
        vr.registerProcessType( "Volume", "S8", &VipReader::doit<int8_t> );
      if( dtype == U16BIT || dtype == ANY_TYPE || dtype == ANY_INT )
        vr.registerProcessType( "Volume", "U16", &VipReader::doit<uint16_t> );
      if( dtype == S16BIT || dtype == ANY_TYPE || dtype == ANY_INT )
        vr.registerProcessType( "Volume", "S16", &VipReader::doit<int16_t> );
      if( dtype == U32BIT || dtype == ANY_TYPE || dtype == ANY_INT )
        vr.registerProcessType( "Volume", "U32", &VipReader::doit<uint32_t> );
      if( dtype == S32BIT || dtype == ANY_TYPE || dtype == ANY_INT )
        vr.registerProcessType( "Volume", "S32", &VipReader::doit<int32_t> );
      if( dtype == VFLOAT || dtype == ANY_TYPE || dtype == ANY_FLOAT )
        vr.registerProcessType( "Volume", "FLOAT", &VipReader::doit<float> );
      if( dtype == VDOUBLE || dtype == ANY_TYPE || dtype == ANY_FLOAT )
        vr.registerProcessType( "Volume", "DOUBLE", &VipReader::doit<double> );

      bool	res = vr.execute( ima_name );
      if( res )
        return vr.volume;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }

  return NULL;
}


int VipVolumeCartoWrite( ::Volume* volume, const char* ima_name, int format )
{
#ifdef VIP_CARTOVOL_DEBUG
  cout << "VipVolumeCartoWrite " << ima_name << ", format: " << format 
       << endl;
#endif

  switch( volume->type )
    {
    case U8BIT:
      return writeCartoVipVolume<uint8_t>( volume, ima_name, format );
      break;
    case S8BIT:
      return writeCartoVipVolume<int8_t>( volume, ima_name, format );
      break;
    case U16BIT:
      return writeCartoVipVolume<uint16_t>( volume, ima_name, format );
      break;
    case S16BIT:
      return writeCartoVipVolume<int16_t>( volume, ima_name, format );
      break;
    case U32BIT:
      return writeCartoVipVolume<uint32_t>( volume, ima_name, format );
      break;
    case S32BIT:
      return writeCartoVipVolume<int32_t>( volume, ima_name, format );
      break;
    case VFLOAT:
      return writeCartoVipVolume<float>( volume, ima_name, format );
      break;
    case VDOUBLE:
      return writeCartoVipVolume<double>( volume, ima_name, format );
      break;
    default:
      cerr << "VipVolumeCartoWrite: Volume with unknown type " 
           << volume->type << endl;
      return PB;
    }

  cerr << "bug in VipVolumeCartoWrite (vol_carto.cc): I shouldn't be here!\n";
  return PB;
}


int VipTestCartoImageFileExist( const char *ima_name )
{
  Finder	f;
  if( !f.check( ima_name ) )
    return 0;
  string	fmt =  f.format();
  if( fmt == "SPM" )
    return SPM;
  if( fmt == "VIDA" )
    return VIDA;
  return TIVOLI;
  // other formats are unknown to VIP...
}


namespace vip
{

  template<typename T>
  rc_ptr<carto::Volume<T> > volumeFromVip( ::Volume* volume )
  {
    if( volume->type != S16BIT )
      {
        cerr << "volumeFromVip: not a volume of " << DataTypeCode<T>::name() 
             << endl;
        return rc_ptr<carto::Volume<T> >();
      }

    if( volume->carto && volume->carto->vol.get() )
      try
        {
          return volume->carto->vol
            ->GenericObject::value<rc_ptr<carto::Volume<T> > >();
        }
      catch( exception & e )
        {
          cerr << "volumeFromVip: " << e.what() << endl;
          return rc_ptr<carto::Volume<T> >();
        }
    else
      {
        rc_ptr<carto::Volume<T> > 
          vol( new carto::Volume<T>( volume->size.x + volume->borderWidth * 2, 
                                     volume->size.y + volume->borderWidth * 2, 
                                     volume->size.z + volume->borderWidth * 2, 
                                     volume->size.t, 
                                     (T *) volume->data ) );
        vector<float>	vs(4);
        vs[0] = volume->voxelSize.x;
        vs[1] = volume->voxelSize.y;
        vs[2] = volume->voxelSize.z;
        vs[3] = volume->voxelSize.t;
        vol->header().setProperty( "voxel_size", vs );
        return vol;
      }
    cerr << "bug in volumeFromVip (vol_carto.cc): I shouldn't be here!\n";
    return rc_ptr<carto::Volume<T> >();
  }


  template rc_ptr<carto::Volume<uint8_t> > volumeFromVip( ::Volume* );
  template rc_ptr<carto::Volume<int8_t> > volumeFromVip( ::Volume* );
  template rc_ptr<carto::Volume<uint16_t> > volumeFromVip( ::Volume* );
  template rc_ptr<carto::Volume<int16_t> > volumeFromVip( ::Volume* );
  template rc_ptr<carto::Volume<uint32_t> > volumeFromVip( ::Volume* );
  template rc_ptr<carto::Volume<int32_t> > volumeFromVip( ::Volume* );
  template rc_ptr<carto::Volume<float> > volumeFromVip( ::Volume* );
  template rc_ptr<carto::Volume<double> > volumeFromVip( ::Volume* );

}

#endif

