TEMPLATE	= lib
TARGET		= vip${BUILDMODEEXT}

#!include ../../config

#PREPROCESSOR	= VipPreprocessing

INCLUDEPATH	+= ../../include_static

win32:LIBS += -lregex

HEADERS	=			\
  activations.h			\
  alloc.h			\
  brain.h			\
  bucket.h			\
  connex.h			\
  cylinder.h			\
  cylinder/cylinder.h		\
  depla.h			\
  deriche.h			\
  distmap.h			\
  eigen.h			\
  epidistorsion/convolution.h	\
  epidistorsion/cpp_defs.h	\
  epidistorsion/debug.h		\
  epidistorsion/dyn_alloc.h	\
  epidistorsion/fourier.h	\
  epidistorsion/gravity.h	\
  epidistorsion/morpho.h	\
  epidistorsion/param.h		\
  epidistorsion/param_io.h	\
  epidistorsion/readwrite.h	\
  epidistorsion/sort.h		\
  epidistorsion/utils.h		\
  epidistorsion/volume_io.h	\
  epidistorsion/volume_utils.h	\
  fold_arg.h			\
  fold_arg/attribut.h		\
  fold_arg/cortex_rel.h		\
  fold_arg/fold_arg_io.h	\
  fold_arg/junction.h		\
  fold_arg/mesh_aims.h          \
  fold_arg/plidepassage.h	\
  fold_arg/simplesurface.h	\
  gaussian.h			\
  geometry.h			\
  histo.h			\
  histo/histo_ss.h		\
  label.h			\
  linresamp.h			\
  matching.h			\
  matching/dilatation.h		\
  matching/gravite.h		\
  matching/list.h		\
  matching/matching.h		\
  matching/rotation.h		\
  matching/translation.h	\
  matching/distance.h		\
  matching/interpol.h		\
  matching/mask.h		\
  matching/recalage.h		\
  matching/surface.h		\
  math.h			\
  math/matrix.h			\
  math/random.h			\
  math/trigo.h			\
  minc.h			\
  moment.h			\
  morpho.h			\
  nomen.h			\
  nomen/nomen_def.h		\
  pca.h				\
  pca/pca.h			\
  pvec.h			\
  pvec/gtm_method.h		\
  pyramid.h			\
  skeleton.h			\
  splineresamp.h		\
  spm.h				\
  talairach.h			\
  TivoliIO.h			\
  TivoliIO/TivoliIO_volume.h	\
  TivoliIO/TivoliIO_util.h	\
  topology.h			\
  transform.h			\
  util.h			\
  util/file.h			\
  util/shelltools.h		\
  util/slice.h			\
  util/strlist.h		\
  vcomplex.h			\
  vfilter.h			\
  vida.h			\
  voi.h				\
  volume.h			\
  volume/structure.h		\
  volume/struct_volume.h        \
  volume/vol_carto_cpp.h

HEADERS_STATIC	=		\
  brain_static.h		\
  bucket_static.h		\
  connex_static.h		\
  cylinder_static.h		\
  depla_static.h		\
  deriche_static.h		\
  distmap_static.h		\
  fold_arg_static.h		\
  gaussian_static.h		\
  histo_static.h		\
  histo/histo_ss_static.h	\
  linresamp_static.h		\
  nomen_static.h		\
  pca_static.h			\
  pvec_static.h			\
  skeleton_static.h		\
  splineresamp_static.h		\
  splineresamp/affine.h		\
  spm_static.h			\
  spm/analyze_db.h		\
  topology_static.h		\
  util_static.h			\
  voi_static.h			\
  volume_static.h		\
  volume/vol_carto.h

GENERICS	=		\
  alloc/alloc.gen		\
  bucket/bucket.gen		\
  bucket/bucket_struct.gen	\
  bucket/bucket_write.gen	\
  connex/3Dconnex.gen		\
  deriche/deriche.gen		\
  deriche/extedge.gen		\
  eigen/eigen.gen		\
  geometry/geometry.gen		\
  linresamp/linresamp.gen	\
  math/matrix.gen		\
  math/vector.gen		\
  pca/pca.gen			\
  skeleton/skeleton.gen		\
  TivoliIO/byteswapping.gen	\
  topology/topology.gen		\
  topology/wellcomposed.gen		\
  transform/transform.gen	\
  util/util.gen			\
  util/util2.gen		\
  vcomplex/vcomplex.gen		\
  volume/subvolume.gen		\
  volume/volume.gen

SOURCES		=		\
  activations/cluster.c		\
  alloc/alloc.c			\
  brain/brain.c			\
  brain/potts.c			\
  bucket/bucket.c		\
  connex/3Dconnex.c		\
  connex/connex.c		\
  cylinder/cylinder.c		\
  depla/depla.c			\
  depla/mat.c			\
  deriche/deriche3D.c		\
  deriche/deriche.c		\
  distmap/distmap.c		\
  distmap/distmap_mask.c	\
  distmap/distmap_sweeping.c	\
  distmap/front.c		\
  distmap/voronoi.c		\
  epidistorsion/convolution.c	\
  epidistorsion/debug.c		\
  epidistorsion/dyn_alloc.c	\
  epidistorsion/fourier.c	\
  epidistorsion/gravity.c	\
  epidistorsion/morpho.c	\
  epidistorsion/param_io.c	\
  epidistorsion/readwrite.c	\
  epidistorsion/sort.c		\
  epidistorsion/utils.c		\
  epidistorsion/volume_io.c	\
  epidistorsion/volume_utils.c	\
  fold_arg/arg.c		\
  fold_arg/arg_io.c		\
  fold_arg/cortex_rel.c		\
  fold_arg/junction.c		\
  fold_arg/plidepassage.c	\
  fold_arg/simplesurface.c	\
  gaussian/gaussian.c		\
  gaussian/recursif.c		\
  histo/histo_analysis.c	\
  histo/histo.c			\
  histo/histo_extrema.c		\
  histo/histo_ss.c		\
  label/label_contour.c		\
  linresamp/util.c		\
  matching/dilatation.c		\
  matching/distance.c		\
  matching/gravite.c		\
  matching/interpol.c		\
  matching/list.c		\
  matching/mask.c		\
  matching/matching.c		\
  matching/recalage.c		\
  matching/rotation.c		\
  matching/surface.c		\
  matching/translation.c	\
  math/random.c			\
  math/trigo.c			\
  minc/minc.c			\
  minc/mincread.c		\
  moment/moment.c		\
  morpho/gmorpho.c		\
  morpho/morpho.c		\
  morpho/watershed.c		\
  nomen/nomen.c			\
  pvec/striata_gtm_method.c	\
  pyramid/pyramid.c		\
  skeleton/dilation.c		\
  skeleton/erosion.c		\
  skeleton/prune.c		\
  skeleton/pyramid.c		\
  skeleton/skeleton.c		\
  splineresamp/affine.c		\
  splineresamp/spline.c		\
  spm/spm.c			\
  talairach/talairach.c		\
  TivoliIO/tivoli_util.c	\
  TivoliIO/tivoli_volume.c	\
  topology/topology.c		\
  topology/wellcomposed.c	\
  topology/topology_clas.c	\
  topology/topology_test.c	\
  util/file.c			\
  util/slice.c			\
  util/shelltools.c		\
  util/strlist.c		\
  util/util.c			\
  vfilter/vfilter.c		\
  vida/vida_volume.c		\
  voi/voi_protected.c		\
  voi/voi_resamp.c		\
  voi/voi_utils.c		\
  volume/volume.c               \
  fold_arg/mesh_aims.cc         \
  volume/vol_carto.cc
