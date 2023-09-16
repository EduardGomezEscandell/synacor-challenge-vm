add_library(archlib
    arch.hpp
)

set_target_properties(archlib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(archlib INTERFACE ..)
