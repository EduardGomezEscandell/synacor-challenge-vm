add_library(testutils utils.hpp utils.cpp)

set_target_properties(testutils PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(testutils INTERFACE .)

target_link_libraries(testutils PUBLIC doctest)