include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG master
  UPDATE_DISCONNECTED ON
)

FetchContent_MakeAvailable(glm)

if(IS_DIRECTORY "${glm_SOURCE_DIR}")
    set_property(DIRECTORY ${glm_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)
endif()
