#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_GET_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_GET_HPP


#include <vector>
#include <string>


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;


std::vector<std::string> operator_get(const std::string& pText,
                                      const SemanticMemory& pSemanticMemory,
                                      const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_GET_HPP
