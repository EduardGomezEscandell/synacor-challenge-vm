add_executable(runvm runvm.cpp)
set_target_properties(runvm PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(runvm INTERFACE ..)
target_link_libraries(runvm PUBLIC libvm)

add_executable(vmctl vmctl.cpp)
set_target_properties(vmctl PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(vmctl INTERFACE ..)
target_link_libraries(vmctl PUBLIC libvm)