cmake_minimum_required (VERSION 3.0)
set(CMAKE_LEGACY_CYGWIN_WIN32 0) 

project (vlsicad)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

IF(MSVC)
 #  set(Boost_USE_STATIC_LIBS ON) 
ENDIF(MSVC)

find_package(Boost 
		1.60.0 
		COMPONENTS thread system filesystem
		REQUIRED) 

# manage compilation and linkage
if(Boost_FOUND)
 

    message("BOOST " Boost_VERSION)
    message("BOOST INCLUDE: " ${Boost_INCLUDE_DIRS})
    foreach(lib ${Boost_LIBRARIES})
    	message(${lib})
    endforeach()

   include_directories(${Boost_INCLUDE_DIRS})

endif()

add_definitions( -DBOOST_ALL_NO_LIB )
add_definitions( -DBOOST_ALL_DYN_LINK )



set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#include_directories ("${PROJECT_SOURCE_DIR}/..")

add_subdirectory ("unate_recursive_complement")
add_subdirectory ("core_router")
add_subdirectory ("placer")

# does the application run
add_test (core_router_runs core_router 25)
add_test (core_router_runs core_router benchmark bench1)
  

######################################################################
add_custom_target(clean-cmake-files
   COMMAND ${CMAKE_COMMAND} -P clean-all.cmake
)

add_custom_target(clean-all
   COMMAND ${CMAKE_BUILD_TOOL} clean
   COMMAND ${CMAKE_COMMAND} -P clean-all.cmake
)
