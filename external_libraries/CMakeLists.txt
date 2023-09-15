option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                    Tests in tests/*.cpp will still be enabled." ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(Doctest.cmake)