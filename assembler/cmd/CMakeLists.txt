add_executable(tokenize tokenizer.cpp)

set_target_properties(tokenize PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(tokenize INTERFACE ..)

target_link_libraries(tokenize PUBLIC libassembler)