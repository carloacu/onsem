#ifndef ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_ADD_HPP
#define ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_ADD_HPP

#include <string>
#include <list>
#include <onsem/common/enum/semanticlanguagetype.hpp>

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
                          const std::list<std::string>& pReferences = std::list<std::string>());

void triggers_addToSemExpAnswer(
    const std::string& pTriggerText,
    UniqueSemanticExpression pAnswerSemExp,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);

} // End of namespace onsem


#endif // ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_ADD_HPP