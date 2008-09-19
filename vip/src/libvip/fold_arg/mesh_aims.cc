/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipClusterArg        * TYPE     : Command line
 * AUTHOR      : RIVIERE D.           * CREATION : 12/12/2004
 * VERSION     : 2.6                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : 
 ****************************************************************************
 *
 * DESCRIPTION :  
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/fold_arg/mesh_aims.h>

#ifdef VIP_CARTO_VOLUME_WRAPPING

#include <vip/volume/vol_carto_cpp.h>
#include <aims/mesh/mesher.h>
#include <aims/io/writer.h>

using namespace aims;
using namespace std;

void meshAims( ::Volume* vol, const char* outfname )
{
  cout << "meshAims: " << outfname << endl;
  Mesher	mesher;
  mesher.setDecimation( 100.0, 5, 3, 180.0 );
  mesher.setMinFacetNumber( 50 );
  AimsSurfaceTriangle surface;
  AimsData<int16_t>	data( vip::volumeFromVip<int16_t>( vol ) );
  /*
  cout << "data dim: " << data.dimX() << ", " << data.dimY() << "," 
       << data.dimZ() << ", "  << data.dimT() << ", border: " 
       << data.borderWidth() << endl;
  */
  data.fillBorder( -1 );
  mesher.getBrain( data, surface );
  Writer<AimsSurfaceTriangle>	w( outfname );
  w.write( surface );
}

#endif

