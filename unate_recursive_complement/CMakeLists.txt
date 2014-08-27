project (unate_recursive_complement)

set (OUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${QR_CONFIGURATION}/unate_recursive_complement)

set (HEADER_FILES
	stdafx.h
)

set (SOURCE_FILES
	stdafx.cpp
	vlsicad_urc.cpp
)

source_group ("header files" FILES ${HEADER_FILES})
source_group ("source files" FILES ${SOURCE_FILES})

add_executable (unate_recursive_complement WIN32 ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries (unate_recursive_complement
	${lib_dl} 
	${lib_rt}
)

set (FILES_TO_USE_PCH ${SOURCE_FILES})
set (FILES_TO_USE_PCH ${SOURCE_FILES} ${GENEGATED_SOURCE_FILES})
IF(MSVCO)
   use_precompiled_header (unate_recursive_complement stdafx.h stdafx.cpp FILES_TO_USE_PCH)
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
   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(unate_recursive_complement PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(unate_recursive_complement PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(unate_recursive_complement ${Boost_LIBRARIES})
  
