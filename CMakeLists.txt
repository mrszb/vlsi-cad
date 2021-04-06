cmake_minimum_required(VERSION 3.15)
project (vlsicad CXX)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
#add_library(project_options INTERFACE)
#target_compile_features(project_options INTERFACE cxx_std_17)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
#set_project_warnings(project_warnings)

# Set up some extra Conan dependencies based on our needs before loading Conan
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

include(cmake/Conan.cmake)
run_conan()

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests. Be sure to check out test/constexpr_tests for constexpr testing")
  add_subdirectory(test)
endif()

# set(CMAKE_LEGACY_CYGWIN_WIN32 0) 
# cmake_minimum_required (VERSION 3.0)
# set(CMAKE_LEGACY_CYGWIN_WIN32 0) 

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

IF(MSVC)
 set(Boost_USE_STATIC_LIBS ON) 
ENDIF(MSVC)

# Include Conan info
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

message(STATUS "CONAN_BOOST_ROOT: ${CONAN_BOOST_ROOT}")
# Dependencies -- Boost
#Set(BOOST_ROOT ${CONAN_BOOST_ROOT})
#Set(BOOST_LIBRARY ${CONAN_BOOST_ROOT})
find_package(Boost 
		1.73.0 
		COMPONENTS thread system filesystem
		REQUIRED) 

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

# manage compilation and linkage
if(Boost_FOUND)
    message("BOOST " ${Boost_VERSION})
    message("BOOST INCLUDE: " ${Boost_INCLUDE_DIRS})
    foreach(lib ${Boost_LIBRARIES})
    	message(${lib})
    endforeach()
endif()

#add_definitions( -DBOOST_ALL_NO_LIB )
#add_definitions( -DBOOST_ALL_DYN_LINK )
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory ("unate_recursive_complement")
add_subdirectory ("core_router")
add_subdirectory ("placer")

# does the application run
# add_test (core_router_runs core_router 25)
# add_test (core_router_runs core_router benchmark bench1)

######################################################################
# add_custom_target(clean-cmake-files
#    COMMAND ${CMAKE_COMMAND} -P clean-all.cmake
# )

# add_custom_target(clean-all
#    COMMAND ${CMAKE_BUILD_TOOL} clean
#    COMMAND ${CMAKE_COMMAND} -P clean-all.cmake
# )
