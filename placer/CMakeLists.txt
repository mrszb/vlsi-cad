project (placer)

set (OUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${QR_CONFIGURATION}/placer)

set (HEADER_FILES
	stdafx.h
)

set (SOURCE_FILES
	stdafx.cpp
	placer.cpp
	solver.cpp
)

source_group ("header files" FILES ${HEADER_FILES})
source_group ("source files" FILES ${SOURCE_FILES})

add_executable (placer WIN32 ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries (placer
	${lib_dl} 
	${lib_rt}
)

set (FILES_TO_USE_PCH ${SOURCE_FILES})

IF(MSVCO)
   use_precompiled_header (placer stdafx.h stdafx.cpp FILES_TO_USE_PCH)
ENDIF(MSVCO)


macro (copy_files_post_build _target _files _dir)
	foreach (file ${_files})
		get_filename_component (file_name ${file} NAME)
		set (input ${file})
		set (output "${_dir}/${file_name}")

		add_custom_command (
			TARGET ${_target}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${input} ${output}
			COMMENT "Copying ${input} to ${output}"
		)
	endforeach ()
endmacro ()

set (out_dir ${CMAKE_CURRENT_BINARY_DIR}/${QR_CONFIGURATION})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set_target_properties(placer PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(placer PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(placer PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(placer PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
   set_target_properties(placer PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(placer PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

  message("dynamic boost placer")
  add_definitions( -DBOOST_ALL_NO_LIB )

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include_directories(${Boost_INCLUDE_DIRS})
#target_link_libraries(placer ${Boost_LIBRARIES})
target_link_libraries(placer Boost::filesystem Boost::system Boost::thread)

if (WIN32)
  # disable autolinking in boost
  message("dynamic")
  add_definitions( -DBOOST_ALL_NO_LIB )

  # force all boost libraries to dynamic link (we already disabled
  # autolinking, so I don't know why we need this, but we do!)
  add_definitions( -DBOOST_ALL_DYN_LINK )
endif()