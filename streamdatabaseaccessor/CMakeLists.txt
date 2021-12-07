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
  Boost::system
  Boost::filesystem
  ${ONSEMCOMMON_LIBRARIES}
)

install(TARGETS onsemstreamdatabaseaccessor DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

