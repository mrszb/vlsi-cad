project (core_router)
#


set (OUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${QR_CONFIGURATION}/core_router)

set (HEADER_FILES
	stdafx.h
)

set (SOURCE_FILES
	stdafx.cpp
	core_router.cpp
)

include_directories ("${GENEGATED_SOURCE_FILES}/..")


source_group ("header files" FILES ${HEADER_FILES})
source_group ("source files" FILES ${SOURCE_FILES})
source_group ("generated files" FILES ${GENEGATED_SOURCE_FILES})

add_executable (core_router WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${GENEGATED_SOURCE_FILES} ${UI_FILES} ${MOC_FILES})


target_link_libraries (core_router 
	${lib_dl} 
	${lib_rt}
)

set (FILES_TO_USE_PCH ${SOURCE_FILES} ${GENEGATED_SOURCE_FILES})
IF(MSVC)
   use_precompiled_header (core_router stdafx.h stdafx.cpp FILES_TO_USE_PCH)
ENDIF(MSVC)

foreach (generated_file ${GENEGATED_SOURCE_FILES})
	force_include (${generated_file} stdafx.h)
endforeach ()

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
   set_target_properties(core_router PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(core_router PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(core_router PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(core_router PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
   set_target_properties(core_router PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(core_router PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(core_router ${Boost_LIBRARIES})
