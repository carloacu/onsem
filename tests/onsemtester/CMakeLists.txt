project(onsemtester)
cmake_minimum_required(VERSION 2.8)



# Making a lib
set(ONSEMTESTER_HPPS
  include/onsem/tester/api.hpp
  include/onsem/tester/detailedreactionanswer.hpp
  include/onsem/tester/memorybinarization.hpp
  include/onsem/tester/reactOnTexts.hpp
  include/onsem/tester/resourcelabelfortests.hpp
  include/onsem/tester/scenariocontainer.hpp
  include/onsem/tester/sentencesloader.hpp
  include/onsem/tester/syntacticanalysisxmlloader.hpp
  include/onsem/tester/syntacticanalysisxmlsaver.hpp
)

set(ONSEMTESTER_SRCS
  src/detailedreactionanswer.cpp
  src/memorybinarization.cpp
  src/reactOnTexts.cpp
  src/scenariocontainer.cpp
  src/sentencesloader.cpp
  src/syntacticanalysisxmlloader.cpp
  src/syntacticanalysisxmlsaver.cpp
)

include_directories(include)

find_package(Boost REQUIRED)

if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()
if(NOT ONSEMTEXTTOSEMANTIC_FOUND)
  find_package(onsemtexttosemantic)
endif()
if(NOT ONSEMSEMANTICTOTEXT_FOUND)
  find_package(onsemsemantictotext)
endif()
if(NOT ONSEMSEMANTOCDEBUGGER_FOUND)
  find_package(onsemsemanticdebugger)
endif()

add_library(onsemtester
  ${ONSEMTESTER_HPPS}
  ${ONSEMTESTER_SRCS}
)

# Add position independent code flag if building shared libs or if explicitly requested
if(BUILD_SHARED_LIBS OR POSITION_INDEPENDENT_CODE)
    set_target_properties(onsemtester PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
  ${ONSEMSEMANTICTOTEXT_INCLUDE_DIRS}
  ${ONSEMSEMANTOCDEBUGGER_INCLUDE_DIRS}
)

target_link_libraries(onsemtester
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
  ${ONSEMSEMANTICTOTEXT_LIBRARIES}
  ${ONSEMSEMANTOCDEBUGGER_LIBRARIES}
)

