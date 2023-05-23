#ifndef ONSEM_SEMANTICTOTEXT_SEMEXPOPERATORS_HPP
#define ONSEM_SEMANTICTOTEXT_SEMEXPOPERATORS_HPP

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

namespace memoryOperation
{

enum class SemanticActionOperatorEnum
{
  BEHAVIOR,
  CONDITION,
  INFORMATION
};


/// Reply to a direct question according.
/// @todo check that it does not trigger a response, but that it only answers questions.
ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> answer(
    UniqueSemanticExpression pSemExp,
    bool pAnswerIDontKnow,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> answerIDontKnow(const SemanticExpression& pSemExp);

ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> answerICannotDo(const SemanticExpression& pSemExp);

ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> notKnowing(const SemanticExpression& pSemExp);

ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> sayFeedback(const SemanticExpression& pSemExp,
                                                      SemanticTypeOfFeedback pTypeOfFeedBack,
                                                      const SemanticMemory& pSemanticMemory,
                                                      const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
SemanticExpressionCategory categorize(const SemanticExpression& pSemExp);

ONSEMSEMANTICTOTEXT_API
TruenessValue check(const SemanticExpression& pSemExp,
                    const SemanticMemoryBlock& pMemBlock,
                    const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
SemanticEngagementValue extractEngagement(const SemanticExpression& pSemExp);

/**
  * @brief Exctract a part of a semantic expression.
  * @param[in,out] pInputSemExp Input knowledge in which we will remove the matched part.
  * @param[in] pSemExpToMatch Semantic expression to extract.
  * @param[in] pLingDb A linguistic database.
  * @return The extracted semantic expression.
  */
ONSEMSEMANTICTOTEXT_API
bool isASubpart(const SemanticExpression& pInputSemExp,
                const SemanticExpression& pSemExpToFind,
                const SemanticMemory& pSemanticMemory,
                const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void get(std::vector<std::unique_ptr<GroundedExpression> >& pAnswers,
         UniqueSemanticExpression pSemExp,
         const SemanticMemory& pSemanticMemory,
         const linguistics::LinguisticDatabase& pLingDb);

/// Reply with how a statement is known by the robot.
ONSEMSEMANTICTOTEXT_API
void howYouKnow(std::vector<std::unique_ptr<GroundedExpression> >& pAnswers,
                const SemanticExpression& pSemExp,
                const SemanticMemory& pSemanticMemory,
                const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void track(SemanticMemory& pSemanticMemory,
           UniqueSemanticExpression pSemExp,
           std::shared_ptr<SemanticTracker>& pSemTracker,
           const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void untrack(SemanticMemoryBlock& pMemBlock,
             std::shared_ptr<SemanticTracker>& pSemTracker,
             const linguistics::LinguisticDatabase& pLingDb,
             std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr);

ONSEMSEMANTICTOTEXT_API
void notifyPunctually(const SemanticExpression& pSemExp,
                      InformationType pInformationType,
                      SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> inform(
    UniqueSemanticExpression pSemExp,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const mystd::radix_map_str<std::string>* pLinkedInfosPtr = nullptr,
    std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);

ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> informAxiom(
    UniqueSemanticExpression pSemExp,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const mystd::radix_map_str<std::string>* pLinkedInfosPtr = nullptr,
    std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);

ONSEMSEMANTICTOTEXT_API
void learnSayCommand(SemanticMemory& pSemanticMemory,
                     const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void allowToInformTheUserHowToTeach(SemanticMemory& pSemanticMemory);

ONSEMSEMANTICTOTEXT_API
void defaultKnowledge(SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> addFallback(
    UniqueSemanticExpression pSemExp,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const mystd::radix_map_str<std::string>* pLinkedInfosPtr = nullptr);


ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> resolveCommandFromMemBlock(const SemanticExpression& pSemExp,
                                                                     const SemanticMemoryBlock& pMemblock,
                                                                     const std::string& pCurrentUserId,
                                                                     const linguistics::LinguisticDatabase& pLingDb);


/**
 * Find the knowledge representing actions requested in the imperative form.
 * @param pKnowledge The request knowledge.
 * @param pSemanticMemory The memory in which to find the actions.
 * @param pLingDb A linguistic database.
 * @return The knowledge result corresponding to executing the actions.
 */
ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> resolveCommand(const SemanticExpression& pSemExp,
                                                         const SemanticMemory& pSemanticMemory,
                                                         const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> executeFromCondition(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb);

// resolveCommand + resolveCommandFromCondition
ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> execute(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> externalRequester(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void mergeWithContext(UniqueSemanticExpression& pSemExp,
                      const SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void pingTime(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
              SemanticMemory& pSemanticMemory,
              const SemanticDuration& pNowTimeDuration,
              const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void resolveAgentAccordingToTheContext(UniqueSemanticExpression& pSemExp,
                                       const SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void addAgentInterpretations(UniqueSemanticExpression& pSemExp,
                             const SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> react(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                                           SemanticMemory& pSemanticMemory,
                                           UniqueSemanticExpression pSemExp,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           const ReactionOptions* pReactionOptions = nullptr);

ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> teach(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                                           SemanticMemory& pSemanticMemory,
                                           UniqueSemanticExpression pSemExp,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           SemanticActionOperatorEnum pActionOperator);

ONSEMSEMANTICTOTEXT_API
void show(std::vector<std::unique_ptr<GroundedExpression> >& pAnswers,
          const SemanticExpression& pSemExp,
          const SemanticMemory& pSemanticMemory,
          const linguistics::LinguisticDatabase& pLingDb);



} // End of namespace memoryOperation
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SEMEXPOPERATORS_HPP
