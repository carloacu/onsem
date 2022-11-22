#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/serialization.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/tester/sentencesloader.hpp>
#include "../util/util.hpp"

using namespace onsem;


void _checkSemExp_binarization(
    const SemanticExpression& pSemExp,
    const linguistics::LinguisticDatabase& pLingDb)
{
  boost::property_tree::ptree firstPropTree;
  serialization::saveSemExp(firstPropTree, pSemExp);

  auto semExp2 = serialization::loadSemExp(firstPropTree);
  boost::property_tree::ptree secondPropTree;
  serialization::saveSemExp(secondPropTree, *semExp2);

  ASSERT_FALSE(firstPropTree.empty());
  if (pSemExp != *semExp2)
  {
    std::cerr << "exp1:\n" << printSemExp(pSemExp) << std::endl;
    std::cerr << "exp2:\n" << printSemExp(*semExp2) << std::endl;
    EXPECT_EQ(pSemExp, *semExp2);
  }
  ASSERT_EQ(firstPropTree, secondPropTree);
}


void _test_binarization(const std::string& pTextCorpusFolder,
                         const std::string& pLanguageStr,
                         const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = semanticLanguageTypeGroundingEnumFromStr(pLanguageStr);

  SentencesLoader sentencesXml;
  sentencesXml.loadFolder(pTextCorpusFolder + "/" + pLanguageStr);
  const std::vector<std::string>& sentences = sentencesXml.getSentences();
  for (const std::string& sent : sentences)
  {
    auto semExp = textToContextualSemExp(sent, pLingDb, language);
    _checkSemExp_binarization(*semExp, pLingDb);
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
  _test_binarization(_corpusInputFolder, frenchStr, lingdb);
  _test_binarization(_corpusInputFolder, englishStr, lingdb);
  _test_binarization(_corpusInputFolder, japaneseStr, lingdb);
  _test_serialization_memory(lingdb);

  // Check agent grounding serialization
  auto agentGrdExp = UniqueSemanticExpression(
        std::make_unique<GroundedExpression>(
          std::make_unique<SemanticAgentGrounding>("a", "b", std::vector<std::string>{"b"})));
  _checkSemExp_binarization(*agentGrdExp, lingdb);
}


