#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include "operator_inform.hpp"
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>


using namespace onsem;

SemanticExpressionCategory operator_categorize
(const std::string& pText,
 const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExp =
      converter::textToContextualSemExp(pText,
                                        TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                              SemanticAgentGrounding::me,
                                                              SemanticLanguageEnum::UNKNOWN),
                                        SemanticSourceEnum::WRITTENTEXT,
                                        pLingDb);
  return memoryOperation::categorize(*semExp);
}


TEST_F(SemanticReasonerGTests, operator_categorize_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  EXPECT_EQ(SemanticExpressionCategory::COMMAND, operator_categorize("say hello", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::COMMAND, operator_categorize("climb", lingDb));

  EXPECT_EQ(SemanticExpressionCategory::EXTERNALTEACHING, operator_categorize("I will teach you how to salute", lingDb));

  EXPECT_EQ(SemanticExpressionCategory::ACTIONDEFINITION, operator_categorize("to salute is to say hello", lingDb));

  EXPECT_EQ(SemanticExpressionCategory::AFFIRMATION, operator_categorize("Paul is a good guy", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::AFFIRMATION, operator_categorize("to say hello", lingDb));

  EXPECT_EQ(SemanticExpressionCategory::CONDITION, operator_categorize("if it's raining then it's a bad weather", lingDb));

  EXPECT_EQ(SemanticExpressionCategory::CONDITIONTOCOMMAND, operator_categorize("if it's raining then say that it's raining", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::CONDITIONTOCOMMAND, operator_categorize("whenever your left hand is touched sleep", lingDb));

  EXPECT_EQ(SemanticExpressionCategory::NOMINALGROUP, operator_categorize("a pen", lingDb));

  EXPECT_EQ(SemanticExpressionCategory::QUESTION, operator_categorize("is it raining", lingDb));
}


TEST_F(SemanticReasonerGTests, operator_categorize_orderNotAtImperativeForm)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  EXPECT_EQ(SemanticExpressionCategory::COMMAND, operator_categorize("you have to say hello", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::COMMAND, operator_categorize("I want you to say hello", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::AFFIRMATION, operator_categorize("I want to know how I can teach you to smile", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::AFFIRMATION, operator_categorize("You want me to say hello", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::AFFIRMATION, operator_categorize("I want to teach you something", lingDb));
  EXPECT_EQ(SemanticExpressionCategory::AFFIRMATION, operator_categorize("I want chocolate", lingDb));
}



