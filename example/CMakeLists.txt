project(DSPatchExample)

file(GLOB srcs *.cpp rtaudio/*.cpp)
file(GLOB hdrs *.h rtaudio/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/rtaudio
)

# Add definition of example audio file
add_definitions(-DEXAMPLE_WAV_FILE="${CMAKE_CURRENT_SOURCE_DIR}/Sample.wav")

add_executable(
    ${PROJECT_NAME}
    ${srcs}
    ${hdrs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)

# Definition for RtAudio Windows, using direct sound
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-D__WINDOWS_DS__)

    target_link_libraries(
        ${PROJECT_NAME}
        dsound.lib
    )
    
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/DSPatch.dll
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
    )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Definition for RtAudio Linux, using ASLA
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-D__LINUX_ALSA__)

    target_link_libraries(
        ${PROJECT_NAME}
        asound
    )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Definition for RtAudio Mac OSX, using Core Audio
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-D__MACOSX_CORE__)

    target_link_libraries(
        ${PROJECT_NAME}
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
