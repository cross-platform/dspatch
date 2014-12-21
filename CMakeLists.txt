cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor")

project(DSPatch)

file(GLOB srcs src/*.cpp)
file(GLOB hdrs include/*.h)
file(GLOB in_hdrs include/dspatch/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Build shared (LGPL)
add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
    ${in_hdrs}
)

# Link pthread and dl on Unix
if(UNIX)
    target_link_libraries(
        ${PROJECT_NAME}
        pthread
        dl
    )
endif(UNIX)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION lib
)

install(
    FILES ${hdrs}
    DESTINATION include
)

install(
    FILES ${in_hdrs}
    DESTINATION include/dspatch
)

option(BUILD_EXAMPLES "Build Examples" ON)

if(${BUILD_EXAMPLES})
    add_subdirectory(example)
    add_subdirectory(tutorial)
endif(${BUILD_EXAMPLES})

add_subdirectory(doc)
