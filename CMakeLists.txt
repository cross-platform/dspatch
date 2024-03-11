cmake_minimum_required(VERSION 3.4)

project(DSPatch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
elseif(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor -Wno-unknown-pragmas")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -Wno-gnu-zero-variadic-macro-arguments -Wno-vla -Wno-vla-extension")
endif()

file(GLOB root_hdrs include/*.h)
file(GLOB dspatch_hdrs include/dspatch/*.h)
file(GLOB fast_any_hdrs include/fast_any/*.h)

install(FILES ${root_hdrs} DESTINATION include)
install(FILES ${dspatch_hdrs} DESTINATION include/dspatch)
install(FILES ${fast_any_hdrs} DESTINATION include/fast_any)

add_subdirectory(docs)
add_subdirectory(tests)
add_subdirectory(tutorial)
