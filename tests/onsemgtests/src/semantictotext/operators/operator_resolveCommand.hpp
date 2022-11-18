#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_RESOLVECOMMAND_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_RESOLVECOMMAND_HPP

#include <string>
#include <onsem/common/enum/semanticlanguagetype.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;


std::string operator_resolveCommand(const std::string& pText,
                                    SemanticMemory& pSemanticMemory,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);


} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_RESOLVECOMMAND_HPP
