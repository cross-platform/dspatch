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

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)

if(WIN32)
    if(MSVC)
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/../$<CONFIGURATION>/DSPatch.dll
            ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
        )
    endif(MSVC)

    if(MINGW)
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/../libDSPatch.dll
            ${CMAKE_CURRENT_BINARY_DIR}
        )

        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/../app.exe.manifest
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.exe.manifest
        )
    endif(MINGW)

endif(WIN32)

add_test(
    ${PROJECT_NAME}
    ${PROJECT_NAME}
)
