project(DSPatchTutorial)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB srcs *.cpp)

add_executable(
    ${PROJECT_NAME}
    ${srcs}
)
