add_library(libvm
    cpu.hpp    cpu.cpp
    word.hpp
    memory.hpp
)

set_target_properties(libvm PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(libvm INTERFACE ..)
