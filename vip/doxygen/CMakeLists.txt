FIND_PACKAGE( Doxygen )
IF ( DOXYGEN_FOUND )
  set(component_name "vip")
  SET( DOXYFILE_HTML_HEADER "\"${CMAKE_CURRENT_SOURCE_DIR}/head_${component_name}.html\"" )
  set( _doxygenInput ${HEADERS} )
  SET( DOXYFILE_OPTIMIZE_OUTPUT_FOR_C YES )
  SET( DOXYFILE_BUILTIN_STL_SUPPORT NO )
  BRAINVISA_GENERATE_DOXYGEN_DOC( _doxygenInput  head_${component_name}.html ${component_name}.gif
                                  INPUT_PREFIX "${CMAKE_BINARY_DIR}/include/${component_name}"
                                  COMPONENT "${component_name}" )
ENDIF ( DOXYGEN_FOUND )
