cmake_minimum_required(VERSION 2.8)

project(DSPatch)

add_subdirectory(doc)

file(GLOB lib_srcs src/*.cpp)
file(GLOB lib_hdrs include/*.h src/*.h)

# build only shared, makes sense for LGPL license
add_library(${PROJECT_NAME} SHARED ${lib_srcs} ${lib_hdrs})

# Definition for RtAudio Linux, using ASLA
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        target_link_libraries(${PROJECT_NAME}
                pthread
        )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
)

install(FILES ${lib_hdrs}        
         DESTINATION include
)

option(BUILD_EXAMPLES "Build Examples" ON)

if(${BUILD_EXAMPLES})
	add_subdirectory(example)
	add_subdirectory(tutorial)
endif(${BUILD_EXAMPLES})
