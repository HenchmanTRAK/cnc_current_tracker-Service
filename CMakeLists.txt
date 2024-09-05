# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (CurrentTrackerAsService CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake
include(FetchContent)
set(mariadbcpp_Dir "C:\\Program Files\\MariaDB\\MariaDB C++ Connector 64-bit")
find_library(mariadbcpp PATHS ${mariadbcpp_Dir})
find_package(ODBC)

FetchContent_Declare(
	SimpleIni
	GIT_REPOSITORY "https://github.com/brofield/simpleini.git"
	GIT_TAG v4.22
)

FetchContent_MakeAvailable(SimpleIni)

set(SAMPLE_RES_DIR ${CMAKE_SOURCE_DIR}/sample.res)
set(SAMPLE_RC_DIR ${CMAKE_SOURCE_DIR}/sample.rc)
set(SAMPLE_HEADER_DIR ${CMAKE_SOURCE_DIR}/sample.h)
message(WARNING "sample: ${SAMPLE_RES_DIR}")

# Include sub-projects.
add_subdirectory ("sqlConnection")
add_subdirectory ("currentTracker")
add_subdirectory ("CurrentTrackerAsService")

configure_file(".\\settings.ini" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\\settings.ini" COPYONLY)
configure_file(".\\sample.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\\sample.dll" COPYONLY)
#install(TARGETS CNC_Current_Tracker RUNTIME_DEPENDENCIES PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" POST_EXCLUDE_REGEXES ".*system32/.*\\.dll")
#install (FILES ".\\settings.ini" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})