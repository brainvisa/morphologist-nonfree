add_subdirectory( src )
BRAINVISA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/shared"
  "share/vip-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}"
  ${PROJECT_NAME} )
  
add_subdirectory(doxygen)


