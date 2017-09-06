/**
 * Wrapping to cartobase::Object
 */


#ifndef VIP_OBJECT_STRUCTURE_H
#define VIP_OBJECT_STRUCTURE_H

#ifndef VTRUE
#define VTRUE   1
#endif
#ifndef VFALSE
#define VFALSE  0
#endif

#ifndef YES
#define YES     1
#endif
#ifndef NO
#define NO      0
#endif

#ifndef OK
#define OK      1
#endif
#ifndef PB
#define PB      0
#endif

#ifndef ON
#define ON      1
#endif
#ifndef OFF
#define OFF     0
#endif

struct VipCartoObjectPrivate;

typedef struct {
  int state;
  struct VipCartoObjectPrivate *carto;
} Object;

#endif // VIP_OBJECT_STRUCTURE_H

