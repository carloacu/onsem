#include "../semanticreasonergtests.hpp"
#include "operators/operator_answer.hpp"
#include <onsem/semantictotext/executor/executorlogger.hpp>
#include <onsem/semantictotext/executor/textexecutor.hpp>
#include <onsem/semantictotext/type/naturallanguageexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>

using namespace onsem;

NaturalLanguageExpression _somethingTime(const std::string& pLemma)
{
  NaturalLanguageExpression res;
  res.word = NaturalLanguageText("time", NaturalLanguageTypeOfText::NOUN, SemanticLanguageEnum::ENGLISH);
  res.reference = SemanticReferenceType::DEFINITE;
  res.children.emplace(GrammaticalType::SPECIFIER, [&] {
    NaturalLanguageExpression specifier;
    specifier.word = NaturalLanguageText(pLemma, NaturalLanguageTypeOfText::NOUN, SemanticLanguageEnum::ENGLISH);
    return specifier;
  }());
  return res;
}

NaturalLanguageExpression _constructEnWord(const std::string& pLemma, SemanticReferenceType pReferenceType)
{
  NaturalLanguageExpression res;
  res.word = NaturalLanguageText(pLemma, NaturalLanguageTypeOfText::NOUN, SemanticLanguageEnum::ENGLISH);
  res.reference = pReferenceType;
  return res;
}


