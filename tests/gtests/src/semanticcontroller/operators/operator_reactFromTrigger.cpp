#include "operator_reactFromTrigger.hpp"
#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "operator_addATrigger.hpp"

using namespace onsem;

namespace
{

DetailedReactionAnswer _operator_reactFromTrigger_fromSemExp(UniqueSemanticExpression pSemExp,
                                                             SemanticMemory& pSemanticMemory,
                                                             const linguistics::LinguisticDatabase& pLingDb,
                                                             SemanticLanguageEnum pTextLanguage,
                                                             const ReactionOptions* pReactionOptions)
{
  if (pTextLanguage == SemanticLanguageEnum::UNKNOWN)
    pTextLanguage = pSemanticMemory.defaultLanguage;
  mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
  memoryOperation::resolveAgentAccordingToTheContext(pSemExp, pSemanticMemory, pLingDb);
  memoryOperation::reactFromTrigger(reaction, pSemanticMemory, std::move(pSemExp), pLingDb,
                                    pReactionOptions);
  return reactionToAnswer(reaction, pSemanticMemory, pLingDb, pTextLanguage);
}

}

namespace onsem
{

DetailedReactionAnswer operator_reactFromTrigger(const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pTextLanguage,
    const ReactionOptions* pReactionOptions)
{
  SemanticLanguageEnum textLanguage = pTextLanguage == SemanticLanguageEnum::UNKNOWN ?
      linguistics::getLanguage(pText, pLingDb) : pTextLanguage;
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  textLanguage);
  auto semExp =
      converter::textToContextualSemExp(pText, inContext,
                                        SemanticSourceEnum::ASR, pLingDb);
  return _operator_reactFromTrigger_fromSemExp(std::move(semExp), pSemanticMemory, pLingDb,
                                               textLanguage, pReactionOptions);
}

} // End of namespace onsem



TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string questionStr = "Qui es tu ?";
  const std::string answerStr = "Je suis ton ami.";
  ONSEM_NOANSWER(operator_reactFromTrigger(questionStr, semMem, lingDb));
  operator_addATrigger(questionStr, answerStr, semMem, lingDb);
  ONSEM_ANSWER_EQ(answerStr, operator_reactFromTrigger(questionStr, semMem, lingDb));
}
