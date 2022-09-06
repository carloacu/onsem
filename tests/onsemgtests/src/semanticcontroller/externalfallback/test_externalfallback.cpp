#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/tester/reactOnTexts.hpp>
#include "tabletfallback.hpp"


using namespace onsem;


TEST_F(SemanticReasonerGTests, test_externalFallback)
{
  const auto& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string questionStr = "why are you happy?";
  ONSEM_ANSWERNOTFOUND_EQ("I don't know why I am happy.",
                          operator_react(questionStr, semMem, lingDb));
  semMem.registerExternalFallback(mystd::make_unique<TabletFallback>());
  ONSEM_ANSWERNOTFOUND_EQ("I don't know why I am happy. You can use my tablet in order to find the answer.",
                          operator_react(questionStr, semMem, lingDb));
}


