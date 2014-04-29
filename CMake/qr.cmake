## Mr
if (MSVC)
	string (REGEX REPLACE "\\\\" "/" MR_ROOT $ENV{MR_ROOT})
endif ()

set (DEVPKGS_PATH $ENV{DEVPKGS_PATH})

## Platform
if (MSVC)
	if (CMAKE_CL_64)
		set (QR_PLATFORM x64)
	else ()
		set (QR_PLATFORM win32)
	endif ()
else ()
	set (QR_PLATFORM macrohard)
endif ()

## Compiler
if (MSVC11)
	set (QR_COMPILER msvc-11.0)
elseif (MSVC10)
	set (QR_COMPILER msvc-10.0)
elseif (MSVC90)
	set (QR_COMPILER msvc-9.0)
elseif (MSVC80)
	set (QR_COMPILER msvc-8.0)
elseif (MSVC71)
	set (QR_COMPILER msvc-7.1)
elseif (MSVC70)
	set (QR_COMPILER msvc-7.0)
elseif (MSVC60)
	set (QR_COMPILER msvc-6.0)
else ()
	set (QR_COMPILER generic)
endif ()

if (MSVC_IDE)
	set (QR_CONFIGURATION ${CMAKE_CFG_INTDIR})
else ()
	set (QR_CONFIGURATION ${CMAKE_BUILD_TYPE})
endif ()

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#	set (QR_CONFIGURATION "debug")
#elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
#	set (QR_CONFIGURATION "release")
#endif ()

## Compiler Settings
if (MSVC)
	if (CMAKE_CXX_FLAGS MATCHES "/Zm[0-9]+")
		string (REGEX REPLACE "/Zm[0-9]+" "/Zm300" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else ()
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm300")
	endif ()

	# Force to always compile with W4
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string (REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else ()
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif ()
elseif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# Update if necessary
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif ()

