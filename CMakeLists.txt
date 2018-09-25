cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(vulkan-voxels)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(APP_NAME vv-client)
set(INSTALL_DIR /usr/bin)
set(SHARED_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCES
    src/clients/main.cc)

find_package(GLM REQUIRED)
find_package(Vulkan REQUIRED)
find_package(XCB REQUIRED)

configure_file("${SHARED_INCLUDE_DIR}/config.h.in" "${SHARED_INCLUDE_DIR}/config.hh")

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${VULKAN_INCLUDE_DIR}
    ${XCB_INCLUDES}
    external
)

target_compile_definitions(${APP_NAME} PUBLIC ${APP_DEFINES})

if (UNIX)
    target_compile_options(${APP_NAME} PUBLIC
        -pthread
        -pedantic
        -Wall
        -Wextra
        -Wshadow
        -Wpointer-arith
        -Wcast-qual
        -Wunused-result
        )
elseif (MSVC)
    target_compile_options(${APP_NAME} PUBLIC
        /W3
#        /WX
        )
endif()

target_link_libraries(${APP_NAME} PUBLIC
    ${APP_LIBRARIES}
    ${VULKAN_LIBRARY}
    ${XCB_LIBRARIES}
)

target_compile_definitions(${APP_NAME} PUBLIC -DVK_USE_PLATFORM_XCB_KHR)

install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
