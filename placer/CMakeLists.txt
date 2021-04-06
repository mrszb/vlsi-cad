project (placer)

set (HEADER_FILES
	#stdafx.h
)

set (SOURCE_FILES
	#stdafx.cpp
	placer.cpp
	solver.cpp
)

add_executable (placer WIN32 placer.cpp solver.cpp)

set_property(TARGET placer PROPERTY CXX_STANDARD 17)
set_property(TARGET placer PROPERTY CXX_STANDARD_REQUIRED ON)

IF(MSVC)
  target_link_options(placer PRIVATE "/ENTRY:mainCRTStartup")
  target_link_options(placer PRIVATE "/SUBSYSTEM:WINDOWS")
ENDIF(MSVC)

