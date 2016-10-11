#------------------------------------------------------------------------------
# Extract source file lists from pro file
#------------------------------------------------------------------------------
BRAINVISA_GET_FILE_LIST_FROM_PRO( ${CMAKE_CURRENT_SOURCE_DIR}/libvip.pro
                             "HEADERS" _headers
                             "HEADERS_STATIC" _headersStatic 
                             "SOURCES" _sources
                             "GENERICS" _generics )

set(LIBVIP_HEADERS ${_headers} ${_headersStatic} PARENT_SCOPE)
#------------------------------------------------------------------------------
# Manage headers existing in source tree
#------------------------------------------------------------------------------
#SET( _doxygenInput )
SET( _headersDir vip )
# Copy headers in build directory and install them
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers vip headersTarget )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headersStatic vip headersTarget2 )


#------------------------------------------------------------------------------
# Generate code with VipPreprocessing and manage generated code
#------------------------------------------------------------------------------
set( _genericsGenerated )
FOREACH( _currentGeneric ${_generics} )
  # Copy *.gen file in build directory because VipPreprocessing generate
  # files in the same directory as the gen file.
  GET_FILENAME_COMPONENT(_p "${CMAKE_CURRENT_BINARY_DIR}/${_currentGeneric}" PATH )
#   FILE( MAKE_DIRECTORY "${_p}" )
  GET_FILENAME_COMPONENT(_f "${CMAKE_CURRENT_BINARY_DIR}/${_currentGeneric}" NAME_WE )
  SET( _g "${_p}/${_f}" )
  get_filename_component( _d "${_currentGeneric}" PATH )
  set( _tmpincdir "${CMAKE_BINARY_DIR}/include/${_headersDir}/${_d}" )
  # Call VipPreprocessing. Warning: it only works with a relative path to the *.gen file.
  ADD_CUSTOM_COMMAND( OUTPUT "${_g}_gen.c" "${_tmpincdir}/${_f}_gen.h" "${_tmpincdir}/${_f}_gen_static.h"
                      COMMAND "${CMAKE_COMMAND}" -E make_directory "${_p}"
                      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${_currentGeneric}" "${_g}.gen"
                      COMMAND "${CMAKE_COMMAND}" -E chdir "${_p}" "${CMAKE_BINARY_DIR}/bin/VipPreprocessing${CMAKE_EXECUTABLE_SUFFIX}" "${_f}.gen"
                      COMMAND "${CMAKE_COMMAND}" -E copy "${_g}_gen.h" "${_tmpincdir}/${_f}_gen.h"
                      COMMAND "${CMAKE_COMMAND}" -E copy "${_g}_gen_static.h" "${_tmpincdir}/${_f}_gen_static.h"
                      COMMAND "${CMAKE_COMMAND}" -E remove -f "${_g}.gen" "${_g}_gen.h" "${_g}_gen_static.h"
                      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_currentGeneric}" VipPreprocessing
 )
  set_source_files_properties( "${_g}_gen.c" "${_tmpincdir}/${_f}_gen.h" "${_tmpincdir}/${_f}_gen_static.h" PROPERTIES GENERATED true )
  set( _genericsGenerated ${_genericsGenerated} "${_g}_gen.c" "${_tmpincdir}/${_f}_gen.h" "${_tmpincdir}/${_f}_gen_static.h" )
  # Add the generated *.c file to the list of sources for the library
  SET( _sources ${_sources} "${_g}_gen.c" )
  # Install non static generated headers
  INSTALL( FILES "${_tmpincdir}/${_f}_gen.h" DESTINATION include/${_headersDir}/${_d} COMPONENT vip-dev )
#  SET( _doxygenInput ${_doxygenInput} "${_g}_gen.h" )
ENDFOREACH( _currentGeneric ${_generics} )


#------------------------------------------------------------------------------
# Define vip library
#------------------------------------------------------------------------------
ADD_LIBRARY( vip SHARED ${_sources} )
# This custom target is needed because add_dependencies cannot be used to add
# dependencies on files.
add_custom_target( vip-generated DEPENDS ${_genericsGenerated} )
add_dependencies( vip ${headersTarget} ${headersTarget2} vip-generated )
set_property( TARGET vip PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
set_target_properties( vip PROPERTIES LINKER_LANGUAGE CXX )
TARGET_LINK_LIBRARIES( vip ${AIMS-FREE_LIBRARIES} )
BRAINVISA_INSTALL( TARGETS vip DESTINATION lib COMPONENT ${PROJECT_NAME} )

#FILE( WRITE "${DOXYGEN_BINARY_DIR}/doxygenInput" " ${_doxygenInput}" )

