#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_REACTFROMTRIGGER_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_REACTFROMTRIGGER_HPP

#include <string>
#include <onsem/common/enum/semanticlanguagetype.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;
struct DetailedReactionAnswer;
struct ReactionOptions;


DetailedReactionAnswer operator_reactFromTrigger(const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pTextLanguage = SemanticLanguageEnum::UNKNOWN,
    const ReactionOptions* pReactionOptions = nullptr);

} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_REACTFROMTRIGGER_HPP
