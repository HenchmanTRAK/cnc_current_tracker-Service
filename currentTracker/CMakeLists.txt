


add_library (Current_Recorder "cnc_current_recorder.cpp")

target_include_directories(Current_Recorder PUBLIC "include")
target_link_libraries(Current_Recorder PRIVATE SQL_CONNECTION EventReporter)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Current_Recorder PROPERTY CXX_STANDARD 20)
endif()