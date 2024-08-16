


add_library (Current_Tracker "cnc_current_tracker.cpp")

target_include_directories(Current_Tracker PUBLIC "include")
target_link_libraries(Current_Tracker PUBLIC SQL_CONNECTION)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Current_Tracker PROPERTY CXX_STANDARD 20)
endif()