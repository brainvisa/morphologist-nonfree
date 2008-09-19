
ADD_SUBDIRECTORY(preprocessor)
ADD_SUBDIRECTORY(libvip)

#------------------------------------------------------------------------------
# Build targets for commands by looking into "Vip*" directories
#------------------------------------------------------------------------------
INCLUDE( SomaBuildConfig )
FILE( GLOB _commands "${CMAKE_CURRENT_SOURCE_DIR}/Vip*" )
FOREACH( _command ${_commands} )
  FILE( GLOB _pro "${_command}/*.pro" )
  SOMA_GET_FILE_LIST_FROM_PRO( "${_pro}"
                               TARGET _target
                               SOURCES _proSources )
  SET( _sources )
  FOREACH( _f ${_proSources} )
    SET( _sources ${_sources} ${_command}/${_f} )
  ENDFOREACH( _f ${_proSources} )
#   MESSAGE( STATUS "Processing ${_command}" )
#   MESSAGE( "target = \"${_target}\"" )
#   MESSAGE( "sources = \"${_sources}\"" )
  ADD_EXECUTABLE( ${_target} ${_sources} )
  TARGET_LINK_LIBRARIES( ${_target} vip )
  INSTALL(TARGETS ${_target} DESTINATION bin )
ENDFOREACH( _command ${commands} )
