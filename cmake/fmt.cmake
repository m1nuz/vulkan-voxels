include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0
    CMAKE_ARGS -DFMT_DOC=OFF -DFMT_INSTALL=OFF -DFMT_TEST=OFF
)

FetchContent_MakeAvailable(fmt)

if(IS_DIRECTORY "${fmt_SOURCE_DIR}")
    set_property(DIRECTORY ${fmt_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)
endif()
