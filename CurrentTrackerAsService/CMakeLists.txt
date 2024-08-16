# CMakeList.txt : CMake project for CurrentTrackerAsService, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (CNC_Current_Tracker "CurrentTrackerAsService.cpp")

target_include_directories(CNC_Current_Tracker PRIVATE "include")
target_link_libraries(CNC_Current_Tracker PUBLIC Current_Tracker SQL_CONNECTION)

set_target_properties(CNC_Current_Tracker PROPERTIES LINK_FLAGS
	"/MANIFESTUAC:\"level='requireAdministrator'\"")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CNC_Current_Tracker PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
