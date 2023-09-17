add_executable(assemble cmd.cpp)

set_target_properties(assemble PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(assemble INTERFACE ..)

target_link_libraries(assemble PUBLIC libassembler)