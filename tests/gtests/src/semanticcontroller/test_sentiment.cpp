#include <gtest/gtest.h>
#include <onsem/texttosemantic/printer/expressionprinter.hpp>
#include <onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/sentiment/sentimentanalyzer.hpp>
#include "../util/util.hpp"
#include "../semanticreasonergtests.hpp"


using namespace onsem;


TEST_F(SemanticReasonerGTests, sentimentWithoutContext)
{
  UniqueSemanticExpression meSemExp = mystd::make_unique<GroundedExpression>
      (SemanticAgentGrounding::getRobotAgentPtr());
  UniqueSemanticExpression userSemExp = mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticAgentGrounding>(SemanticAgentGrounding::currentUser));
  EXPECT_NE(*meSemExp, *userSemExp);

  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("I like you", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(2, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*meSemExp, *firstSentContext.receiver);
    EXPECT_NE(*userSemExp, *firstSentContext.receiver);
  }

  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("Je t'aime", lingDb, SemanticLanguageEnum::FRENCH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(3, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*meSemExp, *firstSentContext.receiver);
    EXPECT_NE(*userSemExp, *firstSentContext.receiver);
  }

  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("You are cool", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_joy", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*meSemExp, *firstSentContext.receiver);
    EXPECT_NE(*userSemExp, *firstSentContext.receiver);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("You seem cool", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_joy", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*meSemExp, *firstSentContext.receiver);
    EXPECT_NE(*userSemExp, *firstSentContext.receiver);
  }

  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("tu es cool", lingDb, SemanticLanguageEnum::FRENCH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_joy", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*meSemExp, *firstSentContext.receiver);
    EXPECT_NE(*userSemExp, *firstSentContext.receiver);
  }

  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("j'aime le chocolat", lingDb, SemanticLanguageEnum::FRENCH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(3, firstSentContext.sentimentStrengh);
    EXPECT_NE(*meSemExp, *firstSentContext.receiver);
    EXPECT_NE(*userSemExp, *firstSentContext.receiver);
  }

  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("I am a little bit sad, actually", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_negative_sadness", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*userSemExp, *firstSentContext.receiver);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("ok but you look happy", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*meSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_happiness", firstSentContext.sentiment);
    EXPECT_EQ(5, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*meSemExp, *firstSentContext.receiver);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("Aaa likes chocolate.", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    std::string authorStr;
    printer::oneWordPrint(authorStr, firstSentContext.author, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("human", authorStr);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(2, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("thing", receiverStr);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("j'aime mon frère", lingDb, SemanticLanguageEnum::FRENCH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(3, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("human", receiverStr);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("super", lingDb, SemanticLanguageEnum::FRENCH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_joy", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("context", receiverStr);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("merci", lingDb, SemanticLanguageEnum::FRENCH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_thanks", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("context", receiverStr);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("cool", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_joy", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("context", receiverStr);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("not cool", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_negative_*", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("context", receiverStr);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentsContext;
    converter::semExpToSentiments
        (sentsContext,
         *textToContextualSemExp("thanks", lingDb, SemanticLanguageEnum::ENGLISH),
         lingDb.conceptSet);
    ASSERT_EQ(1u, sentsContext.size());
    auto& firstSentContext = *sentsContext.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_thanks", firstSentContext.sentiment);
    EXPECT_EQ(4, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("context", receiverStr);
  }

}



TEST_F(SemanticReasonerGTests, sentimentWithContext)
{
  UniqueSemanticExpression meSemExp = mystd::make_unique<GroundedExpression>
      (SemanticAgentGrounding::getRobotAgentPtr());
  SemanticAgentGrounding user(SemanticAgentGrounding::currentUser);
  UniqueSemanticExpression userSemExp = mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticAgentGrounding>(user));
  EXPECT_NE(*meSemExp, *userSemExp);


  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SentimentAnalyzer sentAnalyer(lingDb);

  {
    std::list<std::unique_ptr<SentimentContext>> sentSpecs;
    sentAnalyer.extract(sentSpecs,
                        textToContextualSemExp("yes", lingDb, SemanticLanguageEnum::ENGLISH),
                        user);
    EXPECT_EQ(0u, sentSpecs.size());
  }

  sentAnalyer.inform(textToSemExpFromRobot("do you like me", lingDb, SemanticLanguageEnum::ENGLISH));

  {
    std::list<std::unique_ptr<SentimentContext>> sentSpecs;
    sentAnalyer.extract(sentSpecs,
                        textToContextualSemExp("yes", lingDb, SemanticLanguageEnum::ENGLISH),
                        user);
    ASSERT_EQ(1u, sentSpecs.size());
    auto& firstSentContext = *sentSpecs.front();
    EXPECT_EQ(*userSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(2, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*meSemExp, *firstSentContext.receiver);
  }


  {
    std::list<std::unique_ptr<SentimentContext>> sentSpecs;
    sentAnalyer.extract(sentSpecs,
                        textToSemExpFromRobot("Thanks, I like you too.", lingDb, SemanticLanguageEnum::ENGLISH),
                        user);
    ASSERT_EQ(1u, sentSpecs.size());
    auto& firstSentContext = *sentSpecs.front();
    EXPECT_EQ(*meSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(2, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*userSemExp, *firstSentContext.receiver);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentSpecs;
    sentAnalyer.extract(sentSpecs,
                        textToSemExpFromRobot("Merci, je t'aime aussi.", lingDb, SemanticLanguageEnum::FRENCH),
                        user);
    ASSERT_EQ(1u, sentSpecs.size());
    auto& firstSentContext = *sentSpecs.front();
    EXPECT_EQ(*meSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(3, firstSentContext.sentimentStrengh);
    EXPECT_EQ(*userSemExp, *firstSentContext.receiver);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentSpecs;
    sentAnalyer.extract(sentSpecs,
                        textToSemExpFromRobot("J'aime Paul aussi.", lingDb, SemanticLanguageEnum::FRENCH),
                        user);
    ASSERT_EQ(1u, sentSpecs.size());
    auto& firstSentContext = *sentSpecs.front();
    EXPECT_EQ(*meSemExp, *firstSentContext.author);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(3, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("human", receiverStr);
  }
  {
    std::list<std::unique_ptr<SentimentContext>> sentSpecs;
    sentAnalyer.extract(sentSpecs,
                        textToSemExpFromRobot("Oui, Toto aime Paul.", lingDb, SemanticLanguageEnum::FRENCH),
                        user);
    ASSERT_EQ(1u, sentSpecs.size());
    auto& firstSentContext = *sentSpecs.front();
    std::string authorStr;
    printer::oneWordPrint(authorStr, firstSentContext.author, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("human", authorStr);
    EXPECT_EQ("sentiment_positive_*", firstSentContext.sentiment);
    EXPECT_EQ(3, firstSentContext.sentimentStrengh);
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, firstSentContext.receiver, SemanticAgentGrounding::currentUser);
    EXPECT_EQ("human", receiverStr);
  }
}
