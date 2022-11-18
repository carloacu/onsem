#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_ADDATRIGGER_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_ADDATRIGGER_HPP

#include <string>
#include <list>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;


void operator_addATrigger(const std::string& pTriggerText,
                          const std::string& pAnswerText,
                          SemanticMemory& pSemanticMemory,
                          const linguistics::LinguisticDatabase& pLingDb,
                          const std::list<std::string>& pReferences = std::list<std::string>());


} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_ADDATRIGGER_HPP
