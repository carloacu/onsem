#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_EXECUTEFROMTRIGGER_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_EXECUTEFROMTRIGGER_HPP

#include <string>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;


std::string operator_executeFromTrigger(const std::string& pText,
                                        SemanticMemory& pSemanticMemory,
                                        const linguistics::LinguisticDatabase& pLingDb);

std::string operator_executeFromSemExpTrigger(const SemanticExpression& pSemExp,
                                              SemanticLanguageEnum pLanguage,
                                              SemanticMemory& pSemanticMemory,
                                              const linguistics::LinguisticDatabase& pLingDb);

std::string operator_execute(const std::string& pText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_EXECUTEFROMTRIGGER_HPP
