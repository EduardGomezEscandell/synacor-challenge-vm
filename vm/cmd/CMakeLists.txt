add_executable(runvm runvm.cpp)
set_target_properties(runvm PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(runvm INTERFACE ..)
target_link_libraries(runvm PUBLIC libvm)

add_executable(instr-viewer instr-viewer.cpp)
set_target_properties(instr-viewer PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(instr-viewer INTERFACE ..)
target_link_libraries(instr-viewer PUBLIC libvm)

add_executable(coverage coverage.cpp)
set_target_properties(coverage PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(coverage INTERFACE ..)
target_link_libraries(coverage PUBLIC libvm)