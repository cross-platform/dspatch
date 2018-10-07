cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -Wno-gnu-zero-variadic-macro-arguments -Wno-vla-extension")

project(DSPatch)

enable_testing()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

file(GLOB hdrs include/*.h include/dspatch/*.h src/internal/*.h)
file(GLOB srcs src/*.cpp src/internal/*.cpp)

# Build shared (LGPL)
add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
)

if(MINGW)
    SET_TARGET_PROPERTIES(
        ${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "--output-def, ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.def"
    )
endif(MINGW)

# Link pthread and dl on Unix
if(UNIX)
    target_link_libraries(
        ${PROJECT_NAME}
        pthread
        dl
    )
endif(UNIX)

add_subdirectory(docs)
add_subdirectory(tests)
add_subdirectory(tutorial)
