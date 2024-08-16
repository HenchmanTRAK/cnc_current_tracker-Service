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

project ("CurrentTrackerAsService" CXX)
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake

set(mariadbcpp_Dir "C:\\Program Files\\MariaDB\\MariaDB C++ Connector 64-bit")
find_library(mariadbcpp PATHS ${mariadbcpp_Dir})
find_package(ODBC)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Include sub-projects.
add_subdirectory ("sqlConnection")
add_subdirectory ("currentTracker")
add_subdirectory ("CurrentTrackerAsService")

configure_file(".\\settings.ini" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\\settings.ini" COPYONLY)