/*
 * Allow to do matching of sentences.<br/>
 * Can be usefull for hard coded chatbots or to have triggers that point to actions to do.
 */
#ifndef ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP
#define ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP

#include <map>
#include <string>
#include <vector>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include "api.hpp"

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}
struct UniqueSemanticExpression;
struct SemanticMemory;
struct ExpressionWithLinks;
struct ReactionOptions;

namespace triggers {

/**
 * @brief Register a reaction to a trigger.
 * @param pTriggerSemExp Trigger semantic expression.
 * @param pReactionSemExp Reaction semantic expression.
 * @param pSemanticMemory Memory object where this mapping is stored.
 * @param pLingDb Linguistic database.
 */
ONSEMSEMANTICTOTEXT_API
void add(UniqueSemanticExpression pTriggerSemExp,
         UniqueSemanticExpression pReactionSemExp,
         SemanticMemory& pSemanticMemory,
         const linguistics::LinguisticDatabase& pLingDb);

/**
 * @brief Match a user utterance to a reaction stored in the memory object.
 * @param pReaction Result of the matching.
 * @param pSemanticMemory Memory object where this mappings are stored.
 * @param pUtteranceSemExp Semantic expression of the user utterance.
 * @param pLingDb Linguistic database.
 * @param pReactionOptions Some options to add filters in the matching.
 */
ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> match(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                                           SemanticMemory& pSemanticMemory,
                                           UniqueSemanticExpression pUtteranceSemExp,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           const ReactionOptions* pReactionOptions = nullptr);

}    // End of namespace triggers
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP
