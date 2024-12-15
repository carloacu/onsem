project(texttosemantic)
cmake_minimum_required(VERSION 2.8)

set(ONSEMTEXTTOSEMANTIC_HPPS
    include/onsem/texttosemantic/api.hpp
    include/onsem/texttosemantic/dbtype/inflection/inflectiontype.hpp
    include/onsem/texttosemantic/dbtype/inflection/inflections.hpp
    include/onsem/texttosemantic/dbtype/inflection/adjectivalinflections.hpp
    include/onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp
    include/onsem/texttosemantic/dbtype/inflection/pronominalinflections.hpp
    include/onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp
    include/onsem/texttosemantic/dbtype/inflection/detail/adjectivalinflections.hxx
    include/onsem/texttosemantic/dbtype/inflection/detail/nominalinflections.hxx
    include/onsem/texttosemantic/dbtype/inflection/detail/pronominalinflections.hxx
    include/onsem/texttosemantic/dbtype/inflection/detail/verbalinflections.hxx
    include/onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp
    include/onsem/texttosemantic/dbtype/linguistic/wordtosynthesize.hpp
    include/onsem/texttosemantic/dbtype/linguistic/detail/lingtypetoken.hxx
    include/onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/treeconverter.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/synthesizerdictionary.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/statictranslationdictionary.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/translationdictionary.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/binarydatabasessizeprinter.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/detail/semtreeconversiondatabase.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/detail/staticsynthesizerdictionary.hxx
    include/onsem/texttosemantic/dbtype/linguisticdatabase/detail/staticlinguisticdictionary.hxx
    include/onsem/texttosemantic/dbtype/linguisticdatabase/detail/metawordtreedb.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/detail/virtualsembinarydatabase.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase/detail/metawordtreedb.hxx
    include/onsem/texttosemantic/dbtype/semanticexpression/details/semanticexpression.hxx
    include/onsem/texttosemantic/dbtype/semanticexpression/details/groundedexpression.hxx
    include/onsem/texttosemantic/dbtype/semanticexpression/annotatedexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/commandexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/feedbackexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/fixedsynthesisexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/comparisonexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/interpretationexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticexpression/setofformsexpression.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticanglegrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticdurationgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticconceptualgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticlengthgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticpercentagegrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticrelativedurationgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticrelativelocationgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticrelativetimegrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp
    include/onsem/texttosemantic/dbtype/semanticgrounding/semanticunitygrounding.hpp
    include/onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp
    include/onsem/texttosemantic/dbtype/detail/inflectedword.hxx
    include/onsem/texttosemantic/dbtype/misc/imbricationType.hpp
    include/onsem/texttosemantic/dbtype/misc/truenessvalue.hpp
    include/onsem/texttosemantic/dbtype/misc/parameterswithvalue.hpp
    include/onsem/texttosemantic/dbtype/misc/conditionspecification.hpp
    include/onsem/texttosemantic/dbtype/misc/coreference.hpp
    include/onsem/texttosemantic/dbtype/misc/sign.hpp
    include/onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp
    include/onsem/texttosemantic/dbtype/misc/typeofunity.hpp
    include/onsem/texttosemantic/dbtype/binary/semexploader.hpp
    include/onsem/texttosemantic/dbtype/binary/semexpsaver.hpp
    include/onsem/texttosemantic/dbtype/resourcegroundingextractor.hpp
    include/onsem/texttosemantic/dbtype/inflections.hpp
    include/onsem/texttosemantic/dbtype/interactioncontext.hpp
    include/onsem/texttosemantic/dbtype/linguisticdatabase.hpp
    include/onsem/texttosemantic/dbtype/inflectedword.hpp
    include/onsem/texttosemantic/dbtype/linguisticmeaning.hpp
    include/onsem/texttosemantic/dbtype/staticlinguisticmeaning.hpp
    include/onsem/texttosemantic/dbtype/semanticexpressions.hpp
    include/onsem/texttosemantic/dbtype/semanticgroundings.hpp
    include/onsem/texttosemantic/dbtype/semanticquantity.hpp
    include/onsem/texttosemantic/dbtype/semanticword.hpp
    include/onsem/texttosemantic/dbtype/linguisticanalysisconfig.hpp
    include/onsem/texttosemantic/dbtype/textprocessingcontext.hpp
    include/onsem/texttosemantic/type/enum/linguisticanalysisfinishdebugstepenum.hpp
    include/onsem/texttosemantic/type/enum/lingverbform.hpp
    include/onsem/texttosemantic/type/enum/chunktype.hpp
    include/onsem/texttosemantic/type/debug/synthanalendingstepfordebug.hpp
    include/onsem/texttosemantic/type/chunk.hpp
    include/onsem/texttosemantic/type/chunklink.hpp
    include/onsem/texttosemantic/type/enumsconvertions.hpp
    include/onsem/texttosemantic/type/syntacticgraph.hpp
    include/onsem/texttosemantic/tool/iscomplete.hpp
    include/onsem/texttosemantic/tool/semexpmodifier.hpp
    include/onsem/texttosemantic/tool/semexpgetter.hpp
    include/onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp
    include/onsem/texttosemantic/tool/inflectionschecker.hpp
    include/onsem/texttosemantic/tool/partofspeech/partofspeechcontextfilter.hpp
    include/onsem/texttosemantic/tool/partofspeech/partofspeechcustomfilter.hpp
    include/onsem/texttosemantic/printer/expressionprinter.hpp
    include/onsem/texttosemantic/printer/semlinetoprint.hpp
    include/onsem/texttosemantic/languagedetector.hpp
    include/onsem/texttosemantic/linguisticanalyzer.hpp
    include/onsem/texttosemantic/algorithmsetforalanguage.hpp
)

