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

add_library(onsemguiutility STATIC
  ${ONSEMGUIUTILITY_HPPS}
  ${ONSEMGUIUTILITY_SRCS}
)

target_link_libraries(onsemguiutility
  Qt5::Core
  Qt5::Widgets
)



