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

add_library(onsemstreamdatabaseaccessor
  ${ONSEMSTREADATABASEACCESSOR_HPPS}
  ${ONSEMSTREADATABASEACCESSOR_SRCS}
)

# Add position independent code flag if building shared libs or if explicitly requested
if(BUILD_SHARED_LIBS OR POSITION_INDEPENDENT_CODE)
    set_target_properties(onsemstreamdatabaseaccessor PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
)
target_link_libraries(onsemstreamdatabaseaccessor
  ${ONSEMCOMMON_LIBRARIES}
)


# Create a proper installed package that can be included.
install(TARGETS onsemstreamdatabaseaccessor
    EXPORT onsemstreamdatabaseaccessorTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/onsem DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT onsemstreamdatabaseaccessorTargets
    FILE onsemstreamdatabaseaccessorTargets.cmake
    NAMESPACE onsem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/onsem
)
