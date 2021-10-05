#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_SEMEXPCONTROLLER_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_SEMEXPCONTROLLER_HPP

#include <list>
#include <memory>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/semantictotext/type/reactionoptions.hpp>
#include "../type/enum/semanticoperatorenum.hpp"
#include "../type/semanticdetailledanswer.hpp"
#include "steps/type/alsemexpcontrollertypes.hpp"

namespace onsem
{
struct SemanticExpression;
struct ListExpression;
struct ComparisonExpression;
struct SemanticStatementGrounding;
struct ExpressionHandleInMemory;
struct SemanticMemory;
struct SemanticMemoryBlockViewer;
struct SemanticDuration;


namespace controller
{

void applyOperatorResolveAgentAccordingToTheContext(UniqueSemanticExpression& pSemExp,
                                                    const SemanticMemory& pSemanticMemory,
                                                    const linguistics::LinguisticDatabase& pLingDb);

void applyOperatorOnExpHandleInMemory(std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
                                      ExpressionHandleInMemory& pExpressionHandleInMemory,
                                      SemanticOperatorEnum pReactionOperator,
                                      InformationType pInformationType,
                                      SemanticMemory& pSemanticMemory,
                                      std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
                                      const linguistics::LinguisticDatabase& pLingDb,
                                      const ReactionOptions* pReactionOptions);

void applyOperatorOnSemExp(SemControllerWorkingStruct& pWorkStruct,
                           SemanticMemoryBlockViewer& pMemViewer,
                           const SemanticExpression& pSemExp);

void applyOperatorOnSemExp(std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
                           const SemanticExpression& pSemExp,
                           SemanticOperatorEnum pReactionOperator,
                           InformationType pInformationType,
                           SemanticMemory& pSemanticMemory,
                           const linguistics::LinguisticDatabase& pLingDb);

void applyOperatorOnSemExpConstMem(std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
                                   const SemanticExpression& pSemExp,
                                   SemanticOperatorEnum pReactionOperator,
                                   InformationType pInformationType,
                                   const SemanticMemoryBlock& pConstMemBlock,
                                   const std::string& pCurrentUserId,
                                   const ProativeSpecifications* pProativeSpecificationsPtr,
                                   const ExternalFallback* pExternalFallbackPtr,
                                   const std::list<mystd::unique_propagate_const<MemBlockAndExternalCallback>>* pCallbackToSentencesCanBeAnsweredPtr,
                                   std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentState,
                                   const linguistics::LinguisticDatabase& pLingDb,
                                   SemanticTypeOfFeedback* pTypeOfFeedback = nullptr,
                                   bool* pAnswerIDontKnow = nullptr);

void applyOperatorOnGrdExp(SemControllerWorkingStruct& pWorkStruct,
                           SemanticMemoryBlockViewer& pMemViewer,
                           const GroundedExpression& pGrdExp,
                           const std::list<const GroundedExpression*>& pOtherGrdExps,
                           const GroundedExpression& pOriginalGrdExp);

void uninform(const SemanticContextAxiom& pContextAxiom,
              SemanticMemoryBlock& pMemBlock,
              const linguistics::LinguisticDatabase& pLingDb,
              std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr);

void sendActionProposalIfNecessary(CompositeSemAnswer& pCompSemAnswer,
                                   SemanticMemoryBlock& pMemBlock);

void compAnswerToSemExp(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                        CompositeSemAnswer& pCompositeSemAnswer);
ContextualAnnotation compAnswerToContextualAnnotation(CompositeSemAnswer& pCompositeSemAnswer);

void linkConditionalReactions(std::list<std::unique_ptr<SemAnswer> >& pSemAnswers,
                              ExpressionHandleInMemory& pMemKnowledge,
                              SemanticMemory& pSemanticMemory,
                              const linguistics::LinguisticDatabase& pLingDb,
                              InformationType pInformationType);

void convertToDetalledAnswer(std::list<std::unique_ptr<SemAnswer>>& pDetailledAnswers,
                             SemControllerWorkingStruct& pWorkStruct);

void manageQuestion(SemControllerWorkingStruct& pWorkStruct,
                    SemanticMemoryBlockViewer& pMemViewer,
                    const SemanticRequests& pRequests,
                    const GroundedExpression& pGrdExp,
                    const std::list<const GroundedExpression*>& pOtherGrdExps,
                    const GroundedExpression& pOriginalGrdExp);

TruenessValue operator_check_semExp(const SemanticExpression& pSemExp,
                                    const SemanticMemoryBlock& pConstMemBlock,
                                    const std::string& pCurrentUserId,
                                    const linguistics::LinguisticDatabase& pLingDb);

void notifyCurrentTime(std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
                       SemanticMemory& pSemanticMemory,
                       std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
                       const SemanticDuration& pNowTimeDuration,
                       const linguistics::LinguisticDatabase& pLingDb);

} // End of namespace controller
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_SEMEXPCONTROLLER_HPP
