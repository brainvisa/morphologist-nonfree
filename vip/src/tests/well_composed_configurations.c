/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/topology         * TYPE     : Test
 * AUTHOR      : Yann Leprince        * CREATION : 18/06/2015
 ****************************************************************************
 * Copyright Télécom ParisTech (2015).
 ****************************************************************************
 *
 * DESCRIPTION : Well-composed images and topology preservation
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *         18/06/2015 |  Y Leprince  |  introduce well-composed topology
 *           /  /     |              |
 ****************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "vip/volume.h"
#include "vip/skeleton.h"

static void
Generate8NeighbourhoodCriticalConfigurationLookupTable (char table[256])
{
  Vip_U8BIT n8[2][2][2], n4[2][2];
  int x, y, z;
  int num_c1, num_c2, num_in, num_n4;

  int n8lookup;

  /* Enumerate all 256 (2^8) configurations of the binary 8-neighbourhood */
  for (n8lookup = 0; n8lookup < 256; n8lookup++)
    {
      n8[0][0][0] = (n8lookup & 0x01) != 0;
      n8[1][0][0] = (n8lookup & 0x02) != 0;
      n8[0][1][0] = (n8lookup & 0x04) != 0;
      n8[1][1][0] = (n8lookup & 0x08) != 0;
      n8[0][0][1] = (n8lookup & 0x10) != 0;
      n8[1][0][1] = (n8lookup & 0x20) != 0;
      n8[0][1][1] = (n8lookup & 0x40) != 0;
      n8[1][1][1] = (n8lookup & 0x80) != 0;

      /* For each 4-neighbourhood (6 cases), count critical configuration 1 */
      num_c1 = 0;
      /* additionally, count voxels that are in the object */
      num_in = 0;
      /* sanity check */
      num_n4 = 0;

      /* Note: this code could be adapted to generate a classification such as
       * that of M. Siqueira et al, Topological Rpairing of 3D Digital Images,
       * J Math Imaging Vis (2008), 30: 249–274, doi:10.1007/s10851-007-0054-1.
       * In that case, only the 4-neighbourhoods containing the voxel (0, 0, 0)
       * shall be checked.
       */

      for (z = 0; z <= 1; ++z)
        for (y = 0; y <= 1; ++y)
          for (x = 0; x <= 1; ++x)
            {
              if (n8[x][y][z])
                ++num_in;

              if (x == 0 && y == 0)
                {
                  ++num_n4;
                  n4[0][0] = n8[x][y][z];
                  n4[1][0] = n8[x + 1][y][z];
                  n4[0][1] = n8[x][y + 1][z];
                  n4[1][1] = n8[x + 1][y + 1][z];
                  if ((n4[0][0] && n4[1][1] && !n4[0][1] && !n4[1][0]) ||
                      (!n4[0][0] && !n4[1][1] && n4[0][1] && n4[1][0]))
                    ++num_c1;
                }
              if (y == 0 && z == 0)
                {
                  ++num_n4;
                  n4[0][0] = n8[x][y][z];
                  n4[1][0] = n8[x][y + 1][z];
                  n4[0][1] = n8[x][y][z + 1];
                  n4[1][1] = n8[x][y + 1][z + 1];
                  if ((n4[0][0] && n4[1][1] && !n4[0][1] && !n4[1][0]) ||
                      (!n4[0][0] && !n4[1][1] && n4[0][1] && n4[1][0]))
                    ++num_c1;
                }
              if (x == 0 && z == 0)
                {
                  ++num_n4;
                  n4[0][0] = n8[x][y][z];
                  n4[1][0] = n8[x + 1][y][z];
                  n4[0][1] = n8[x][y][z + 1];
                  n4[1][1] = n8[x + 1][y][z + 1];
                  if ((n4[0][0] && n4[1][1] && !n4[0][1] && !n4[1][0]) ||
                      (!n4[0][0] && !n4[1][1] && n4[0][1] && n4[1][0]))
                    ++num_c1;
                }
            }

      assert (num_n4 == 6);
      assert (num_in <= 8);

      if (num_c1)
        {
          table[n8lookup] = WELL_COMPOSED_CRITICAL_C1;
          continue;
        }

      num_c2 = 0;
      /* Check critical configuration 2 */
      if (num_in == 2)
        {
          /* Enumerate all pairs of 26-adjacent (but not 18-adjacent) voxels */
          for (y = 0; y <= 1; ++y)
            for (x = 0; x <= 1; ++x)
              {
                if (n8[x][y][0] && n8[1 - x][1 - y][1])
                  {
                    ++num_c2;
                  }
              }
        }
      else if (num_in == 6)
        {
          /* Enumerate all pairs of 26-adjacent (but not 18-adjacent) voxels */
          for (y = 0; y <= 1; ++y)
            for (x = 0; x <= 1; ++x)
              {
                if (!n8[x][y][0] && !n8[1 - x][1 - y][1])
                  {
                    ++num_c2;
                  }
              }
        }

      if (num_c2)
        {
          assert (num_c1 == 0);
          table[n8lookup] = WELL_COMPOSED_CRITICAL_C2;
        }
      else
        {
          table[n8lookup] = WELL_COMPOSED_CRITICAL_NONE;
        }
    }
}

