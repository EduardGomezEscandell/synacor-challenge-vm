include(FetchContent)
FetchContent_Declare(
    CxxOpts
    GIT_REPOSITORY "https://github.com/jarro2783/cxxopts"
    GIT_TAG "v3.1.1"
)

FetchContent_MakeAvailable(CxxOpts)
include_directories(${CXXOPTS_INCLUDE_DIR})
