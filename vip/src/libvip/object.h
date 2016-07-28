/**
 * Wrapping to cartobase::Object
 */

#ifndef VIP_OBJECT_H
#define VIP_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/object/structure.h>
#include <sys/types.h>

// Structure state
// Copied from Volume (same values)
#define STATE_DECLARED   1
#define STATE_ALLOCATED  2
#define STATE_FILLED     3
#define STATE_FREED      4

#define VIP_DEC_OBJECT(aObjectPtr) Object *aObjectPtr = NULL

#define mVipObjState(obj) (obj)->state

// Memory management
// All VipCreateObject* methods copy the value into the Object
// (in the vector case, all the members are copied into an std::vector)
// It is then possible to free the pointed array without risk.
// Only exception: VipCreateObjectPtr only copies the pointer, and not the
// pointed variable.
extern Object *VipCreateObjectNone();
extern Object *VipCreateObjectPtr( void *val );
extern Object *VipCreateObjectU8( unsigned char val );
extern Object *VipCreateObjectS8( signed char val );
extern Object *VipCreateObjectU16( unsigned short val );
extern Object *VipCreateObjectS16( short val );
extern Object *VipCreateObjectU32( unsigned val );
extern Object *VipCreateObjectS32( int val );
extern Object *VipCreateObjectU64( unsigned long val );
extern Object *VipCreateObjectS64( long val );
extern Object *VipCreateObjectFloat( float val );
extern Object *VipCreateObjectDouble( double val );
extern Object *VipCreateObjectString( const char *val );
extern Object *VipCreateObjectVectorU8( unsigned char *val, size_t size );
extern Object *VipCreateObjectVectorS8( signed char *val, size_t size );
extern Object *VipCreateObjectVectorU16( unsigned short *val, size_t size );
extern Object *VipCreateObjectVectorS16( short *val, size_t size );
extern Object *VipCreateObjectVectorU32( unsigned *val, size_t size );
extern Object *VipCreateObjectVectorS32( int *val, size_t size );
extern Object *VipCreateObjectVectorU64( unsigned long *val, size_t size );
extern Object *VipCreateObjectVectorS64( long *val, size_t size );
extern Object *VipCreateObjectVectorFloat( float *val, size_t size );
extern Object *VipCreateObjectVectorDouble( double *val, size_t size );
extern Object *VipCreateObjectVectorString( const char **val, size_t size );
extern Object *VipCreateObjectPropertySet();
extern int VipFreeObject( Object *obj );
extern int VipObjectIsNone( Object *obj );

// IO
extern Object *VipReadObject( const char *name );
extern int VipWriteObject( Object *obj, const char *name );

// ScalarInterface
extern int VipObjectIsScalar( Object *obj );
extern double VipObjectGetScalar( Object *obj );
extern int VipObjectSetScalar( Object *obj, double val );

// StringInterface
extern int VipObjectIsString( Object *obj );
extern const char *VipObjectGetString( Object *obj );   // no need to free char* (?)
extern int VipObjectSetString( Object *obj, const char *val );

// ArrayInterface
extern int VipObjectIsArray( Object *obj );
extern Object *VipObjectGetArrayItem( Object *obj, int item );
extern int VipObjectSetArrayItem( Object *obj, int item, Object *val );

// DynArrayInterface
extern int VipObjectIsDynArray( Object *obj );
extern int VipObjectReserveArray( Object *obj, size_t size );
extern int VipObjectResizeArray( Object *obj, size_t size);
extern int VipObjectRemoveArrayItem( Object *obj, int item );
extern int VipObjectInsertArrayItem( Object *obj, int item, Object *val );

// DictionaryInterface
extern int VipObjectIsDictionary( Object *obj );
extern Object *VipObjectGetProperty( Object *obj, const char *key );
extern int VipObjectSetProperty( Object *obj, const char *key, Object *val );
extern int VipObjectRemoveProperty( Object *obj, const char *key );
extern int VipObjectHasProperty( Object *obj, const char *key );
extern int VipObjectClearProperties( Object *obj );
extern int VipObjectCopyProperties( Object *obj, Object *source );
// Specializations
extern int VipObjectSetPropertyDouble( Object *obj, const char *key, double val );
extern int VipObjectSetPropertyString( Object *obj, const char *key, const char *val );

// SizeInterface
extern size_t VipObjectSize( Object *obj );

// IterableInterface
extern int VipObjectIsIterable( Object *obj );
extern Object *VipObjectGetIterator( Object *obj );

// IteratorInterface
extern int VipObjectIsIterator( Object *obj );
extern int VipObjectIsValid( Object *obj );
extern Object *VipObjectCurrentValue( Object *obj );
extern int VipObjectNext( Object *obj );

// DictionaryIteratorInterface
extern int VipObjectisDictionaryIterator( Object *obj );
extern const char *VipObjectKey( Object *obj );


#ifdef __cplusplus
}
#endif

#endif // VIP_OBJECT_H
