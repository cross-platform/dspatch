cmake_minimum_required(VERSION 2.8)

project(dspatch)

file(GLOB srcs src/*.cpp)
file(GLOB hdrs include/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Build shared (LGPL)
add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
)

# Link pthread on Linux
if(UNIX)
    target_link_libraries(
        ${PROJECT_NAME}
        pthread
    )
endif(UNIX)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION lib
)

install(
    FILES ${hdrs}
    DESTINATION include/dspatch
)

option(BUILD_EXAMPLES "Build Examples" ON)

if(${BUILD_EXAMPLES})
    add_subdirectory(example)
    add_subdirectory(tutorial)
endif(${BUILD_EXAMPLES})

add_subdirectory(doc)
