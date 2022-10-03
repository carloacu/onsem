project(dictionaryextractor)
cmake_minimum_required(VERSION 2.8)


include_directories(include)

if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()
if(NOT ONSEMTEXTTOSEMANTIC_FOUND)
  find_package(onsemtexttosemantic)
endif()


add_library(onsemdictionaryextractor SHARED
  include/onsem/dictionaryextractor/api.hpp
  include/onsem/dictionaryextractor/auxiliariesextractor.hpp
  src/auxiliariesextractor.cpp
)

include_directories(
    ${ONSEMCOMMON_INCLUDE_DIRS}
    ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
)

target_link_libraries(onsemdictionaryextractor
    ${ONSEMCOMMON_LIBRARIES}
    ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
)


