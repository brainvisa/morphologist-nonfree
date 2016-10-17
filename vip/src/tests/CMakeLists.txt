set(_tests
  well_composed_configurations
  well_composed_topology
)

foreach(_test ${_tests})
  BRAINVISA_ADD_EXECUTABLE(vip_test_${_test} ${_test}.c)
  target_link_libraries(vip_test_${_test} vip)
  set_target_properties( vip_test_${_test} PROPERTIES LINKER_LANGUAGE CXX )
  add_test(vip_test_${_test} "${CMAKE_BINARY_DIR}/bin/bv_env${CMAKE_EXECUTABLE_SUFFIX}" vip_test_${_test})
endforeach()
