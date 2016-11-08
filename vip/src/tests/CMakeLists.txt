set(_tests
  well_composed_configurations
  well_composed_topology
)

set( _targets )

foreach(_test ${_tests})
  BRAINVISA_ADD_EXECUTABLE(vip_test_${_test} ${_test}.c)
  target_link_libraries(vip_test_${_test} vip)
  set_target_properties( vip_test_${_test} PROPERTIES LINKER_LANGUAGE CXX )
  add_test(vip_test_${_test} "${CMAKE_BINARY_DIR}/bin/bv_env_test" vip_test_${_test})
  list( APPEND _targets vip_test_${_test} )
endforeach()

BRAINVISA_INSTALL( TARGETS ${_targets}
              DESTINATION bin
              COMPONENT ${PROJECT_NAME}-test )
