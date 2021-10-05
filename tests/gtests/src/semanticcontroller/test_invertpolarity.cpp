#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;

namespace
{
std::string invertPolarity(const std::string& pText,
                           const SemanticMemory& pSemanticMemory,
                           const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExp1 = textToSemExp(pText, pLingDb);
  SemExpModifier::invertPolarity(*semExp1);
  return semExpToText(std::move(semExp1), SemanticLanguageEnum::ENGLISH,
                      pSemanticMemory, pLingDb);
}

}


TEST_F(SemanticReasonerGTests, test_invertPolarity)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semanticMemory;

  EXPECT_EQ("I am not sad.", invertPolarity("you are sad", semanticMemory, lingDb));
  EXPECT_EQ("I am sad.", invertPolarity("you are not sad", semanticMemory, lingDb));
  EXPECT_EQ("Am I not sad?", invertPolarity("are you sad", semanticMemory, lingDb));
  EXPECT_EQ("Am I sad?", invertPolarity("are you not sad", semanticMemory, lingDb));
}
