cmake_minimum_required(VERSION 2.8)
project(databasegenerator)


if(BUILD_ONSEM_DATABASE)

  if(NOT ONSEMCOMMON_FOUND)
    find_package(onsemcommon)
  endif()
  if(NOT ONSEMCOMPILERMODEL_FOUND)
    find_package(onsemcompilermodel)
  endif()

  add_executable(onsemdatabasegenerator
    "src/main.cpp"
  )
  include_directories(
    ${ONSEMCOMMON_INCLUDE_DIRS}
    ${ONSEMCOMPILERMODEL_INCLUDE_DIRS}
  )
  target_link_libraries(onsemdatabasegenerator
    ${ONSEMCOMMON_LIBRARIES}
    ${ONSEMCOMPILERMODEL_LIBRARIES}
  )
  target_compile_features(onsemdatabasegenerator PRIVATE cxx_std_17)

else(BUILD_ONSEM_DATABASE)

  set(ONSEMDATABASEGENERATOR_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/onsemdatabasegenerator")
  message(STATUS "ONSEMDATABASEGENERATOR_EXECUTABLE=${ONSEMDATABASEGENERATOR_EXECUTABLE}")
  set_property(GLOBAL PROPERTY onsem_database_generator_executable "${ONSEMDATABASEGENERATOR_EXECUTABLE}")

endif(BUILD_ONSEM_DATABASE)