set(ONSEMTEXTTOSEMANTIC_SRCS
    src/dbtype/misc/conditionspecification.cpp
    src/dbtype/inflection/inflections.cpp
    src/dbtype/inflection/nominalinflections.cpp
    src/dbtype/linguisticdatabase/detail/metawordtreedb.cpp
    src/dbtype/linguisticdatabase/detail/virtualsembinarydatabase.cpp
    src/dbtype/linguisticdatabase/childspecification.hpp
    src/dbtype/linguisticdatabase/childspecification.cpp
    src/dbtype/linguisticdatabase/partofspeechrules_english.hpp
    src/dbtype/linguisticdatabase/partofspeechrules_english.cpp
    src/dbtype/linguisticdatabase/partofspeechrules_french.hpp
    src/dbtype/linguisticdatabase/partofspeechrules_french.cpp
    src/dbtype/linguisticdatabase/conceptset.cpp
    src/dbtype/linguisticdatabase/semanticframedictionary.cpp
    src/dbtype/linguisticdatabase/treeconverter.cpp
    src/dbtype/linguisticdatabase/linguisticdictionary.cpp
    src/dbtype/linguisticdatabase/staticconceptset.cpp
    src/dbtype/linguisticdatabase/staticlinguisticdictionary.cpp
    src/dbtype/linguisticdatabase/staticsynthesizerdictionary.cpp
    src/dbtype/linguisticdatabase/statictranslationdictionary.cpp
    src/dbtype/linguisticdatabase/statictreeconverter.hpp
    src/dbtype/linguisticdatabase/statictreeconverter.cpp
    src/dbtype/linguisticdatabase/synthesizerdictionary.cpp
    src/dbtype/linguisticdatabase/translationdictionary.cpp
    src/dbtype/linguisticdatabase/binarydatabasessizeprinter.cpp
    src/dbtype/linguisticdatabase/semtreeconversiondatabase.cpp
    src/dbtype/semanticexpression/semanticexpression.cpp
    src/dbtype/semanticexpression/annotatedexpression.cpp
    src/dbtype/semanticexpression/commandexpression.cpp
    src/dbtype/semanticexpression/comparisonexpression.cpp
    src/dbtype/semanticexpression/conditionexpression.cpp
    src/dbtype/semanticexpression/feedbackexpression.cpp
    src/dbtype/semanticexpression/fixedsynthesisexpression.cpp
    src/dbtype/semanticexpression/groundedexpression.cpp
    src/dbtype/semanticexpression/listexpression.cpp
    src/dbtype/semanticexpression/interpretationexpression.cpp
    src/dbtype/semanticexpression/metadataexpression.cpp
    src/dbtype/semanticexpression/setofformsexpression.cpp
    src/dbtype/semanticgrounding/semanticgrounding.cpp
    src/dbtype/semanticgrounding/semanticanglegrounding.cpp
    src/dbtype/semanticgrounding/semantictimegrounding.cpp
    src/dbtype/semanticgrounding/semanticdurationgrounding.cpp
    src/dbtype/semanticgrounding/semanticgenericgrounding.cpp
    src/dbtype/semanticgrounding/semanticagentgrounding.cpp
    src/dbtype/semanticgrounding/semanticlengthgrounding.cpp
    src/dbtype/semanticgrounding/semanticmetagrounding.cpp
    src/dbtype/semanticgrounding/semanticnamegrounding.cpp
    src/dbtype/semanticgrounding/semanticresourcegrounding.cpp
    src/dbtype/semanticgrounding/semantictextgrounding.cpp
    src/dbtype/semanticgrounding/semanticunitygrounding.cpp
    src/dbtype/binary/semexploader.cpp
    src/dbtype/binary/semexpsaver.cpp
    src/dbtype/sentiment/sentimentcontext.cpp
    src/dbtype/interactioncontext.cpp
    src/dbtype/resourcegroundingextractor.cpp
    src/dbtype/linguisticdatabase.cpp
    src/dbtype/semanticquantity.cpp
    src/dbtype/textprocessingcontext.cpp
    src/printer/expressionprinter.cpp
    src/type/chunklink.cpp
    src/type/chunk.cpp
    src/type/enumsconvertions.cpp
    src/type/syntacticgraph.cpp
    src/tool/listiter.hpp
    src/tool/chunkshandler.hpp
    src/tool/chunkshandler.cpp
    src/tool/iscomplete.cpp
    src/tool/isaurl.hpp
    src/tool/isaurl.cpp
    src/tool/semexpmodifier.cpp
    src/tool/semexpgetter.cpp
    src/tool/semexpgenerator.hpp
    src/tool/semexpgenerator.cpp
    src/tool/syntacticanalyzertokenshandler.cpp
    src/tool/inflectionschecker.cpp
    src/tool/inflection/inflectionscheckervirtual.hpp
    src/tool/inflection/inflectionscheckervirtual.cpp
    src/tool/inflection/inflectionscheckerenglish.hpp
    src/tool/inflection/inflectionscheckerenglish.cpp
    src/tool/inflection/inflectionscheckerfrench.hpp
    src/tool/inflection/inflectionscheckerfrench.cpp
    src/tool/inflection/inflectionsfiltrer.cpp
    src/tool/partofspeech/type/taggertypes.hpp
    src/tool/partofspeech/type/taggertypes.cpp
    src/tool/partofspeech/partofspeechdelbigramimpossibilities.hpp
    src/tool/partofspeech/partofspeechdelbigramimpossibilities.cpp
    src/tool/partofspeech/partofspeechcustomfilter.cpp
    src/tool/partofspeech/partofspeechpatternmatcher.hpp
    src/tool/partofspeech/partofspeechpatternmatcher.cpp
    src/tokenizer/partofspeechfilterer.hpp
    src/tokenizer/partofspeechfilterer.cpp
    src/tokenizer/tokenizer.hpp
    src/tokenizer/tokenizer.cpp
    src/partofspeechpriorities/englishthendisambiguator.hpp
    src/partofspeechpriorities/englishthendisambiguator.cpp
    src/partofspeechpriorities/nounadjpriorities.hpp
    src/partofspeechpriorities/nounadjpriorities.cpp
    src/syntacticgraphgenerator/questionwords.hpp
    src/syntacticgraphgenerator/questionwords.cpp
    src/syntacticgraphgenerator/listextractor.hpp
    src/syntacticgraphgenerator/listextractor.cpp
    src/syntacticgraphgenerator/subordinateextractor.hpp
    src/syntacticgraphgenerator/chunkslinker.hpp
    src/syntacticgraphgenerator/chunkslinker.cpp
    src/syntacticgraphgenerator/entityrecognizer.hpp
    src/syntacticgraphgenerator/errordetector.hpp
    src/syntacticgraphgenerator/interjectinalchunker.hpp
    src/syntacticgraphgenerator/interjectinalchunker.cpp
    src/syntacticgraphgenerator/interjectionsadder.hpp
    src/syntacticgraphgenerator/interjectionsadder.cpp
    src/syntacticgraphgenerator/nominalchunker.hpp
    src/syntacticgraphgenerator/verbalchunker.hpp
    src/syntacticgraphgenerator/subordinateextractor.cpp
    src/syntacticgraphgenerator/entityrecognizer.cpp
    src/syntacticgraphgenerator/extractsubchunkinsidelinkedtokens.hpp
    src/syntacticgraphgenerator/extractsubchunkinsidelinkedtokens.cpp
    src/syntacticgraphgenerator/errordetector.cpp
    src/syntacticgraphgenerator/nominalchunker.cpp
    src/syntacticgraphgenerator/incompletelistsresolver.hpp
    src/syntacticgraphgenerator/incompletelistsresolver.cpp
    src/syntacticgraphgenerator/verbalchunker.cpp
    src/syntacticgraphgenerator/verbalchunker.cpp
    src/syntacticgraphgenerator/verbaltomoninalchunkslinker.hpp
    src/syntacticgraphgenerator/verbaltomoninalchunkslinker.cpp
    src/syntacticgraphgenerator/notunderstoodadder.hpp
    src/syntacticgraphgenerator/notunderstoodadder.cpp
    src/syntacticgraphgenerator/teachingparser.hpp
    src/syntacticgraphgenerator/teachingparser.cpp
    src/tosemantic/perlanguage/tosemanticen.hpp
    src/tosemantic/perlanguage/tosemanticen.cpp
    src/tosemantic/perlanguage/tosemanticfr.hpp
    src/tosemantic/perlanguage/tosemanticfr.cpp
    src/tosemantic/syntacticgraphtosemantic.hpp
    src/tosemantic/syntacticgraphtosemantic.cpp
    src/tosemantic/syntacticgraphtosemantichourtimehandler.cpp
    src/tosemantic/syntacticgraphtosemanticlengthhandler.cpp
    src/tosemantic/syntacticgraphtosemanticlocationhandler.cpp
    src/tosemantic/syntacticgraphtosemanticpercentagehandler.cpp
    src/tosemantic/syntacticgraphtosemantictimehandler.cpp
    src/tosemantic/semtreehardcodedconverter.hpp
    src/tosemantic/semtreehardcodedconverter.cpp
    src/algorithmsetforalanguage.cpp
    src/linguisticanalyzer.cpp
    src/languagedetector.cpp
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


if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()
add_library(onsemtexttosemantic
  ${ONSEMTEXTTOSEMANTIC_HPPS}
  ${ONSEMTEXTTOSEMANTIC_SRCS}
)

# Add position independent code flag if building shared libs or if explicitly requested
if(BUILD_SHARED_LIBS OR POSITION_INDEPENDENT_CODE)
    set_target_properties(onsemtexttosemantic PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
)
target_link_libraries(onsemtexttosemantic
  ${ONSEMCOMMON_LIBRARIES}
)

target_include_directories(onsemtexttosemantic PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Create a proper installed package that can be included.
install(TARGETS onsemtexttosemantic
    EXPORT onsemtexttosemanticTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/onsem DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT onsemtexttosemanticTargets
    FILE onsemtexttosemanticTargets.cmake
    NAMESPACE onsem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/onsem
)
