cmake_minimum_required(VERSION 3.4)

project(DSPatch)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set version information.
set(DSPATCH_MAJOR 7)
set(DSPATCH_MINOR 0)
set(DSPATCH_MICRO 1)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
    set(dspatch_dbg ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/${PROJECT_NAME}.pdb)
    set(VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    set(MANIFEST_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dll.manifest)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in ${VERSION_FILE})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/manifest.in ${MANIFEST_FILE})
elseif(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor -Wno-unknown-pragmas")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -Wno-gnu-zero-variadic-macro-arguments -Wno-vla-extension")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DSPatchVersion.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/DSPatchVersion.h)

enable_testing()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

file(GLOB root_hdrs include/*.h)
file(GLOB dspatch_hdrs include/dspatch/*.h)
file(GLOB internal_hdrs src/internal/*.h)
file(GLOB srcs src/*.cpp src/internal/*.cpp)

# Build shared
add_library(
    ${PROJECT_NAME} SHARED
    ${root_hdrs}
    ${dspatch_hdrs}
    ${internal_hdrs}
    ${srcs}
    ${VERSION_FILE}
    ${MANIFEST_FILE}
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    SOVERSION ${DSPATCH_MAJOR}
    VERSION ${DSPATCH_MAJOR}.${DSPATCH_MINOR}.${DSPATCH_MICRO}
)

if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES
	LINK_FLAGS "/MANIFESTUAC:NO /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")
  set_target_properties(${PROJECT_NAME} PROPERTIES 
    COMPILE_PDB_NAME ${PROJECT_NAME} 
    COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
    )
endif()

# Link pthread and dl on Unix
if(UNIX)
    target_link_libraries(
        ${PROJECT_NAME}
        pthread
        dl
    )
endif(UNIX)

install(FILES ${root_hdrs} DESTINATION include)
install(FILES ${dspatch_hdrs} DESTINATION include/dspatch)
install(FILES ${dspatch_dbg} DESTINATION lib)
install(TARGETS
  ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Config
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
export(TARGETS ${PROJECT_NAME}
  NAMESPACE ${PROJECT_NAME}::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  )
install(EXPORT
  ${PROJECT_NAME}Config
  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME}"
  NAMESPACE ${PROJECT_NAME}::
  )
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_subdirectory(docs)
add_subdirectory(tests)
add_subdirectory(tutorial)
