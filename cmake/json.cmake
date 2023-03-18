include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.10.5
  UPDATE_DISCONNECTED ON
  CMAKE_ARGS -DJSON_Install=OFF -DJSON_BuildTests=OFF
)

#FetchContent_MakeAvailable(json)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)

  set(JSON_BuildTests OFF CACHE BOOL "" FORCE)

  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
