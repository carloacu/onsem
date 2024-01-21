cmake_minimum_required(VERSION 2.8)
project(onsem)


## Command line for database compilation
# cmake -DBUILD_ONSEM_DATABASE=ON -DCMAKE_BUILD_TYPE=Debug ../ && make -j4

## Command line for compilation
# cmake  -DBUILD_ONSEM_DATABASE=OFF -DCMAKE_BUILD_TYPE=Debug -DBUILD_ONSEM_TESTS=ON ../ && make -j4

#add_subdirectory(boost-submodules-for-onsem)


include(onsem-config.cmake)
add_subdirectory(common)

add_subdirectory(typeallocatorandserializer)
add_subdirectory(compilermodel)
add_subdirectory(databasegenerator)

if (NOT BUILD_ONSEM_DATABASE)
  # Generate the semantic databases
  get_property(ONSEMDATABASEGENERATOR_EXECUTABLE GLOBAL PROPERTY onsem_database_generator_executable)
  message(STATUS "executable_to_generate_the_databases=${ONSEMDATABASEGENERATOR_EXECUTABLE}")

  set(SEMANTICDATABASE_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}")
  message(STATUS "SEMANTICDATABASE_OUTDIR=${SEMANTICDATABASE_OUTDIR}")

  set(SHARE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/share")
  message(STATUS "SHARE_FOLDER=${SHARE_FOLDER}")

  add_custom_target(
    generate_semantic_databases ALL
    DEPENDS check_if_semantic_databases_are_needed)
  add_custom_command(
    OUTPUT
      check_if_semantic_databases_are_needed  # ensure we run the binary, but the binary is no op if nothing is needed!
    COMMAND "${ONSEMDATABASEGENERATOR_EXECUTABLE}" "${SEMANTICDATABASE_OUTDIR}" "${SHARE_FOLDER}"
  )

  install(DIRECTORY "${SEMANTICDATABASE_OUTDIR}/linguistic/databases" DESTINATION share/linguistic)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/share/semantic/dynamicdictionary" DESTINATION share/semantic)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/share/semantic/people" DESTINATION share/semantic)

  add_subdirectory(texttosemantic)
  add_subdirectory(semantictotext)
  add_subdirectory(streamdatabaseaccessor)
  add_subdirectory(semanticdebugger)

  if (BUILD_DB_GENERATOR_PREPROCESSING)
    include_directories("subdirectories/openai-cpp/include/openai")
    add_subdirectory(databasegeneratorprepocessing)
  endif()

  if (BUILD_ONSEM_TESTS)
    add_subdirectory(tests)
    add_subdirectory(onsemgui)
  endif()

endif()



