/*
 * Allow to do matching of sentences.<br/>
 * Can be usefull for hard coded chatbots or to have triggers that point to actions to do.
 */
#ifndef ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP
#define ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP

#include <list>
#include <vector>
#include <memory>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/infomationtype.hpp>
#include <onsem/common/utility/radix_map.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/semantictotext/enum/semanticexpressioncategory.hpp>
#include <onsem/semantictotext/enum/semanticengagementvalue.hpp>
#include <onsem/semantictotext/enum/semantictypeoffeedback.hpp>
#include <onsem/semantictotext/type/reactionoptions.hpp>
#include "api.hpp"


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticExpression;
struct UniqueSemanticExpression;
struct GroundedExpression;
class SemanticTracker;
struct SemanticMemory;
struct ExpressionWithLinks;
struct SentenceWithLinks;
struct SemanticMemoryBlock;
struct SemanticDuration;

namespace triggers
{

enum class SemanticActionOperatorEnum
{
  BEHAVIOR,
  CONDITION,
  INFORMATION
};


/// Register a reaction to an event.
/// The response semantic expression will be emitted through the
/// actionProposalSignal of the semantic memory, or returned by a react.
ONSEMSEMANTICTOTEXT_API
void add(UniqueSemanticExpression pTriggerSemExp,
         UniqueSemanticExpression pAnswerSemExp,
         SemanticMemory& pSemanticMemory,
         const linguistics::LinguisticDatabase& pLingDb);


ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> match(
    mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
    SemanticMemory& pSemanticMemory,
    UniqueSemanticExpression pSemExp,
    const linguistics::LinguisticDatabase& pLingDb,
    const ReactionOptions* pReactionOptions = nullptr);


} // End of namespace triggers
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP
