project(semantictotext)
cmake_minimum_required(VERSION 2.8)


set(ONSEMSEMANTICTOTEXT_HPPS
    include/onsem/semantictotext/api.hpp
    include/onsem/semantictotext/enum/semanticexpressioncategory.hpp
    include/onsem/semantictotext/enum/semanticengagementvalue.hpp
    include/onsem/semantictotext/enum/semantictypeoffeedback.hpp
    include/onsem/semantictotext/tool/semexpagreementdetector.hpp
    include/onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp
    include/onsem/semantictotext/semanticmemory/links/groundedexpwithlinks.hpp
    include/onsem/semantictotext/semanticmemory/links/groundedexpwithlinksid.hpp
    include/onsem/semantictotext/semanticmemory/links/groundedexpwithlinkslist.hpp
    include/onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp
    include/onsem/semantictotext/semanticmemory/referencesgetter.hpp
    include/onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp
    include/onsem/semantictotext/semanticmemory/semanticbehaviordefinition.hpp
    include/onsem/semantictotext/semanticmemory/semanticmemory.hpp
    include/onsem/semantictotext/semanticmemory/semantictracker.hpp
    include/onsem/semantictotext/greet.hpp
    include/onsem/semantictotext/semexpoperators.hpp
    include/onsem/semantictotext/semanticconverter.hpp
    include/onsem/semantictotext/semanticintent.hpp
    include/onsem/semantictotext/recommendations.hpp
    include/onsem/semantictotext/semexpsimplifer.hpp
    include/onsem/semantictotext/serialization.hpp
    include/onsem/semantictotext/triggers.hpp
    include/onsem/semantictotext/outputter/outputtercontext.hpp
    include/onsem/semantictotext/outputter/virtualoutputter.hpp
    include/onsem/semantictotext/outputter/executiondataoutputter.hpp
    include/onsem/semantictotext/sentiment/sentimentanalyzer.hpp
    include/onsem/semantictotext/sentiment/sentimentdetector.hpp
    include/onsem/semantictotext/tool/peoplefiller.hpp
    include/onsem/semantictotext/tool/semexpcomparator.hpp
    include/onsem/semantictotext/type/naturallanguageexpression.hpp
    include/onsem/semantictotext/type/reactionoptions.hpp
)

set(ONSEMSEMANTICTOTEXT_SRCS
    src/type/enum/semanticoperatorenum.hpp
    src/type/answerexp.hpp
    src/type/answerexp.cpp
    src/type/referencesfiller.hpp
    src/type/referencesfiller.cpp
    src/type/semanticdetailledanswer.hpp
    src/type/semanticdetailledanswer.cpp
    src/utility/semexpcreator.hpp
    src/utility/semexpcreator.cpp
    src/utility/utility.hpp
    src/utility/utility.cpp
    src/io/loadmodel.hpp
    src/io/loadmodel.cpp
    src/io/savemodel.hpp
    src/io/savemodel.cpp
    src/serialization.cpp
    src/controller/steps/type/semcontrollerworkingstruct.hpp
    src/controller/steps/type/semcontrollerworkingstruct.cpp
    src/controller/steps/answerfromdatastoredinsidethequestion.hpp
    src/controller/steps/answerfromdatastoredinsidethequestion.cpp
    src/controller/steps/answertospecificassertions.hpp
    src/controller/steps/answertospecificassertions.cpp
    src/controller/steps/answertospecificquestions.hpp
    src/controller/steps/answertospecificquestions.cpp
    src/controller/steps/proactivereaction.hpp
    src/controller/steps/proactivereaction.cpp
    src/controller/steps/proactivereactionfromnominalgroups.hpp
    src/controller/steps/proactivereactionfromnominalgroups.cpp
    src/controller/steps/unknowninfosgetter.hpp
    src/controller/steps/unknowninfosgetter.cpp
    src/controller/steps/howyouthatanswer.hpp
    src/controller/steps/howyouthatanswer.cpp
    src/controller/steps/managechoice.hpp
    src/controller/steps/managechoice.cpp
    src/controller/steps/managecondition.hpp
    src/controller/steps/managecondition.cpp
    src/controller/steps/semanticmemorygetter.hpp
    src/controller/steps/semanticmemorygetter.cpp
    src/controller/steps/semanticmemorylinker.hpp
    src/controller/steps/semanticmemorylinker.cpp
    src/controller/steps/similaritieswithmemoryfinder.hpp
    src/controller/steps/similaritieswithmemoryfinder.cpp
    src/controller/steps/specificactionshandler.hpp
    src/controller/steps/specificactionshandler.cpp
    src/controller/semexpcontroller.hpp
    src/controller/semexpcontroller.cpp
    src/conversion/conditionsadder.hpp
    src/conversion/conditionsadder.cpp
    src/conversion/mandatoryformconverter.hpp
    src/conversion/mandatoryformconverter.cpp
    src/conversion/occurrencerankconverter.hpp
    src/conversion/occurrencerankconverter.cpp
    src/conversion/simplesentencesplitter.hpp
    src/conversion/simplesentencesplitter.cpp
    src/conversion/reasonofrefactor.hpp
    src/conversion/reasonofrefactor.cpp
    src/outputter/executiondataoutputter.cpp
    src/outputter/virtualoutputter.cpp
    src/outputter/grdexptooutputterinformation.hpp
    src/outputter/grdexptooutputterinformation.cpp
    src/interpretation/addagentinterpretation.hpp
    src/interpretation/addagentinterpretation.cpp
    src/interpretation/completewithcontext.hpp
    src/interpretation/completewithcontext.cpp
    src/operator/answeridontknow.hpp
    src/operator/answeridontknow.cpp
    src/operator/externalteachingrequester.hpp
    src/operator/externalteachingrequester.cpp
    src/operator/greet.cpp
    src/operator/semanticcategorizer.hpp
    src/operator/semanticcategorizer.cpp
    src/tool/peoplefiller.cpp
    src/tool/semexpagreementdetector.cpp
    src/tool/semexpcomparator.cpp
    src/tool/semexpsimilaritycoef.hpp
    src/tool/semexpsimilaritycoef.cpp
    src/tool/userinfosfiller.hpp
    src/tool/userinfosfiller.cpp
    src/semanticmemory/links/expressionwithlinks.cpp
    src/semanticmemory/links/groundedexpwithlinks.cpp
    src/semanticmemory/links/groundedexpwithlinkslist.cpp
    src/semanticmemory/links/sentencewithlinks.cpp
    src/semanticmemory/answerelement.hpp
    src/semanticmemory/answerelement.cpp
    src/semanticmemory/semanticannotation.hpp
    src/semanticmemory/usernames.hpp
    src/semanticmemory/usernames.cpp
    src/semanticmemory/semanticmemorygrdexp.hpp
    src/semanticmemory/semanticmemoryblock.cpp
    src/semanticmemory/semanticmemory.cpp
    src/semanticmemory/semanticlinkstogrdexps.hpp
    src/semanticmemory/semanticlinkstogrdexps.cpp
    src/semanticmemory/memorymodifier.hpp
    src/semanticmemory/memorymodifier.cpp
    src/semanticmemory/semanticmemoryblockprivate.hpp
    src/semanticmemory/semanticmemoryblockprivate.cpp
    src/semanticmemory/semanticmemoryblockviewer.hpp
    src/semanticmemory/semanticmemoryblockviewer.cpp
    src/semanticmemory/semanticmemoryblockbinaryreader.hpp
    src/semanticmemory/semanticmemoryblockbinaryreader.cpp
    src/semanticmemory/sentenceslinks.hpp
    src/semanticmemory/sentenceslinks.cpp
    src/sentiment/sentimentdetector.cpp
    src/sentiment/sentimentanalyzer.cpp
    src/semanticintent.cpp
    src/recommendations.cpp
    src/semexpoperators.cpp
    src/semanticconverter.cpp
    src/semexpsimplifer.cpp
    src/triggers.cpp
)


