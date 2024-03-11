project(DSPatchTests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB srcs *.cpp)

add_executable(
    ${PROJECT_NAME}
    ${srcs}
)

enable_testing()

add_test(
    ${PROJECT_NAME}
    ${PROJECT_NAME}
)
