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

IF(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
ENDIF(MSVC)

add_executable (core_router ${HEADER_FILES} ${SOURCE_FILES})

include_directories(${Boost_FILESYSTEM_INCLUDE_DIRS})
target_link_libraries(core_router PRIVATE Boost::filesystem Boost::system)





