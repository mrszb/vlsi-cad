project (core_router)

set (HEADER_FILES
	#stdafx.h
)

set (SOURCE_FILES
	#stdafx.cpp
	core_router.cpp
)

source_group ("header files" FILES ${HEADER_FILES})
source_group ("source files" FILES ${SOURCE_FILES})

add_executable (core_router WIN32 ${HEADER_FILES} ${SOURCE_FILES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  # set_target_properties(core_router PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
  #  set_target_properties(core_router PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
  #  set_target_properties(core_router PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
  # target_link_options(core_router PUBLIC /entry:mainCRTStartup /SUBSYSTEM:WINDOWS)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(core_router PRIVATE Boost::filesystem Boost::system CONAN_PKG::fmt)

# if (WIN32)
#   # disable autolinking in boost
#   message("dynamic")
#   add_definitions( -DBOOST_ALL_NO_LIB )

#   # force all boost libraries to dynamic link (we already disabled
#   # autolinking, so I don't know why we need this, but we do!)
#   add_definitions( -DBOOST_ALL_DYN_LINK )
# endif()
