cmake_minimum_required(VERSION 2.8)
project(databasegenerator)


if(BUILD_ONSEM_DATABASE)

  if(NOT ONSEMCOMMON_FOUND)
    find_package(onsemcommon)
  endif()
  if(NOT ONSEMLINGDBEDITOR_FOUND)
    find_package(onsemlingdbeditor)
  endif()

  add_executable(onsemdatabasegenerator
    "src/main.cpp"
  )
  include_directories(
    ${ONSEMCOMMON_INCLUDE_DIRS}
    ${ONSEMLINGDBEDITOR_INCLUDE_DIRS}
  )
  target_link_libraries(onsemdatabasegenerator
    ${ONSEMCOMMON_LIBRARIES}
    ${ONSEMLINGDBEDITOR_LIBRARIES}
  )

else(BUILD_ONSEM_DATABASE)

  set(ONSEMDATABASEGENERATOR_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/onsemdatabasegenerator")
  message(STATUS "ONSEMDATABASEGENERATOR_EXECUTABLE=${ONSEMDATABASEGENERATOR_EXECUTABLE}")
  set_property(GLOBAL PROPERTY onsem_database_generator_executable "${ONSEMDATABASEGENERATOR_EXECUTABLE}")

endif(BUILD_ONSEM_DATABASE)

