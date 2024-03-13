#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_EXECUTEFROMCONDITION_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_EXECUTEFROMCONDITION_HPP

#include <string>

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}
struct SemanticMemory;

std::string operator_executeFromCondition(const std::string& pText,
                                          SemanticMemory& pSemanticMemory,
                                          const linguistics::LinguisticDatabase& pLingDb);

std::string operator_executeFromSemExpCondition(const SemanticExpression& pSemExp,
                                                SemanticLanguageEnum pLanguage,
                                                SemanticMemory& pSemanticMemory,
                                                const linguistics::LinguisticDatabase& pLingDb);

std::string operator_execute(const std::string& pText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb);

}    // End of namespace onsem

#endif    // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_EXECUTEFROMCONDITION_HPP
