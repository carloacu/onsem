project(guiutility)
cmake_minimum_required(VERSION 2.8)



# Making a lib
set(ONSEMGUIUTILITY_HPPS
    include/onsem/guiutility/lineedithistoricwrapper.hpp
)

set(ONSEMGUIUTILITY_SRCS
    src/lineedithistoricwrapper.cpp
)


include_directories(include)


find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_library(onsemguiutility
  ${ONSEMGUIUTILITY_HPPS}
  ${ONSEMGUIUTILITY_SRCS}
)

# Add position independent code flag if building shared libs or if explicitly requested
if(BUILD_SHARED_LIBS OR POSITION_INDEPENDENT_CODE)
    set_target_properties(onsemguiutility PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(onsemguiutility
  Qt5::Core
  Qt5::Widgets
)



