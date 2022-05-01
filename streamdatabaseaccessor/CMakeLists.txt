cmake_minimum_required(VERSION 2.8)
project(streamdatabaseaccessor)

set(ONSEMSTREADATABASEACCESSOR_HPPS
  include/onsem/streamdatabaseaccessor/api.hpp
  include/onsem/streamdatabaseaccessor/streamdatabaseaccessor.hpp
)

set(ONSEMSTREADATABASEACCESSOR_SRCS
  src/streamdatabaseaccessor.cpp
)

include_directories(include)

if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()

if (USE_BOOST_FROM_SYSTEM)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
endif ()

add_library(onsemstreamdatabaseaccessor SHARED
  ${ONSEMSTREADATABASEACCESSOR_HPPS}
  ${ONSEMSTREADATABASEACCESSOR_SRCS}
)
include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
)
target_link_libraries(onsemstreamdatabaseaccessor
  boost_system
  boost_filesystem
  ${ONSEMCOMMON_LIBRARIES}
)
target_compile_features(onsemstreamdatabaseaccessor PRIVATE cxx_std_14)


install(TARGETS onsemstreamdatabaseaccessor DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

