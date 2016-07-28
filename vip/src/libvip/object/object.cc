/**
 * Wrapping to cartobase::Object
 * C++ implementation
 */

#include <vip/object.h>
#include <vip/alloc.h>
#include <soma-io/io/reader.h>
#include <soma-io/io/writer.h>
#include <cartobase/object/object.h>
#include <cartobase/object/property.h>

using namespace std;

//============================================================================
// static declarations
//============================================================================
static ::Object *VipCreateObjectCarto( carto::Object );
static ::Object *VipAllocObjectStructure();
static int VipAllocObjectData( ::Object *obj );
static int VipFreeObjectData( ::Object *obj );

//============================================================================
// private structure
//============================================================================
struct VipCartoObjectPrivate
{
  VipCartoObjectPrivate(): object() {}
  carto::Object object;
};

//============================================================================
// static definitions
//============================================================================
static ::Object *VipAllocObjectStructure()
{
  ::Object *new_object = new ::Object;
  if( !new_object ) return PB;
  new_object->state = STATE_DECLARED;
  new_object->carto = NULL;
  return new_object;
}

static int VipAllocObjectData( ::Object *obj )
{
  if( !obj || obj->state != STATE_DECLARED || obj->carto ) return PB;
  obj->carto = new VipCartoObjectPrivate();
  if( !obj->carto ) return PB;
  obj->state = STATE_ALLOCATED;
  return OK;
}

static int VipFreeObjectData( ::Object *obj )
{
  if( !obj || obj->state != STATE_ALLOCATED || !obj->carto ) return PB;
  delete obj->carto;
  obj->carto = NULL;
  obj->state = STATE_FREED;
  return OK;
}

static ::Object *VipCreateObjectCarto( carto::Object val )
{
  ::Object *new_object = VipAllocObjectStructure();
  if( new_object == PB ) return PB;
  if( VipAllocObjectData( new_object ) == PB ) return PB;
  new_object->carto->object = val;
  new_object->state = STATE_FILLED;
  return new_object;
}

//============================================================================
// extern definitions
//============================================================================
//--- Memory management ------------------------------------------------------
::Object *VipCreateObjectNone()
{
  ::Object *new_object = VipAllocObjectStructure();
  if( new_object == PB ) return PB;
  if( VipAllocObjectData( new_object ) == PB ) return PB;
  new_object->state = STATE_FILLED;
  return new_object;
}

#define VIP_CREATE_OBJECT( METHOD, TYPE )                                    \
::Object *METHOD( TYPE val )                                                   \
{                                                                            \
  ::Object *new_object = VipAllocObjectStructure();                            \
  if( new_object == PB ) return PB;                                          \
  if( VipAllocObjectData( new_object ) == PB ) return PB;                    \
  new_object->carto->object = carto::Object::value( val );                   \
  new_object->state = STATE_FILLED;                                          \
  return new_object;                                                         \
}

VIP_CREATE_OBJECT( VipCreateObjectPtr, void * )
VIP_CREATE_OBJECT( VipCreateObjectU8, unsigned char )
VIP_CREATE_OBJECT( VipCreateObjectS8, signed char )
VIP_CREATE_OBJECT( VipCreateObjectU16, unsigned short )
VIP_CREATE_OBJECT( VipCreateObjectS16, short )
VIP_CREATE_OBJECT( VipCreateObjectU32, unsigned )
VIP_CREATE_OBJECT( VipCreateObjectS32, int )
VIP_CREATE_OBJECT( VipCreateObjectU64, unsigned long )
VIP_CREATE_OBJECT( VipCreateObjectS64, long )
VIP_CREATE_OBJECT( VipCreateObjectFloat, float )
VIP_CREATE_OBJECT( VipCreateObjectDouble, double )

::Object *VipCreateObjectString( const char *val )
{
  ::Object *new_object = VipAllocObjectStructure();
  if( new_object == PB ) return PB;
  if( VipAllocObjectData( new_object ) == PB ) return PB;
  new_object->carto->object = carto::Object::value( string(val) );
  new_object->state = STATE_FILLED;
  return new_object;
}

#define VIP_CREATE_OBJECT_VECTOR( METHOD, TYPE )                             \
::Object *METHOD( TYPE *val, size_t size )                                      \
{                                                                            \
  ::Object *new_object = VipAllocObjectStructure();                            \
  if( new_object == PB ) return PB;                                          \
  if( VipAllocObjectData( new_object ) == PB ) return PB;                    \
  new_object->carto->object = carto::Object::value( vector<TYPE>( val, val+size ) ); \
  new_object->state = STATE_FILLED;                                          \
  return new_object;                                                         \
}

VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorU8, unsigned char )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorS8, signed char )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorU16, unsigned short )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorS16, short )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorU32, unsigned )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorS32, int )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorU64, unsigned long )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorS64, long )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorFloat, float )
VIP_CREATE_OBJECT_VECTOR( VipCreateObjectVectorDouble, double )

::Object *VipCreateObjectVectorString( const char **val, size_t size )
{
  ::Object *new_object = VipAllocObjectStructure();
  if( new_object == PB ) return PB;
  if( VipAllocObjectData( new_object ) == PB ) return PB;
  vector<string> vec( size );
  for( size_t i=0; i<size; ++i )
  {
    vec[i] = string( *(val+i) );
  }
  new_object->carto->object = carto::Object::value( vec );
  new_object->state = STATE_FILLED;
  return new_object;
}

