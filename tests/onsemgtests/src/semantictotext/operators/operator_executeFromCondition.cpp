#include "../../semanticreasonergtests.hpp"
#include "operator_executeFromCondition.hpp"
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>


using namespace onsem;

namespace onsem
{

namespace
{
std::string _operator_execOrExecuteFromSemExpCondition(const SemanticExpression& pSemExp,
                                                       SemanticLanguageEnum pLanguage,
                                                       SemanticMemory& pSemanticMemory,
                                                       const linguistics::LinguisticDatabase& pLingDb,
                                                       bool pExecOrExecuteFromCondition)
{
  auto resSemExp = pExecOrExecuteFromCondition ?
        memoryOperation::execute(pSemExp, pSemanticMemory, pLingDb) :
        memoryOperation::executeFromCondition(pSemExp, pSemanticMemory, pLingDb);
  if (resSemExp)
    return semExpToTextExectionResult(std::move(*resSemExp), pLanguage, pSemanticMemory, pLingDb, &pSemExp);
  return "";
}

std::string _operator_execOrExecuteFromCondition(const std::string& pText,
                                                 SemanticMemory& pSemanticMemory,
                                                 const linguistics::LinguisticDatabase& pLingDb,
                                                 bool pExecOrExecuteFromCondition)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, pLingDb);
  TextProcessingContext textProc(SemanticAgentGrounding::currentUser,
                                 SemanticAgentGrounding::me,
                                 language);
  auto semExp = converter::textToSemExp(pText, textProc, pLingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return _operator_execOrExecuteFromSemExpCondition(*semExp, language, pSemanticMemory, pLingDb, pExecOrExecuteFromCondition);
}
}


std::string operator_executeFromCondition(const std::string& pText,
                                          SemanticMemory& pSemanticMemory,
                                          const linguistics::LinguisticDatabase& pLingDb)
{
  return _operator_execOrExecuteFromCondition(pText, pSemanticMemory, pLingDb, false);
}


std::string operator_executeFromSemExpCondition(const SemanticExpression& pSemExp,
                                                SemanticLanguageEnum pLanguage,
                                                SemanticMemory& pSemanticMemory,
                                                const linguistics::LinguisticDatabase& pLingDb)
{
  return _operator_execOrExecuteFromSemExpCondition(pSemExp, pLanguage, pSemanticMemory, pLingDb, false);
}

std::string operator_execute(const std::string& pText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb)
{
  return _operator_execOrExecuteFromCondition(pText, pSemanticMemory, pLingDb, true);
}


} // End of namespace onsem




