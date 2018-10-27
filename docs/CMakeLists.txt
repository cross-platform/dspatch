option(RUN_DOXYGEN "Run Doxygen" OFF)

if(${RUN_DOXYGEN})

find_package(Doxygen)

  if(DOXYGEN_FOUND)

    add_custom_target(
      docs ALL
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

  endif(DOXYGEN_FOUND)

endif(${RUN_DOXYGEN})
