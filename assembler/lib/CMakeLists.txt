add_library(libassembler
    grammar.cpp
    tokenizer.cpp
    parser.cpp
    code_generation.cpp
)

set_target_properties(libassembler PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(libassembler INTERFACE ..)

target_link_libraries(libassembler PUBLIC archlib)