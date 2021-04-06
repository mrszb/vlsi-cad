project (unate_recursive_complement)
add_executable (unate_recursive_complement WIN32 vlsicad_urc.cpp)

set_property(TARGET unate_recursive_complement PROPERTY CXX_STANDARD 17)
set_property(TARGET unate_recursive_complement PROPERTY CXX_STANDARD_REQUIRED ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   
   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")

   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "_CONSOLE")

   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_RELEASE "_CONSOLE")

   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")

#   message("dynamic boost urc")
#   add_definitions( -DBOOST_ALL_NO_LIB )

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include_directories(${Boost_INCLUDE_DIRS})
target_include_directories(unate_recursive_complement PRIVATE ${BOOST_FILESYSTEM_INCLUDE_DIRS})
target_link_libraries(unate_recursive_complement Boost::filesystem Boost::system Boost::thread)
  
