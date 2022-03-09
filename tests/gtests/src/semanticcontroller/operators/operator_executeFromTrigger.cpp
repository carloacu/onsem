#include "../../semanticreasonergtests.hpp"
#include "operator_executeFromTrigger.hpp"
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>


using namespace onsem;

namespace onsem
{

namespace
{
std::string _operator_execOrExecuteFromSemExpTrigger(const SemanticExpression& pSemExp,
                                                     SemanticLanguageEnum pLanguage,
                                                     SemanticMemory& pSemanticMemory,
                                                     const linguistics::LinguisticDatabase& pLingDb,
                                                     bool pExecOrExecuteFromTrigger)
{
  auto resSemExp = pExecOrExecuteFromTrigger ?
        memoryOperation::execute(pSemExp, pSemanticMemory, pLingDb) :
        memoryOperation::executeFromTrigger(pSemExp, pSemanticMemory, pLingDb);
  if (resSemExp)
    return semExpToTextExectionResult(std::move(*resSemExp), pLanguage, pSemanticMemory, pLingDb);
  return "";
}

std::string _operator_execOrExecuteFromTrigger(const std::string& pText,
                                               SemanticMemory& pSemanticMemory,
                                               const linguistics::LinguisticDatabase& pLingDb,
                                               bool pExecOrExecuteFromTrigger)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, pLingDb);
  TextProcessingContext textProc(SemanticAgentGrounding::currentUser,
                                 SemanticAgentGrounding::me,
                                 language);
  auto semExp = converter::textToSemExp(pText, textProc, pLingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return _operator_execOrExecuteFromSemExpTrigger(*semExp, language, pSemanticMemory, pLingDb, pExecOrExecuteFromTrigger);
}
}


std::string operator_executeFromTrigger(const std::string& pText,
                                        SemanticMemory& pSemanticMemory,
                                        const linguistics::LinguisticDatabase& pLingDb)
{
  return _operator_execOrExecuteFromTrigger(pText, pSemanticMemory, pLingDb, false);
}


std::string operator_executeFromSemExpTrigger(const SemanticExpression& pSemExp,
                                              SemanticLanguageEnum pLanguage,
                                              SemanticMemory& pSemanticMemory,
                                              const linguistics::LinguisticDatabase& pLingDb)
{
  return _operator_execOrExecuteFromSemExpTrigger(pSemExp, pLanguage, pSemanticMemory, pLingDb, false);
}

std::string operator_execute(const std::string& pText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb)
{
  return _operator_execOrExecuteFromTrigger(pText, pSemanticMemory, pLingDb, true);
}


} // End of namespace onsem




