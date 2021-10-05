#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpsimplifer.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "operator_inform.hpp"

using namespace onsem;

namespace
{

DetailedReactionAnswer operator_solveConditions(const std::string& pText,
                                                const SemanticMemory& pSemanticMemory,
                                                const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, pLingDb);
  auto semExp =
      converter::textToSemExp(pText,
                              TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                    SemanticAgentGrounding::me,
                                                    language),
                              pLingDb);

  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  simplifier::solveConditionsInplace(semExp, pSemanticMemory.memBloc, pLingDb);

  DetailedReactionAnswer res;
  if (semExp->isEmpty())
    res.reactionType = ContextualAnnotation::ANSWERNOTFOUND;
  else
    res.reactionType = ContextualAnnotation::ANSWER;

  converter::semExpToText(res.answer, std::move(semExp),
                          TextProcessingContext(SemanticAgentGrounding::me,
                                                SemanticAgentGrounding::currentUser,
                                                language),
                          false, pSemanticMemory, pLingDb, nullptr);
  return res;
}

}


TEST_F(SemanticReasonerGTests, operator_solveConditions_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_ANSWER_EQ("It's normal.",
                  operator_solveConditions("If 2 is greater than 1 then it's normal.", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("",
                          operator_solveConditions("If 1 is greater than 2 then it's a bug.", semMem, lingDb));

  ONSEM_ANSWER_EQ("A chocolate",
                  operator_solveConditions("If 1 is greater than 0 then a chocolate else a banana.", semMem, lingDb));
  ONSEM_ANSWER_EQ("If Paul is happy then a chocolate else a banana.",
                  operator_solveConditions("If Paul is happy then a chocolate else a banana.", semMem, lingDb));
  operator_inform("Paul is not happy", semMem, lingDb);
  ONSEM_ANSWER_EQ("A banana",
                  operator_solveConditions("If Paul is happy then a chocolate else a banana.", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("",
                          operator_solveConditions("If Paul is happy then a chocolate.", semMem, lingDb));
  ONSEM_ANSWER_EQ("You will drink water.",
                  operator_solveConditions("If Paul is happy then a chocolate. If 1 is greater than 0 then I will drink water.", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("",
                          operator_solveConditions("If Paul is happy then a chocolate. If 1 is greater than 2 then I will drink water.", semMem, lingDb));
}
