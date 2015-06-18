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
#include <stdio.h>

#include "vip/volume.h"
#include "vip/skeleton.h"

static int
IsCriticalC3Configuration_ITK (Vip_U8BIT neighbourhood[3][3][3])
{
  unsigned int num_faces = 0, num_critical = 0;

  /*
   * This implementation is copied from ITK v4.7.2 (http://itk.org/)
   * FastMarchingImageFilterBase::DoesVoxelChangeViolateStrictTopology. This
   * library calls "critical C3 configuration" a configuration where changing
   * the label of the central voxel does not preserve the topology of the
   * object.
   *
   * KNOWN BUG: the critical configuration with all voxels set to zero is not
   * recognized by this algorithm (see comment in
   * TestWellComposedTopologyPreservationCriterion).
   */

  if (neighbourhood[0][1][1])
    ++num_faces;
  if (neighbourhood[2][1][1])
    ++num_faces;
  if (neighbourhood[0][1][1] && neighbourhood[2][1][1])
    ++num_critical;

  if (neighbourhood[1][0][1])
    ++num_faces;
  if (neighbourhood[1][2][1])
    ++num_faces;
  if (neighbourhood[1][0][1] && neighbourhood[1][2][1])
    ++num_critical;

  if (neighbourhood[1][1][0])
    ++num_faces;
  if (neighbourhood[1][1][2])
    ++num_faces;
  if (neighbourhood[1][1][0] && neighbourhood[1][1][2])
    ++num_critical;

  return (num_critical > 0 &&
          num_faces % 2 == 0 && num_critical * 2 == num_faces);
}

int main ()
{
  Volume *test_vol3;
  Vip_U8BIT *base_address;
  VipOffsetStruct *vos;
  Vip_U8BIT binary_enumerator;
  int reference_result, vip_result;
  int success = VTRUE;
  Vip_U8BIT neighbourhood[3][3][3];

  test_vol3 = VipCreate3DVolume(3, 3, 3,
                               1.f, 1.f, 1.f,
                               U8BIT, "test_vol3", 0);
  if(!test_vol3)
    return EXIT_FAILURE;
  if(VipSetVolumeLevel(test_vol3, 0) != OK)
    return EXIT_FAILURE;
  vos = VipGetOffsetStructure(test_vol3);
  if(!vos)
    return EXIT_FAILURE;
  base_address = VipGetDataPtr_U8BIT(test_vol3) + vos->oFirstPoint;
  if(!base_address)
    return EXIT_FAILURE;

  /* enumerate all configurations of the 6-neighbours */
  for (binary_enumerator = 0; binary_enumerator < 64; ++binary_enumerator)
    {
      *(base_address + 4) = neighbourhood[0][1][1] =
        (binary_enumerator & 0x01) != 0;
      *(base_address + 22) = neighbourhood[2][1][1] =
        (binary_enumerator & 0x02) != 0;
      *(base_address + 10) = neighbourhood[1][0][1] =
        (binary_enumerator & 0x04) != 0;
      *(base_address + 16) = neighbourhood[1][2][1] =
        (binary_enumerator & 0x08) != 0;
      *(base_address + 12) = neighbourhood[1][1][0] =
        (binary_enumerator & 0x10) != 0;
      *(base_address + 14) = neighbourhood[1][1][2] =
        (binary_enumerator & 0x20) != 0;

      reference_result = !IsCriticalC3Configuration_ITK (neighbourhood);
      if (binary_enumerator == 0)
        {
          /* The case where no 6-neighbour is in the object is buggy in the ITK
             implementation (obviously adding an isolated point to the object
             changes its topology!) */
          reference_result = 0;
        }

      /* The central voxel is base_address + 13 ([1][1][1]). We need to pass
         candidate_label=1 in order to ensure the well-composedness. */
      vip_result =
        VipIsTopologicallyWellComposedForLabel_U8BIT(vos, base_address + 13,
                                                     1, 0);

      if (vip_result != reference_result)
        {
          fprintf (stderr,
                   "Mismatch of the well-composed topology preservation"
                   " (reference_result = %d, vip_result = %d)\n" "  %d%d\n"
                   "  %dx%d\n" "   %d%d\n", reference_result, vip_result,
                   neighbourhood[1][1][0],
                   neighbourhood[1][0][1],
                   neighbourhood[0][1][1],
                   neighbourhood[2][1][1],
                   neighbourhood[1][2][1],
                   neighbourhood[1][1][2]);
          success = 0;
        }
    }

  VipFree(vos);
  VipFreeVolume(test_vol3);
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
