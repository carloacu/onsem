#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_SHOW_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_SHOW_HPP

#include <string>
#include <vector>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;


std::vector<std::string> operator_show(const std::string& pText,
                                       const SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_SHOW_HPP
