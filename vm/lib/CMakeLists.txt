add_library(libvm dummy.hpp)

set_target_properties(libvm PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(libvm INTERFACE ..)
