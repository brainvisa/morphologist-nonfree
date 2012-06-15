BRAINVISA_ADD_EXECUTABLE(VipPreprocessing preprocessing.c)
BRAINVISA_INSTALL( TARGETS VipPreprocessing
                   DESTINATION bin
                   COMPONENT ${PROJECT_NAME} )