set(ONSEMSEMANTICTOTEXT_LINGUISTICSYNTHESIZER_SRCS
  src/linguisticsynthesizer/conversion/tokenstostringconverter.hpp
  src/linguisticsynthesizer/conversion/tokenstostringconverter.cpp
  src/linguisticsynthesizer/grounding/linguisticsynthesizergrounding.hpp
  src/linguisticsynthesizer/grounding/linguisticsynthesizergrounding.cpp
  src/linguisticsynthesizer/grounding/linguisticsynthesizergroundingenglish.hpp
  src/linguisticsynthesizer/grounding/linguisticsynthesizergroundingenglish.cpp
  src/linguisticsynthesizer/grounding/linguisticsynthesizergroundingfrench.hpp
  src/linguisticsynthesizer/grounding/linguisticsynthesizergroundingfrench.cpp
  src/linguisticsynthesizer/merger/synthesizerchunksmerger.hpp
  src/linguisticsynthesizer/merger/synthesizerchunksmerger.cpp
  src/linguisticsynthesizer/merger/synthesizerchunksmergerenglish.hpp
  src/linguisticsynthesizer/merger/synthesizerchunksmergerenglish.cpp
  src/linguisticsynthesizer/merger/synthesizerchunksmergerfrench.hpp
  src/linguisticsynthesizer/merger/synthesizerchunksmergerfrench.cpp
  src/linguisticsynthesizer/tool/synthesizerconditions.hpp
  src/linguisticsynthesizer/tool/synthesizercontextconditions.hpp
  src/linguisticsynthesizer/tool/synthesizeradder.hpp
  src/linguisticsynthesizer/tool/synthesizergetter.hpp
  src/linguisticsynthesizer/tool/synthesizergetter.cpp
  src/linguisticsynthesizer/linguisticsynthesizer.hpp
  src/linguisticsynthesizer/linguisticsynthesizer.cpp
  src/linguisticsynthesizer/synthesizertypes.hpp
  src/linguisticsynthesizer/synthesizertypes.cpp
  src/linguisticsynthesizer/synthesizerresulttypes.hpp
  src/linguisticsynthesizer/linguisticsynthesizerprivate.hpp
  src/linguisticsynthesizer/linguisticsynthesizerprivate.cpp
  src/linguisticsynthesizer/linguisticsynthesizerenglish.hpp
  src/linguisticsynthesizer/linguisticsynthesizerenglish.cpp
  src/linguisticsynthesizer/linguisticsynthesizerfrench.hpp
  src/linguisticsynthesizer/linguisticsynthesizerfrench.cpp
)


if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()


include_directories(include)

find_package(Boost REQUIRED)

if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()

if(NOT ONSEMTEXTTOSEMANTIC_FOUND)
  find_package(onsemtexttosemantic)
endif()

add_library(onsemsemantictotext SHARED
  ${ONSEMSEMANTICTOTEXT_HPPS}
  ${ONSEMSEMANTICTOTEXT_SRCS}
  ${ONSEMSEMANTICTOTEXT_LINGUISTICSYNTHESIZER_SRCS}
)
include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
)
target_link_libraries(onsemsemantictotext
  z
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
)
target_compile_features(onsemsemantictotext PRIVATE cxx_std_17)


install(TARGETS onsemsemantictotext DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

