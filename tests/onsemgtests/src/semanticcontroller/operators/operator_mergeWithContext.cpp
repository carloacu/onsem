#include <gtest/gtest.h>
#include "operator_inform.hpp"
#include "operator_check.hpp"
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "../../semanticreasonergtests.hpp"
#include "../../util/util.hpp"
#include "operator_answer.hpp"

using namespace onsem;

namespace
{
bool _hasACoreferenceFromStr(const std::string& pText,
                             const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExp =
      converter::textToSemExp(pText,
                              TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                    SemanticAgentGrounding::me,
                                                    SemanticLanguageEnum::UNKNOWN),
                              pLingDb);
  return SemExpGetter::hasACoreference(*semExp);
}

}


TEST_F(SemanticReasonerGTests, operator_mergeWithContext_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  SemanticLanguageEnum enLanguage = SemanticLanguageEnum::ENGLISH;
  SemanticLanguageEnum frLanguage = SemanticLanguageEnum::FRENCH;

  {
    auto heIsCoolSentence = textToSemExp("he is cool", lingDb, enLanguage);
    EXPECT_EQ("He is cool.", semExpToText(heIsCoolSentence->clone(), enLanguage, semMem, lingDb));
    operator_inform("Paul likes chocolate", semMem, lingDb);
    memoryOperation::mergeWithContext(heIsCoolSentence, semMem, lingDb);
    EXPECT_EQ("Paul is cool.", semExpToText(std::move(heIsCoolSentence), enLanguage, semMem, lingDb));
  }

  {
    std::string theyHoldAMaskStr = "Ils portent un masque.";
    auto theyHoldAMask = textToSemExp(theyHoldAMaskStr, lingDb, frLanguage);
    operator_inform("Vianney", semMem, lingDb);
    memoryOperation::mergeWithContext(theyHoldAMask, semMem, lingDb);
    EXPECT_EQ(theyHoldAMaskStr, semExpToText(std::move(theyHoldAMask), frLanguage, semMem, lingDb));
  }

  {
    const std::string thisTableIsCoolStr = "This table is cool.";
    auto thisTableIsCoolSentence = textToSemExp(thisTableIsCoolStr, lingDb, enLanguage);
    EXPECT_EQ(thisTableIsCoolStr, semExpToText(thisTableIsCoolSentence->clone(), enLanguage, semMem, lingDb));
    operator_inform("I like you", semMem, lingDb);
    memoryOperation::mergeWithContext(thisTableIsCoolSentence, semMem, lingDb);
    EXPECT_EQ(thisTableIsCoolStr, semExpToText(std::move(thisTableIsCoolSentence), enLanguage, semMem, lingDb));
  }

  {
    const std::string itsAGreatDayStr = "It's a great day.";
    auto itsAGreatDaySentence = textToSemExp(itsAGreatDayStr, lingDb, enLanguage);
    EXPECT_EQ(itsAGreatDayStr, semExpToText(itsAGreatDaySentence->clone(), enLanguage, semMem, lingDb));
    operator_inform("I like you", semMem, lingDb);
    memoryOperation::mergeWithContext(itsAGreatDaySentence, semMem, lingDb);
    EXPECT_EQ(itsAGreatDayStr, semExpToText(std::move(itsAGreatDaySentence), enLanguage, semMem, lingDb));
  }

  {
    operator_inform_fromRobot("Do you like beer?", semMem, lingDb);
    auto yesKnowledge = textToSemExp("yes", lingDb, enLanguage);
    memoryOperation::mergeWithContext(yesKnowledge, semMem, lingDb);
    ONSEM_UNKNOWN(operator_check("I like beer", semMem, lingDb));
    memoryOperation::inform(std::move(yesKnowledge), semMem, lingDb);
    ONSEM_TRUE(operator_check("I like beer", semMem, lingDb));
  }

  // answer about the time
  {
    operator_inform_fromRobot("Quand es-tu né ?", semMem, lingDb);
    auto answerSmExp = textToSemExp("1987", lingDb, frLanguage);
    memoryOperation::mergeWithContext(answerSmExp, semMem, lingDb);
    EXPECT_EQ("Tu es né en 1987.", semExpToText(std::move(answerSmExp), frLanguage, semMem, lingDb));
  }

  // answer about a day
  {
    operator_inform_fromRobot("Quel jour es-tu né ?", semMem, lingDb);
    auto answerSmExp = textToSemExp("5 février", lingDb, frLanguage);
    memoryOperation::mergeWithContext(answerSmExp, semMem, lingDb);
    EXPECT_EQ("Tu es né le 5 février.", semExpToText(std::move(answerSmExp), frLanguage, semMem, lingDb));
  }

  // allow to merge with context in 2 different places
  {
    const std::string cmdStr = "\\" + resourceLabelForTests_cmd + "=a_command2\\";
    operator_inform_fromRobot(cmdStr, semMem, lingDb);
    auto itIsToLookLeft = textToSemExp("It's to smile. It's funny", lingDb, enLanguage);
    memoryOperation::mergeWithContext(itIsToLookLeft, semMem, lingDb);
    memoryOperation::inform(std::move(itIsToLookLeft), semMem, lingDb);
    ONSEM_ANSWER_EQ("a_command2", operator_answer("what is to smile", semMem, lingDb));
    ONSEM_ANSWER_EQ("a_command2", operator_answer("what is funny", semMem, lingDb));
  }

  {
    operator_inform_fromRobot("\\" + resourceLabelForTests_cmd + "=a_command\\", semMem, lingDb);
    auto itIsToLookLeft = textToSemExp("it is nice", lingDb, enLanguage);
    memoryOperation::mergeWithContext(itIsToLookLeft, semMem, lingDb);
    EXPECT_EQ("a_command is nice.", semExpToText(std::move(itIsToLookLeft), enLanguage, semMem, lingDb));
  }

  {
    operator_inform_fromRobot("\\" + resourceLabelForTests_cmd + "=/path/to/thriller_dance\\", semMem, lingDb);
    auto itIsThriller = textToSemExp("C'est Thriller.", lingDb, frLanguage);
    memoryOperation::mergeWithContext(itIsThriller, semMem, lingDb);
    EXPECT_EQ("/path/to/thriller_dance est Thriller.", semExpToText(std::move(itIsThriller), frLanguage, semMem, lingDb));
  }

  {
    operator_inform("I went to the", semMem, lingDb);
    operator_mergeAndInform("cinema", semMem, lingDb);
    ONSEM_TRUE(operator_check("I went to the cinema", semMem, lingDb));
  }

  // No replacement from the third person of singular to the author
  {
    operator_inform("Je suis là", semMem, lingDb);
    operator_mergeAndInform("Il est content", semMem, lingDb);
    ONSEM_UNKNOWN(operator_check("Je suis content", semMem, lingDb));
  }

  // No replacement from the third person of singular to the receiver
  {
    operator_inform("Tu es là", semMem, lingDb);
    operator_mergeAndInform("Il aime manger", semMem, lingDb);
    ONSEM_UNKNOWN(operator_check("Tu aimes manger", semMem, lingDb));
  }

  // no merge with a engagement interjection
  {
    static const std::string whatIsYourName = "What is your name?";
    static const std::string whatIsMyName = "What is my name?";
    static const std::string idontKbowYourName = "I don't know what my name is.";
    ONSEM_ANSWERNOTFOUND_EQ(idontKbowYourName, operator_react(whatIsYourName, semMem, lingDb));
    operator_inform_fromRobot(whatIsMyName, semMem, lingDb);
    operator_mergeAndInform("hello N5", semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ(idontKbowYourName, operator_react(whatIsYourName, semMem, lingDb));
    operator_inform_fromRobot(whatIsMyName, semMem, lingDb);
    operator_mergeAndInform("bye N5", semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ(idontKbowYourName, operator_react(whatIsYourName, semMem, lingDb));
    operator_inform_fromRobot(whatIsMyName, semMem, lingDb);
    operator_mergeAndInform("N5", semMem, lingDb);
    ONSEM_ANSWER_EQ("My name is N5.", operator_react(whatIsYourName, semMem, lingDb));
  }

  {
    operator_inform_fromRobot("Quelle est la différence entre un amant et un mari ?", semMem, lingDb);
    auto iDontKnowSemExp = textToSemExp("je ne sais pas", lingDb, frLanguage);
    memoryOperation::mergeWithContext(iDontKnowSemExp, semMem, lingDb);
    EXPECT_EQ("Tu ne sais pas quelle est la différence entre un amant et un mari.",
              semExpToText(std::move(iDontKnowSemExp), frLanguage, semMem, lingDb));
  }

  {
    operator_inform_fromRobot("Quelle formule voulez-vous avoir ?", semMem, lingDb);
    auto basicSemExp = textToSemExp("basique", lingDb, frLanguage);
    memoryOperation::mergeWithContext(basicSemExp, semMem, lingDb);
    EXPECT_EQ("Tu veux avoir la formule basique.",
              semExpToText(std::move(basicSemExp), frLanguage, semMem, lingDb));
  }

  {
    operator_inform_fromRobot("Comment ça va ?", semMem, lingDb);
    auto helloSemExp = textToSemExp("salut", lingDb, frLanguage);
    memoryOperation::mergeWithContext(helloSemExp, semMem, lingDb);
    EXPECT_EQ("Salut",
              semExpToText(std::move(helloSemExp), frLanguage, semMem, lingDb));
  }

  {
    operator_inform_fromRobot("Comment ça va ?", semMem, lingDb);
    auto helloSemExp = textToSemExp("bien", lingDb, frLanguage);
    memoryOperation::mergeWithContext(helloSemExp, semMem, lingDb);
    EXPECT_EQ("Tu vas bien.",
              semExpToText(std::move(helloSemExp), frLanguage, semMem, lingDb));
  }

}


TEST_F(SemanticReasonerGTests, operator_mergeWithContext_hasACoreference)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  EXPECT_TRUE(_hasACoreferenceFromStr("What does it mean?", lingDb));
  EXPECT_FALSE(_hasACoreferenceFromStr("What does it mean to raise the left arm?", lingDb));
}
