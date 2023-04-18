#ifndef ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_ADD_HPP
#define ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_ADD_HPP

#include <string>
#include <list>
#include <onsem/common/enum/semanticlanguageenum.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;
struct UniqueSemanticExpression;


void triggers_add(const std::string& pTriggerText,
                  const std::string& pAnswerText,
                  SemanticMemory& pSemanticMemory,
                  const linguistics::LinguisticDatabase& pLingDb,
                  const std::list<std::string>& pReferences = std::list<std::string>(),
                  SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);

void triggers_addToSemExpAnswer(
    const std::string& pTriggerText,
    UniqueSemanticExpression pAnswerSemExp,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);

void triggers_addAnswerWithOneParameter(const std::string& pTriggerText,
    const std::vector<std::string>& pParameterQuestions,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);

void triggers_addAnswerWithManyParameters(
    const std::string& pTriggerText,
    const std::map<std::string, std::vector<std::string>>& pParameterLabelToQuestionsStrs,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);


} // End of namespace onsem


#endif // ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_ADD_HPP
