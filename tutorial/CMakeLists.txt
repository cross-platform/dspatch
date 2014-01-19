project(DSPatchTutorial)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/src
)

file(GLOB srcs *.cpp rtaudio/*.cpp)
file(GLOB hdrs *.h rtaudio/*.h)

add_executable(${PROJECT_NAME} ${srcs} ${hdrs})

target_link_libraries(${PROJECT_NAME}
	DSPatch
)
