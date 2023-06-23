project(onsemgtests)
cmake_minimum_required(VERSION 2.8)


set(ONSEMGTESTS_SRCS
  src/util/util.hpp
  src/util/util.cpp
  src/semanticreasonergtests.hpp
  src/semanticreasonergtests.cpp
  src/common/test_replacer.cpp
  src/common/test_uppercasehandler.cpp
  src/texttosemantic/test_binarization.cpp
  src/texttosemantic/test_conceptset.cpp
  src/texttosemantic/test_databasemodifications.cpp
  src/texttosemantic/test_serialization.cpp
  src/texttosemantic/test_semanticduration.cpp
  src/texttosemantic/test_semanticfloat.cpp
  src/texttosemantic/test_semanticmodel_tool.cpp
  src/texttosemantic/test_semanticlengthgrounding.cpp
  src/linguisticanalyzer/test_isComplete.cpp
  src/linguisticanalyzer/test_inflections.cpp
  src/linguisticanalyzer/test_dates.cpp
  src/linguisticanalyzer/test_durationorder.cpp
  src/linguisticanalyzer/test_durationsubstraction.cpp
  src/linguisticanalyzer/test_extractnewpropernouns.cpp
  src/linguisticanalyzer/test_removesemexppartsthatdoesnthaveanagent.cpp
  src/linguisticsynthesizer/test_synthesizer.cpp
  src/semantictotext/externalfallback/tabletfallback.hpp
  src/semantictotext/externalfallback/tabletfallback.cpp
  src/semantictotext/externalfallback/test_externalfallback.cpp
  src/semantictotext/externalinfos/dummyjokeprovider.hpp
  src/semantictotext/externalinfos/dummyjokeprovider.cpp
  src/semantictotext/externalinfos/dummycommentaryprovider.hpp
  src/semantictotext/externalinfos/dummycommentaryprovider.cpp
  src/semantictotext/externalinfos/test_externalinfosprovider.cpp
  src/semantictotext/operators/operator_answer.hpp
  src/semantictotext/operators/operator_answer.cpp
  src/semantictotext/operators/operator_answeridontknow.cpp
  src/semantictotext/operators/operator_categorize.cpp
  src/semantictotext/operators/operator_check.hpp
  src/semantictotext/operators/operator_check.cpp
  src/semantictotext/operators/operator_executeFromCondition.hpp
  src/semantictotext/operators/operator_executeFromCondition.cpp
  src/semantictotext/operators/operator_extractEngagement.cpp
  src/semantictotext/operators/operator_get.hpp
  src/semantictotext/operators/operator_get.cpp
  src/semantictotext/operators/operator_greet.cpp
  src/semantictotext/operators/operator_inform.hpp
  src/semantictotext/operators/operator_inform.cpp
  src/semantictotext/operators/operator_isasubpart.cpp
  src/semantictotext/operators/operator_mergeWithContext.cpp
  src/semantictotext/operators/operator_pingtime.cpp
  src/semantictotext/operators/operator_react.cpp
  src/semantictotext/operators/operator_resolveCommand.hpp
  src/semantictotext/operators/operator_resolveCommand.cpp
  src/semantictotext/operators/operator_sayFeedback.cpp
  src/semantictotext/operators/operator_show.hpp
  src/semantictotext/operators/operator_show.cpp
  src/semantictotext/operators/operator_solveConditions.cpp
  src/semantictotext/operators/operator_teach.cpp
  src/semantictotext/operators/operator_track.cpp
  src/semantictotext/triggers/triggers_add.hpp
  src/semantictotext/triggers/triggers_add.cpp
  src/semantictotext/triggers/triggers_match.hpp
  src/semantictotext/triggers/triggers_match.cpp
  src/semantictotext/test_agreement.cpp
  src/semantictotext/test_bigmemory.cpp
  src/semantictotext/test_confidence.cpp
  src/semantictotext/test_converter.cpp
  src/semantictotext/test_coreference.cpp
  src/semantictotext/test_equivalentSemanticRepresentation.cpp
  src/semantictotext/test_outputter.cpp
  src/semantictotext/test_imbrication.cpp
  src/semantictotext/test_intents.cpp
  src/semantictotext/test_polarity.cpp
  src/semantictotext/test_reactionoptions.cpp
  src/semantictotext/test_reactOnRoboticSentences.cpp
  src/semantictotext/test_recommendations.cpp
  src/semantictotext/test_scenarios.cpp
  src/semantictotext/test_semanticmemory.cpp
  src/semantictotext/test_naturallanguageexpressiontosemanticexpression.cpp
  src/semantictotext/test_sentiment.cpp
  src/semantictotext/test_userdisambiguation.cpp
  src/semantictotext/test_wikipedia.cpp
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
if(NOT ONSEMTESTER_FOUND)
  find_package(onsemtester)
endif()


enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(onsemgtests
  ${ONSEMGTESTS_SRCS}
)

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${BOOST_PROPERTY_TREE_WITH_DEPS_INCLUDE_DIRS}
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
  ${ONSEMSEMANTICTOTEXT_INCLUDE_DIRS}
  ${ONSEMSEMANTOCDEBUGGER_INCLUDE_DIRS}
  ${ONSEMSTREADATABASEACCESSOR_INCLUDE_DIRS}
  ${ONSEMTESTER_INCLUDE_DIRS}
)

target_link_libraries(onsemgtests
  ${GTEST_LIBRARIES}
  pthread
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
  ${ONSEMSEMANTICTOTEXT_LIBRARIES}
  ${ONSEMSEMANTOCDEBUGGER_LIBRARIES}
  ${ONSEMSTREADATABASEACCESSOR_LIBRARIES}
  ${ONSEMTESTER_LIBRARIES}
)
target_compile_features(onsemgtests PRIVATE cxx_std_17)


add_test(
  NAME onsemgtests
  COMMAND onsemgtests
)

