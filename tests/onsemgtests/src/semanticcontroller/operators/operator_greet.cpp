#include <gtest/gtest.h>
#include "operator_inform.hpp"
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/greet.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/common/utility/noresult.hpp>
#include "../../semanticreasonergtests.hpp"

using namespace onsem;

namespace
{

std::string operator_greet(const std::string& pText,
                           SemanticMemory& pSemanticMemory,
                           const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, pLingDb);
  TextProcessingContext inTextProc(SemanticAgentGrounding::currentUser,
                                   SemanticAgentGrounding::me,
                                   language);
  auto semExp = converter::textToContextualSemExp(pText, inTextProc,
                                                  SemanticSourceEnum::ASR,
                                                  pLingDb, nullptr);
  memoryOperation::mergeWithContext(semExp, pSemanticMemory, pLingDb);
  auto outSemExp = greetInResponseTo(*semExp, pSemanticMemory, pLingDb);
  if (!outSemExp)
    return constant::noResult;
  std::string res;
  converter::semExpToText(res,
                          std::move(*outSemExp),
                          TextProcessingContext(SemanticAgentGrounding::me,
                                                SemanticAgentGrounding::currentUser,
                                                language),
                          false,
                          pSemanticMemory,
                          pLingDb,
                          nullptr);
  return res;
}

}



TEST_F(SemanticReasonerGTests, operator_sayFeedback_greet)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  EXPECT_EQ(constant::noResult, operator_greet("Paul eats chocolate", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_greet("look left", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_greet("what does Paul eat?", semMem, lingDb));

  operator_inform("Paul doesn't eat chocolate", semMem, lingDb);
  EXPECT_EQ("Nice to meet you Dede", operator_greet("I am Dede", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_greet("Paul eats chocolate", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_greet("You are Titi", semMem, lingDb));
  EXPECT_EQ("Enchanté Titi", operator_greet("Je m'appelle Titi", semMem, lingDb));

  EXPECT_EQ("Hello, what is your name?", operator_greet("hello", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_greet("hello N5", semMem, lingDb));
  operator_mergeAndInform("you are N5", semMem, lingDb);
  EXPECT_EQ("Hello, what is your name?", operator_greet("hello N5", semMem, lingDb));
  EXPECT_EQ("Bye", operator_greet("bye", semMem, lingDb));
  operator_mergeAndInform("my name is Jerome", semMem, lingDb);
  EXPECT_EQ("Bye Jerome", operator_greet("bye", semMem, lingDb));
}




