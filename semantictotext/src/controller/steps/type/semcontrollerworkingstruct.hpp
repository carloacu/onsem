#ifndef ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_TYPE_SEMCONTROLLERWORKINGSTRUCT_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_TYPE_SEMCONTROLLERWORKINGSTRUCT_HPP

#include <list>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include "../../../type/enum/semanticoperatorenum.hpp"
#include "../../../type/semanticdetailledanswer.hpp"
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/infomationtype.hpp>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/semantictotext/enum/semantictypeoffeedback.hpp>
#include <onsem/semantictotext/type/reactionoptions.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct GroundedExpression;
struct AnswerExp;
struct ExpressionWithLinks;
struct SemanticAgentGrounding;
struct ProativeSpecifications;
struct ExternalFallback;
struct MemBlockAndExternalCallback;
class ReferencesFiller;


struct SemControllerWorkingStruct
{
  SemControllerWorkingStruct
  (InformationType pInformationType,
   const SemanticExpression* pAuthorSemExpPtr,
   SemanticLanguageEnum pFromLanguage,
   ExpressionWithLinks* pMemKnowledge,
   SemanticOperatorEnum pReactOperator,
   const ProativeSpecifications* pProativeSpecificationsPtr,
   const ExternalFallback* pExternalFallbackPtr,
   const std::list<mystd::unique_propagate_const<MemBlockAndExternalCallback>>* pCallbackToSentencesCanBeAnsweredPtr,
   std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
   const linguistics::LinguisticDatabase& pLingDb);

  SemControllerWorkingStruct
  (const SemControllerWorkingStruct& pOther);

  SemControllerWorkingStruct() = delete;
  SemControllerWorkingStruct& operator=(const SemControllerWorkingStruct&) = delete;

  bool askForNewRecursion();

  void getAnswersForRequest
  (std::list<const AnswerExp*>& pAnswers,
   const SemanticRequests& pRequests) const;

  void addAnswerWithoutReferences(ContextualAnnotation pType,
                                  UniqueSemanticExpression pReaction);
  void addQuestion(UniqueSemanticExpression pReaction);
  void addAnswer(ContextualAnnotation pType,
                 UniqueSemanticExpression pReaction,
                 const ReferencesFiller& pReferencesFiller);
  void addConditionForAUserAnswer(ContextualAnnotation pType,
                                  UniqueSemanticExpression pReaction,
                                  ConditionForAUser pConditionForAUser);
  void addConditionalAnswer(ContextualAnnotation pType,
                            UniqueSemanticExpression pReaction,
                            const mystd::optional<ConditionResult>& pCondition);

  void addAnswers(SemControllerWorkingStruct& pOther);
  void addAnswers(ListExpressionType pListExpType,
                  SemControllerWorkingStruct& pOther);
  bool haveAnAnswer() const;
  bool isFinished() const;
  bool canHaveAnotherTextualAnswer() const;
  bool canBeANewAnswer(const SemanticExpression& pSemExp) const;
  void getSourceContextAxiom(RelatedContextAxiom& pRes) const;
  TruenessValue agreementTypeOfTheAnswer();
  std::string getAuthorUserId() const;

  bool isAtRoot;
  InformationType informationType;
  const SemanticExpression* authorSemExp;
  const SemanticExpression* originalSemExpPtr;
  const SemanticAgentGrounding* author;
  SemanticLanguageEnum fromLanguage;
  ExpressionWithLinks* expHandleInMemory;
  std::map<GrammaticalType, const SemanticExpression*> annotatedExps;
  SemanticOperatorEnum reactOperator;
  mystd::optional<SemanticTypeOfFeedback> typeOfFeedback;
  const ProativeSpecifications* proativeSpecificationsPtr;
  const ExternalFallback* externalFallbackPtr;
  const std::list<mystd::unique_propagate_const<MemBlockAndExternalCallback>>* callbackToSentencesCanBeAnsweredPtr;
  std::map<const SentenceWithLinks*, TruenessValue>* axiomToConditionCurrentStatePtr;
  const linguistics::LinguisticDatabase& lingDb;
  SemExpComparator::ComparisonExceptions comparisonExceptions;
  int nbRecurssiveCallsRemaining; // to avoid infinite loops for sure
  mystd::optional<ContextualAnnotation> contAnnotationOfPreviousAnswers;
  ReactionOptions reactionOptions;
  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;

private:
  bool _canBeANewAnswer(const SemControllerWorkingStruct& pOther) const;
  bool _isAResource() const;
  bool _canBeANewAnswer(bool pIsInputAResource) const;
};


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_TYPE_SEMCONTROLLERWORKINGSTRUCT_HPP
