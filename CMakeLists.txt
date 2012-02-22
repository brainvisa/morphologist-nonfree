cmake_minimum_required( VERSION 2.6 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

BRAINVISA_FIND_PACKAGE( aims-free REQUIRED )

include( "${AIMS-FREE_USE_FILE}" )

set( MORPHOLOGIST-PRIVATE_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include" ${AIMS-FREE_INCLUDE_DIRS} )
include_directories( ${MORPHOLOGIST-PRIVATE_INCLUDE_DIRS} )

add_subdirectory( vip )

BRAINVISA_CREATE_CMAKE_CONFIG_FILES()