int main()
{
  int success = VTRUE, res0, res1;
  unsigned int n8lookup;
  char n8lookup_table[256];
  Vip_U8BIT *base_address;
  Volume *test_vol;

  Generate8NeighbourhoodCriticalConfigurationLookupTable(n8lookup_table);

  test_vol = VipCreate3DVolume(2, 2, 2,
                               1.f, 1.f, 1.f,
                               U8BIT, "test_vol", 0);
  if(!test_vol)
    return EXIT_FAILURE;
  if(VipSetVolumeLevel(test_vol, 0) != OK)
    return EXIT_FAILURE;
  base_address = VipGetDataPtr_U8BIT(test_vol) + VipOffsetFirstPoint(test_vol);
  if(!base_address)
    return EXIT_FAILURE;

  for (n8lookup = 0; n8lookup < 256; n8lookup++)
    {
      *(base_address + 0) = (n8lookup & 0x01) != 0;
      *(base_address + 1) = (n8lookup & 0x02) != 0;
      *(base_address + 2) = (n8lookup & 0x04) != 0;
      *(base_address + 3) = (n8lookup & 0x08) != 0;
      *(base_address + 4) = (n8lookup & 0x10) != 0;
      *(base_address + 5) = (n8lookup & 0x20) != 0;
      *(base_address + 6) = (n8lookup & 0x40) != 0;
      *(base_address + 7) = (n8lookup & 0x80) != 0;

      res0 = VipVolumeIsWellComposedForLabel_U8BIT(test_vol, 0);
      res1 = VipVolumeIsWellComposedForLabel_U8BIT(test_vol, 1);

      if(res0 != res1)
        {
          fprintf(stderr, "mismatch between VipVolumeIsWellComposedForLabel "
                  "for a label and its complement (n8lookup = 0x%02x)\n",
                  n8lookup);
          success = VFALSE;
        }
      if(res0 != !n8lookup_table[n8lookup])
        {
          fprintf(stderr, "result of VipVolumeIsWellComposedForLabel "
                  "does not match the computed critical configurations "
                  "(n8lookup = 0x%02x)\n", n8lookup);
          success = VFALSE;
        }
      if(n8lookup_table[n8lookup] !=
         vip_well_composed_critical_configuration_n8lookup[n8lookup])
        {
          fprintf(stderr, "stored critical configuration lookup table "
                  "does not match computed table (n8lookup = 0x%02x)\n",
                  n8lookup);
          success = VFALSE;
        }
    }

  VipFreeVolume(test_vol);

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
