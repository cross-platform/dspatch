cmake_minimum_required(VERSION 3.1)

project(DSPatch)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
elseif(MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor -Wno-unknown-pragmas")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -Wno-gnu-zero-variadic-macro-arguments -Wno-vla-extension")
endif()

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
