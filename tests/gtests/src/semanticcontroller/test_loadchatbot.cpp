#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/tester/loadchatbot.hpp>

using namespace onsem;



TEST_F(SemanticReasonerGTests, test_loadchatbotDomain)
{
  std::stringstream ss;
  ss << "{                                                         \n";
  ss << "  \"language\" : \"fr\",                                  \n";
  ss << "  \"inform\" : [                                          \n";
  ss << "    \"Paul aime le chocolat\"                             \n";
  ss << "  ],                                                      \n";
  ss << "  \"actions\": [                                          \n";
  ss << "    {                                                     \n";
  ss << "      \"text\": \"un texte\",                             \n";
  ss << "      \"shouldBeDoneAsapWithoutHistoryCheck\": true,      \n";
  ss << "      \"parameters\": [                                   \n";
  ss << "        {                                                 \n";
  ss << "          \"text\": \"oui\"                               \n";
  ss << "        }                                                 \n";
  ss << "      ],                                                  \n";
  ss << "      \"input\": {                                        \n";
  ss << "        \"fact\": \"a-fact\"                              \n";
  ss << "      }                                                   \n";
  ss << "    },                                                    \n";
  ss << "    {                                                     \n";
  ss << "      \"text\": \"un autre texte\"                        \n";
  ss << "    }                                                     \n";
  ss << "  ]                                                       \n";
  ss << "}                                                         \n";

  ChatbotDomain chatbotDomain;
  loadChatbotDomain(chatbotDomain, ss);
  ASSERT_EQ(2, chatbotDomain.actions.size());
  auto itAction = chatbotDomain.actions.begin();
  auto& firstActionWithId = *itAction;
  EXPECT_EQ("0", firstActionWithId.first);
  auto& firstAction = firstActionWithId.second;
  EXPECT_EQ("un texte", firstAction.text);
  EXPECT_TRUE(firstAction.shouldBeDoneAsapWithoutHistoryCheck);
  ASSERT_EQ(1, firstAction.parameters.size());
  auto& firstActionfirstParam = firstAction.parameters[0];
  EXPECT_EQ("oui", firstActionfirstParam.text);
  ASSERT_TRUE(firstAction.inputPtr.operator bool());
  EXPECT_EQ(cp::Fact("a-fact"), firstAction.inputPtr->fact);
  ++itAction;
  auto& secondActionWithId = *itAction;
  auto& secondAction = secondActionWithId.second;
  EXPECT_EQ("un autre texte", secondAction.text);
  EXPECT_FALSE(secondAction.shouldBeDoneAsapWithoutHistoryCheck);
}



TEST_F(SemanticReasonerGTests, test_loadchatbotProblem)
{
  std::stringstream ss;
  ss << "{                                           \n";
  ss << "  \"language\" : \"fr\",                    \n";
  ss << "  \"goals\" : [                             \n";
  ss << "    \"remonter-le-moral\"                   \n";
  ss << "  ]                                         \n";
  ss << "}                                           \n";

  ChatbotProblem chatbotProblem;
  loadChatbotProblem(chatbotProblem, ss);
  EXPECT_EQ(SemanticLanguageEnum::FRENCH, chatbotProblem.language);
  ASSERT_EQ(1, chatbotProblem.problem.goals().size());
  EXPECT_EQ(cp::Goal("remonter-le-moral"), chatbotProblem.problem.goals()[0]);
}

