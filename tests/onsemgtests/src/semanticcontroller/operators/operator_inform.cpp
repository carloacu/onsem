#include "operator_inform.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/semanticexpression/annotatedexpression.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/serialization.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "operator_answer.hpp"
#include "operator_check.hpp"
#include "../../semanticreasonergtests.hpp"

using namespace onsem;

namespace onsem
{
namespace
{
std::shared_ptr<ExpressionHandleInMemory> operator_inform_withTextProc(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::list<std::string>& pReferences,
    std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
    const TextProcessingContext& pTextProcContext,
    std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>())
{
  auto semExp =
      converter::textToContextualSemExp(pText, pTextProcContext,
                                        SemanticSourceEnum::ASR, pLingDb, &pReferences,
                                        std::move(pAgentWeAreTalkingAbout));
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return memoryOperation::inform(std::move(semExp), pSemanticMemory, pLingDb, nullptr,
                                 pAxiomToConditionCurrentStatePtr);
}

}


std::shared_ptr<ExpressionHandleInMemory> operator_inform(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::list<std::string>& pReferences,
    std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
    const TextProcessingContext* pTextProcContextPtr)
{
  if (pTextProcContextPtr != nullptr)
    return operator_inform_withTextProc(pText, pSemanticMemory, pLingDb, pReferences,
                                        pAxiomToConditionCurrentStatePtr, *pTextProcContextPtr);

  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  SemanticLanguageEnum::UNKNOWN);
  inContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{resourceLabelForTests_cmd, resourceLabelForTests_url});

  return operator_inform_withTextProc(pText, pSemanticMemory, pLingDb, pReferences,
                                      pAxiomToConditionCurrentStatePtr, inContext);
}


std::shared_ptr<ExpressionHandleInMemory> operator_inform_fromRobot(const std::string& pText,
                                                                      SemanticMemory& pSemanticMemory,
                                                                      const linguistics::LinguisticDatabase& pLingDb,
                                                                      const std::list<std::string>& pReferences,
                                                                      std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  TextProcessingContext outContext(SemanticAgentGrounding::me,
                                   SemanticAgentGrounding::currentUser,
                                   SemanticLanguageEnum::UNKNOWN);
  outContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{resourceLabelForTests_cmd, resourceLabelForTests_url});
  auto semExp =
      converter::textToContextualSemExp(pText, outContext,
                                        SemanticSourceEnum::ASR, pLingDb, &pReferences);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return memoryOperation::inform(std::move(semExp), pSemanticMemory, pLingDb, nullptr,
                                 pAxiomToConditionCurrentStatePtr);
}


void operator_mergeAndInform(const std::string& pText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb,
                             const std::list<std::string>& pReferences)
{
  TextProcessingContext inTextProc(SemanticAgentGrounding::currentUser,
                                   SemanticAgentGrounding::me,
                                   SemanticLanguageEnum::UNKNOWN);
  auto semExp =
      converter::textToContextualSemExp(pText, inTextProc,
                                        SemanticSourceEnum::ASR, pLingDb, &pReferences);
  memoryOperation::mergeWithContext(semExp, pSemanticMemory, pLingDb);
  memoryOperation::inform(std::move(semExp), pSemanticMemory, pLingDb);
}


std::shared_ptr<ExpressionHandleInMemory> operator_informAxiom(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::list<std::string>& pReferences)
{
  TextProcessingContext inTextProc(SemanticAgentGrounding::currentUser,
                                   SemanticAgentGrounding::me,
                                   SemanticLanguageEnum::UNKNOWN);
  auto semExp =
      converter::textToContextualSemExp(pText, inTextProc,
                                        SemanticSourceEnum::ASR, pLingDb, &pReferences);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return memoryOperation::informAxiom(std::move(semExp), pSemanticMemory, pLingDb);
}

std::shared_ptr<ExpressionHandleInMemory> operator_informAxiom_fromRobot(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::list<std::string>& pReferences,
    std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  TextProcessingContext textContext(SemanticAgentGrounding::me,
                                    SemanticAgentGrounding::currentUser,
                                    SemanticLanguageEnum::UNKNOWN);
  textContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{resourceLabelForTests_cmd, resourceLabelForTests_url});
  auto semExp =
      converter::textToContextualSemExp(pText, textContext,
                                        SemanticSourceEnum::ASR, pLingDb, &pReferences);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return memoryOperation::informAxiom(std::move(semExp), pSemanticMemory, pLingDb, nullptr,
                                            pAxiomToConditionCurrentStatePtr);
}


std::shared_ptr<ExpressionHandleInMemory> operator_addFallback(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExp =
      converter::textToContextualSemExp(pText,
                                        TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                              SemanticAgentGrounding::me,
                                                              SemanticLanguageEnum::UNKNOWN),
                                        SemanticSourceEnum::ASR, pLingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return memoryOperation::addFallback(std::move(semExp), pSemanticMemory, pLingDb);
}


