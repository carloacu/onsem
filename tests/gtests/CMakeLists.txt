project(onsem_gtests)
cmake_minimum_required(VERSION 2.8)


set(ONSEM_GTESTS_SRCS
  src/util/util.hpp
  src/util/util.cpp
  src/semanticreasonergtests.hpp
  src/semanticreasonergtests.cpp
  src/bnfparser/test_bnfparser.cpp
  src/semanticmodel/test_conceptset.cpp
  src/semanticmodel/test_databasemodifications.cpp
  src/semanticmodel/test_serialization.cpp
  src/semanticmodel/test_semanticmodel_tool.cpp
  src/linguisticanalyzer/test_isComplete.cpp
  src/linguisticanalyzer/test_inflections.cpp
  src/linguisticanalyzer/test_dates.cpp
  src/linguisticanalyzer/test_durationorder.cpp
  src/linguisticanalyzer/test_durationsubstraction.cpp
  src/linguisticanalyzer/test_extractnewpropernouns.cpp
  src/linguisticanalyzer/test_removesemexppartsthatdoesnthaveanagent.cpp
  src/linguisticsynthesizer/test_synthesizer.cpp
  src/semanticcontroller/externalfallback/tabletfallback.hpp
  src/semanticcontroller/externalfallback/tabletfallback.cpp
  src/semanticcontroller/externalfallback/test_externalfallback.cpp
  src/semanticcontroller/externalinfos/dummyjokeprovider.hpp
  src/semanticcontroller/externalinfos/dummyjokeprovider.cpp
  src/semanticcontroller/externalinfos/dummycommentaryprovider.hpp
  src/semanticcontroller/externalinfos/dummycommentaryprovider.cpp
  src/semanticcontroller/externalinfos/test_externalinfosprovider.cpp
  src/semanticcontroller/operators/operator_addATrigger.hpp
  src/semanticcontroller/operators/operator_addATrigger.cpp
  src/semanticcontroller/operators/operator_answer.hpp
  src/semanticcontroller/operators/operator_answer.cpp
  src/semanticcontroller/operators/operator_answeridontknow.cpp
  src/semanticcontroller/operators/operator_categorize.cpp
  src/semanticcontroller/operators/operator_check.hpp
  src/semanticcontroller/operators/operator_check.cpp
  src/semanticcontroller/operators/operator_executeFromTrigger.hpp
  src/semanticcontroller/operators/operator_executeFromTrigger.cpp
  src/semanticcontroller/operators/operator_extractEngagement.cpp
  src/semanticcontroller/operators/operator_find.cpp
  src/semanticcontroller/operators/operator_get.hpp
  src/semanticcontroller/operators/operator_get.cpp
  src/semanticcontroller/operators/operator_greet.cpp
  src/semanticcontroller/operators/operator_inform.hpp
  src/semanticcontroller/operators/operator_inform.cpp
  src/semanticcontroller/operators/operator_isasubpart.cpp
  src/semanticcontroller/operators/operator_mergeWithContext.cpp
  src/semanticcontroller/operators/operator_pingtime.cpp
  src/semanticcontroller/operators/operator_react.cpp
  src/semanticcontroller/operators/operator_reactFromTrigger.hpp
  src/semanticcontroller/operators/operator_reactFromTrigger.cpp
  src/semanticcontroller/operators/operator_resolveCommand.hpp
  src/semanticcontroller/operators/operator_resolveCommand.cpp
  src/semanticcontroller/operators/operator_sayFeedback.cpp
  src/semanticcontroller/operators/operator_show.hpp
  src/semanticcontroller/operators/operator_show.cpp
  src/semanticcontroller/operators/operator_solveConditions.cpp
  src/semanticcontroller/operators/operator_teach.cpp
  src/semanticcontroller/operators/operator_track.cpp
  src/semanticcontroller/test_agreement.cpp
  src/semanticcontroller/test_coreference.cpp
  src/semanticcontroller/test_equivalentSemanticRepresentation.cpp
  src/semanticcontroller/test_executor.cpp
  src/semanticcontroller/test_imbrication.cpp
  src/semanticcontroller/test_intents.cpp
  src/semanticcontroller/test_invertpolarity.cpp
  src/semanticcontroller/test_loadchatbot.cpp
  src/semanticcontroller/test_reactionoptions.cpp
  src/semanticcontroller/test_reactOnRoboticSentences.cpp
  src/semanticcontroller/test_recommendations.cpp
  src/semanticcontroller/test_scenarios.cpp
  src/semanticcontroller/test_semanticmemory.cpp
  src/semanticcontroller/test_naturallanguageexpressiontosemanticexpression.cpp
  src/semanticcontroller/test_sentiment.cpp
  src/semanticcontroller/test_userdisambiguation.cpp
  src/semanticcontroller/test_wikipedia.cpp
  src/main.cpp
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
if(NOT ONSEMSEMANTOCDEBUGGER_FOUND)
  find_package(onsemsemanticdebugger)
endif()
if(NOT ONSEMSTREADATABASEACCESSOR_FOUND)
  find_package(onsemstreamdatabaseaccessor)
endif()
if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(contextualplanner)
endif()
if(NOT ONSEMTESTER_FOUND)
  find_package(onsemtester)
endif()



enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(semanticreasoner_gtests
  ${ONSEM_GTESTS_SRCS}
)

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
  ${ONSEMSEMANTICTOTEXT_INCLUDE_DIRS}
  ${ONSEMSEMANTOCDEBUGGER_INCLUDE_DIRS}
  ${ONSEMSTREADATABASEACCESSOR_INCLUDE_DIRS}
  ${CONTEXTUALPLANNER_INCLUDE_DIRS}
  ${ONSEMTESTER_INCLUDE_DIRS}
)

target_link_libraries(semanticreasoner_gtests
  ${GTEST_LIBRARIES}
  pthread
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
  ${ONSEMSEMANTICTOTEXT_LIBRARIES}
  ${ONSEMSEMANTOCDEBUGGER_LIBRARIES}
  ${ONSEMSTREADATABASEACCESSOR_LIBRARIES}
  ${CONTEXTUALPLANNER_LIBRARIES}
  ${ONSEMTESTER_LIBRARIES}
)
target_compile_features(semanticreasoner_gtests PRIVATE cxx_std_14)


add_test(
  NAME semanticreasoner_gtests
  COMMAND semanticreasoner_gtests
)

