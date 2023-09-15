add_executable(runvm cmd.cpp)

set_target_properties(runvm PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(runvm INTERFACE ..)

target_link_libraries(runvm PUBLIC libvm)