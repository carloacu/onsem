project(dictionaryextractor)
cmake_minimum_required(VERSION 2.8)


include_directories(include)

find_package(Boost REQUIRED)

if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()
if(NOT ONSEMTEXTTOSEMANTIC_FOUND)
  find_package(onsemtexttosemantic)
endif()


add_library(onsemdictionaryextractor
  include/onsem/dictionaryextractor/api.hpp
  include/onsem/dictionaryextractor/auxiliariesextractor.hpp
  src/auxiliariesextractor.cpp
)

# Add position independent code flag if building shared libs or if explicitly requested
if(BUILD_SHARED_LIBS OR POSITION_INDEPENDENT_CODE)
    set_target_properties(onsemdictionaryextractor PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

include_directories(
    ${ONSEMCOMMON_INCLUDE_DIRS}
    ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
)

target_link_libraries(onsemdictionaryextractor
    ${ONSEMCOMMON_LIBRARIES}
    ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
)


