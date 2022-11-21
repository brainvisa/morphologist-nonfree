cmake_minimum_required( VERSION 3.10 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

# This defines BUILD_TESTING and is thus preferred to plain enable_testing()
include(CTest)

BRAINVISA_FIND_PACKAGE( aims-free REQUIRED )
include( "${AIMS-FREE_USE_FILE}" )

BRAINVISA_FIND_PACKAGE( soma-io REQUIRED )
include( "${SOMA-IO_USE_FILE}" )

math( EXPR result "${aims-free_VERSION_MINOR}+1" )
BRAINVISA_DEPENDENCY( RUN DEPENDS "aims-free" RUN ">= ${aims-free_VERSION};<< ${aims-free_VERSION_MAJOR}.${result}" )

math( EXPR result "${soma-io_VERSION_MINOR}+1" )
BRAINVISA_DEPENDENCY( RUN DEPENDS "soma-io" RUN ">= ${soma-io_VERSION};<< ${soma-io_VERSION_MAJOR}.${result}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS "soma-io" DEV ">= ${soma-io_VERSION};<< ${soma-io_VERSION_MAJOR}.${result}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS "${PROJECT_NAME}" RUN "= ${${PROJECT_NAME}_VERSION}" )

set( MORPHOLOGIST-NONFREE_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include" ${AIMS-FREE_INCLUDE_DIRS} ${SOMA-IO_INCLUDE_DIRS} )
include_directories( ${MORPHOLOGIST-NONFREE_INCLUDE_DIRS} )

#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall" )
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall" )

add_subdirectory( vip )

BRAINVISA_CREATE_CMAKE_CONFIG_FILES()
