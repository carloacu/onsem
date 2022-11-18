#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_ANSWER_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_ANSWER_HPP

#include <string>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;
struct DetailedReactionAnswer;


DetailedReactionAnswer operator_answer(const std::string& pText,
                                       const SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& pLingDb,
                                       bool pCanAnswerIDontKnow = true);

} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_ANSWER_HPP