std::shared_ptr<ExpressionHandleInMemory> operator_inform_withAgentNameFilter(
    const std::string& pAgentName,
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    SemanticLanguageEnum pLanguage,
    const linguistics::LinguisticDatabase& pLingDb)
{
  TextProcessingContext inContext(SemanticAgentGrounding::userNotIdentified,
                                  SemanticAgentGrounding::userNotIdentified,
                                  pLanguage);
  auto agentWeAreTalkingAbout = SemanticMemoryBlock::generateNewAgentGrd(pAgentName, pLanguage, pLingDb);
  const std::list<std::string> references{1, pAgentName};

  auto semExp =
      converter::textToContextualSemExp(pText, inContext,
                                        SemanticSourceEnum::ASR, pLingDb, &references,
                                        mystd::make_unique<SemanticAgentGrounding>(*agentWeAreTalkingAbout));
  memoryOperation::addAgentInterpretations(semExp, pSemanticMemory, pLingDb);
  SemExpModifier::removeSemExpPartsThatDoesntHaveAnAgent(semExp, *agentWeAreTalkingAbout);
  return memoryOperation::inform(std::move(semExp), pSemanticMemory, pLingDb, nullptr, nullptr);
}

} // End of namespace onsem



TEST_F(SemanticReasonerGTests, operator_informAndAssert)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  std::map<const SemanticContextAxiom*, TruenessValue> axiomToConditionCurrentState;

  // test inform
  {
    const std::string paulLikesChocolate = "Paul likes chocolate";
    operator_inform(paulLikesChocolate, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulLikesChocolate, semMem, lingDb));
    operator_inform("Paul doesn't like chocolate", semMem, lingDb);
    ONSEM_FALSE(operator_check(paulLikesChocolate, semMem, lingDb));
  }
  // test informWithBindId & remove
  {
    const std::string paulLikesPotatoes = "Paul likes potatoes";
    ONSEM_UNKNOWN(operator_check(paulLikesPotatoes, semMem, lingDb));
    auto id1 = operator_inform(paulLikesPotatoes, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulLikesPotatoes, semMem, lingDb));
    semMem.memBloc.removeExpression(*id1, lingDb, &axiomToConditionCurrentState);
    ONSEM_UNKNOWN(operator_check(paulLikesPotatoes, semMem, lingDb));
  }
  // test informWithBindId & replace
  {
    const std::string paulLikesMyPen = "Paul likes my pen";
    const std::string paulRuns = "Paul runs";
    auto id2 = operator_inform(paulLikesMyPen, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulLikesMyPen, semMem, lingDb));
    ONSEM_UNKNOWN(operator_check(paulRuns, semMem, lingDb));
    semMem.memBloc.removeExpression(*id2, lingDb, &axiomToConditionCurrentState);
    operator_inform(paulRuns, semMem, lingDb);
    ONSEM_UNKNOWN(operator_check(paulLikesMyPen, semMem, lingDb));
    ONSEM_TRUE(operator_check(paulRuns, semMem, lingDb));
    operator_inform("Paul doesn't run", semMem, lingDb);
    ONSEM_FALSE(operator_check(paulRuns, semMem, lingDb));
  }
  // test informAxiom
  {
    const std::string paulIsHappy = "Paul is happy";
    ONSEM_UNKNOWN(operator_check(paulIsHappy, semMem, lingDb));
    operator_informAxiom(paulIsHappy, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulIsHappy, semMem, lingDb));
    operator_inform("Paul is not happy", semMem, lingDb);
    ONSEM_TRUE(operator_check(paulIsHappy, semMem, lingDb));
    operator_informAxiom("Paul is not happy", semMem, lingDb);
    ONSEM_FALSE(operator_check(paulIsHappy, semMem, lingDb));
    operator_informAxiom(paulIsHappy, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulIsHappy, semMem, lingDb));
  }
  // test informAxiom & remove
  {
    const std::string paulLikesAndre = "Paul likes André";
    ONSEM_UNKNOWN(operator_check(paulLikesAndre, semMem, lingDb));
    auto id3 = operator_informAxiom(paulLikesAndre, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulLikesAndre, semMem, lingDb));
    semMem.memBloc.removeExpression(*id3, lingDb, &axiomToConditionCurrentState);
    ONSEM_UNKNOWN(operator_check(paulLikesAndre, semMem, lingDb));
  }
  // test informAxiom & replace
  {
    const std::string paulEatsBananas = "Paul eats bananas";
    const std::string paulLikesN5 = "Paul likes N5";
    ONSEM_UNKNOWN(operator_check(paulEatsBananas, semMem, lingDb));
    auto id4 = operator_informAxiom(paulEatsBananas, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulEatsBananas, semMem, lingDb));
    ONSEM_UNKNOWN(operator_check(paulLikesN5, semMem, lingDb));
    semMem.memBloc.removeExpression(*id4, lingDb, &axiomToConditionCurrentState);
    auto id5 = operator_informAxiom(paulLikesN5, semMem, lingDb);
    ONSEM_UNKNOWN(operator_check(paulEatsBananas, semMem, lingDb));
    ONSEM_TRUE(operator_check(paulLikesN5, semMem, lingDb));
    operator_inform("Paul doesn't like N5", semMem, lingDb);
    ONSEM_TRUE(operator_check(paulLikesN5, semMem, lingDb));
    semMem.memBloc.removeExpression(*id5, lingDb, &axiomToConditionCurrentState);
    ONSEM_UNKNOWN(operator_check(paulLikesN5, semMem, lingDb));
  }

  // test fallbacks
  {
    const std::string paulisWalking = "Paul is walking";
    const std::string paulisNotWalking = "Paul not is walking";
    ONSEM_UNKNOWN(operator_check(paulisWalking, semMem, lingDb));
    auto id1 = operator_inform(paulisWalking, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulisWalking, semMem, lingDb));
    auto id2 = operator_addFallback(paulisNotWalking, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulisWalking, semMem, lingDb));
    semMem.memBloc.removeExpression(*id1, lingDb, &axiomToConditionCurrentState);
    ONSEM_FALSE(operator_check(paulisWalking, semMem, lingDb));
    auto id3 = operator_informAxiom(paulisWalking, semMem, lingDb);
    ONSEM_TRUE(operator_check(paulisWalking, semMem, lingDb));
    semMem.memBloc.removeExpression(*id3, lingDb, &axiomToConditionCurrentState);
    ONSEM_FALSE(operator_check(paulisWalking, semMem, lingDb));
    ASSERT_NE(nullptr, semMem.memBloc.getFallbacksBlockPtr());
    semMem.memBloc.getFallbacksBlockPtr()->removeExpression(*id2, lingDb, &axiomToConditionCurrentState);
    ONSEM_UNKNOWN(operator_check(paulisWalking, semMem, lingDb));
  }

  // test many information removal
  {
    const std::string PaulLookAtAPen = "Paul looks at a book"; // pen
    const std::string PaulLookAtADoor = "Paul looks at a door";

    ONSEM_UNKNOWN(operator_check(PaulLookAtAPen, semMem, lingDb));
    ONSEM_UNKNOWN(operator_check(PaulLookAtADoor, semMem, lingDb));
    operator_inform(PaulLookAtAPen, semMem, lingDb);
    ONSEM_TRUE(operator_check(PaulLookAtAPen, semMem, lingDb));
    ONSEM_UNKNOWN(operator_check(PaulLookAtADoor, semMem, lingDb));
    operator_inform(PaulLookAtADoor, semMem, lingDb);
    ONSEM_TRUE(operator_check(PaulLookAtAPen, semMem, lingDb));
    ONSEM_TRUE(operator_check(PaulLookAtADoor, semMem, lingDb));
    operator_inform("Paul looks at nothing", semMem, lingDb);
    ONSEM_FALSE(operator_check(PaulLookAtAPen, semMem, lingDb));
    ONSEM_FALSE(operator_check(PaulLookAtADoor, semMem, lingDb));
  }
}



