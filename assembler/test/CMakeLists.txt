if (BUILD_TESTS)
    set(ENABLE_DOCTESTS 1)

    add_executable(testassembler test.cpp)

    set_target_properties(testassembler PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(testassembler INTERFACE ..)

    target_link_libraries(testassembler PUBLIC doctest libassembler)
endif()