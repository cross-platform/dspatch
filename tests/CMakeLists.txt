project(DSPatchTests)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB srcs *.cpp)

add_executable(
    ${PROJECT_NAME}
    ${srcs}
)

add_test(
    ${PROJECT_NAME}
    ${PROJECT_NAME}
)
