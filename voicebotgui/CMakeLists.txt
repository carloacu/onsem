project(voicebotgui)
cmake_minimum_required(VERSION 2.8)

# Avoid compile failed between boost "signals" and Qt "signals"
add_definitions(-DQT_NO_KEYWORDS)

add_subdirectory(guiutility)


# Define Source files
set(${PROJECT_NAME}_SRC
  src/qobject/scrollpanel.cpp
  src/mainwindow.cpp
  src/main.cpp
  )

# Define ui file
set(${PROJECT_NAME}_UI
  src/mainwindow.ui
  )


# Define Moc Header files
set(${PROJECT_NAME}_MOC
  src/qobject/scrollpanel.h
  src/mainwindow.h
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})


set(ONSEMGUIUTILITY_FOUND TRUE)
set(ONSEMGUIUTILITY_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/guiutility/include")
set(ONSEMGUIUTILITY_LIBRARIES "onsemguiutility")

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# This is called *before* any call to qi_use_lib() is made.
# Here we need to define the qt macros, such as qt5_wrap_cpp,
# qt5_add_resources, qt5_wrap_ui
set(CMAKE_AUTOMOC ON)


qt5_wrap_ui(${PROJECT_NAME}_UI_SRC ${${PROJECT_NAME}_UI})


add_executable(${PROJECT_NAME}
  ${${PROJECT_NAME}_SRC}
  ${${PROJECT_NAME}_UI_SRC}
)

if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()
if(NOT ONSEMTEXTTOSEMANTIC_FOUND)
  find_package(onsemtexttosemantic)
endif()
if(NOT ONSEMSEMANTICTOTEXT_FOUND)
  find_package(onsemsemantictotext)
endif()
if(NOT ONSEMSTREADATABASEACCESSOR_FOUND)
  find_package(onsemstreamdatabaseaccessor)
endif()
if(NOT ONSEMSEMANTOCDEBUGGER_FOUND)
  find_package(onsemsemanticdebugger)
endif()
if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(contextualplanner)
endif()
if(NOT ONSEMTESTER_FOUND)
  find_package(onsemtester)
endif()

include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
  ${ONSEMSEMANTICTOTEXT_INCLUDE_DIRS}
  ${ONSEMSTREADATABASEACCESSOR_INCLUDE_DIRS}
  ${ONSEMSEMANTOCDEBUGGER_INCLUDE_DIRS}
  ${ONSEMTESTER_INCLUDE_DIRS}
  ${CONTEXTUALPLANNER_INCLUDE_DIRS}
  ${ONSEMGUIUTILITY_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core Qt5::Widgets
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
  ${ONSEMSEMANTICTOTEXT_LIBRARIES}
  ${ONSEMSTREADATABASEACCESSOR_LIBRARIES}
  ${ONSEMSEMANTOCDEBUGGER_LIBRARIES}
  ${ONSEMTESTER_LIBRARIES}
  ${CONTEXTUALPLANNER_LIBRARIES}
  ${ONSEMGUIUTILITY_LIBRARIES}
)

