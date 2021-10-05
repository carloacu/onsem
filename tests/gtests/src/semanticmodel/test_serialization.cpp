#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/serialization.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/tester/sentencesloader.hpp>
#include "../util/util.hpp"

using namespace onsem;



void _test_serialization(const std::string& pTextCorpusFolder,
                         const std::string& pLanguageStr,
                         const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = semanticLanguageTypeGroundingEnumFromStr(pLanguageStr);

  SentencesLoader sentencesXml;
  sentencesXml.loadFolder(pTextCorpusFolder + "/" + pLanguageStr);
  const std::vector<std::string>& sentences = sentencesXml.getSentences();
  for (const std::string& sent : sentences)
  {
    auto semExp = textToSemExp(sent, pLingDb, language);
    boost::property_tree::ptree firstPropTree;
    serialization::saveSemExp(firstPropTree, *semExp);

    auto semExp2 = serialization::loadSemExp(firstPropTree);
    boost::property_tree::ptree secondPropTree;
    serialization::saveSemExp(secondPropTree, *semExp2);

    ASSERT_FALSE(firstPropTree.empty());
    ASSERT_EQ(firstPropTree, secondPropTree);
  }
}


void _test_serialization_memory(const linguistics::LinguisticDatabase& pLingDb)
{
  boost::property_tree::ptree propTree;
  {
    SemanticMemory semMem;
    memoryOperation::inform(textToSemExp("Paul works at Google", pLingDb), semMem, pLingDb);
    serialization::saveSemMemory(propTree, semMem);
  }

  {
    SemanticMemory semMem2;
    ONSEM_UNKNOWN(memoryOperation::check(*textToSemExp("Paul works at Google", pLingDb), semMem2.memBloc, pLingDb));
    serialization::loadSemMemory(propTree, semMem2, pLingDb);
    ONSEM_TRUE(memoryOperation::check(*textToSemExp("Paul works at Google", pLingDb), semMem2.memBloc, pLingDb));
  }
}



TEST_F(SemanticReasonerGTests, serialization)
{
  const linguistics::LinguisticDatabase& lingdb = *lingDbPtr;
  _test_serialization(_corpusInputFolder, frenchStr, lingdb);
  _test_serialization(_corpusInputFolder, englishStr, lingdb);
  _test_serialization(_corpusInputFolder, japaneseStr, lingdb);
  _test_serialization_memory(lingdb);
}


