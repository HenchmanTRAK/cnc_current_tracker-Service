
add_library(EventReporter "EventReporter.cpp")

target_include_directories(EventReporter PUBLIC "include" ${CMAKE_SOURCE_DIR})
#target_link_libraries(EventReporter PRIVATE Current_Recorder)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET EventReporter PROPERTY CXX_STANDARD 20)
endif()
