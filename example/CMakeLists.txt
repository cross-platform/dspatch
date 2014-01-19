project(DSPatchExample)

file(GLOB srcs *.cpp rtaudio/*.cpp)
file(GLOB hdrs *.h rtaudio/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/rtaudio
)

# Add definition of example audio file
set(
    EXAMPLE_WAV_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/Tchaikovski-Swan-Lake-Scene.wav
)

add_definitions(-DEXAMPLE_WAV_FILE="${EXAMPLE_WAV_FILE}")

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
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
