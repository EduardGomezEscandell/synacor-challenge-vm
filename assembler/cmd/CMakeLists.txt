add_executable(tokenize tokenize.cpp)
set_target_properties(tokenize PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(tokenize INTERFACE ..)
target_link_libraries(tokenize PUBLIC libassembler)

add_executable(parse parse.cpp)
set_target_properties(parse PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(parse INTERFACE ..)
target_link_libraries(parse PUBLIC libassembler)

add_executable(assemble assemble.cpp)
set_target_properties(assemble PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(assemble INTERFACE ..)
target_link_libraries(assemble PUBLIC libassembler)
