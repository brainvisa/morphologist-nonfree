TARGET		= VipEpiPhaseMap

#!include ../../config-app

HEADERS		=	\
  error.h\
  extrapolation.h \
  hgaussj.h \
  lfit.h \
  models.h \
  reconstruct.h \
  segmentation.h \
  unwrap_cst.h \
  unwrap.h

SOURCES		=	\
  phasemap.c  \
  reconstruct.c \
  hgaussj.c \
  lfit.c  \
  models.c  \
  error.c \
  segmentation.c  \
  unwrap.c \
  unwrap_cst.c  \
  extrapolation.c
