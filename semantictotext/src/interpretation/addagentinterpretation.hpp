#ifndef ONSEM_SEMANTICTOTEXT_SRC_INTERPRETATION_ADDAGENTINTERPRETATION_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_INTERPRETATION_ADDAGENTINTERPRETATION_HPP

#include <memory>

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}
struct UniqueSemanticExpression;
struct SemanticMemory;

namespace agentInterpretations {

void addAgentInterpretations(UniqueSemanticExpression& pSemExp,
                             const SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb);

}    // End of namespace agentInterpretations
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SRC_INTERPRETATION_ADDAGENTINTERPRETATION_HPP
