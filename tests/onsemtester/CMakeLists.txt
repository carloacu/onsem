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
  include/onsem/tester/loadchatbot.hpp
)

set(ONSEMTESTER_SRCS
  src/detailedreactionanswer.cpp
  src/memorybinarization.cpp
  src/reactOnTexts.cpp
  src/scenariocontainer.cpp
  src/sentencesloader.cpp
  src/syntacticanalysisxmlloader.cpp
  src/syntacticanalysisxmlsaver.cpp
  src/loadchatbot.cpp
)

include_directories(include)


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
if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(contextualplanner)
endif()

add_library(onsemtester SHARED
  ${ONSEMTESTER_HPPS}
  ${ONSEMTESTER_SRCS}
)

include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
  ${ONSEMSEMANTICTOTEXT_INCLUDE_DIRS}
  ${ONSEMSEMANTOCDEBUGGER_INCLUDE_DIRS}
  ${CONTEXTUALPLANNER_INCLUDE_DIRS}
)

target_link_libraries(onsemtester
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
  ${ONSEMSEMANTICTOTEXT_LIBRARIES}
  ${ONSEMSEMANTOCDEBUGGER_LIBRARIES}
  ${CONTEXTUALPLANNER_LIBRARIES}
)
target_compile_features(onsemtester PRIVATE cxx_std_14)

