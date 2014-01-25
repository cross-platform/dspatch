project(DSPatchTutorial)

file(GLOB srcs *.cpp)
file(GLOB hdrs *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(
    ${PROJECT_NAME}
    ${srcs}
    ${hdrs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/DSPatch.dll
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
    )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
