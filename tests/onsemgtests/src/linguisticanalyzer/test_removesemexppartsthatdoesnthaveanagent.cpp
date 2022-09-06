#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;


namespace
{
std::string _removesemexppartsthatdoesnthaveanagent(
    const std::string& pText,
    const std::string& pReferenceText,
    SemanticMemory& pSemMem,
    SemanticLanguageEnum pLanguage,
    const linguistics::LinguisticDatabase& pLingDb)
{
  TextProcessingContext textProc(SemanticAgentGrounding::currentUser,
                                 SemanticAgentGrounding::me,
                                 pLanguage);
  auto agentWeAreTalkingAbout = SemanticMemoryBlock::generateNewAgentGrd(pReferenceText, pLanguage, pLingDb);
  auto semExp = converter::textToSemExp(pText, textProc, pLingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemMem, pLingDb);
  SemExpModifier::removeSemExpPartsThatDoesntHaveAnAgent(semExp, *agentWeAreTalkingAbout);
  return semExpToText(std::move(semExp), pLanguage, pSemMem, pLingDb);
}
}

TEST_F(SemanticReasonerGTests, removesemexppartsthatdoesnthaveanagent)
{
  const auto& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  const auto frLanguage = SemanticLanguageEnum::FRENCH;

  EXPECT_EQ("", _removesemexppartsthatdoesnthaveanagent
            ("Toto est content.",
             "Paul Ftrer", semMem, frLanguage, lingDb));
  EXPECT_EQ("Paul Ftrer est dans la cuisine.", _removesemexppartsthatdoesnthaveanagent
            ("Paul Ftrer est dans la cuisine. Toto est content.",
             "Paul Ftrer", semMem, frLanguage, lingDb));
  EXPECT_EQ("Paul Ftrer est dans la cuisine.", _removesemexppartsthatdoesnthaveanagent
            ("Paul Ftrer est dans la cuisine. Toto est content.",
             "Paul Ftrer (cuisinier)", semMem, frLanguage, lingDb));
}