TEST_F(SemanticReasonerGTests, test_naturalLanguageExpressionToSemanticExpression)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  auto enTP = TextProcessingContext::getTextProcessingContextFromRobot(SemanticLanguageEnum::ENGLISH);
  const std::string paulHumanId = "paul-human";

  NaturalLanguageExpression robotNle;
  robotNle.word = NaturalLanguageText(SemanticAgentGrounding::me, NaturalLanguageTypeOfText::AGENT);
  NaturalLanguageExpression paulNle;
  paulNle.word = NaturalLanguageText(paulHumanId, NaturalLanguageTypeOfText::AGENT);
  NaturalLanguageExpression getCompanyInfoNle;
  getCompanyInfoNle.word = NaturalLanguageText("to get information about the company", NaturalLanguageTypeOfText::EXPRESSION, SemanticLanguageEnum::ENGLISH);
  NaturalLanguageExpression getNaulHands;
  getNaulHands.word = NaturalLanguageText("hand", NaturalLanguageTypeOfText::NOUN, SemanticLanguageEnum::ENGLISH);
  getNaulHands.quantity.setPlural();
  getNaulHands.reference = SemanticReferenceType::DEFINITE;
  getNaulHands.children.emplace(GrammaticalType::OWNER, paulNle);
  NaturalLanguageExpression getAnUmbrella = _constructEnWord("umbrella", SemanticReferenceType::INDEFINITE);
  NaturalLanguageExpression morningExp = _constructEnWord("morning", SemanticReferenceType::DEFINITE);
  NaturalLanguageExpression lunchExp = _somethingTime("lunch");

  NaturalLanguageExpression paulWantsToGetCompanyInfoNle;
  paulWantsToGetCompanyInfoNle.word = NaturalLanguageText("want", NaturalLanguageTypeOfText::VERB, SemanticLanguageEnum::ENGLISH);
  paulWantsToGetCompanyInfoNle.verbTense = SemanticVerbTense::PRESENT;
  paulWantsToGetCompanyInfoNle.children.emplace(GrammaticalType::SUBJECT, paulNle);
  paulWantsToGetCompanyInfoNle.children.emplace(GrammaticalType::OBJECT, getCompanyInfoNle);

  NaturalLanguageExpression paulCannotBeIdentifiedNle;
  paulCannotBeIdentifiedNle.word = NaturalLanguageText("identify", NaturalLanguageTypeOfText::VERB, SemanticLanguageEnum::ENGLISH);
  paulCannotBeIdentifiedNle.verbTense = SemanticVerbTense::PRESENT;
  paulCannotBeIdentifiedNle.verbGoal = VerbGoalEnum::ABILITY;
  paulCannotBeIdentifiedNle.polarity = false;
  paulCannotBeIdentifiedNle.children.emplace(GrammaticalType::OBJECT, paulNle);

  NaturalLanguageExpression iAmLocalizedNle;
  iAmLocalizedNle.word = NaturalLanguageText("localize", NaturalLanguageTypeOfText::VERB, SemanticLanguageEnum::ENGLISH);
  iAmLocalizedNle.verbTense = SemanticVerbTense::PRESENT;
  iAmLocalizedNle.children.emplace(GrammaticalType::OBJECT, robotNle);

  NaturalLanguageExpression paulShouldWashHandsNle;
  paulShouldWashHandsNle.word = NaturalLanguageText("wash", NaturalLanguageTypeOfText::VERB, SemanticLanguageEnum::ENGLISH);
  paulShouldWashHandsNle.verbTense = SemanticVerbTense::PRESENT;
  paulShouldWashHandsNle.verbGoal = VerbGoalEnum::ADVICE;
  paulShouldWashHandsNle.children.emplace(GrammaticalType::SUBJECT, paulNle);
  paulShouldWashHandsNle.children.emplace(GrammaticalType::OBJECT, getNaulHands);

  NaturalLanguageExpression paulShouldWearAnUmbrellaNle;
  paulShouldWearAnUmbrellaNle.word = NaturalLanguageText("wear", NaturalLanguageTypeOfText::VERB, SemanticLanguageEnum::ENGLISH);
  paulShouldWearAnUmbrellaNle.verbTense = SemanticVerbTense::PRESENT;
  paulShouldWearAnUmbrellaNle.verbGoal = VerbGoalEnum::ADVICE;
  paulShouldWearAnUmbrellaNle.children.emplace(GrammaticalType::SUBJECT, paulNle);
  paulShouldWearAnUmbrellaNle.children.emplace(GrammaticalType::OBJECT, getAnUmbrella);

  NaturalLanguageExpression lunchIsAfterMorningNle;
  lunchIsAfterMorningNle.word = NaturalLanguageText("be~after", NaturalLanguageTypeOfText::VERB, SemanticLanguageEnum::ENGLISH);
  lunchIsAfterMorningNle.verbTense = SemanticVerbTense::PRESENT;
  lunchIsAfterMorningNle.children.emplace(GrammaticalType::SUBJECT, lunchExp);
  lunchIsAfterMorningNle.children.emplace(GrammaticalType::OBJECT, morningExp);

  NaturalLanguageExpression aRandomQuoteNle;
  aRandomQuoteNle.word = NaturalLanguageText("Aa bb ok ffsgsg", NaturalLanguageTypeOfText::QUOTE, SemanticLanguageEnum::UNKNOWN);

  NaturalLanguageExpression resourceExNle;
  resourceExNle.word = NaturalLanguageText("\\resLabel=resValue\\", NaturalLanguageTypeOfText::EXPRESSION, SemanticLanguageEnum::ENGLISH);

  semMem.setCurrUserId(paulHumanId);

  auto semExp1 = converter::naturalLanguageExpressionToSemanticExpression(paulWantsToGetCompanyInfoNle, lingDb);
  EXPECT_EQ("You want to get information about the company.",
            semExpToText(semExp1->clone(),
                         SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  EXPECT_EQ("Tu veux obtenir des informations sur l'entreprise.",
            semExpToText(semExp1->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));
  const std::vector<std::string> paulGustaveNames{"Paul", "Gustave"};
  auto paulNameSemExp = converter::agentIdWithNameToSemExp(paulHumanId, paulGustaveNames);
  memoryOperation::resolveAgentAccordingToTheContext(paulNameSemExp, semMem, lingDb);
  memoryOperation::inform(std::move(paulNameSemExp), semMem, lingDb);
  semMem.setCurrUserId("other-human");
  EXPECT_EQ("Paul Gustave veut obtenir des informations sur l'entreprise.",
            semExpToText(semExp1->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));
  semMem.setCurrUserId(paulHumanId);
  memoryOperation::resolveAgentAccordingToTheContext(semExp1, semMem, lingDb);
  memoryOperation::inform(std::move(semExp1), semMem, lingDb);
  ONSEM_ANSWER_EQ("You want to get information about the company.",
                  operator_answer("What do I want?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Tu veux obtenir des informations sur l'entreprise.",
                  operator_answer("Qu'est-ce que je veux ?", semMem, lingDb));


  auto semExp2 = converter::naturalLanguageExpressionToSemanticExpression(paulCannotBeIdentifiedNle, lingDb);
  EXPECT_EQ("You can't be identified.",
            semExpToText(semExp2->clone(),
                         SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  EXPECT_EQ("Tu ne peux pas être identifié.",
            semExpToText(semExp2->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));
  memoryOperation::resolveAgentAccordingToTheContext(semExp2, semMem, lingDb);
  memoryOperation::inform(std::move(semExp2), semMem, lingDb);
  ONSEM_ANSWER_EQ("No, you can't be identified.",
                  operator_answer("Can I be identified?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Non, tu ne peux pas être identifié.",
                  operator_answer("Est-ce que je peux être identifié ?", semMem, lingDb));


  auto semExp3 = converter::naturalLanguageExpressionToSemanticExpression(iAmLocalizedNle, lingDb);
  EXPECT_EQ("I am localized.",
            semExpToText(semExp3->clone(),
                         SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  EXPECT_EQ("Je suis localisé.",
            semExpToText(semExp3->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));
  memoryOperation::resolveAgentAccordingToTheContext(semExp3, semMem, lingDb);
  memoryOperation::inform(std::move(semExp3), semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, I am localized.",
                  operator_answer("Are you localized?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Oui, je suis localisé.",
                  operator_answer("Es-tu localisé ?", semMem, lingDb));


  auto semExp4 = converter::naturalLanguageExpressionToSemanticExpression(paulShouldWashHandsNle, lingDb);
  EXPECT_EQ("You should wash your hands.",
            semExpToText(semExp4->clone(),
                         SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  EXPECT_EQ("Tu devrais laver tes mains.",
            semExpToText(semExp4->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));
  memoryOperation::resolveAgentAccordingToTheContext(semExp4, semMem, lingDb);
  memoryOperation::inform(std::move(semExp4), semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, you should wash your hands.",
                  operator_answer("Should I wash my hands?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Oui, tu devrais laver tes mains.",
                  operator_answer("Devrais-je laver mes mains ?", semMem, lingDb));


  auto semExp5 = converter::naturalLanguageExpressionToSemanticExpression(paulShouldWearAnUmbrellaNle, lingDb);
  EXPECT_EQ("You should wear an umbrella.",
            semExpToText(semExp5->clone(),
                         SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  EXPECT_EQ("Tu devrais porter un parapluie.",
            semExpToText(semExp5->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));
  memoryOperation::resolveAgentAccordingToTheContext(semExp5, semMem, lingDb);
  memoryOperation::inform(std::move(semExp5), semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, you should wear an umbrella.",
                  operator_answer("Should I wear an umbrella?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Oui, tu devrais porter un parapluie.",
                  operator_answer("Devrais-je porter un parapluie ?", semMem, lingDb));


  auto semExp6 = converter::naturalLanguageExpressionToSemanticExpression(aRandomQuoteNle, lingDb);
  EXPECT_EQ(aRandomQuoteNle.word.text,
            semExpToText(semExp6->clone(),
                         SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  EXPECT_EQ(aRandomQuoteNle.word.text,
            semExpToText(semExp6->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));

  auto semExp7 = converter::naturalLanguageExpressionToSemanticExpression(resourceExNle, lingDb, {"resLabel"});
  TextProcessingContext outContext(SemanticAgentGrounding::me,
                                   SemanticAgentGrounding::currentUser,
                                   SemanticLanguageEnum::ENGLISH);
  auto execContext = std::make_shared<ExecutorContext>(outContext);
  std::string logStr;
  ExecutorLoggerWithoutMetaInformation logger(logStr);
  TextExecutor textExec(semMem, lingDb, logger);
  textExec.runSemExp(std::move(semExp7), execContext);
  EXPECT_EQ("\\resLabel=resValue\\", logStr);

  auto semExp8 = converter::naturalLanguageExpressionToSemanticExpression(lunchIsAfterMorningNle, lingDb);
  EXPECT_EQ("Lunch time is after morning.",
            semExpToText(semExp8->clone(),
                         SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  EXPECT_EQ("Le temps de déjeuner est après le matin.",
            semExpToText(semExp8->clone(),
                         SemanticLanguageEnum::FRENCH, semMem, lingDb));
}

