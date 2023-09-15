if (BUILD_TESTS)
    set(ENABLE_DOCTESTS 1)

    add_executable(testvm test.cpp)

    set_target_properties(testvm PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(testvm INTERFACE ..)

    target_link_libraries(testvm PUBLIC libvm doctest)

endif()