TEST_F(SemanticReasonerGTests, operator_informWithAnnotations)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  SemanticLanguageEnum::UNKNOWN);
  boost::property_tree::ptree propTree;

  {
    SemanticMemory semMem;
    auto annExp = mystd::make_unique<AnnotatedExpression>(converter::textToSemExp("I said hello", inContext, lingDb));
    annExp->annotations.emplace(GrammaticalType::TIME,
                                converter::textToSemExp("Paul leaves", inContext, lingDb));
    UniqueSemanticExpression semExp(std::move(annExp));
    memoryOperation::resolveAgentAccordingToTheContext(semExp, semMem, lingDb);
    memoryOperation::inform(std::move(semExp), semMem, lingDb);
    ONSEM_ANSWER_EQ("Yes, you said hello when Paul leaves.",
                    operator_react("I said hello when Paul leaves ?", semMem, lingDb));
    ONSEM_TRUE(operator_check("I said hello when Paul leaves", semMem, lingDb));
    serialization::saveSemMemory(propTree, semMem);
  }

  {
    SemanticMemory semMem2;
    ONSEM_UNKNOWN(operator_check("I said hello when Paul leaves", semMem2, lingDb));
    serialization::loadSemMemory(propTree, semMem2, lingDb);
    ONSEM_TRUE(operator_check("I said hello when Paul leaves", semMem2, lingDb));
    ONSEM_ANSWER_EQ("You said hello when Paul leaves.", operator_answer("when did I say hello", semMem2, lingDb));
    ONSEM_ANSWER_EQ("You said hello when Paul leaves.", operator_react("when did I say hello", semMem2, lingDb));
    ONSEM_ANSWER_EQ("Yes, you said hello.", operator_react("did I say hello", semMem2, lingDb));
    ONSEM_ANSWER_EQ("It was when Paul leaves.", operator_react("when was it", semMem2, lingDb));
  }
}
