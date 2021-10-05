#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "../../semanticreasonergtests.hpp"

using namespace onsem;

namespace
{
SemanticEngagementValue operator_extractEngagement(const std::string& pText,
                                                   const linguistics::LinguisticDatabase& pLingDb,
                                                   SemanticLanguageEnum pLanguage)
{
  auto semExp =
      converter::textToSemExp(pText,
                              TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                    SemanticAgentGrounding::me,
                                                    pLanguage),
                              pLingDb);
  return memoryOperation::extractEngagement(*semExp);
}
}


TEST_F(SemanticReasonerGTests, operator_extractEngagement)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  // engage
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("hello", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("hi", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("hello Buddy", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("hi robot", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("hi Matthieu", lingDb, SemanticLanguageEnum::ENGLISH));

  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("bonjour", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("salut", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("hello", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("bonjour Buddy", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::ENGAGE,
            operator_extractEngagement("salut robot", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("salut Paul", lingDb, SemanticLanguageEnum::FRENCH));

  // disengage goodbye
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("goodbye", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("goodbye Buddy", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("goodbye robot", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("bye-bye", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("Bye-bye", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("bye-bye robot", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("Bye-bye robot", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("bye-bye Buddy", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("bye-bye Buddy", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("bye-bye man", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("bye", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("bye robot", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("bye Buddy", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("goodbye man", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("goodbye Dede", lingDb, SemanticLanguageEnum::ENGLISH));

  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("バイバイ　ペッパー", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("じゃぁね", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("さようなら", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("またね", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("じゃぁねバ", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("たねじゃぁねバ", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("じゃぁねさようなら", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("バイバイ", lingDb, SemanticLanguageEnum::JAPANESE));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("さよ", lingDb, SemanticLanguageEnum::JAPANESE));

  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_GOODBYE,
            operator_extractEngagement("au revoir", lingDb, SemanticLanguageEnum::FRENCH));


  // disengage needToGo
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I need to go", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I need to leave", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I must go", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I have to go", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I need to go here", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I have to go here now", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I have to go home now", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I have to go to Paris now", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("I have to leave", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("You need to go", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("I needed to go", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("You need to go home", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("You need to go to Paris", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("I need to speak", lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("You have to leave", lingDb, SemanticLanguageEnum::ENGLISH));

  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("je dois y aller", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::DISENGAGE_NEEDTOGO,
            operator_extractEngagement("je dois partir", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("tu dois partir", lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ(SemanticEngagementValue::UNKNWON,
            operator_extractEngagement("je dois manger", lingDb, SemanticLanguageEnum::FRENCH));
}
