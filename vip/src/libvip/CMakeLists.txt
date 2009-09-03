#------------------------------------------------------------------------------
# Extract source file lists from pro file
#------------------------------------------------------------------------------
BRAINVISA_GET_FILE_LIST_FROM_PRO( ${CMAKE_CURRENT_SOURCE_DIR}/libvip.pro
                             "HEADERS" _headers
                             "HEADERS_STATIC" _headersStatic 
                             "SOURCES" _sources
                             "GENERICS" _generics )

#------------------------------------------------------------------------------
# Manage headers existing in source tree
#------------------------------------------------------------------------------
#SET( _doxygenInput )
SET( _headersDir vip )
# Copy static headers in build directory but do not install them
FOREACH( _currentHeader ${_headersStatic} )
  CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/${_currentHeader}
                  ${CMAKE_BINARY_DIR}/include/${_headersDir}/${_currentHeader}
                  COPYONLY )
ENDFOREACH( _currentHeader ${_proHeaders} )
# Copy headers in build directory and install them
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers vip )


#------------------------------------------------------------------------------
# Generate code with VipPreprocessing and manage generated code
#------------------------------------------------------------------------------
FOREACH( _currentGeneric ${_generics} )
  # Copy *.gen file in build directory because VipPreprocessing generate
  # files in the same directory as the gen file.
  CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/${_currentGeneric}
                  ${CMAKE_CURRENT_BINARY_DIR}/${_currentGeneric}
                  COPYONLY )
  GET_FILENAME_COMPONENT(_p "${CMAKE_CURRENT_BINARY_DIR}/${_currentGeneric}" PATH )
#   FILE( MAKE_DIRECTORY "${_p}" )
  GET_FILENAME_COMPONENT(_f "${CMAKE_CURRENT_BINARY_DIR}/${_currentGeneric}" NAME_WE )
  SET( _g "${_p}/${_f}" )
  # Call VipPreprocessing. Warning: it only works with a relative path to the *.gen file.
  ADD_CUSTOM_COMMAND( OUTPUT "${_g}_gen.c" "${_g}_gen.h" "${_g}_gen_static.h"
                      COMMAND "${CMAKE_BINARY_DIR}/bin/VipPreprocessing" ARGS "${_f}.gen"
                      DEPENDS "${_g}.gen" VipPreprocessing
                      WORKING_DIRECTORY "${_p}" )
  # Add the generated *.c file to the list of sources for the library
  SET( _sources ${_sources} "${_g}_gen.c" )
  # Install non static generated headers
  GET_FILENAME_COMPONENT( _path "_currentGeneric" PATH )
  INSTALL( FILES "${_g}_gen.h" DESTINATION include/${_headersDir}/${_path} COMPONENT vip-devel )
#  SET( _doxygenInput ${_doxygenInput} "${_g}_gen.h" )
ENDFOREACH( _currentGeneric ${_generics} )


#------------------------------------------------------------------------------
# Define vip library
#------------------------------------------------------------------------------
ADD_LIBRARY( vip SHARED ${_sources} )
set_property( TARGET vip PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
TARGET_LINK_LIBRARIES( vip ${AIMS-FREE_LIBRARIES} )
BRAINVISA_INSTALL( TARGETS vip DESTINATION lib COMPONENT vip )

#FILE( WRITE "${DOXYGEN_BINARY_DIR}/doxygenInput" " ${_doxygenInput}" )

