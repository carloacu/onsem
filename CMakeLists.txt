cmake_minimum_required(VERSION 3.12)
project(onsem)


# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


## Command line for database compilation
# cmake -DBUILD_ONSEM_DATABASE=ON -DCMAKE_BUILD_TYPE=Debug ../ && make -j4

## Command line for compilation
# cmake -DBUILD_ONSEM_DATABASE=OFF -DCMAKE_BUILD_TYPE=Debug -DBUILD_DB_GENERATOR_PREPROCESSING=ON -DBUILD_ONSEM_TESTS=ON ../ && make -j4

#add_subdirectory(boost-submodules-for-onsem)

option(
  BUILD_DB_GENERATOR_PREPROCESSING
  "Build gui to add information to linguistic database."
  OFF)

option(
  BUILD_ONSEM_TESTS
  "Build the onsem tests."
  OFF)

option(
  GENERATE_DATABASE
  "Generate the databases"
  ON
)


option(
  CMAKE_INSTALL_INCLUDEDIR
  "Where to install the includes"
  include
)




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

  if (GENERATE_DATABASE)
    add_custom_target(
      generate_semantic_databases ALL
      DEPENDS check_if_semantic_databases_are_needed)
    add_custom_command(
      OUTPUT
        check_if_semantic_databases_are_needed  # ensure we run the binary, but the binary is no op if nothing is needed!
      COMMAND "${ONSEMDATABASEGENERATOR_EXECUTABLE}" "${SEMANTICDATABASE_OUTDIR}" "${SHARE_FOLDER}"
    )
    install(DIRECTORY "${SEMANTICDATABASE_OUTDIR}/linguistic/databases" DESTINATION share/linguistic)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/share/semantic/relations" DESTINATION share/semantic)
  endif()

  add_subdirectory(texttosemantic)
  add_subdirectory(semantictotext)
  add_subdirectory(streamdatabaseaccessor)
  add_subdirectory(semanticdebugger)

  if (BUILD_DB_GENERATOR_PREPROCESSING)
    add_subdirectory(databasegeneratorprepocessing)
  endif()

  if (BUILD_ONSEM_TESTS)
    add_subdirectory(tests)
    add_subdirectory(onsemgui)
  endif()

endif()



