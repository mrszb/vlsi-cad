project (placer)

set (HEADER_FILES
	#stdafx.h
)

set (SOURCE_FILES
	#stdafx.cpp
	placer.cpp
	solver.cpp
)

IF(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") 
ENDIF(MSVC)

add_executable (placer WIN32 placer.cpp solver.cpp)

set_property(TARGET placer PROPERTY CXX_STANDARD 17)
set_property(TARGET placer PROPERTY CXX_STANDARD_REQUIRED ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set_target_properties(placer PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(placer PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   
   set_target_properties(placer PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(placer PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")

   set_target_properties(placer PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

   
   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_RELEASE "_CONSOLE")

   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")


  #message("dynamic boost placer")
  #add_definitions( -DBOOST_ALL_NO_LIB )

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#include_directories(${Boost_INCLUDE_DIRS})
#target_link_libraries(placer Boost::system)

# if (WIN32)
#   # disable autolinking in boost
#   message("dynamic")
#   add_definitions( -DBOOST_ALL_NO_LIB )

#   # force all boost libraries to dynamic link (we already disabled
#   # autolinking, so I don't know why we need this, but we do!)
#   add_definitions( -DBOOST_ALL_DYN_LINK )
# endif()