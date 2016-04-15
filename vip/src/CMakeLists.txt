
add_subdirectory(preprocessor)
add_subdirectory(libvip)
set(HEADERS ${LIBVIP_HEADERS} PARENT_SCOPE)
#------------------------------------------------------------------------------
# Build targets for commands by looking into "Vip*" directories
#------------------------------------------------------------------------------
file( GLOB _commands "${CMAKE_CURRENT_SOURCE_DIR}/Vip*" )
foreach( _command ${_commands} )
  file( GLOB _pro "${_command}/*.pro" )
  BRAINVISA_GET_FILE_LIST_FROM_PRO( "${_pro}"
                               TARGET _target
                               SOURCES _proSources )
  set( _sources )
  foreach( _f ${_proSources} )
    set( _sources ${_sources} ${_command}/${_f} )
  endforeach( _f ${_proSources} )
  brainvisa_add_executable( ${_target} ${_sources} )
  target_link_libraries( ${_target} vip )
  set_target_properties( ${_target} PROPERTIES LINKER_LANGUAGE CXX )
  BRAINVISA_INSTALL( TARGETS ${_target}
                DESTINATION bin
                COMPONENT ${PROJECT_NAME} )
endforeach( _command )

if(BUILD_TESTING)
  add_subdirectory(tests)
endif(BUILD_TESTING)