::Object *VipCreateObjectPropertySet()
{
  ::Object *new_object = VipAllocObjectStructure();
  if( new_object == PB ) return PB;
  if( VipAllocObjectData( new_object ) == PB ) return PB;
  new_object->carto->object = carto::Object::value( carto::PropertySet() );
  new_object->state = STATE_FILLED;
  return new_object;
}

int VipFreeObject( ::Object *obj )
{
  if( !obj ) return PB;
  if( VipFreeObjectData( obj ) == PB ) return PB;
  delete obj;
  return OK;
}

int VipObjectIsNone( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return obj->carto->object.isNone();
}

//--- IO ---------------------------------------------------------------------
::Object *VipReadObject( const char *name )
{
  if( !name ) return PB;
  ::Object *new_object = VipCreateObjectNone();
  if( new_object == PB ) return PB;
  soma::Reader<carto::GenericObject> reader;
  reader.attach( string(name) );
  new_object->carto->object.reset( reader.read() );
  new_object->state = STATE_FILLED;
  return new_object;
}

int VipWriteObject( ::Object *obj, const char *name )
{
  if( !obj || !obj->carto || !name ) return PB;
  soma::Writer<carto::GenericObject> writer;
  writer.attach( string(name) );
  writer.write( *(obj->carto->object) );
  return OK;
}

//--- ScalarInterface --------------------------------------------------------
int VipObjectIsScalar( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isScalar());
}

double VipObjectGetScalar( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return obj->carto->object->getScalar();
}

int VipObjectSetScalar( ::Object *obj, double val )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->setScalar( val );
  return OK;
}

//--- StringInterface --------------------------------------------------------
int VipObjectIsString( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isString());
}

const char *VipObjectGetString( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return obj->carto->object->getString().c_str();
}

int VipObjectSetString( ::Object *obj, const char *val )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->setString( string(val) );
  return OK;
}

//--- ArrayInterface ---------------------------------------------------------
int VipObjectIsArray( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isArray());
}

::Object *VipObjectGetArrayItem( ::Object *obj, int item )
{
  if( !obj || !obj->carto ) return PB;
  return VipCreateObjectCarto( obj->carto->object->getArrayItem(item) );
}

int VipObjectSetArrayItem( ::Object *obj, int item, ::Object *val )
{
  if( !obj || !obj->carto || !val || !val->carto ) return PB;
  obj->carto->object->setArrayItem( item, val->carto->object );
  return OK;
}

//--- DynArrayInterface ------------------------------------------------------
int VipObjectIsDynArray( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isDynArray());
}

int VipObjectReserveArray( ::Object *obj, size_t size )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->reserveArray( size );
  return OK;
}

int VipObjectResizeArray( ::Object *obj, size_t size )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->resizeArray( size );
  return OK;
}

int VipObjectRemoveArrayItem( ::Object *obj, int item )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->removeArrayItem( item );
  return OK;
}

int VipObjectInsertArrayItem( ::Object *obj, int item, ::Object *val )
{
  if( !obj || !obj->carto || !val || !val->carto ) return PB;
  obj->carto->object->insertArrayItem( item, val->carto->object );
  return OK;
}

//--- DictionaryInterface ----------------------------------------------------
int VipObjectIsDictionary( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isDictionary());
}

::Object *VipObjectGetProperty( ::Object *obj, const char *key )
{
  if( !obj || !obj->carto ) return PB;
  return VipCreateObjectCarto( obj->carto->object->getProperty( string(key) ) );
}

int VipObjectSetProperty( ::Object *obj, const char *key, ::Object *val )
{
  if( !obj || !obj->carto || !val || !val->carto ) return PB;
  obj->carto->object->setProperty( string(key), val->carto->object );
  return OK;
}

int VipObjectRemoveProperty( ::Object *obj, const char *key )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->removeProperty( string(key) );
  return OK;
}

int VipObjectHasProperty( ::Object *obj, const char *key )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->hasProperty( string(key) ));
}

int VipObjectClearProperties( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->clearProperties();
  return OK;
}

int VipObjectCopyProperties( ::Object *obj, ::Object *source )
{
  if( !obj || !obj->carto || !source || !source->carto ) return PB;
  obj->carto->object->copyProperties( source->carto->object );
  return OK;
}

// Specializations
int VipObjectSetPropertyDouble( ::Object *obj, const char *key, double val )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->setProperty( string(key), val );
  return OK;
}

int VipObjectSetPropertyString( ::Object *obj, const char *key, const char *val )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->setProperty( string(key), string(val) );
  return OK;
}

//--- SizeInterface ----------------------------------------------------------
size_t VipObjectSize( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return obj->carto->object->size();
}

//--- IterableInterface ------------------------------------------------------
int VipObjectIsIterable( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isIterable());
}

::Object *VipObjectGetIterator( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return VipCreateObjectCarto( obj->carto->object->objectIterator() );
}

//--- IteratorInterface ------------------------------------------------------
int VipObjectIsIterator( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isIterator());
}

int VipObjectIsValid( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isValid());
}

::Object *VipObjectCurrentValue( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return VipCreateObjectCarto( obj->carto->object->currentValue() );
}

int VipObjectNext( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  obj->carto->object->next();
  return OK;
}

//--- DictionaryIteratorInterface --------------------------------------------
int VipObjectisDictionaryIterator( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return (int)(obj->carto->object->isDictionaryIterator());
}

const char *VipObjectKey( ::Object *obj )
{
  if( !obj || !obj->carto ) return PB;
  return obj->carto->object->key().c_str();
}
