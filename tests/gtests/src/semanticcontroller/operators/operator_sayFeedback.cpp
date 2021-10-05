#include <gtest/gtest.h>
#include "operator_inform.hpp"
#include <onsem/common/utility/noresult.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "../../semanticreasonergtests.hpp"

using namespace onsem;

namespace
{


std::string operator_sayFeedback(const std::string& pText,
                                 SemanticTypeOfFeedback pTypeOfFeedback,
                                 SemanticMemory& pSemanticMemory,
                                 const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, pLingDb);
  auto semExp =
      converter::textToContextualSemExp(pText,
                                        TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                              SemanticAgentGrounding::me,
                                                              language),
                                        SemanticSourceEnum::UNKNOWN, pLingDb);
  memoryOperation::mergeWithContext(semExp, pSemanticMemory, pLingDb);
  auto outSemExp = memoryOperation::sayFeedback(*semExp, pTypeOfFeedback, pSemanticMemory, pLingDb);
  if (!outSemExp)
    return constant::noResult;
  std::string res;
  converter::semExpToText(res, std::move(*outSemExp),
                          TextProcessingContext(SemanticAgentGrounding::me,
                                                SemanticAgentGrounding::currentUser,
                                                language),
                          false, pSemanticMemory, pLingDb, nullptr);
  return res;
}


std::string operator_askForMoreInformation(const std::string& pText,
                                           SemanticMemory& pSemanticMemory,
                                           const linguistics::LinguisticDatabase& pLingDb)
{
  return operator_sayFeedback(pText, SemanticTypeOfFeedback::ASK_FOR_ADDITIONAL_INFORMATION, pSemanticMemory, pLingDb);
}

std::string operator_reactOnSimilarities(const std::string& pText,
                                         SemanticMemory& pSemanticMemory,
                                         const linguistics::LinguisticDatabase& pLingDb)
{
  return operator_sayFeedback(pText, SemanticTypeOfFeedback::REACT_ON_SIMILARITIES, pSemanticMemory, pLingDb);
}

std::string operator_reactOnSentiment(const std::string& pText,
                                      SemanticMemory& pSemanticMemory,
                                      const linguistics::LinguisticDatabase& pLingDb)
{
  return operator_sayFeedback(pText, SemanticTypeOfFeedback::SENTIMENT, pSemanticMemory, pLingDb);
}

}



TEST_F(SemanticReasonerGTests, operator_sayFeedback_askForMoreInformation)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  EXPECT_EQ(constant::noResult, operator_askForMoreInformation("I am 12", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_askForMoreInformation("say hello", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_askForMoreInformation("Nice", semMem, lingDb));

  EXPECT_EQ(constant::noResult, operator_askForMoreInformation("what do you like ?", semMem, lingDb));
  EXPECT_EQ("What do you like?",
            operator_askForMoreInformation("what do I like ?", semMem, lingDb));
  EXPECT_EQ("Why are you sad?",
            operator_askForMoreInformation("I am sad", semMem, lingDb));
  EXPECT_EQ("Why are you happy?",
            operator_askForMoreInformation("I am happy", semMem, lingDb));
  EXPECT_EQ("Why do you like me?",
            operator_askForMoreInformation("I like you", semMem, lingDb));
  EXPECT_EQ("When did you play football?",
            operator_askForMoreInformation("I played football", semMem, lingDb));
  EXPECT_EQ("What is Mickey Mouse?",
            operator_askForMoreInformation("do you know Mickey Mouse", semMem, lingDb));
  EXPECT_EQ("Pourquoi est-ce cool ?",
            operator_askForMoreInformation("C'est cool", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_sayFeedback_reactOnSimilarities)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("Paul eats chocolate", semMem, lingDb);
  operator_inform("Jérôme cracks a pen", semMem, lingDb);
  operator_inform("Paul liked to eat kiwi", semMem, lingDb);
  operator_inform("Michel a travaillé aux Etats Unis.", semMem, lingDb);

  EXPECT_EQ("Yes, I know Paul eats chocolate.",
            operator_reactOnSimilarities("Paul eats chocolate", semMem, lingDb));
  EXPECT_EQ("Paul also eats chocolate.",
            operator_reactOnSimilarities("André eats chocolate", semMem, lingDb));
  EXPECT_EQ("I thought the opposite.",
            operator_reactOnSimilarities("Paul doesn't eat chocolate", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSimilarities("Gustave eats", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSimilarities("I am Gustave", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSimilarities("what does Paul eat?", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSimilarities("Nice", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSimilarities("look left", semMem, lingDb));
  EXPECT_EQ("Paul also liked to eat the kiwi.", operator_reactOnSimilarities("I like to eat kiwi", semMem, lingDb));
  EXPECT_EQ("Michel a aussi travaillé aux États-Unis.", operator_reactOnSimilarities("Je travaille aux Etats Unis", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSimilarities("ça travaille", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_sayFeedback_reactOnSentiments)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("Paul eats chocolate", semMem, lingDb);

  EXPECT_EQ(constant::noResult, operator_reactOnSentiment("Gustave eats", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSentiment("I am Gustave", semMem, lingDb));
  EXPECT_EQ(constant::noResult, operator_reactOnSentiment("what does Paul eat?", semMem, lingDb));

  EXPECT_EQ("Thanks, you are kind.", operator_reactOnSentiment("I like you", semMem, lingDb));
  EXPECT_EQ("It's not kind.", operator_reactOnSentiment("I don't like you", semMem, lingDb));
  EXPECT_EQ("Nice, you like that.", operator_reactOnSentiment("Nice", semMem, lingDb));
  // TODO: Cool, tu aimes ça.
  EXPECT_EQ("Cool, tu l'aimes.", operator_reactOnSentiment("C'est cool", semMem, lingDb));
}



