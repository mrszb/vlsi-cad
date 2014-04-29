
macro (use_precompiled_header _target _pch_header _pch_source _sources)

	if (MSVC)

		set (PCH_INTDIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")

		#make_directory ("${PCH_INTDIR}")

		set (_pch_file "${PCH_INTDIR}/${_target}.pch")

		set (_pch_source_copy ${_pch_source})

		foreach (_source ${${_sources}})
			if (_source STREQUAL _pch_source_copy)
				set_source_files_properties (${_source} PROPERTIES COMPILE_FLAGS "/Yc\"${_pch_header}\" /Fp\"${_pch_file}\"" OBJECT_OUTPUTS "${_pch_file}")
			else (_source STREQUAL _pch_source_copy)
				set_source_files_properties (${_source} PROPERTIES COMPILE_FLAGS "/Yu\"${_pch_header}\" /Fp\"${_pch_file}\"" OBJECT_DEPENDS "${_pch_file}")
			endif (_source STREQUAL _pch_source_copy)
		endforeach ()

		set_target_properties (${_target} PROPERTIES COMPILE_FLAGS "/Yu\"${_pch_header}\" /Fp\"${_pch_file}\"")

	endif (MSVC)

	if (CMAKE_COMPILER_IS_GNUCXX)

		set (PCH_INTDIR "${CMAKE_CURRENT_BINARY_DIR}")

		string (TOUPPER "CMAKE_CXX_FLAGS" _var)
		set (_compiler_flags ${${_var}})

		list (APPEND _compiler_flags "-I${PCH_INTDIR}")

		get_directory_property (_include_directories INCLUDE_DIRECTORIES)
		foreach (item ${_include_directories})
			list (APPEND _compiler_flags "-I${item}")
		endforeach ()

		get_directory_property (_definitions DEFINITIONS)
		foreach (item ${_definitions})
			list (APPEND _compiler_flags "${item}")
		endforeach ()

		separate_arguments (_compiler_flags)

		get_filename_component (_pch_file_name ${_pch_header} NAME)
		set (_pch_header_full_path "${CMAKE_CURRENT_SOURCE_DIR}/${_pch_header}")
		set (_pch_file "${PCH_INTDIR}/${_pch_file_name}.gch")
		set (_pch_source_copy ${_pch_source})

		add_custom_command (
			OUTPUT ${_pch_file}
			COMMAND ${CMAKE_CXX_COMPILER} -x c++-header ${_pch_header_full_path} ${_compiler_flags} -DQR_GCH_PRAGMA_ONCE_NO_WARNING -o ${_pch_file}
			DEPENDS ${_pch_header_full_path}
			COMMENT "Generating precompiled header ${_pch_file}"
		)

		add_custom_target (${_target}_gch DEPENDS ${_pch_file})
		add_dependencies (${_target} ${_target}_gch)
		set_target_properties (${_target} PROPERTIES COMPILE_FLAGS "-Winvalid-pch")
	endif ()

endmacro ()

macro (ignore_precompiled_header _source)

	if (MSVC)
		set_source_files_properties (${_source} PROPERTIES COMPILE_FLAGS "/Y-")
	endif ()

	if (CMAKE_COMPILER_IS_GNUCXX)
		get_source_file_property (_compile_flags ${_source} COMPILE_FLAGS)
		string (REGEX REPLACE "-include ${_header}" "" _compile_flags "${_compile_flags}")
	endif ()

endmacro()

macro (force_include _source _header)

	if (MSVC)
		set_source_files_properties (${_source} PROPERTIES COMPILE_FLAGS "/FI\"${_header}\"")		
	endif ()

	if (CMAKE_COMPILER_IS_GNUCXX)
		set_source_files_properties (${_source} PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR} -include ${_header}")
	endif ()

endmacro ()
