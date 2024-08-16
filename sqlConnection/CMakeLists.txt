
add_library(SQL_CONNECTION "sql_connection.cpp")

target_include_directories(SQL_CONNECTION PUBLIC "include" "${mariadbcpp_Dir}\\include")
target_link_libraries(SQL_CONNECTION PUBLIC ${ODBC_LIBRARY} "${mariadbcpp_Dir}\\mariadbcpp.lib" Current_Tracker)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SQL_CONNECTION PROPERTY CXX_STANDARD 20)
endif()
