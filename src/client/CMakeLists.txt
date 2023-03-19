configure_file("${SHARED_INCLUDE_DIR}/config.hpp.in" "${CMAKE_BINARY_DIR}/config.hpp")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

set(APP_NAME "vkvoxels")

add_executable(${APP_NAME}
    Journal.cpp
    Application.cpp
    Window.cpp
    Renderer.cpp
    World.cpp
    Chunk.cpp
    Block.cpp
    Camera.cpp
    Frustum.cpp
    Plane.cpp
    Storage.cpp
    TextureAtlas.cpp
    ImageLoader.cpp
    main.cpp
)

target_compile_options(${APP_NAME}
    PUBLIC
    -pthread
    -pedantic
    -Wall
    -Wextra
    -Werror
)

target_compile_features(${APP_NAME}
    PUBLIC
    cxx_std_20
)

target_include_directories(${APP_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${Vulkan_INCLUDE_DIRS}>
    PRIVATE
    $<BUILD_INTERFACE:${stb_image_SOURCE_DIR}>
)

target_link_libraries(${APP_NAME}
    PUBLIC
    fmt::fmt
    stdc++
    stdc++fs
    Threads::Threads
    PRIVATE
    ${GLFW_STATIC_LIBRARIES}
    ${Vulkan_LIBRARIES}
    nlohmann_json::nlohmann_json
)